/*
 * ui_bind.c
 *
 *  Implements the get_var_xxx() functions EEZ Studio declares in vars.h.
 *
 *  With Flow disabled, an EEZ LVGL project only emits the declarations - where
 *  the values come from is up to us. This file is the seam:
 *
 *      vehicle_data  ->  [ui_bind.c]  ->  EEZ-generated screens.c
 *
 *  Moving widgets, restyling, or swapping fonts in EEZ does not touch this
 *  file. Only adding or renaming a variable requires a new getter here.
 *
 *  What this replaces: an updatescreen() switch that hard-coded several hundred
 *  objects.xxx widget names, so regenerating the layout broke the whole build.
 *
 *  -- Stale signal handling --
 *  Every getter checks VehicleData_IsStale() first. When CAN drops, the display
 *  shows "---" instead of freezing on the last value. A driver looking at a
 *  frozen 600V reading assumes everything is fine, which is a safety problem.
 */

#include "ui_bind.h"
#include "vehicle_data.h"
#include "screens.h"
#include "stm32h7xx_hal.h"
#include <stddef.h>
#include <stdio.h>

/* Shown in place of a value whose signal has timed out */
#define STALE_TEXT "---"

/*
 * Startup sweep: the speed readout runs 0 -> 150 -> 0 once, as a gauge
 * self-test in the same spirit as a car's needle sweep at ignition. It proves
 * the display path works end to end before the driver trusts it.
 *
 * It is armed when the main screen appears but only starts once a speed frame
 * has actually arrived. Running it earlier looked like a fault: the numbers
 * would animate, finish, and then drop to "---" because the bus was not up
 * yet. Waiting means the sweep says "data is flowing" rather than contradicting
 * itself a second later.
 *
 * Driven from get_var_speed() rather than by animating the widget, so it stays
 * inside the binding layer and survives any layout change.
 */
#define SWEEP_PEAK_KPH    150u
#define SWEEP_DURATION_MS 1400u

typedef enum {
    SWEEP_IDLE = 0,   /* before the main screen is shown */
    SWEEP_ARMED,      /* main screen up, waiting for the first speed frame */
    SWEEP_RUNNING,
    SWEEP_DONE        /* terminal - the sweep is a boot ceremony, not a
                       * reconnect animation, so a later dropout does not
                       * replay it */
} sweep_state_t;

static sweep_state_t s_sweep_state;
static uint32_t      s_sweep_start_tick;

/*
 * One buffer per getter.
 *
 * A local would go out of scope before LVGL reads it, and a single shared
 * buffer would not work either: LVGL calls several getters back to back within
 * one refresh, so later calls would overwrite earlier results.
 */
static char s_speed_buf[8];
static char s_soc_buf[12];
static char s_lv_buf[16];
static char s_hv_buf[16];

/**
 * Vehicle speed. Integer, no leading zeros - padding wastes horizontal space
 * at the large font size used on the main screen.
 */
const char *get_var_speed(void)
{
    const bool speed_fresh = !VehicleData_IsStale(VD_GROUP_VCU_SENSOR2,
                                                  VD_DEFAULT_TIMEOUT_MS);

    if (s_sweep_state == SWEEP_ARMED) {
        if (!speed_fresh) {
            return STALE_TEXT;      /* bus not up yet - say so honestly */
        }
        s_sweep_state = SWEEP_RUNNING;
        s_sweep_start_tick = HAL_GetTick();
    }

    if (s_sweep_state == SWEEP_RUNNING) {
        const uint32_t elapsed = HAL_GetTick() - s_sweep_start_tick;

        if (elapsed >= SWEEP_DURATION_MS) {
            s_sweep_state = SWEEP_DONE;
        }
        else {
            /* Triangle ramp: up over the first half, back down over the second.
             * Integer maths throughout - no float, and the peak is hit exactly. */
            const uint32_t half = SWEEP_DURATION_MS / 2u;
            const uint32_t phase = (elapsed < half) ? elapsed
                                                    : (SWEEP_DURATION_MS - elapsed);
            snprintf(s_speed_buf, sizeof(s_speed_buf), "%u",
                     (unsigned)((phase * SWEEP_PEAK_KPH) / half));
            return s_speed_buf;
        }
    }

    if (!speed_fresh) {
        return STALE_TEXT;
    }

    snprintf(s_speed_buf, sizeof(s_speed_buf), "%u", (unsigned)g_vehicle.car_speed_kph);
    return s_speed_buf;
}

