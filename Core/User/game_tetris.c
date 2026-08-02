/*
 * game_tetris.c
 */

#include "game_tetris.h"

#include "screens.h"
#include "vehicle_data.h"
#include "stm32h7xx_hal.h"

#include "lvgl.h"

#include <stdio.h>
#include <string.h>

/* --- board ---------------------------------------------------------------- */

#define BOARD_W 12
#define BOARD_H 20

/*
 * Canvas geometry.
 *
 * These derive the canvas size from the board rather than fitting the board
 * into whatever size the canvas happens to be, so there is no leftover margin
 * and no rounding. The EEZ canvases must be set to match:
 *
 *     tetris_game_canva   150 x 240
 *     next_block           80 x  40
 *
 * 12 px is the largest square cell this panel allows - 20 rows at 13 px is 260,
 * which leaves no room above or below on a 272 px screen. The board is 12
 * columns rather than the usual 10 so that 150 px is nearly filled: 12 x 12 is
 * 144, leaving 3 px each side instead of 15.
 *
 * A wider board is an easier game - there is more room to place a piece and a
 * line takes two more blocks to complete.
 *
 * Every piece is 4 cells wide and 2 tall in its spawn rotation, which is why
 * the preview is 4x2 rather than 4x4.
 */
#define CELL      12
#define NEXT_CELL 20

#define PLAY_W 150
#define PLAY_H (BOARD_H * CELL)
#define PLAY_X ((PLAY_W - (BOARD_W * CELL)) / 2)
#define PLAY_Y 0

#define NEXT_W (4 * NEXT_CELL)
#define NEXT_H (2 * NEXT_CELL)
#define NEXT_X 0
#define NEXT_Y 0

/*
 * Canvas buffers. RGB565 to match the panel, so LVGL blits them without
 * converting. 111 KB together, which is why they are static rather than on the
 * LVGL heap: the allocation is fixed, and a failure at runtime would be a
 * blank game with no obvious cause.
 */
static uint16_t s_play_buf[PLAY_W * PLAY_H] __attribute__((aligned(32)));
static uint16_t s_next_buf[NEXT_W * NEXT_H] __attribute__((aligned(32)));

/* --- pieces --------------------------------------------------------------- */

/*
 * Each rotation is a 4x4 bitmap in one uint16_t, bit 15 being the top left
 * cell and running left to right, top to bottom.
 */
static const uint16_t PIECES[7][4] = {
    { 0x0F00, 0x2222, 0x00F0, 0x4444 },   /* I */
    { 0x8E00, 0x6440, 0x0E20, 0x44C0 },   /* J */
    { 0x2E00, 0x4460, 0x0E80, 0xC440 },   /* L */
    { 0x6600, 0x6600, 0x6600, 0x6600 },   /* O */
    { 0x6C00, 0x4620, 0x06C0, 0x8C40 },   /* S */
    { 0x4E00, 0x4640, 0x0E40, 0x4C40 },   /* T */
    { 0xC600, 0x2640, 0x0C60, 0x4C80 },   /* Z */
};

/* Indexed by piece + 1, so 0 stays "empty". */
static const uint16_t COLOURS[8] = {
    0x0000,   /* empty */
    0x07FF,   /* I cyan */
    0x001F,   /* J blue */
    0xFD20,   /* L orange */
    0xFFE0,   /* O yellow */
    0x07E0,   /* S green */
    0xF81F,   /* T magenta */
    0xF800,   /* Z red */
};

#define GRID_COLOUR  0x2104   /* faint grey, so the empty field reads as a grid */
#define BG_COLOUR    0x0000

/* --- state ---------------------------------------------------------------- */

#define DROP_START_MS 600u
#define DROP_MIN_MS   120u
/*
 * Short press moves, long press rotates.
 *
 * A move therefore lands on release, not on press. That is the cost of telling
 * the two apart with one button each, and a tap is short enough that it does
 * not read as lag. Rotation fires the moment the hold crosses the threshold,
 * so it feels immediate, and repeats while held so a piece can be spun round
 * without letting go.
 */
