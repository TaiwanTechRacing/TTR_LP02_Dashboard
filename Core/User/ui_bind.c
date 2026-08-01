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

#include "vehicle_data.h"
#include <stdio.h>

/* Shown in place of a value whose signal has timed out */
#define STALE_TEXT "---"

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
    if (VehicleData_IsStale(VD_GROUP_VCU_SENSOR2, VD_DEFAULT_TIMEOUT_MS)) {
        return STALE_TEXT;
    }

    snprintf(s_speed_buf, sizeof(s_speed_buf), "%u", (unsigned)g_vehicle.car_speed_kph);
    return s_speed_buf;
}

/**
 * Ready-to-drive state.
 *
 * Returns the text to display, not a state code - colour changes belong in the
 * EEZ style, the firmware only supplies content.
 */
const char *get_var_ready(void)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_STATE, VD_DEFAULT_TIMEOUT_MS)) {
        return STALE_TEXT;
    }

    return g_vehicle.rtd_active ? "READY" : "NOT READY";
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
 * Value driving the bar widget.
 *
 * The bar in EEZ is currently configured min=18 max=30, the low voltage battery
 * range, while the label above it reads SOC. This returns GLV voltage to match
 * the variable name. If that bar is meant to show pack SOC instead, change the
 * EEZ range to 0..100 and return pack_soc here.
 *
 * On timeout it returns 0 so the bar empties, which reads as abnormal far more
 * clearly than a bar frozen part way up.
 */
int32_t get_var_lv(void)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_SYSTEM, VD_DEFAULT_TIMEOUT_MS)) {
        return 0;
    }

    return (int32_t)(g_vehicle.glv_voltage + 0.5f);
}
