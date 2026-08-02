/*
 * racer.c
 */

#include "racer.h"

#include "bsp_sdram.h"
#include "screens.h"
#include "vehicle_data.h"

#include "lvgl.h"

#include <math.h>
#include <string.h>

/* --- screen --------------------------------------------------------------- */

#define W 480
#define H 272

static uint16_t *s_buf;

/* --- world ---------------------------------------------------------------- */

/*
 * The projection, in the usual form for this kind of racer:
 *
 *   scale   = s_tune.camera_depth / (segment z - camera z)
 *   screenY = H/2 - scale * (segment height - camera height) * H/2
 *   screenX = W/2 + scale * (segment centre - camera x)     * W/2
 *   width   = scale * s_tune.road_width * W/2
 *
 * s_tune.camera_depth is 1/tan(fov/2); 0.84 is a field of view of about 100 degrees,
 * which is wide enough that a corner does not appear out of nowhere.
 */
#define SEGMENT_COUNT   600          /* the track loops after this many */
#define DRAW_SEGMENTS_MAX SEGMENT_COUNT

/* The compiled-in tuning. Racer_Defaults() puts these back. */
#define DEF_SEGMENT_LENGTH  200.0f
#define DEF_ROAD_WIDTH     2000.0f
#define DEF_CAMERA_HEIGHT  1200.0f
#define DEF_CAMERA_DEPTH      0.84f
#define DEF_DRAW_SEGMENTS   140

static racer_tuning_t s_tune;

typedef struct {
    float curve;      /* how hard this segment turns */
    float height;     /* world height at its far edge */
    bool  cone;       /* an orange cone sits on the course here */
    bool  cone_hit;   /* already knocked over this lap */
    float cone_x;     /* where, in road half-widths from the centre */
} segment_t;

static segment_t s_road[SEGMENT_COUNT];

/* --- colours -------------------------------------------------------------- */

#define SKY        0x3D7Fu   /* pale blue */
#define GRASS_A    0x0400u   /* the two greens that make the motion stripes */
#define GRASS_B    0x0560u
#define ROAD_A     0x39E7u   /* two greys, likewise */
#define ROAD_B     0x4208u
#define RUMBLE_A   0xF800u   /* red and white kerbs */
#define RUMBLE_B   0xFFFFu
#define LANE       0xFFFFu

/* --- cones ---------------------------------------------------------------- */

/*
 * One cone, three colours.
 *
 * Laid out as text so it can be edited by looking at it. '.' is transparent,
 * 'x' takes whichever colour the caller wants - which is what lets the same
 * shape be a blue boundary cone, a yellow one, or an orange marker on the
 * course. 'w' is the band, 'k' the base.
 *
 * Drawn rather than imported: RacerJS ships a spritesheet, but its art is
 * CC-BY-SA and importing it means an asset pipeline and a flash budget for
 * shapes a dozen pixels tall. A cone is a triangle.
 */
#define CONE_W    16
#define CONE_ROWS 16

/*
 * Squarer than a plain triangle, because a traffic cone is not one: it is
 * closer to a column that tapers, standing on a flat base plate. A straight
 * triangle read as a pyramid.
 */
static const char CONE_ART[] =
    "......xxxx......"
    "......xxxx......"
    "......xxxx......"
    ".....xxxxxx....."
    ".....wwwwww....."
    ".....wwwwww....."
    ".....xxxxxx....."
    ".....xxxxxx....."
    "....xxxxxxxx...."
    "....wwwwwwww...."
    "....xxxxxxxx...."
    "...xxxxxxxxxx..."
    "...xxxxxxxxxx..."
    "..xxxxxxxxxxxx.."
    ".kkkkkkkkkkkkkk."
    "kkkkkkkkkkkkkkkk";

/* Formula Student marks the course this way: blue on the left of the
 * direction of travel, yellow on the right, orange for anything you are
 * meant to notice. */
#define CONE_BLUE   0x03BFu
#define CONE_YELLOW 0xFEE0u
#define CONE_ORANGE 0xFC00u
#define CONE_BAND   0xFFFFu
#define CONE_BASE   0x2124u

/*
 * How far out the boundary cones sit, in road half-widths, and how often.
 *
 * Just inside the asphalt, not out on the grass. A Formula Student course is
 * cones laid on a pad - the cones are the edge, the asphalt carries on past
 * them. Standing them off in the grass made the asphalt look like a road with
 * a verge, which is a different thing entirely.
 */