#define LONG_PRESS_MS    350u
#define ROTATE_REPEAT_MS 400u
#define GAMEOVER_HOLD_MS 2500u

/*
 * A completed line blinks before it disappears.
 *
 * Without it the row simply vanishes between two frames, which at speed is easy
 * to miss entirely - the stack drops and it is not obvious why. Six steps of
 * 70 ms is three blinks and a little over a third of a second: long enough to
 * register, short enough that it never feels like the game has stalled.
 */
#define FLASH_STEP_MS 70u
#define FLASH_STEPS    6u

/*
 * Playing with the wheel and pedals.
 *
 * The entry gesture is holding the throttle past half as the page opens. It has
 * to be something the driver would never do by accident while reaching the
 * page, and it cannot be the wheel, because the wheel is rarely at dead centre.
 *
 * 15 degrees of steering is well outside the slop around centre but nowhere
 * near lock, so a column at a time is a small deliberate movement. The wheel
 * repeats while held over, because returning to centre between every column
 * would make crossing the board tedious.
 */
#define STEER_ENTRY_THROTTLE  50    /* % held as the page opens */
#define STEER_DEADZONE_DEG    15
#define STEER_BRAKE_PCT       20    /* brake counts as held past this */
#define STEER_DROP_THROTTLE   30    /* throttle past this drops fast */
#define STEER_REPEAT_MS      180u
#define STEER_DROP_MS         70u

static uint8_t s_board[BOARD_H][BOARD_W];

static uint8_t s_piece, s_next_piece, s_rot;
static int8_t  s_px, s_py;

static uint32_t s_score;
static uint32_t s_lines;
static bool     s_active;
static bool     s_over;
static bool     s_dirty;
static uint32_t s_last_drop;
static uint32_t s_over_since;

/* Rows waiting to be cleared, one bit per row, while they blink. */
static uint32_t s_flash_rows;
static uint8_t  s_flash_step;
static uint32_t s_flash_next;

typedef struct {
    bool     prev;
    uint32_t since;         /* tick the press began */
    bool     rotated;       /* the hold has already turned into a rotate */
    uint32_t next_rotate;
} button_t;

static button_t s_btn[2];   /* 0 = left, 1 = right */

static bool     s_steer_mode;
static uint32_t s_steer_next;      /* next move or rotate the wheel may make */
static int8_t   s_steer_last_dir;  /* 0 while the wheel is near centre */

static uint32_t s_rng = 0x12345678u;
static char     s_score_text[12];

/* --- helpers -------------------------------------------------------------- */

static uint32_t rng_next(void)
{
    /* xorshift32: no library dependency, and the sequence only has to be
     * unpredictable to a person, not to anything else. */
    s_rng ^= s_rng << 13;
    s_rng ^= s_rng >> 17;
    s_rng ^= s_rng << 5;
    return s_rng;
}

static bool cell_filled(uint8_t piece, uint8_t rot, int r, int c)
{
    return (PIECES[piece][rot] & (0x8000u >> ((r * 4) + c))) != 0u;
}

/** Whether the piece fits at this position without leaving the board or
 *  overlapping something already placed. */
static bool fits(uint8_t piece, uint8_t rot, int px, int py)
{
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (!cell_filled(piece, rot, r, c)) {
                continue;
            }

            const int x = px + c;
            const int y = py + r;

            if (x < 0 || x >= BOARD_W || y >= BOARD_H) {
                return false;
            }
            if (y >= 0 && s_board[y][x] != 0u) {
                return false;
            }
        }
    }

    return true;
}

static void spawn(void)
{
    s_piece = s_next_piece;
    s_next_piece = (uint8_t)(rng_next() % 7u);
    s_rot = 0;
    s_px = (BOARD_W / 2) - 2;
    s_py = -1;

    if (!fits(s_piece, s_rot, s_px, s_py)) {
        s_over = true;
        s_over_since = HAL_GetTick();
    }

    s_dirty = true;
}

