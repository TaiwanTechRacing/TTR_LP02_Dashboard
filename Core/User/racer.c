/*
 * racer.c
 */

#include "racer.h"

#include "bsp_sdram.h"
#include "screens.h"
#include "vehicle_data.h"

#include "lvgl.h"

#include <string.h>

/* --- screen --------------------------------------------------------------- */

#define W 480
#define H 272

static uint16_t *s_buf;

/* --- world ---------------------------------------------------------------- */

/*
 * The projection, in the usual form for this kind of racer:
 *
 *   scale   = CAMERA_DEPTH / (segment z - camera z)
 *   screenY = H/2 - scale * (segment height - camera height) * H/2
 *   screenX = W/2 + scale * (segment centre - camera x)     * W/2
 *   width   = scale * ROAD_WIDTH * W/2
 *
 * CAMERA_DEPTH is 1/tan(fov/2); 0.84 is a field of view of about 100 degrees,
 * which is wide enough that a corner does not appear out of nowhere.
 */
#define SEGMENT_LENGTH  200.0f
#define ROAD_WIDTH     2000.0f
#define CAMERA_HEIGHT  1200.0f
#define CAMERA_DEPTH      0.84f
#define DRAW_SEGMENTS   140          /* how far down the road is drawn */

#define SEGMENT_COUNT   600          /* the track loops after this many */

typedef struct {
    float curve;      /* how hard this segment turns */
    float height;     /* world height at its far edge */
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

/* --- state ---------------------------------------------------------------- */

static bool     s_active;
static uint32_t s_last;

static float s_position;      /* how far along the track, in world units */
static float s_player_x;      /* -1 to 1 across the road */
static float s_speed;         /* world units per second */

static bool s_btn_left, s_btn_right;

#define MAX_SPEED   (SEGMENT_LENGTH * 60.0f)
#define ACCEL       (MAX_SPEED / 2.5f)
#define BRAKING     (MAX_SPEED / 1.2f)
#define DECEL       (MAX_SPEED / 6.0f)
#define OFF_ROAD_DECEL (MAX_SPEED / 2.0f)
#define CENTRIFUGAL 0.35f

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

static void render(void)
{
    const int base = (int)(s_position / SEGMENT_LENGTH);
    const float offset = s_position - ((float)base * SEGMENT_LENGTH);

    /* Sky and the ground behind everything, so hills never show a gap. */
    fill_rows(0, H / 2, SKY);
    fill_rows(H / 2, H, GRASS_A);

    /*
     * Walk away from the camera, nearest first, remembering the highest row
     * drawn so far. A segment that projects above that is behind a hill, and
     * skipping it is what hides the road that a crest cuts off.
     */
    int maxy = H;

    float x = 0.0f;      /* accumulated sideways shift from the curves */
    float dx = 0.0f;

    float cam_h = CAMERA_HEIGHT + s_road[base % SEGMENT_COUNT].height;

    int   py = H;
    float px = (float)W * 0.5f;
    float pw = 0.0f;
    bool  have_prev = false;

    for (int n = 0; n < DRAW_SEGMENTS; n++) {
        const int index = (base + n) % SEGMENT_COUNT;
        const segment_t *seg = &s_road[index];

        const float z = ((float)(n + 1) * SEGMENT_LENGTH) - offset;
        if (z < 1.0f) {
            continue;
        }

        const float scale = CAMERA_DEPTH / z;

        const int   sy = (int)(((float)H * 0.5f) -
                               (scale * (seg->height - cam_h) * (float)H * 0.5f));
        const float sx = ((float)W * 0.5f) +
                         (scale * (x - (s_player_x * ROAD_WIDTH * 0.5f)) * (float)W * 0.5f);
        const float sw = scale * ROAD_WIDTH * 0.5f * (float)W * 0.5f;

        x += dx;
        dx += seg->curve;

        if (have_prev && sy < maxy && sy < py) {
            /* Which shade this segment gets. Two segments per stripe is what
             * makes the speed readable at a glance. */
            const bool light = ((index / 2) & 1) != 0;

            /* Grass first, full width, then the road on top of it. */
            trapezoid(sy, (float)W * 0.5f, (float)W, py, (float)W * 0.5f, (float)W,
                      light ? GRASS_B : GRASS_A);

            trapezoid(sy, sx, sw * 1.18f, py, px, pw * 1.18f,
                      light ? RUMBLE_B : RUMBLE_A);

            trapezoid(sy, sx, sw, py, px, pw, light ? ROAD_B : ROAD_A);

            if (light) {
                /* Centre line, only on the light stripes so it dashes. */
                trapezoid(sy, sx, sw * 0.03f, py, px, pw * 0.03f, LANE);
            }

            maxy = sy;
        }

        py = sy;
        px = sx;
        pw = sw;
        have_prev = true;
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

    const float speed_pu = s_speed / MAX_SPEED;

    if (brake > 0.05f) {
        s_speed -= BRAKING * brake * dt;
    }
    else if (throttle > 0.05f) {
        s_speed += ACCEL * throttle * dt;
    }
    else {
        s_speed -= DECEL * dt;
    }

    /* Off the road, the grass slows the car down. */
    if ((s_player_x < -1.0f || s_player_x > 1.0f) && s_speed > (MAX_SPEED * 0.35f)) {
        s_speed -= OFF_ROAD_DECEL * dt;
    }

    if (s_speed < 0.0f) {
        s_speed = 0.0f;
    }
    if (s_speed > MAX_SPEED) {
        s_speed = MAX_SPEED;
    }

    /* Steering authority falls away as the car slows, as it would. */
    s_player_x += steer * 2.2f * speed_pu * dt;

    /* Thrown to the outside of a corner, harder the faster you take it. */
    const int index = ((int)(s_position / SEGMENT_LENGTH)) % SEGMENT_COUNT;
    s_player_x -= s_road[index].curve * 0.0015f * speed_pu * speed_pu * CENTRIFUGAL;

    if (s_player_x < -2.0f) {
        s_player_x = -2.0f;
    }
    if (s_player_x > 2.0f) {
        s_player_x = 2.0f;
    }

    s_position += s_speed * dt;

    const float track_length = (float)SEGMENT_COUNT * SEGMENT_LENGTH;
    while (s_position >= track_length) {
        s_position -= track_length;
    }
}

/* --- lifecycle ------------------------------------------------------------ */

void Racer_Init(void)
{
    s_buf = (uint16_t *)BSP_SDRAM_Alloc((uint32_t)W * H * sizeof(uint16_t));
    if (s_buf == NULL) {
        return;     /* nothing to draw into; the page stays blank */
    }

    build_track();

    lv_canvas_set_buffer(objects.racer_canvas, s_buf, W, H, LV_COLOR_FORMAT_RGB565);

    render();
    lv_obj_invalidate(objects.racer_canvas);
}

void Racer_SetActive(bool active)
{
    if (active == s_active) {
        return;
    }

    s_active = active;

    if (active) {
        s_position = 0.0f;
        s_player_x = 0.0f;
        s_speed = 0.0f;
        s_last = 0;
        s_btn_left = false;
        s_btn_right = false;
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