#define BOUNDARY_X       0.90f
#define BOUNDARY_EVERY   3

/*
 * Cone height in world units, the same units as road_width.
 *
 * Scaled for legibility rather than realism. To scale against a 3.5 m course
 * a cone would be about 170 here, and at 480x272 that leaves it a few pixels
 * tall until it is almost under the wheels - too late to steer around. Twice
 * life size reads as a cone from far enough away to react to.
 *
 * Sizing them from the road's projected pixel width instead, as a first
 * attempt did, made them taller than the car at the near edge: that number is
 * a width in pixels, not a height in the world.
 */
#define CONE_WORLD_H     330.0f
#define CONE_WORLD_H_BIG 460.0f

/* Clipping a cone costs speed. Hitting one in autocross costs two seconds, so
 * losing a chunk of the straight afterwards is about the right feeling. */
#define CONE_PENALTY     0.45f
#define CONE_HIT_WIDTH   0.30f

static uint32_t s_cones_hit;

/* --- state ---------------------------------------------------------------- */

static bool     s_active;
static uint32_t s_last;

static float s_position;      /* how far along the track, in world units */
static float s_player_x;      /* -1 to 1 across the road */
static float s_speed;         /* world units per second */

static bool s_btn_left, s_btn_right;

#define DEF_MAX_SPEED   (DEF_SEGMENT_LENGTH * 60.0f)
#define DEF_ACCEL       (DEF_MAX_SPEED / 2.5f)
#define DEF_BRAKING     (DEF_MAX_SPEED / 1.2f)
#define DEF_DECEL       (DEF_MAX_SPEED / 6.0f)
#define DEF_OFF_ROAD    (DEF_MAX_SPEED / 2.0f)
#define DEF_STEER_RATE  2.2f
#define DEF_CENTRIFUGAL 0.35f

racer_tuning_t *Racer_Tuning(void)
{
    return &s_tune;
}

void Racer_Defaults(void)
{
    s_tune.segment_length = DEF_SEGMENT_LENGTH;
    s_tune.road_width     = DEF_ROAD_WIDTH;
    s_tune.camera_height  = DEF_CAMERA_HEIGHT;
    s_tune.camera_depth   = DEF_CAMERA_DEPTH;
    s_tune.draw_segments  = DEF_DRAW_SEGMENTS;

    s_tune.max_speed      = DEF_MAX_SPEED;
    s_tune.accel          = DEF_ACCEL;
    s_tune.braking        = DEF_BRAKING;
    s_tune.decel          = DEF_DECEL;
    s_tune.off_road_decel = DEF_OFF_ROAD;
    s_tune.steer_rate     = DEF_STEER_RATE;
    s_tune.centrifugal    = DEF_CENTRIFUGAL;
}

/* --- track ---------------------------------------------------------------- */

/** Fill a stretch with a constant curve and a hill, easing in and out. */
static void add_stretch(int *at, int count, float curve, float hill)
{
    const int ease = count / 4;

    for (int i = 0; i < count && *at < SEGMENT_COUNT; i++, (*at)++) {
        float k = 1.0f;
        if (i < ease) {
            k = (float)i / (float)ease;
        }
        else if (i > (count - ease)) {
            k = (float)(count - i) / (float)ease;
        }

        s_road[*at].curve = curve * k;
        s_road[*at].height = hill * k;
    }
}

/*
 * A repeatable shuffle. The course has to be the same every run so a corner can
 * be learned, which rules out seeding from the clock.
 */
static uint32_t track_rng(uint32_t *state)
{
    *state ^= *state << 13;
    *state ^= *state >> 17;
    *state ^= *state << 5;
    return *state;
}

static void place_cones(void)
{
    uint32_t rng = 0xC0FFEEu;

    for (int i = 0; i < SEGMENT_COUNT; i++) {
        const uint32_t r = track_rng(&rng);

        s_road[i].cone = false;
        s_road[i].cone_hit = false;

        /* Roughly one stretch in fourteen has something to miss. Denser and it
         * stops being a course and becomes a slalom. */
        if ((r % 14u) != 0u) {
            continue;
        }

        s_road[i].cone = true;
        /* Never dead centre and never on the boundary line - always a choice
         * of which side to pass. */
        s_road[i].cone_x = -0.62f + ((float)((r >> 8) % 125u) / 100.0f);
    }
}