static void reset_game(void)
{
    memset(s_board, 0, sizeof(s_board));
    s_flash_rows = 0;
    s_flash_step = 0;
    s_score = 0;
    s_lines = 0;
    s_over = false;
    s_last_drop = HAL_GetTick();

    /* Seeded from the tick so the sequence differs between runs. The user
     * reaching this page at all is what makes the seed unpredictable. */
    s_rng ^= HAL_GetTick() * 2654435761u;
    if (s_rng == 0u) {
        s_rng = 0x12345678u;
    }

    s_next_piece = (uint8_t)(rng_next() % 7u);
    spawn();
}

static uint32_t drop_interval(void)
{
    const uint32_t step = s_lines / 10u;
    const uint32_t faster = step * 60u;

    if (faster >= (DROP_START_MS - DROP_MIN_MS)) {
        return DROP_MIN_MS;
    }

    return DROP_START_MS - faster;
}

/** Bitmask of the rows that are currently complete. */
static uint32_t full_rows(void)
{
    uint32_t mask = 0;

    for (int y = 0; y < BOARD_H; y++) {
        bool full = true;
        for (int x = 0; x < BOARD_W; x++) {
            if (s_board[y][x] == 0u) {
                full = false;
                break;
            }
        }
        if (full) {
            mask |= (1u << y);
        }
    }

    return mask;
}

/** Drop everything above the marked rows down over them, and score. */
static void remove_rows(uint32_t mask)
{
    int cleared = 0;

    for (int y = BOARD_H - 1; y >= 0; y--) {
        if ((mask & (1u << y)) == 0u) {
            continue;
        }

        for (int yy = y; yy > 0; yy--) {
            memcpy(s_board[yy], s_board[yy - 1], BOARD_W);
        }
        memset(s_board[0], 0, BOARD_W);

        /* Everything above shifted down, so the rows still marked above this
         * one moved with it. */
        mask = (mask & ((1u << y) - 1u)) << 1u;

        cleared++;
        y++;    /* the row that dropped into y has not been checked yet */
    }

    if (cleared > 0) {
        static const uint32_t LINE_SCORE[5] = { 0, 100, 300, 500, 800 };
        s_score += LINE_SCORE[cleared];
        s_lines += (uint32_t)cleared;
    }
}

static void lock_piece(void)
{
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (!cell_filled(s_piece, s_rot, r, c)) {
                continue;
            }

            const int x = s_px + c;
            const int y = s_py + r;
            if (y >= 0 && y < BOARD_H && x >= 0 && x < BOARD_W) {
                s_board[y][x] = (uint8_t)(s_piece + 1u);
            }
        }
    }

    s_flash_rows = full_rows();

    if (s_flash_rows != 0u) {
        /* Hold the piece where it landed and blink the completed rows. The
         * next piece waits until the blinking is over, so the board the player
         * is looking at is the board that scored. */
        s_flash_step = 0;
        s_flash_next = HAL_GetTick() + FLASH_STEP_MS;
        s_dirty = true;
    }
    else {
        spawn();
    }
}

/* --- input ---------------------------------------------------------------- */

/**
 * Move one column, or stay put if a wall or a stack is in the way.
 *
 * @param dir  -1 for left, +1 for right
 *
 * This used to wrap around the edges, back when one button had to reach every
 * column. With both directions available the wrap only made it easy to shoot
 * a piece off the side you were aiming at, so the walls block now.
 */
static void step(int dir)
{
    if (s_over || s_flash_rows != 0u) {
        return;
    }

    if (fits(s_piece, s_rot, s_px + dir, s_py)) {
        s_px = (int8_t)(s_px + dir);
        s_dirty = true;
    }
}

static void rotate(int dir)
{
    if (s_over || s_flash_rows != 0u) {
        return;
    }

    const uint8_t next_rot = (uint8_t)((s_rot + (dir > 0 ? 1u : 3u)) & 3u);

    /*
     * Kick sideways if rotating in place does not fit. Without this a piece
     * against either wall simply refuses to turn, which reads as the button
     * being broken.
     */
    static const int8_t KICKS[5] = { 0, -1, 1, -2, 2 };

    for (int i = 0; i < 5; i++) {
        if (fits(s_piece, next_rot, s_px + KICKS[i], s_py)) {
            s_px = (int8_t)(s_px + KICKS[i]);
            s_rot = next_rot;
            s_dirty = true;
            return;
        }
    }
}

