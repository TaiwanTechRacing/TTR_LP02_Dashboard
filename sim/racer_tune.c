/*
 * racer_tune.c
 *
 *  A window that is nothing but the racer, with the wheel and pedals on the
 *  keyboard and every parameter adjustable while the road is moving.
 *
 *  It exists because tuning a driving feel through the full dashboard means
 *  booting, waiting for CAN, performing a hidden gesture, and then having no
 *  way to change a number without a rebuild. Here the road starts immediately
 *  and a parameter can be nudged between one corner and the next, which is the
 *  only way to judge any of them.
 *
 *  It links the same Core/User/racer.c the firmware does - the point is to tune
 *  the real thing, not a copy of it. When the numbers are right, press P and
 *  paste what it writes into Racer_Defaults().
 *
 *      arrow keys      steer, throttle, brake - as the wheel and pedals
 *      tab / shift-tab select a parameter
 *      + / -           adjust it by 5%, hold shift for 1%
 *      r               restart the run
 *      d               back to the compiled-in defaults
 *      p               write the current set to racer_tuning.txt
 */

#include "lvgl.h"
#include "ui.h"
#include "screens.h"
#include "racer.h"
#include "vehicle_data.h"

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIM_WIDTH  480
#define SIM_HEIGHT 272
#define SIM_ZOOM   200

/* --- clock ---------------------------------------------------------------- */

static LARGE_INTEGER s_freq;
static LONGLONG      s_start;

uint32_t HAL_GetTick(void)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint32_t)(((now.QuadPart - s_start) * 1000LL) / s_freq.QuadPart);
}

/* --- the parameters, as a list so they can be walked --------------------- */

typedef struct {
    const char *name;
    float      *value;
    const char *format;
} field_t;

static field_t   s_fields[16];
static int       s_field_count;
static int       s_field;
static int       s_draw_segments_as_float_backing;

static void build_fields(void)
{
    racer_tuning_t *t = Racer_Tuning();
    int n = 0;

    s_fields[n++] = (field_t){ "road width",     &t->road_width,     "%.0f" };
    s_fields[n++] = (field_t){ "camera height",  &t->camera_height,  "%.0f" };
    s_fields[n++] = (field_t){ "camera depth",   &t->camera_depth,   "%.3f" };
    s_fields[n++] = (field_t){ "segment length", &t->segment_length, "%.0f" };
    s_fields[n++] = (field_t){ "max speed",      &t->max_speed,      "%.0f" };
    s_fields[n++] = (field_t){ "accel",          &t->accel,          "%.0f" };
    s_fields[n++] = (field_t){ "braking",        &t->braking,        "%.0f" };
    s_fields[n++] = (field_t){ "coast decel",    &t->decel,          "%.0f" };
    s_fields[n++] = (field_t){ "off-road decel", &t->off_road_decel, "%.0f" };
    s_fields[n++] = (field_t){ "steer rate",     &t->steer_rate,     "%.2f" };
    s_fields[n++] = (field_t){ "centrifugal",    &t->centrifugal,    "%.3f" };

    s_field_count = n;
    (void)s_draw_segments_as_float_backing;
}

static void dump_tuning(void)
{
    racer_tuning_t *t = Racer_Tuning();
    FILE *f = fopen("racer_tuning.txt", "w");
    if (f == NULL) {
        return;
    }

    fprintf(f, "/* paste into Racer_Defaults() */\n");
    fprintf(f, "#define DEF_SEGMENT_LENGTH %8.1ff\n", t->segment_length);
    fprintf(f, "#define DEF_ROAD_WIDTH     %8.1ff\n", t->road_width);
    fprintf(f, "#define DEF_CAMERA_HEIGHT  %8.1ff\n", t->camera_height);
    fprintf(f, "#define DEF_CAMERA_DEPTH   %8.3ff\n", t->camera_depth);
    fprintf(f, "#define DEF_DRAW_SEGMENTS  %8d\n",    t->draw_segments);
    fprintf(f, "#define DEF_MAX_SPEED      %8.1ff\n", t->max_speed);
    fprintf(f, "#define DEF_ACCEL          %8.1ff\n", t->accel);
    fprintf(f, "#define DEF_BRAKING        %8.1ff\n", t->braking);
    fprintf(f, "#define DEF_DECEL          %8.1ff\n", t->decel);
    fprintf(f, "#define DEF_OFF_ROAD       %8.1ff\n", t->off_road_decel);
    fprintf(f, "#define DEF_STEER_RATE     %8.2ff\n", t->steer_rate);
    fprintf(f, "#define DEF_CENTRIFUGAL    %8.3ff\n", t->centrifugal);
    fclose(f);
}

/* --- input ---------------------------------------------------------------- */