static void build_track(void)
{
    int at = 0;

    memset(s_road, 0, sizeof(s_road));

    add_stretch(&at,  60,  0.0f,    0.0f);
    add_stretch(&at,  50,  2.5f,  600.0f);
    add_stretch(&at,  40,  0.0f,    0.0f);
    add_stretch(&at,  60, -3.5f, -400.0f);
    add_stretch(&at,  40,  0.0f,  900.0f);
    add_stretch(&at,  70,  4.5f,    0.0f);
    add_stretch(&at,  50,  0.0f, -700.0f);
    add_stretch(&at,  60, -2.0f,    0.0f);
    add_stretch(&at,  50,  1.5f,  500.0f);
    add_stretch(&at, 120,  0.0f,    0.0f);

    /* Whatever is left stays a flat straight, so the loop joins cleanly. */

    place_cones();
}

/* --- drawing -------------------------------------------------------------- */

static void fill_rows(int y0, int y1, uint16_t colour)
{
    if (y0 < 0) {
        y0 = 0;
    }
    if (y1 > H) {
        y1 = H;
    }

    for (int y = y0; y < y1; y++) {
        uint16_t *row = s_buf + ((size_t)y * W);
        for (int x = 0; x < W; x++) {
            row[x] = colour;
        }
    }
}

/**
 * One trapezoid, as a stack of horizontal spans.
 *
 * @param ytop,xtop,wtop  the far edge - higher on screen, narrower
 * @param ybot,xbot,wbot  the near edge
 *
 * Everything this renderer draws is this shape: road, kerbs, lane markings and
 * the grass either side.
 */
static void trapezoid(int ytop, float xtop, float wtop,
                      int ybot, float xbot, float wbot, uint16_t colour)
{
    if (ybot <= ytop) {
        return;
    }

    const float span = (float)(ybot - ytop);
    int y0 = ytop;
    int y1 = ybot;

    if (y0 < 0) {
        y0 = 0;
    }
    if (y1 > H) {
        y1 = H;
    }

    for (int y = y0; y < y1; y++) {
        const float t = (float)(y - ytop) / span;
        const float cx = xtop + ((xbot - xtop) * t);
        const float cw = wtop + ((wbot - wtop) * t);

        int x0 = (int)(cx - cw);
        int x1 = (int)(cx + cw);

        if (x0 < 0) {
            x0 = 0;
        }
        if (x1 > W) {
            x1 = W;
        }
        if (x1 <= x0) {
            continue;
        }

        uint16_t *row = s_buf + ((size_t)y * W) + x0;
        for (int x = x0; x < x1; x++) {
            *row++ = colour;
        }
    }
}

/**
 * One cone, scaled and standing on the road.
 *
 * @param base_y  the row the road surface is at, where it stands
 * @param cx      screen column of its centre
 * @param height  how tall to draw it
 * @param clip_y  nothing above this row: the crest of a hill hides what is
 *                behind it, and without this cones float over the skyline
 *
 * Nearest-neighbour scaling, and transparency by looking for '.' rather than a
 * colour key - so nothing in the palette is reserved and the artwork can use
 * any colour it likes.
 */
