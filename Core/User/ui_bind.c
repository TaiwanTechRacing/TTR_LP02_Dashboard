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
#include "game_tetris.h"
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
 * Boot sequence, in order:
 *
 *   0 ms      the team name alone on black
 *   500 ms    the car name reveals one character at a time, left to right
 *   ~2160 ms  fully revealed, and the screen waits
 *   until     every signal the main screen shows has arrived, or 10 s
 *
 * The pause before the reveal exists so the two titles read as two beats
 * rather than one crowded frame.
 *
 * Waiting for CAN rather than switching on a fixed timer means the main screen
 * appears with real numbers on it. Switching early showed a dashboard full of
 * "---" for a second or two, which looks like a fault on a car that is merely
 * still starting up. The timeout is what stops that from becoming a dashboard
 * that never appears: if the bus really is dead, the driver still needs the
 * screen, and "---" is then the honest reading.
 */
#define SPLASH_NAME       "LEOPARD02"
#define SPLASH_START_MS   500u   /* team name alone before the reveal begins */
#define SPLASH_CHAR_MS    140u   /* per character; 9 chars ~ 1.3 s */
#define SPLASH_HOLD_MS    400u   /* fully shown before the screen may change */
#define BOOT_CAN_TIMEOUT_MS 10000u

static uint32_t s_splash_start_tick;
static bool     s_splash_started;

/** Milliseconds since the splash screen first drew. */
static uint32_t splash_elapsed(void)
{
    if (!s_splash_started) {
        /* Self-arming on first use: the welcome screen is loaded by ui_init()
         * before the main loop starts, so there is no other natural hook. */
        s_splash_started = true;
        s_splash_start_tick = HAL_GetTick();
    }

    return HAL_GetTick() - s_splash_start_tick;
}

/** How many characters of SPLASH_NAME should be visible right now. */
static size_t splash_visible_chars(void)
{
    const size_t len = sizeof(SPLASH_NAME) - 1u;

    const uint32_t elapsed = splash_elapsed();
    if (elapsed < SPLASH_START_MS) {
        return 0u;      /* team name only */
    }

    const size_t shown = ((elapsed - SPLASH_START_MS) / SPLASH_CHAR_MS) + 1u;
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
 * True once every signal the main screen displays has arrived.
 *
 * Deliberately only the four groups Main reads, not every group on the bus.
 * Waiting on data no one is about to look at would hold the splash up for a
 * subsystem the driver cannot see anyway.
 */
static bool boot_signals_ready(void)
{
    static const vd_group_t required[] = {
        VD_GROUP_VCU_STATE,     /* ready flag, drive mode */
        VD_GROUP_VCU_SENSOR2,   /* speed */
        VD_GROUP_VCU_SYSTEM,    /* GLV voltage */
        VD_GROUP_AMS_STATUS,    /* pack voltage and SOC */
    };

    for (size_t i = 0; i < (sizeof(required) / sizeof(required[0])); i++) {
        if (VehicleData_IsStale(required[i], VD_DEFAULT_TIMEOUT_MS)) {
            return false;
        }
    }

    return true;
}

/**
 * Whether the splash screen has finished and the main screen should take over.
 *
 * Lives here rather than in main.c because it is entirely about the splash
 * animation and the data behind it, and because the simulator has to make the
 * same decision - a second copy of this sequence would drift.
 */
bool UIBind_BootComplete(void)
{
    const uint32_t elapsed = splash_elapsed();

    const uint32_t reveal_done = SPLASH_START_MS
                               + ((sizeof(SPLASH_NAME) - 1u) * SPLASH_CHAR_MS)
                               + SPLASH_HOLD_MS;

    if (elapsed < reveal_done) {
        return false;       /* never cut the animation short */
    }

    if (elapsed >= BOOT_CAN_TIMEOUT_MS) {
        return true;        /* bus is not coming up; show the dashboard anyway */
    }

    return boot_signals_ready();
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

/*
 * ECU health on the SYSTEM-ECU page: the node is online and reporting no fault.
 *
 * Two messages have to agree, so both have to be fresh. A stale VCU_ONLINE
 * means we do not know who is on the bus; a stale VCU_ERROR means we do not
 * know whether they are healthy. Either way the honest answer is not "good",
 * and the bar empties - same direction as the SDC page, and for the same
 * reason.
 *
 * The dashboard cannot see these nodes itself. Its CAN filter admits about
 * sixteen IDs and none of the per-MCU status messages are among them, so this
 * is the VCU's view of the bus, not the dashboard's.
 */
static int32_t ecu_ok(uint8_t online_bit, uint8_t error_bit)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_ONLINE, VD_DEFAULT_TIMEOUT_MS) ||
        VehicleData_IsStale(VD_GROUP_VCU_ERROR, VD_DEFAULT_TIMEOUT_MS)) {
        return 0;
    }

    const bool online = (g_vehicle.online_flags & online_bit) != 0u;
    const bool faulted = (error_bit != 0u) &&
                         ((g_vehicle.error_flags & error_bit) != 0u);

    return (online && !faulted) ? 1 : 0;
}

