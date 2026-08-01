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
#include <stdio.h>

/* Shown in place of a value whose signal has timed out */
#define STALE_TEXT "---"

/*
 * Startup sweep: the speed readout runs 0 -> 150 -> 0 once, when the main
 * screen first appears.
 *
 * It is a gauge self-test in the same spirit as a car's needle sweep at
 * ignition: it proves the display path works end to end before the driver
 * trusts it. It also covers the moment right after boot when no CAN frame has
 * arrived yet and the readout would otherwise sit at "---".
 *
 * Driven from get_var_speed() rather than by animating the widget, so it stays
 * inside the binding layer and survives any layout change.
 */
#define SWEEP_PEAK_KPH    150u
#define SWEEP_DURATION_MS 1400u

static uint32_t s_sweep_start_tick;
static bool     s_sweep_active;

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
    unsigned value;

    if (s_sweep_active) {
        const uint32_t elapsed = HAL_GetTick() - s_sweep_start_tick;

        if (elapsed >= SWEEP_DURATION_MS) {
            s_sweep_active = false;
            value = 0u;
        }
        else {
            /* Triangle ramp: up over the first half, back down over the second.
             * Integer maths throughout - no float, and the peak is hit exactly. */
            const uint32_t half = SWEEP_DURATION_MS / 2u;
            const uint32_t phase = (elapsed < half) ? elapsed : (SWEEP_DURATION_MS - elapsed);
            value = (unsigned)((phase * SWEEP_PEAK_KPH) / half);
        }

        snprintf(s_speed_buf, sizeof(s_speed_buf), "%u", value);
        return s_speed_buf;
    }

    if (VehicleData_IsStale(VD_GROUP_VCU_SENSOR2, VD_DEFAULT_TIMEOUT_MS)) {
        return STALE_TEXT;
    }

    snprintf(s_speed_buf, sizeof(s_speed_buf), "%u", (unsigned)g_vehicle.car_speed_kph);
    return s_speed_buf;
}

/**
 * Begin the startup sweep. Called once, when the main screen is first shown.
 */
void UIBind_StartStartupSweep(void)
{
    s_sweep_start_tick = HAL_GetTick();
    s_sweep_active = true;
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

/**
 * Kept only so the tree still builds against the previous EEZ export, which
 * bound the bar to a variable called "lv". Delete once everyone has pulled a
 * build generated after the bar was rebound to "soc".
 */
int32_t get_var_lv(void)
{
    return get_var_soc();
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
    static const lv_color_t green = LV_COLOR_MAKE(0x02, 0xff, 0x02);
    static const lv_color_t red   = LV_COLOR_MAKE(0xff, 0x20, 0x20);

    const bool ready = !VehicleData_IsStale(VD_GROUP_VCU_STATE, VD_DEFAULT_TIMEOUT_MS)
                       && g_vehicle.rtd_active;

    lv_obj_set_style_text_color(objects.ready_label,
                                ready ? green : red,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
}