static void draw_cone(int base_y, int cx, int height, int clip_y, uint16_t body)
{
    if (height < 1) {
        return;
    }

    const int width = (height * CONE_W) / CONE_ROWS;
    const int x0 = cx - (width / 2);
    const int top = base_y - height;

    /*
     * Too small to sample the artwork: at three or four rows, nearest
     * neighbour picks a different band each time the height changes by a
     * pixel, and a line of distant cones strobes. A solid blob of the body
     * colour is what they look like at that size anyway, and it holds still.
     */
    if (height < 7) {
        for (int row = 0; row < height; row++) {
            const int y = top + row;
            if (y < 0 || y >= H || y > clip_y) {
                continue;
            }

            /* Narrower at the top, like the shape it stands in for. */
            const int w = 1 + ((width * (row + 2)) / (height + 2));
            int a = cx - (w / 2);
            int b = a + w;
            if (a < 0) a = 0;
            if (b > W) b = W;

            uint16_t *dst = s_buf + ((size_t)y * W);
            for (int x = a; x < b; x++) {
                dst[x] = body;
            }
        }
        return;
    }

    for (int row = 0; row < height; row++) {
        const int y = top + row;
        /* clip_y is the top edge of the nearer road, so anything below it is
         * behind that road and must not be drawn. */
        if (y < 0 || y >= H || y > clip_y) {
            continue;
        }

        const int sr = (row * CONE_ROWS) / height;
        const char *art_row = &CONE_ART[sr * CONE_W];
        uint16_t *dst = s_buf + ((size_t)y * W);

        for (int col = 0; col < width; col++) {
            const int x = x0 + col;
            if (x < 0 || x >= W) {
                continue;
            }

            const char c = art_row[(col * CONE_W) / width];
            if (c == '.') {
                continue;
            }

            dst[x] = (c == 'x') ? body
                   : (c == 'w') ? CONE_BAND
                                : CONE_BASE;
        }
    }
}

/* What the road looked like at each drawn segment, so the cones can be put on
 * top of it afterwards, farthest first. */
typedef struct {
    int   y;        /* screen row of the road surface */
    float x;        /* screen column of the road centre */
    float w;        /* half width of the road, in pixels */
    float scale;    /* the projection factor, for sizing anything standing here */
    int   clip;     /* nothing below this row is visible here */
    int   index;    /* which segment this was */
    bool  drawn;
} projected_t;

#define PROJECTED_MAX 220
static projected_t s_proj[PROJECTED_MAX];
static int         s_proj_count;

static void render(void)
{
    const int base = (int)(s_position / s_tune.segment_length);
    const float offset = s_position - ((float)base * s_tune.segment_length);

    /* Sky and the ground behind everything, so hills never show a gap. */
    fill_rows(0, H / 2, SKY);
    fill_rows(H / 2, H, GRASS_A);

    /*
     * Walk away from the camera, nearest first, remembering the highest row
     * drawn so far. A segment that projects above that is behind a hill, and
     * skipping it is what hides the road that a crest cuts off.
     */
    int maxy = H;
    s_proj_count = 0;

    float x = 0.0f;      /* accumulated sideways shift from the curves */
    float dx = 0.0f;

    float cam_h = s_tune.camera_height + s_road[base % SEGMENT_COUNT].height;

    int   py = H;
    float px = (float)W * 0.5f;
    float pw = 0.0f;
    bool  have_prev = false;

    int draw = s_tune.draw_segments;
    if (draw > DRAW_SEGMENTS_MAX) {
        draw = DRAW_SEGMENTS_MAX;
    }

    for (int n = 0; n < draw; n++) {
        const int index = (base + n) % SEGMENT_COUNT;
        const segment_t *seg = &s_road[index];

        const float z = ((float)(n + 1) * s_tune.segment_length) - offset;
        if (z < 1.0f) {
            continue;
        }

        const float scale = s_tune.camera_depth / z;

        const int   sy = (int)(((float)H * 0.5f) -
                               (scale * (seg->height - cam_h) * (float)H * 0.5f));
        const float sx = ((float)W * 0.5f) +
                         (scale * (x - (s_player_x * s_tune.road_width * 0.5f)) * (float)W * 0.5f);
        const float sw = scale * s_tune.road_width * 0.5f * (float)W * 0.5f;

        x += dx;
        dx += seg->curve;

        if (s_proj_count < PROJECTED_MAX) {
            s_proj[s_proj_count].y = sy;
            s_proj[s_proj_count].x = sx;
            s_proj[s_proj_count].w = sw;
            s_proj[s_proj_count].scale = scale;
            s_proj[s_proj_count].clip = maxy;
            s_proj[s_proj_count].index = index;
            s_proj[s_proj_count].drawn = (have_prev && sy < maxy && sy < py);
            s_proj_count++;
        }

        if (have_prev && sy < maxy && sy < py) {
            /* Which shade this segment gets. Two segments per stripe is what
             * makes the speed readable at a glance. */
            const bool light = ((index / 2) & 1) != 0;

            /* Grass first, full width, then the road on top of it. */
            trapezoid(sy, (float)W * 0.5f, (float)W, py, (float)W * 0.5f, (float)W,
                      light ? GRASS_B : GRASS_A);

            /*
             * No kerbs and no centre line. Red and white rumble strips sat
             * badly next to red and white cones, and an autocross pad has
             * neither - it is asphalt with cones on it, and the cones are the
             * edge marking.
             */
            trapezoid(sy, sx, sw, py, px, pw, light ? ROAD_B : ROAD_A);

            maxy = sy;
        }

        py = sy;
        px = sx;
        pw = sw;
        have_prev = true;
    }

    /*
     * Cones last, walking back towards the camera so a near one covers a far
     * one. Doing it in the same pass as the road would put the road of the
     * next segment over the cone of this one.
     */
    for (int i = s_proj_count - 1; i >= 0; i--) {
        const projected_t *p = &s_proj[i];

        /*
         * Deliberately not gated on whether this segment's road was drawn.
         *
         * Far away, consecutive segments project onto the same screen row, so
         * the road skips them - and hanging the cones off that made them blink
         * on and off as rounding flipped the comparison a pixel either way. A
         * cone's visibility has nothing to do with whether the strip of
         * asphalt under it happened to be a pixel tall.
         *
         * What hides a cone behind a crest is the clip line, which is recorded
         * for every segment whether its road was drawn or not.
         */
        const segment_t *seg = &s_road[p->index];

        /* Boundary cones: blue on the left, yellow on the right, the way a
         * Formula Student course is marked. */
        if ((p->index % BOUNDARY_EVERY) == 0) {
            const int h = (int)(p->scale * CONE_WORLD_H * (float)H * 0.5f);
            draw_cone(p->y, (int)(p->x - (p->w * BOUNDARY_X)), h, p->clip, CONE_BLUE);
            draw_cone(p->y, (int)(p->x + (p->w * BOUNDARY_X)), h, p->clip, CONE_YELLOW);
        }

        if (seg->cone && !seg->cone_hit) {
            const int h = (int)(p->scale * CONE_WORLD_H_BIG * (float)H * 0.5f);
            draw_cone(p->y, (int)(p->x + (p->w * seg->cone_x)), h, p->clip, CONE_ORANGE);
        }
    }
}