#define ECU_GETTER(suffix, online_bit, error_bit) \
    int32_t get_var_ecu_##suffix(void) { return ecu_ok(online_bit, error_bit); }

ECU_GETTER(mcu1, VD_ONLINE_MCU1, VD_ERR_MCU1)
ECU_GETTER(mcu2, VD_ONLINE_MCU2, VD_ERR_MCU2)
ECU_GETTER(mcu3, VD_ONLINE_MCU3, VD_ERR_MCU3)
ECU_GETTER(mcu4, VD_ONLINE_MCU4, VD_ERR_MCU4)
ECU_GETTER(ams,  VD_ONLINE_AMS,  VD_ERR_AMS)
ECU_GETTER(imu,  VD_ONLINE_IMU,  VD_ERR_IMU)

/*
 * GPS has an online bit but no fault signal anywhere in the DBC, so this bar
 * means "online" only. It looks identical to the other six, which is worth
 * knowing: a GPS that is present but producing nonsense would still show green.
 */
ECU_GETTER(gps,  VD_ONLINE_GPS,  0)

/*
 * Sensor page: the two throttle channels, the two brake pressures and the
 * steering angle.
 *
 * Both APPS channels and both BSE channels are shown rather than one of each,
 * because the interesting failure is the two disagreeing - that is what the
 * VCU's plausibility check trips on, and a single reading would hide it.
 */
static char s_sensor_text[7][12];

static int32_t sensor_pct(float value)
{
    if (value <= 0.0f) {
        return 0;
    }
    if (value >= 100.0f) {
        return 100;
    }
    return (int32_t)(value + 0.5f);
}

static const char *sensor_pct_text(uint8_t slot, float value, vd_group_t group)
{
    if (VehicleData_IsStale(group, VD_DEFAULT_TIMEOUT_MS)) {
        return STALE_TEXT;
    }

    snprintf(s_sensor_text[slot], sizeof(s_sensor_text[slot]), "%d%%",
             (int)sensor_pct(value));
    return s_sensor_text[slot];
}

int32_t get_var_apps1_bar(void)
{
    return VehicleData_IsStale(VD_GROUP_VCU_SENSOR2, VD_DEFAULT_TIMEOUT_MS)
           ? 0 : sensor_pct(g_vehicle.apps1_pu);
}

int32_t get_var_apps2_bar(void)
{
    return VehicleData_IsStale(VD_GROUP_VCU_SENSOR2, VD_DEFAULT_TIMEOUT_MS)
           ? 0 : sensor_pct(g_vehicle.apps2_pu);
}

int32_t get_var_bse_front_bar(void)
{
    return VehicleData_IsStale(VD_GROUP_VCU_SENSOR1, VD_DEFAULT_TIMEOUT_MS)
           ? 0 : sensor_pct(g_vehicle.bse_front_pu);
}

int32_t get_var_bse_rear_bar(void)
{
    return VehicleData_IsStale(VD_GROUP_VCU_SENSOR1, VD_DEFAULT_TIMEOUT_MS)
           ? 0 : sensor_pct(g_vehicle.bse_rear_pu);
}

const char *get_var_apps1_text(void)
{
    return sensor_pct_text(0, g_vehicle.apps1_pu, VD_GROUP_VCU_SENSOR2);
}

const char *get_var_apps2_text(void)
{
    return sensor_pct_text(1, g_vehicle.apps2_pu, VD_GROUP_VCU_SENSOR2);
}

const char *get_var_bse_front_text(void)
{
    return sensor_pct_text(2, g_vehicle.bse_front_pu, VD_GROUP_VCU_SENSOR1);
}