/**
 * Arm the startup sweep. It begins on the first speed frame, not immediately.
 */
void UIBind_ArmStartupSweep(void)
{
    s_sweep_state = SWEEP_ARMED;
}

/**
 * Ready-to-drive state.
 *
 * "N-RDY" rather than "NOT READY": the label is content-sized, so the longer
 * string grew past the right edge of the 480 px panel.
 *
 * Colour is applied separately in UIBind_ApplyDynamicStyles() - a getter can
 * only return text.
 */
const char *get_var_ready(void)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_STATE, VD_DEFAULT_TIMEOUT_MS)) {
        return STALE_TEXT;
    }

    return g_vehicle.rtd_active ? "READY" : "N-RDY";
}

/** High voltage pack state of charge. */
const char *get_var_label_soc_value(void)
{
    if (VehicleData_IsStale(VD_GROUP_AMS_STATUS, VD_DEFAULT_TIMEOUT_MS)) {
        return STALE_TEXT;
    }

    snprintf(s_soc_buf, sizeof(s_soc_buf), "%.0f%%", (double)g_vehicle.pack_soc);
    return s_soc_buf;
}

/** Low voltage battery. */
const char *get_var_label_lv_value(void)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_SYSTEM, VD_DEFAULT_TIMEOUT_MS)) {
        return "LV:" STALE_TEXT;
    }

    snprintf(s_lv_buf, sizeof(s_lv_buf), "LV:%.1fV", (double)g_vehicle.glv_voltage);
    return s_lv_buf;
}

/** High voltage pack. */
const char *get_var_label_hv_value(void)
{
    if (VehicleData_IsStale(VD_GROUP_AMS_STATUS, VD_DEFAULT_TIMEOUT_MS)) {
        return "HV:" STALE_TEXT;
    }

    snprintf(s_hv_buf, sizeof(s_hv_buf), "HV:%.0fV", (double)g_vehicle.pack_voltage);
    return s_hv_buf;
}

/*
 * Splash reveal: the car name appears one character at a time, right to left.
 *
 * The trick is that the revealed text is always a suffix of the full string,
 * so no buffer is needed - the getter returns a pointer part-way into the
 * literal. "2", then "02", "D02", and so on.
 *
 * Growing leftward needs the right edge pinned, which the label does not do on
 * its own: it is content-sized, so by default it grows rightward from its x.
 * Growing leftward is handled by splash_setup_alignment(), which pins the
 * label to the finished string's width and right-aligns its text.
 */
#define SPLASH_NAME       "LEOPARD02"
#define SPLASH_CHAR_MS    140u   /* per character; 9 chars ~ 1.3 s */
#define SPLASH_HOLD_MS    400u   /* fully shown before the screen changes */

static uint32_t s_splash_start_tick;
static bool     s_splash_started;

/** How many characters of SPLASH_NAME should be visible right now. */
static size_t splash_visible_chars(void)
{
    const size_t len = sizeof(SPLASH_NAME) - 1u;

    if (!s_splash_started) {
        /* Self-arming on first use: the welcome screen is loaded by ui_init()
         * before the main loop starts, so there is no other natural hook. */
        s_splash_started = true;
        s_splash_start_tick = HAL_GetTick();
    }

    const size_t shown = ((HAL_GetTick() - s_splash_start_tick) / SPLASH_CHAR_MS) + 1u;
    return (shown > len) ? len : shown;
}

