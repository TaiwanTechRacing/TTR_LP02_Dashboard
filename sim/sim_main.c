/*
 * sim_main.c
 *
 *  PC simulator for the dashboard UI.
 *
 *  Runs the exact screens.c that EEZ generates and the exact ui_bind.c that
 *  ships in the firmware, against a synthetic vehicle_data. Only three things
 *  differ from the target: the display is a Win32 window instead of LTDC, the
 *  data comes from the generator below instead of CAN, and HAL_GetTick() is
 *  backed by the Windows tick count.
 *
 *  What it is for: iterating on layout, fonts, colours and the stale-signal
 *  behaviour without a flash cycle. What it cannot tell you: frame rate,
 *  memory pressure, or anything about SDRAM, cache and LTDC timing - those are
 *  properties of the hardware and still need a real board.
 */

#include "lvgl.h"
#include "ui.h"
#include "ui_bind.h"
#include "vehicle_data.h"

#include <windows.h>
#include <stdbool.h>

#define SIM_WIDTH   480
#define SIM_HEIGHT  272

/* The panel is small; 200% makes it comfortable to look at on a desktop. */
#define SIM_ZOOM    200

/* Matches UI_UPDATE_PERIOD_MS in the firmware's main loop. */
#define SIM_UI_PERIOD_MS 25u

/*
 * Every so often the generator stops refreshing the signal groups for a few
 * seconds. That exercises the stale path - readings should fall back to "---"
 * and the RTD label should turn red. It is the behaviour hardest to test on the
 * car, because it means unplugging CAN while driving.
 */
#define SIM_DROPOUT_PERIOD_MS  12000u
#define SIM_DROPOUT_LENGTH_MS   3000u

static uint32_t s_start_tick;

uint32_t HAL_GetTick(void)
{
    return (uint32_t)(GetTickCount() - s_start_tick);
}

/*
 * Synthetic vehicle data.
 *
 * Values sweep rather than sit still so that clipping, digit width and font
 * coverage all show up: speed runs the full 0..250 range, voltages and SOC
 * drift, and the ready flag toggles.
 */
static void feed_fake_data(uint32_t now)
{
    const uint32_t cycle = now % SIM_DROPOUT_PERIOD_MS;
    const bool dropout = cycle >= (SIM_DROPOUT_PERIOD_MS - SIM_DROPOUT_LENGTH_MS);

    if (dropout) {
        /* Deliberately do not mark anything fresh, so everything goes stale. */
        return;
    }

    const float phase = (float)cycle / (float)SIM_DROPOUT_PERIOD_MS;   /* 0..1 */
    const float wave  = (phase < 0.5f) ? (phase * 2.0f) : (2.0f - phase * 2.0f);

    g_vehicle.car_speed_kph = (uint16_t)(wave * 250.0f);
    g_vehicle.apps1_pu      = wave * 100.0f;
    g_vehicle.bse_rear_pu   = (1.0f - wave) * 100.0f;
    g_vehicle.steering_pct  = 50.0f + (wave - 0.5f) * 60.0f;
    VehicleData_MarkFresh(VD_GROUP_VCU_SENSOR2);
    VehicleData_MarkFresh(VD_GROUP_VCU_SENSOR1);

    g_vehicle.glv_voltage = 24.0f + wave * 2.0f;
    g_vehicle.glv_current = 3.0f + wave;
    VehicleData_MarkFresh(VD_GROUP_VCU_SYSTEM);

    g_vehicle.pack_voltage = 560.0f + wave * 40.0f;
    g_vehicle.pack_soc     = 15.0f + wave * 80.0f;
    g_vehicle.temp_max     = 30.0f + wave * 25.0f;
    g_vehicle.temp_min     = 28.0f + wave * 20.0f;
    g_vehicle.temp_delta   = g_vehicle.temp_max - g_vehicle.temp_min;
    VehicleData_MarkFresh(VD_GROUP_AMS_STATUS);

    /* Spend most of the cycle ready so the green state is the common case,
     * with a stretch of N-RDY to check the red styling and the shorter text. */
    g_vehicle.rtd_active     = (phase > 0.2f);
    g_vehicle.cooling_active = (phase > 0.5f);
    g_vehicle.drive_mode     = VD_DRIVE_MODE_DYC;
    VehicleData_MarkFresh(VD_GROUP_VCU_STATE);
}

int main(void)
{
    s_start_tick = GetTickCount();

    lv_init();
    lv_tick_set_cb(HAL_GetTick);

    lv_display_t *display = lv_windows_create_display(
        L"TTR LP02 Dashboard - simulator",
        SIM_WIDTH, SIM_HEIGHT,
        SIM_ZOOM,
        false,      /* follow the Windows DPI setting */
        true);      /* simulator mode: fixed size, matching the real panel */

    if (display == NULL) {
        return 1;
    }

    /* The two dashboard buttons have no equivalent here; a mouse pointer is
     * still useful for poking at widgets while checking hit areas. */
    lv_windows_acquire_pointer_indev(display);

    VehicleData_Init();
    ui_init();
    UIBind_StartStartupSweep();

    uint32_t last_ui_update = 0;

    for (;;) {
        const uint32_t now = HAL_GetTick();

        feed_fake_data(now);

        lv_timer_handler();

        if ((now - last_ui_update) >= SIM_UI_PERIOD_MS) {
            last_ui_update = now;
            ui_tick();
            UIBind_ApplyDynamicStyles();
        }

        Sleep(1);
    }
}