/* --- driving -------------------------------------------------------------- */

/** Steering and pedals, from the car when it is talking and the buttons if not. */
static void controls(float dt, float *steer, float *throttle, float *brake)
{
    *steer = 0.0f;
    *throttle = 0.0f;
    *brake = 0.0f;

    (void)dt;

    if (!VehicleData_IsStale(VD_GROUP_VCU_SENSOR2, VD_DEFAULT_TIMEOUT_MS)) {
        /* Positive steering angle is anticlockwise, so left. Same inversion as
         * the tetris page, and for the same reason. */
        *steer = -g_vehicle.steering_deg / 90.0f;
        if (*steer > 1.0f) {
            *steer = 1.0f;
        }
        if (*steer < -1.0f) {
            *steer = -1.0f;
        }

        *throttle = g_vehicle.apps1_pu / 100.0f;

        if (!VehicleData_IsStale(VD_GROUP_VCU_SENSOR1, VD_DEFAULT_TIMEOUT_MS)) {
            const float front = g_vehicle.bse_front_pu;
            const float rear = g_vehicle.bse_rear_pu;
            *brake = ((front > rear) ? front : rear) / 100.0f;
        }
    }

    /*
     * The buttons steer whatever the bus is doing, and drive on their own when
     * it is quiet - otherwise the page would be a static picture on a bench.
     */
    if (s_btn_left) {
        *steer -= 1.0f;
    }
    if (s_btn_right) {
        *steer += 1.0f;
    }

    if (VehicleData_IsStale(VD_GROUP_VCU_SENSOR2, VD_DEFAULT_TIMEOUT_MS)) {
        *throttle = 1.0f;
    }
}