static bool held(int vk)
{
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

/** True on the transition to pressed, so a key repeats only when held. */
static bool tapped(int vk)
{
    static bool prev[256];
    const bool now = held(vk);
    const bool edge = now && !prev[vk & 0xFF];
    prev[vk & 0xFF] = now;
    return edge;
}

/*
 * The wheel and pedals, as the dashboard would see them on CAN.
 *
 * Written straight into vehicle_data and marked fresh, so racer.c reads them
 * through exactly the path it uses in the car - no separate input route that
 * could behave differently from the real one.
 */
static void feed_controls(float dt)
{
    static float steer;      /* degrees */
    static float throttle;   /* percent */
    static float brake;

    const float STEER_RATE = 220.0f;    /* deg per second toward full lock */
    const float RETURN_RATE = 320.0f;   /* self-centring when released */
    const float PEDAL_RATE = 400.0f;

    if (held(VK_LEFT)) {
        steer += STEER_RATE * dt;       /* positive is anticlockwise = left */
    }
    else if (held(VK_RIGHT)) {
        steer -= STEER_RATE * dt;
    }
    else if (steer > 0.0f) {
        steer -= RETURN_RATE * dt;
        if (steer < 0.0f) steer = 0.0f;
    }
    else if (steer < 0.0f) {
        steer += RETURN_RATE * dt;
        if (steer > 0.0f) steer = 0.0f;
    }

    if (steer > 90.0f)  steer = 90.0f;
    if (steer < -90.0f) steer = -90.0f;

    throttle += (held(VK_UP) ? PEDAL_RATE : -PEDAL_RATE) * dt;
    if (throttle < 0.0f)   throttle = 0.0f;
    if (throttle > 100.0f) throttle = 100.0f;

    brake += (held(VK_DOWN) ? PEDAL_RATE : -PEDAL_RATE) * dt;
    if (brake < 0.0f)   brake = 0.0f;
    if (brake > 100.0f) brake = 100.0f;

    g_vehicle.steering_deg = steer;
    g_vehicle.apps1_pu = throttle;
    g_vehicle.apps2_pu = throttle;
    g_vehicle.bse_front_pu = brake;
    g_vehicle.bse_rear_pu = brake;

    VehicleData_MarkFresh(VD_GROUP_VCU_SENSOR1);
    VehicleData_MarkFresh(VD_GROUP_VCU_SENSOR2);
}

static void handle_tuning_keys(void)
{
    if (tapped(VK_TAB)) {
        const bool back = held(VK_SHIFT);
        s_field = (s_field + (back ? -1 : 1) + s_field_count) % s_field_count;
    }

    const bool fine = held(VK_SHIFT);
    const float step = fine ? 0.01f : 0.05f;

    if (tapped(VK_OEM_PLUS) || tapped(VK_ADD)) {
        *s_fields[s_field].value *= (1.0f + step);
    }
    if (tapped(VK_OEM_MINUS) || tapped(VK_SUBTRACT)) {
        *s_fields[s_field].value *= (1.0f - step);
    }

    if (tapped('R')) {
        Racer_Restart();
    }
    if (tapped('D')) {
        Racer_Defaults();
        build_fields();
    }
    if (tapped('P')) {
        dump_tuning();
    }
}

/* --- readout -------------------------------------------------------------- */

static lv_obj_t *s_readout;

static void make_readout(void)
{
    /*
     * Parented to the GAME2 screen itself rather than lv_screen_active():
     * loadScreen() fades, so the active screen is still the old one for the
     * couple of hundred milliseconds after it is called, and the readout would
     * be created on a screen nobody is about to look at.
     */
    s_readout = lv_label_create(objects.game2);
    lv_obj_set_style_bg_color(s_readout, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_readout, 180, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_readout, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_pad_all(s_readout, 3, LV_PART_MAIN);
    lv_obj_align(s_readout, LV_ALIGN_TOP_LEFT, 2, 2);
    lv_label_set_text(s_readout, "");
}

static void update_readout(uint32_t fps)
{
    char buf[256];
    char value[32];

    snprintf(value, sizeof(value), s_fields[s_field].format,
             (double)*s_fields[s_field].value);

    snprintf(buf, sizeof(buf),
             "%lu fps   cones %lu\n%s\n%s %s\nsteer %.0f  thr %.0f  brk %.0f",
             (unsigned long)fps, (unsigned long)Racer_ConesHit(),
             "tab pick  +/- adjust  p save",
             s_fields[s_field].name, value,
             (double)g_vehicle.steering_deg,
             (double)g_vehicle.apps1_pu,
             (double)g_vehicle.bse_front_pu);

    lv_label_set_text(s_readout, buf);
}

/* --- main ----------------------------------------------------------------- */

int main(void)
{
    timeBeginPeriod(1);

    QueryPerformanceFrequency(&s_freq);
    LARGE_INTEGER start;
    QueryPerformanceCounter(&start);
    s_start = start.QuadPart;

    lv_init();
    lv_tick_set_cb(HAL_GetTick);

    lv_display_t *display = lv_windows_create_display(
        L"TTR LP02 - racer tuning", SIM_WIDTH, SIM_HEIGHT, SIM_ZOOM, false, true);
    if (display == NULL) {
        return 1;
    }

    VehicleData_Init();
    ui_init();

    Racer_Init();
    build_fields();

    loadScreen(SCREEN_ID_GAME2);
    Racer_SetActive(true);
    make_readout();

    uint32_t last = HAL_GetTick();
    uint32_t frames = 0, fps = 0, fps_since = last;

    for (;;) {
        const uint32_t now = HAL_GetTick();
        float dt = (float)(now - last) / 1000.0f;
        last = now;
        if (dt > 0.1f) {
            dt = 0.1f;
        }

        feed_controls(dt);
        handle_tuning_keys();

        Racer_Service(now);

        frames++;
        if ((now - fps_since) >= 500u) {
            fps = (frames * 1000u) / (now - fps_since);
            frames = 0;
            fps_since = now;
            update_readout(fps);
        }

        lv_timer_handler();
        Sleep(1);
    }
}
