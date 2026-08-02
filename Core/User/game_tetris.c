/*
 * game_tetris.c
 */

#include "game_tetris.h"

#include "screens.h"
#include "stm32h7xx_hal.h"

#include "lvgl.h"

#include <stdio.h>
#include <string.h>

/* --- board ---------------------------------------------------------------- */

#define BOARD_W 10
#define BOARD_H 20

/*
 * Canvas geometry, from the EEZ layout.
 *
 * The playfield canvas is 186x251, which is not a 10x20 shape. Rather than
 * stretch the cells, the largest square cell that fits both axes is used and
 * the grid is centred - min(186/10, 251/20) is 12. The leftover margin is the
 * price of square blocks, which matters more here than filling the box.
 */
#define PLAY_W 186
#define PLAY_H 251
#define CELL   12
#define PLAY_X ((PLAY_W - (BOARD_W * CELL)) / 2)
#define PLAY_Y ((PLAY_H - (BOARD_H * CELL)) / 2)

#define NEXT_W 116
#define NEXT_H 77
#define NEXT_CELL 14
#define NEXT_X ((NEXT_W - (4 * NEXT_CELL)) / 2)
#define NEXT_Y ((NEXT_H - (4 * NEXT_CELL)) / 2)

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
#define REPEAT_DELAY_MS  260u   /* before a held button starts repeating */
#define REPEAT_RATE_MS    90u
#define GAMEOVER_HOLD_MS 2500u

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

static bool     s_b1_prev, s_b2_prev;
static uint32_t s_b1_since, s_b2_since;
static uint32_t s_b1_next_repeat, s_b2_next_repeat;

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

/** Leftmost column the piece can sit at, used when wrapping around the edge. */
static int leftmost_x(uint8_t piece, uint8_t rot)
{
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            if (cell_filled(piece, rot, r, c)) {
                return -c;   /* shift so this column lands on 0 */
            }
        }
    }

    return 0;
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

static void clear_lines(void)
{
    int cleared = 0;

    for (int y = BOARD_H - 1; y >= 0; y--) {
        bool full = true;
        for (int x = 0; x < BOARD_W; x++) {
            if (s_board[y][x] == 0u) {
                full = false;
                break;
            }
        }

        if (!full) {
            continue;
        }

        for (int yy = y; yy > 0; yy--) {
            memcpy(s_board[yy], s_board[yy - 1], BOARD_W);
        }
        memset(s_board[0], 0, BOARD_W);

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

    clear_lines();
    spawn();
}

/* --- input ---------------------------------------------------------------- */

static void step_right(void)
{
    if (s_over) {
        return;
    }

    if (fits(s_piece, s_rot, s_px + 1, s_py)) {
        s_px++;
        s_dirty = true;
        return;
    }

    /*
     * Blocked. If it was the wall rather than a stack, come back on the far
     * side - that wrap is the whole reason one button is enough to reach every
     * column. If a stack is in the way at the far side too, nothing moves.
     */
    const int wrapped = leftmost_x(s_piece, s_rot);
    if (wrapped != s_px && fits(s_piece, s_rot, wrapped, s_py)) {
        s_px = (int8_t)wrapped;
        s_dirty = true;
    }
}

static void rotate_cw(void)
{
    if (s_over) {
        return;
    }

    const uint8_t next_rot = (uint8_t)((s_rot + 1u) & 3u);

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

/** Edge detect plus auto-repeat for one button. */
static bool pressed_edge(bool now_pressed, bool *prev, uint32_t *since,
                         uint32_t *next_repeat, uint32_t now)
{
    if (!now_pressed) {
        *prev = false;
        return false;
    }

    if (!*prev) {
        *prev = true;
        *since = now;
        *next_repeat = now + REPEAT_DELAY_MS;
        return true;        /* the press itself */
    }

    if (now >= *next_repeat) {
        *next_repeat = now + REPEAT_RATE_MS;
        return true;        /* held long enough to repeat */
    }

    return false;
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

    if (pressed_edge(button2_pressed, &s_b2_prev, &s_b2_since,
                     &s_b2_next_repeat, now)) {
        step_right();
    }

    if (pressed_edge(button1_pressed, &s_b1_prev, &s_b1_since,
                     &s_b1_next_repeat, now)) {
        rotate_cw();
    }
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
                draw_cell(s_play_buf, PLAY_W, px, py, CELL,
                          COLOURS[s_board[y][x]]);
            }
            else {
                fill_rect(s_play_buf, PLAY_W, px, py, CELL, 1, GRID_COLOUR);
                fill_rect(s_play_buf, PLAY_W, px, py, 1, CELL, GRID_COLOUR);
            }
        }
    }

    if (!s_over) {
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
    else {
        /* Grey everything out rather than printing text - there is no font
         * bound to this canvas, and a drained board is unambiguous. */
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

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (!cell_filled(s_next_piece, 0, r, c)) {
                continue;
            }
            draw_cell(s_next_buf, NEXT_W, NEXT_X + (c * NEXT_CELL),
                      NEXT_Y + (r * NEXT_CELL), NEXT_CELL,
                      COLOURS[s_next_piece + 1u]);
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

    if (!s_over && (now_ms - s_last_drop) >= drop_interval()) {
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