/**
 * One button: a tap moves on release, a hold rotates and keeps rotating.
 *
 * @param dir  -1 for the left button, +1 for the right one
 */
static void handle_button(button_t *b, bool now_pressed, int dir, uint32_t now)
{
    if (now_pressed) {
        if (!b->prev) {
            b->prev = true;
            b->since = now;
            b->rotated = false;
            b->next_rotate = now + LONG_PRESS_MS;
            return;                     /* wait and see which it becomes */
        }

        if (now >= b->next_rotate) {
            b->rotated = true;          /* no move when this is released */
            b->next_rotate = now + ROTATE_REPEAT_MS;
            rotate(dir);
        }

        return;
    }

    if (b->prev) {
        b->prev = false;
        if (!b->rotated) {
            step(dir);                  /* it was a tap after all */
        }
    }
}

void GameTetris_Buttons(bool button1_pressed, bool button2_pressed)
{
    if (!s_active) {
        return;
    }

    const uint32_t now = HAL_GetTick();

    if (s_over) {
        /* Ignore input until the game-over board has been up long enough to
         * read, then any press starts a new game. */
        if ((now - s_over_since) >= GAMEOVER_HOLD_MS &&
            (button1_pressed || button2_pressed)) {
            reset_game();
            s_dirty = true;
        }
        return;
    }

    handle_button(&s_btn[0], button1_pressed, -1, now);
    handle_button(&s_btn[1], button2_pressed, +1, now);
}

/* --- wheel and pedals ----------------------------------------------------- */

/** Live sensor readings, or false when the VCU has stopped talking. */
static bool read_controls(int *steer_deg, int *throttle, int *brake)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_SENSOR2, VD_DEFAULT_TIMEOUT_MS)) {
        return false;
    }

    *steer_deg = (int)g_vehicle.steering_deg;
    *throttle = (int)g_vehicle.apps1_pu;

    /* Either circuit counts. A driver pressing the pedal is not thinking about
     * which end of the car the sensor is on, and the higher of the two is the
     * one that says "the brake is on". */
    *brake = 0;
    if (!VehicleData_IsStale(VD_GROUP_VCU_SENSOR1, VD_DEFAULT_TIMEOUT_MS)) {
        const int front = (int)g_vehicle.bse_front_pu;
        const int rear = (int)g_vehicle.bse_rear_pu;
        *brake = (front > rear) ? front : rear;
    }

    return true;
}

/** Steering, turned into a direction once it is clearly off centre. */
static int steer_direction(int steer_deg)
{
    if (steer_deg > STEER_DEADZONE_DEG) {
        return +1;
    }
    if (steer_deg < -STEER_DEADZONE_DEG) {
        return -1;
    }
    return 0;
}

static void service_steering(uint32_t now)
{
    int steer_deg = 0, throttle = 0, brake = 0;

    if (!read_controls(&steer_deg, &throttle, &brake)) {
        s_steer_last_dir = 0;
        return;
    }

    const int dir = steer_direction(steer_deg);

    if (dir == 0) {
        /* Back near centre: the next turn starts a fresh action rather than
         * continuing the last one's repeat. */
        s_steer_last_dir = 0;
        return;
    }

    if (dir != s_steer_last_dir) {
        s_steer_last_dir = (int8_t)dir;
        s_steer_next = now;      /* act immediately on a new turn */
    }

    if (now < s_steer_next) {
        return;
    }

    if (brake >= STEER_BRAKE_PCT) {
        s_steer_next = now + ROTATE_REPEAT_MS;
        rotate(dir);
    }
    else {
        s_steer_next = now + STEER_REPEAT_MS;
        step(dir);
    }
}