const char *get_var_bse_rear_text(void)
{
    return sensor_pct_text(3, g_vehicle.bse_rear_pu, VD_GROUP_VCU_SENSOR1);
}

/*
 * Brake line pressure, to the nearest bar.
 *
 * The unit is on the value because the two bars further left are the same two
 * circuits as a percentage of pedal travel, and an unlabelled number next to
 * them would read as another percentage.
 *
 * Whole bar rather than one decimal, because that is what fits. Orbitron's
 * digits are tabular at this size, so "128 bar" is 88 px whatever the digits
 * are, against 96 px of room beside the caption - but the decimal point adds
 * 13 px and "99.9 bar" spills out of the container. A bar of resolution is
 * plenty for a readout that is being glanced at.
 */
static const char *brake_bar_text(uint8_t slot, float value)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_SENSOR1, VD_DEFAULT_TIMEOUT_MS)) {
        return STALE_TEXT;
    }

    snprintf(s_sensor_text[slot], sizeof(s_sensor_text[slot]), "%.0f bar",
             (double)value);
    return s_sensor_text[slot];
}

const char *get_var_bse_front_press(void)
{
    return brake_bar_text(5, g_vehicle.bse_front_bar);
}

const char *get_var_bse_rear_press(void)
{
    return brake_bar_text(6, g_vehicle.bse_rear_bar);
}

/**
 * Steering angle for the arc, in degrees.
 *
 * The arc is symmetrical over -180..180, so it takes the angle directly. On
 * timeout it centres, which is wrong in the same way an empty bar is wrong -
 * but a needle frozen at full lock would be read as a real reading.
 */
int32_t get_var_steering_deg(void)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_SENSOR2, VD_DEFAULT_TIMEOUT_MS)) {
        return 0;
    }

    return (int32_t)(g_vehicle.steering_deg +
                     (g_vehicle.steering_deg >= 0.0f ? 0.5f : -0.5f));
}

/*
 * EEZ gives the arc a value-changed handler because an arc is draggable, and
 * that handler writes back through this setter. There is no touch controller
 * on this panel, so it can never fire - but the reference is real and has to
 * link. Writing the steering angle from the UI would be nonsense in any case:
 * it is a measurement, and the arc is a readout.
 */
void set_var_steering_deg(int32_t value)
{
    (void)value;
}

const char *get_var_steering_text(void)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_SENSOR2, VD_DEFAULT_TIMEOUT_MS)) {
        return STALE_TEXT;
    }

    snprintf(s_sensor_text[4], sizeof(s_sensor_text[4]), "%d",
             (int)get_var_steering_deg());
    return s_sensor_text[4];
}

/**
 * The corner indicator on the game page.
 *
 * Empty unless the wheel and pedals are driving the game, in which case the
 * player needs telling - the two input schemes look identical otherwise.
 */
const char *get_var_game_mode_text(void)
{
    return GameTetris_SteerMode() ? "STEER" : "";
}

/*
 * The four summary lines under the cell map.
 *
 * Each label carries its own caption so the page needs one widget per line
 * rather than a caption and a value side by side.
 *
 * The numbers are the AMS's own min, max and delta rather than the extremes of
 * the cell array. They are the same thing when everything is healthy, and when
 * they are not, the AMS's view is the one that trips the shutdown circuit.
 */
static char s_bat_text[4][32];

const char *get_var_bat_cell_text(void)
{
    if (VehicleData_IsStale(VD_GROUP_AMS_STATUS, VD_DEFAULT_TIMEOUT_MS)) {
        return "CELL " STALE_TEXT;
    }

    snprintf(s_bat_text[0], sizeof(s_bat_text[0]), "CELL %.3f-%.3fV",
             (double)g_vehicle.cell_v_min, (double)g_vehicle.cell_v_max);
    return s_bat_text[0];
}

/**
 * Cell voltage delta - the highest cell minus the lowest.
 *
 * Named to match CELL_V_DELTA in the DBC rather than invented here, so it is
 * the same word on the dashboard as in the AMS. It is the headline number on
 * this page: a pack is only as good as its worst cell, and the delta is what
 * says how far gone that cell is while there is still time to act.
 */