/**
 * Car name on the splash screen, revealed progressively.
 *
 * Returning a pointer into the literal is safe: it is static storage and LVGL
 * copies the text when the label is set.
 */
const char *get_var_leopard02(void)
{
    static const char name[] = SPLASH_NAME;
    const size_t len = sizeof(name) - 1u;

    return &name[len - splash_visible_chars()];
}

/*
 * Make the splash label grow leftward.
 *
 * A content-sized label grows rightward from its x, which would reveal the name
 * left to right. Instead the label is pinned to the width of the finished
 * string once, with its text right-aligned: shorter text then sits against the
 * right edge of that fixed box and the name appears to extend leftward.
 *
 * Done once rather than by moving x every frame - repositioning per frame has
 * to race LVGL's layout pass, and getting that wrong let partial text run off
 * the screen.
 *
 * The width is measured rather than hard-coded, so changing the font or the
 * string in EEZ cannot silently break the alignment.
 */
static void splash_setup_alignment(void)
{
    static bool done;

    lv_obj_t *label = objects.ready_label_1;

    if (done || label == NULL) {
        return;
    }

    const lv_font_t *font = lv_obj_get_style_text_font(label, LV_PART_MAIN);
    const int32_t letter_space = lv_obj_get_style_text_letter_space(label, LV_PART_MAIN);

    lv_point_t full;
    lv_text_get_size(&full, SPLASH_NAME, font, letter_space, 0,
                     LV_COORD_MAX, LV_TEXT_FLAG_NONE);

    lv_obj_set_width(label, full.x);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);

    done = true;
}

/**
 * Drive mode reported by the VCU.
 *
 * Uses vd_drive_mode_t rather than bare numbers. The code this replaced had two
 * copies of this mapping that disagreed: one listed all four modes, the other
 * collapsed anything above 1 into "DYC", so the racing page showed the wrong
 * mode whenever RATIO was selected.
 */
const char *get_var_mode(void)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_STATE, VD_DEFAULT_TIMEOUT_MS)) {
        return STALE_TEXT;
    }

    switch (g_vehicle.drive_mode) {
    case VD_DRIVE_MODE_OFF:   return "OFF";
    case VD_DRIVE_MODE_EDIFF: return "E-DIFF";
    case VD_DRIVE_MODE_RATIO: return "RATIO";
    case VD_DRIVE_MODE_DYC:   return "DYC";
    default:                  return "?";
    }
}

/**
 * Value driving the SOC bar, 0..100 percent.
 *
 * On timeout this returns 0 so the bar empties, which reads as abnormal far
 * more clearly than a bar frozen part way up.
 */
int32_t get_var_soc(void)
{
    if (VehicleData_IsStale(VD_GROUP_AMS_STATUS, VD_DEFAULT_TIMEOUT_MS)) {
        return 0;
    }

    return (int32_t)(g_vehicle.pack_soc + 0.5f);
}


/*
 * Colour cannot travel through a get_var_* getter - those return text only.
 * This is the single place in the firmware that touches a widget directly, and
 * it stays in ui_bind.c because that is the layer allowed to know widget names.
 *
 * Moving or restyling the label in EEZ does not affect this; only renaming its
 * identifier does.
 */
void UIBind_ApplyDynamicStyles(void)
{
    splash_setup_alignment();

    static const lv_color_t green = LV_COLOR_MAKE(0x02, 0xff, 0x02);
    static const lv_color_t red   = LV_COLOR_MAKE(0xff, 0x20, 0x20);

    const bool ready = !VehicleData_IsStale(VD_GROUP_VCU_STATE, VD_DEFAULT_TIMEOUT_MS)
                       && g_vehicle.rtd_active;

    lv_obj_set_style_text_color(objects.ready_label,
                                ready ? green : red,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
}