/** Drop interval, shortened while the throttle is down in steering mode. */
static uint32_t steer_drop_interval(void)
{
    int steer_deg = 0, throttle = 0, brake = 0;

    if (!read_controls(&steer_deg, &throttle, &brake)) {
        return drop_interval();
    }

    return (throttle >= STEER_DROP_THROTTLE) ? STEER_DROP_MS : drop_interval();
}

bool GameTetris_SteerMode(void)
{
    return s_steer_mode;
}

/* --- drawing -------------------------------------------------------------- */

static void fill_rect(uint16_t *buf, int stride, int x, int y, int w, int h,
                      uint16_t colour)
{
    for (int r = 0; r < h; r++) {
        uint16_t *row = buf + ((y + r) * stride) + x;
        for (int c = 0; c < w; c++) {
            row[c] = colour;
        }
    }
}

/** One block: filled body with a darker edge so a wall of blocks still reads
 *  as individual pieces. */
static void draw_cell(uint16_t *buf, int stride, int x, int y, int size,
                      uint16_t colour)
{
    fill_rect(buf, stride, x, y, size, size, colour);

    /* Halve each channel for the border. */
    const uint16_t edge = (uint16_t)(((colour >> 1) & 0x7BEFu));
    fill_rect(buf, stride, x, y, size, 1, edge);
    fill_rect(buf, stride, x, y + size - 1, size, 1, edge);
    fill_rect(buf, stride, x, y, 1, size, edge);
    fill_rect(buf, stride, x + size - 1, y, 1, size, edge);
}

static void draw_play(void)
{
    for (int i = 0; i < (PLAY_W * PLAY_H); i++) {
        s_play_buf[i] = BG_COLOUR;
    }

    /* Empty grid first, so the playfield has visible bounds even when empty. */
    for (int y = 0; y < BOARD_H; y++) {
        for (int x = 0; x < BOARD_W; x++) {
            const int px = PLAY_X + (x * CELL);
            const int py = PLAY_Y + (y * CELL);

            if (s_board[y][x] != 0u) {
                /* A row being cleared blinks white on the odd steps. */
                const bool lit = ((s_flash_rows & (1u << y)) != 0u) &&
                                 ((s_flash_step & 1u) == 0u);

                draw_cell(s_play_buf, PLAY_W, px, py, CELL,
                          lit ? 0xFFFFu : COLOURS[s_board[y][x]]);
            }
            else {
                fill_rect(s_play_buf, PLAY_W, px, py, CELL, 1, GRID_COLOUR);
                fill_rect(s_play_buf, PLAY_W, px, py, 1, CELL, GRID_COLOUR);
            }
        }
    }

    if (!s_over && s_flash_rows == 0u) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (!cell_filled(s_piece, s_rot, r, c)) {
                    continue;
                }

                const int x = s_px + c;
                const int y = s_py + r;
                if (y < 0 || y >= BOARD_H || x < 0 || x >= BOARD_W) {
                    continue;   /* still partly above the board on spawn */
                }

                draw_cell(s_play_buf, PLAY_W, PLAY_X + (x * CELL),
                          PLAY_Y + (y * CELL), CELL, COLOURS[s_piece + 1u]);
            }
        }
    }
    else if (s_over) {
        /* Grey everything out rather than printing text - there is no font
         * bound to this canvas, and a drained board is unambiguous.
         *
         * Only when the game is actually over: this used to be the plain else
         * of the branch above, which meant the board went grey during a line
         * clear too and painted over the blink. */
        for (int y = 0; y < BOARD_H; y++) {
            for (int x = 0; x < BOARD_W; x++) {
                if (s_board[y][x] == 0u) {
                    continue;
                }
                draw_cell(s_play_buf, PLAY_W, PLAY_X + (x * CELL),
                          PLAY_Y + (y * CELL), CELL, 0x4208);
            }
        }
    }
}

