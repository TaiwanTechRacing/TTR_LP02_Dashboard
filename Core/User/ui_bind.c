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
#include <string.h>
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

/*
 * Upper bound on what the speed readout will show.
 *
 * This is a layout constraint, not a vehicle limit. The label is centred and
 * content-sized, so its width decides how far it reaches, and Orbitron's digits
 * are not tabular: at 160 px "1" advances 63 px where the others take about
 * 133. Holding the hundreds digit at 1 therefore caps the whole reading at
 * roughly 330 px instead of 400, which keeps a comfortable margin from the SOC
 * panel no matter what the other two digits are.
 *
 * The trade is that a genuine reading above this would display wrong rather
 * than merely wide. 199 km/h is far beyond anything the car does, so in
 * practice the clamp never engages - but it is a clamp on the truth, so if the
 * car ever gets quick enough to reach it, widen this and check the layout
 * again rather than leaving it lying.
 */
#define SPEED_DISPLAY_MAX 199u

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

    unsigned kph = (unsigned)g_vehicle.car_speed_kph;
    if (kph > SPEED_DISPLAY_MAX) {
        kph = SPEED_DISPLAY_MAX;
    }

    snprintf(s_speed_buf, sizeof(s_speed_buf), "%u", kph);
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
 * Splash reveal: the car name appears one character at a time, left to right.
 *
 * "L", then "LE", "LEO", and so on. The label keeps the content size EEZ gave
 * it, so it simply grows rightward from its x - no alignment or width pinning
 * involved.
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
 * A prefix needs its own storage - unlike a suffix it cannot be a pointer into
 * the literal, because the terminator has to land part-way through.
 */
const char *get_var_leopard02(void)
{
    static char buf[sizeof(SPLASH_NAME)];

    const size_t shown = splash_visible_chars();

    memcpy(buf, SPLASH_NAME, shown);
    buf[shown] = '\0';

    return buf;
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

/*
 * Shutdown circuit nodes on the SYSTEM-SDC page.
 *
 * One getter per node, each driving a 0..1 bar: full means the node is closed,
 * empty means it has opened and is breaking the circuit.
 *
 * A stale VCU reads as open rather than closed. Both are wrong when there is no
 * data, but only one of them is wrong in the direction that matters: showing a
 * closed circuit for a car whose VCU has gone quiet invites someone to treat it
 * as safe.
 *
 * TSMS is deliberately not on this page, so it has no getter here.
 */
static int32_t sdc_node(vd_sdc_node_t node)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_SDC, VD_DEFAULT_TIMEOUT_MS)) {
        return 0;
    }

    return VehicleData_SdcNode(node) ? 1 : 0;
}

#define SDC_GETTER(suffix, node) \
    int32_t get_var_sdc_##suffix(void) { return sdc_node(node); }

SDC_GETTER(imd,    VD_SDC_IMD)
SDC_GETTER(ams,    VD_SDC_AMS)
SDC_GETTER(bspd,   VD_SDC_BSPD)
SDC_GETTER(pdoc,   VD_SDC_PDOC)
SDC_GETTER(csb,    VD_SDC_CSB)
SDC_GETTER(lsb,    VD_SDC_LSB)
SDC_GETTER(rsb,    VD_SDC_RSB)
SDC_GETTER(inrt,   VD_SDC_INRT)
SDC_GETTER(bots,   VD_SDC_BOTS)
SDC_GETTER(mcu_il, VD_SDC_MCU_IL)
SDC_GETTER(m1_il,  VD_SDC_M1_IL)
SDC_GETTER(m2_il,  VD_SDC_M2_IL)
SDC_GETTER(m3_il,  VD_SDC_M3_IL)
SDC_GETTER(m4_il,  VD_SDC_M4_IL)
SDC_GETTER(msd,    VD_SDC_MSD)

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

    static const lv_color_t green  = LV_COLOR_MAKE(0x02, 0xff, 0x02);
    static const lv_color_t yellow = LV_COLOR_MAKE(0xff, 0xd0, 0x00);
    static const lv_color_t red    = LV_COLOR_MAKE(0xff, 0x20, 0x20);

    const bool ready = !VehicleData_IsStale(VD_GROUP_VCU_STATE, VD_DEFAULT_TIMEOUT_MS)
                       && g_vehicle.rtd_active;

    lv_obj_set_style_text_color(objects.ready_label,
                                ready ? green : red,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    /*
     * SOC bar: green down to 50%, yellow to 30%, red below that. A stale pack
     * reads as 0 through get_var_soc(), so it turns red too - which is the
     * reading we want anyway.
     */
    const int32_t soc = get_var_soc();
    const int band = (soc >= 50) ? 2 : (soc >= 30) ? 1 : 0;

    /*
     * Only write on a change of band. Setting a style invalidates the object,
     * and this runs every UI tick - repainting the bar continuously for a
     * colour that did not move would give away frame time for nothing.
     */
    static int last_band = -1;
    if (band != last_band) {
        last_band = band;
        lv_obj_set_style_bg_color(objects.soc_bar,
                                  (band == 2) ? green : (band == 1) ? yellow : red,
                                  LV_PART_INDICATOR | LV_STATE_DEFAULT);
    }
}