static void advance(float dt)
{
    float steer, throttle, brake;
    controls(dt, &steer, &throttle, &brake);

    const float speed_pu = s_speed / s_tune.max_speed;

    if (brake > 0.05f) {
        s_speed -= s_tune.braking * brake * dt;
    }
    else if (throttle > 0.05f) {
        s_speed += s_tune.accel * throttle * dt;
    }
    else {
        s_speed -= s_tune.decel * dt;
    }

    /* Off the road, the grass slows the car down. */
    if ((s_player_x < -1.0f || s_player_x > 1.0f) && s_speed > (s_tune.max_speed * 0.35f)) {
        s_speed -= s_tune.off_road_decel * dt;
    }

    if (s_speed < 0.0f) {
        s_speed = 0.0f;
    }
    if (s_speed > s_tune.max_speed) {
        s_speed = s_tune.max_speed;
    }

    /* Steering authority falls away as the car slows, as it would. */
    s_player_x += steer * s_tune.steer_rate * speed_pu * dt;

    /* Thrown to the outside of a corner, harder the faster you take it. */
    const int index = ((int)(s_position / s_tune.segment_length)) % SEGMENT_COUNT;
    s_player_x -= s_road[index].curve * 0.0015f * speed_pu * speed_pu * s_tune.centrifugal;

    if (s_player_x < -2.0f) {
        s_player_x = -2.0f;
    }
    if (s_player_x > 2.0f) {
        s_player_x = 2.0f;
    }

    /*
     * Clipping a cone. Checked over the ground actually covered this step, not
     * just where the car ended up - at speed a segment goes by in a couple of
     * frames and a point test would drive straight through them.
     */
    const float travelled = s_speed * dt;
    if (travelled > 0.0f) {
        const int from = (int)(s_position / s_tune.segment_length);
        const int to = (int)((s_position + travelled) / s_tune.segment_length);

        for (int i = from; i <= to; i++) {
            segment_t *seg = &s_road[i % SEGMENT_COUNT];
            if (!seg->cone || seg->cone_hit) {
                continue;
            }

            if (fabsf(s_player_x - seg->cone_x) < CONE_HIT_WIDTH) {
                seg->cone_hit = true;
                s_cones_hit++;
                s_speed *= (1.0f - CONE_PENALTY);
            }
        }
    }

    s_position += s_speed * dt;

    const float track_length = (float)SEGMENT_COUNT * s_tune.segment_length;
    while (s_position >= track_length) {
        s_position -= track_length;

        /* Stand the cones back up for the next lap. */
        for (int i = 0; i < SEGMENT_COUNT; i++) {
            s_road[i].cone_hit = false;
        }
    }
}

/* --- lifecycle ------------------------------------------------------------ */

void Racer_Init(void)
{
    Racer_Defaults();

    s_buf = (uint16_t *)BSP_SDRAM_Alloc((uint32_t)W * H * sizeof(uint16_t));
    if (s_buf == NULL) {
        return;     /* nothing to draw into; the page stays blank */
    }

    build_track();

    lv_canvas_set_buffer(objects.racer_canvas, s_buf, W, H, LV_COLOR_FORMAT_RGB565);

    render();
    lv_obj_invalidate(objects.racer_canvas);
}

uint32_t Racer_ConesHit(void)
{
    return s_cones_hit;
}

void Racer_Restart(void)
{
    s_cones_hit = 0;

    for (int i = 0; i < SEGMENT_COUNT; i++) {
        s_road[i].cone_hit = false;
    }

    s_position = 0.0f;
    s_player_x = 0.0f;
    s_speed = 0.0f;
    s_last = 0;
    s_btn_left = false;
    s_btn_right = false;
}

void Racer_SetActive(bool active)
{
    if (active == s_active) {
        return;
    }

    s_active = active;

    if (active) {
        Racer_Restart();
    }
}

bool Racer_IsActive(void)
{
    return s_active;
}

void Racer_Buttons(bool button1_pressed, bool button2_pressed)
{
    s_btn_left = button1_pressed;
    s_btn_right = button2_pressed;
}

void Racer_Service(uint32_t now_ms)
{
    if (!s_active || s_buf == NULL) {
        return;
    }

    if (s_last == 0u) {
        s_last = now_ms;
        return;
    }

    uint32_t elapsed = now_ms - s_last;
    if (elapsed == 0u) {
        return;
    }

    /*
     * Physics on the real elapsed time rather than a fixed step, so the car
     * covers the same ground whatever the frame rate - and capped, so a long
     * stall does not teleport it through a corner.
     */
    if (elapsed > 100u) {
        elapsed = 100u;
    }

    s_last = now_ms;
    advance((float)elapsed / 1000.0f);

    render();
    lv_obj_invalidate(objects.racer_canvas);
}
