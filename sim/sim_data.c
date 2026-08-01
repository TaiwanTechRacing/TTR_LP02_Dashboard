/*
 * sim_data.c
 */

#include "sim_data.h"
#include "vehicle_data.h"

#include <stdbool.h>

/*
 * Every so often the generator stops refreshing the signal groups for a few
 * seconds. That exercises the stale path - readings should fall back to "---"
 * and the RTD label should turn red. It is the behaviour hardest to test on the
 * car, because it means unplugging CAN while driving.
 */
#define SIM_DROPOUT_PERIOD_MS  SIM_SCENE_PERIOD_MS
#define SIM_DROPOUT_LENGTH_MS  3000u

/*
 * Synthetic vehicle data.
 *
 * Values sweep rather than sit still so that clipping, digit width and font
 * coverage all show up: speed runs the full 0..250 range, voltages and SOC
 * drift, and the ready flag toggles.
 */
void SimData_Feed(uint32_t now)
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