const char *get_var_bat_spread_text(void)
{
    if (VehicleData_IsStale(VD_GROUP_AMS_STATUS, VD_DEFAULT_TIMEOUT_MS)) {
        return "DELTA " STALE_TEXT;
    }

    snprintf(s_bat_text[1], sizeof(s_bat_text[1]), "DELTA %.3fV",
             (double)g_vehicle.cell_v_delta);
    return s_bat_text[1];
}

const char *get_var_bat_temp_text(void)
{
    if (VehicleData_IsStale(VD_GROUP_AMS_STATUS, VD_DEFAULT_TIMEOUT_MS)) {
        return "TEMP " STALE_TEXT;
    }

    snprintf(s_bat_text[2], sizeof(s_bat_text[2]), "TEMP %.0f-%.0f C",
             (double)g_vehicle.temp_min, (double)g_vehicle.temp_max);
    return s_bat_text[2];
}

/**
 * Where the weakest cell is, as segment and cell number.
 *
 * This slot used to show pack current and power. Both are live values that only
 * matter while driving, and while driving nobody is on this page - the main
 * screen is. What this page is for is standing next to the car afterwards, and
 * then the question is which cell to go and look at.
 *
 * Numbered the way the DBC does it: segments from 1, matching AMS_MODULE_1..8,
 * and cells from 0, matching C0..C13. Reading a coordinate off a heat map is
 * easy to get wrong by one, so it is printed.
 */
const char *get_var_bat_low_text(void)
{
    if (VehicleData_IsStale(VD_GROUP_AMS_CELLS, VD_DEFAULT_TIMEOUT_MS)) {
        return "LOW " STALE_TEXT;
    }

    uint16_t lowest = VD_NUM_CELLS;
    float lowest_v = 0.0f;

    for (uint16_t i = 0; i < VD_NUM_CELLS; i++) {
        const float v = g_vehicle.cell_voltage[i];
        if (v <= 0.0f) {
            continue;       /* never received */
        }
        if (lowest == VD_NUM_CELLS || v < lowest_v) {
            lowest = i;
            lowest_v = v;
        }
    }

    if (lowest == VD_NUM_CELLS) {
        return "LOW " STALE_TEXT;
    }

    snprintf(s_bat_text[3], sizeof(s_bat_text[3]), "LOW S%u-C%u",
             (unsigned)((lowest / VD_CELLS_PER_SEG) + 1u),
             (unsigned)(lowest % VD_CELLS_PER_SEG));
    return s_bat_text[3];
}

/*
 * Inverter faults, as one line for the marquee on the inverter page.
 *
 * Every active fault is listed, and the label scrolls when they do not fit -
 * which is the whole reason for a marquee here. Sixteen faults can be active
 * at once and no sensible font shows sixteen labels on a 480 px screen, so the
 * choice is between scrolling them and hiding all but the first.
 *
 * Ordered by inverter and then by kind, so the same fault always appears in
 * the same place in the sequence. Sorting by arrival time would read as more
 * urgent but makes it impossible to tell at a glance whether the list changed.
 */
static const char *const INV_FAULT_KIND[VD_INV_KINDS] = {
    "GATE", "ENC", "OTP", "OCP"
};

const char *get_var_inv_fault_text(void)
{
    static char buf[224];

    if (VehicleData_IsStale(VD_GROUP_VCU_MCU_STATUS, VD_DEFAULT_TIMEOUT_MS)) {
        return "INVERTER " STALE_TEXT;
    }

    size_t at = 0;
    buf[0] = '\0';

    for (uint8_t inv = 0; inv < VD_INV_COUNT; inv++) {
        for (uint8_t kind = 0; kind < VD_INV_KINDS; kind++) {
            if ((g_vehicle.inv_faults & VD_INV_FAULT(inv, kind)) == 0u) {
                continue;
            }

            const int n = snprintf(&buf[at], sizeof(buf) - at, "%sINV%u %s",
                                   (at == 0u) ? "" : "     ",
                                   (unsigned)(inv + 1u), INV_FAULT_KIND[kind]);
            if (n <= 0 || (size_t)n >= (sizeof(buf) - at)) {
                break;      /* out of room; what is already there still reads */
            }
            at += (size_t)n;
        }
    }

    return (at == 0u) ? "NO FAULT" : buf;
}