static void draw_next(void)
{
    for (int i = 0; i < (NEXT_W * NEXT_H); i++) {
        s_next_buf[i] = BG_COLOUR;
    }

    /*
     * Centre on the piece's own bounding box rather than on the 4x4 mask. The
     * masks are not centred within themselves - an S sits to the left, an I
     * fills the width - so drawing them at face value leaves each piece sitting
     * somewhere different in the box.
     */
    int min_c = 4, max_c = -1, min_r = 2, max_r = -1;
    for (int r = 0; r < 2; r++) {
        for (int c = 0; c < 4; c++) {
            if (!cell_filled(s_next_piece, 0, r, c)) {
                continue;
            }
            if (c < min_c) min_c = c;
            if (c > max_c) max_c = c;
            if (r < min_r) min_r = r;
            if (r > max_r) max_r = r;
        }
    }

    if (max_c < 0) {
        return;     /* cannot happen, but do not divide by a bogus width */
    }

    const int off_x = (NEXT_W - ((max_c - min_c + 1) * NEXT_CELL)) / 2;
    const int off_y = (NEXT_H - ((max_r - min_r + 1) * NEXT_CELL)) / 2;

    for (int r = min_r; r <= max_r; r++) {
        for (int c = min_c; c <= max_c; c++) {
            if (!cell_filled(s_next_piece, 0, r, c)) {
                continue;
            }
            draw_cell(s_next_buf, NEXT_W,
                      off_x + ((c - min_c) * NEXT_CELL),
                      off_y + ((r - min_r) * NEXT_CELL),
                      NEXT_CELL, COLOURS[s_next_piece + 1u]);
        }
    }
}

static void redraw(void)
{
    draw_play();
    draw_next();

    lv_obj_invalidate(objects.tetris_game_canva);
    lv_obj_invalidate(objects.next_block);
}

/* --- lifecycle ------------------------------------------------------------ */

void GameTetris_Init(void)
{
    lv_canvas_set_buffer(objects.tetris_game_canva, s_play_buf,
                         PLAY_W, PLAY_H, LV_COLOR_FORMAT_RGB565);
    lv_canvas_set_buffer(objects.next_block, s_next_buf,
                         NEXT_W, NEXT_H, LV_COLOR_FORMAT_RGB565);

    reset_game();
    redraw();
}

void GameTetris_SetActive(bool active)
{
    if (active == s_active) {
        return;
    }

    s_active = active;

    if (active) {
        /*
         * Sampled once here rather than watched continuously: the throttle is
         * also the fast-drop control, so a live test would turn the mode off
         * and on again every time a piece was dropped.
         */
        int steer_deg = 0, throttle = 0, brake = 0;
        s_steer_mode = read_controls(&steer_deg, &throttle, &brake) &&
                       (throttle >= STEER_ENTRY_THROTTLE);

        s_steer_last_dir = 0;

        reset_game();
        redraw();
    }
}

bool GameTetris_IsActive(void)
{
    return s_active;
}

void GameTetris_Service(uint32_t now_ms)
{
    if (!s_active) {
        return;
    }

    /* While rows are blinking nothing else moves - not gravity, not the next
     * piece. The board holds still until the blinking finishes. */
    if (s_flash_rows != 0u) {
        if (now_ms >= s_flash_next) {
            s_flash_next = now_ms + FLASH_STEP_MS;
            s_flash_step++;
            s_dirty = true;

            if (s_flash_step >= FLASH_STEPS) {
                remove_rows(s_flash_rows);
                s_flash_rows = 0;
                s_flash_step = 0;
                s_last_drop = now_ms;
                spawn();
            }
        }

        if (s_dirty) {
            s_dirty = false;
            redraw();
        }
        return;
    }

    if (s_steer_mode && !s_over) {
        service_steering(now_ms);
    }

    const uint32_t interval = s_steer_mode ? steer_drop_interval()
                                           : drop_interval();

    if (!s_over && (now_ms - s_last_drop) >= interval) {
        s_last_drop = now_ms;

        if (fits(s_piece, s_rot, s_px, s_py + 1)) {
            s_py++;
        }
        else {
            lock_piece();
        }

        s_dirty = true;
    }

    if (s_dirty) {
        s_dirty = false;
        redraw();
    }
}

const char *GameTetris_ScoreText(void)
{
    snprintf(s_score_text, sizeof(s_score_text), "%lu", (unsigned long)s_score);
    return s_score_text;
}