/*
 * The inverter table, one getter per cell.
 *
 * A cell per label rather than a formatted line per row, because Orbitron's
 * digits are not the same width - "1" is 7.8 px against 16.7 for "0" - so
 * padding a single string with spaces cannot line the columns up. Fixed label
 * positions can, and are the only thing that can.
 */
static char s_inv_cell[VD_INV_COUNT][2][16];

static bool inv_stale(void)
{
    return VehicleData_IsStale(VD_GROUP_VCU_MCU_STATUS, VD_DEFAULT_TIMEOUT_MS);
}

static const char *inv_motor_text(uint8_t inv)
{
    if (inv_stale()) {
        return STALE_TEXT;
    }

    snprintf(s_inv_cell[inv][0], sizeof(s_inv_cell[inv][0]), "%.0f",
             (double)g_vehicle.motor_temp[inv]);
    return s_inv_cell[inv][0];
}

/*
 * The gate phases as a range rather than just the hottest.
 *
 * Three phases at 90 and one phase alone at 90 are different problems - a
 * hard-working inverter against a phase with something wrong on it - and a
 * single number cannot tell them apart. The spread is the whole reason the
 * twelve phases are decoded instead of collapsed at arrival.
 */
static const char *inv_gate_text(uint8_t inv)
{
    if (inv_stale()) {
        return STALE_TEXT;
    }

    float lo = g_vehicle.gate_temp[inv][0];
    float hi = lo;
    for (uint8_t p = 1; p < 3u; p++) {
        const float t = g_vehicle.gate_temp[inv][p];
        if (t < lo) {
            lo = t;
        }
        if (t > hi) {
            hi = t;
        }
    }

    snprintf(s_inv_cell[inv][1], sizeof(s_inv_cell[inv][1]), "%.0f~%.0f",
             (double)lo, (double)hi);
    return s_inv_cell[inv][1];
}

const char *get_var_inv1_motor(void) { return inv_motor_text(0); }
const char *get_var_inv2_motor(void) { return inv_motor_text(1); }
const char *get_var_inv3_motor(void) { return inv_motor_text(2); }
const char *get_var_inv4_motor(void) { return inv_motor_text(3); }

const char *get_var_inv1_gate(void) { return inv_gate_text(0); }
const char *get_var_inv2_gate(void) { return inv_gate_text(1); }
const char *get_var_inv3_gate(void) { return inv_gate_text(2); }
const char *get_var_inv4_gate(void) { return inv_gate_text(3); }

/**
 * One-time widget setup. See the note in the header.
 */
void UIBind_Init(void)
{
    /*
     * The fault line scrolls when its text is wider than the label, and sits
     * still when it fits. LVGL does the scrolling; all this does is ask for it,
     * because EEZ has no way to.
     */
    lv_label_set_long_mode(objects.inv_fault_label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
}

/** Score on the game page. */
const char *get_var_tetris_score(void)
{
    return GameTetris_ScoreText();
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

    static const lv_color_t green  = LV_COLOR_MAKE(0x02, 0xff, 0x02);
    static const lv_color_t yellow = LV_COLOR_MAKE(0xff, 0xd0, 0x00);
    static const lv_color_t red    = LV_COLOR_MAKE(0xff, 0x20, 0x20);
    static const lv_color_t white  = LV_COLOR_MAKE(0xff, 0xff, 0xff);

    const bool ready = !VehicleData_IsStale(VD_GROUP_VCU_STATE, VD_DEFAULT_TIMEOUT_MS)
                       && g_vehicle.rtd_active;

    lv_obj_set_style_text_color(objects.ready_label,
                                ready ? green : red,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    /*
     * A faulted inverter's name goes red. The marquee below already names it,
     * but that has to be waited for when several are scrolling, and the colour
     * does not.
     */
    {
        lv_obj_t *const names[VD_INV_COUNT] = {
            objects.inv1_name, objects.inv2_name,
            objects.inv3_name, objects.inv4_name,
        };

        const bool stale = VehicleData_IsStale(VD_GROUP_VCU_MCU_STATUS,
                                               VD_DEFAULT_TIMEOUT_MS);

        for (uint8_t i = 0; i < VD_INV_COUNT; i++) {
            const bool faulted = !stale &&
                ((g_vehicle.inv_faults & (uint16_t)(0x0Fu << (i * VD_INV_KINDS))) != 0u);

            lv_obj_set_style_text_color(names[i], faulted ? red : white,
                                        LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }

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
