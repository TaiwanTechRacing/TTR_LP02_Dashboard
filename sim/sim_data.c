/*
 * sim_data.c
 */

#include "sim_data.h"
#include "vehicle_data.h"
#include "racer.h"

#include <stdbool.h>

/*
 * Every so often the generator stops refreshing the signal groups for a few
 * seconds. That exercises the stale path - readings should fall back to "---"
 * and the RTD label should turn red. It is the behaviour hardest to test on the
 * car, because it means unplugging CAN while driving.
 */
#define SIM_DROPOUT_PERIOD_MS  SIM_SCENE_PERIOD_MS
#define SIM_DROPOUT_LENGTH_MS  3000u

/* No traffic at all for this long after start-up. */
#define SIM_BUS_SILENT_MS      4000u

/*
 * Synthetic vehicle data.
 *
 * Values sweep rather than sit still so that clipping, digit width and font
 * coverage all show up: speed runs the full 0..250 range, voltages and SOC
 * drift, and the ready flag toggles.
 */
void SimData_Feed(uint32_t now)
{
    /* The bus takes a moment to come up after power-on. Modelling that is what
     * makes the "armed, waiting for CAN" state visible in the simulator. */
    if (now < SIM_BUS_SILENT_MS) {
        return;
    }

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

    /*
     * The racer is played sitting still, so while it is up the pedals read
     * what a parked car reads. Without this the sweep spends most of its cycle
     * on the brake, the game's car never leaves the start line, and the
     * simulator says nothing useful about it.
     */
    if (Racer_IsActive()) {
        g_vehicle.apps1_pu = 0.0f;
    }
    /* Channel 2 tracks channel 1 with a small offset, which is what a healthy
     * pair looks like - identical readings would hide a wiring mistake. */
    g_vehicle.apps2_pu      = wave * 100.0f * 0.97f;
    g_vehicle.bse_rear_pu   = Racer_IsActive() ? 0.0f : (1.0f - wave) * 100.0f;
    g_vehicle.bse_front_pu  = Racer_IsActive() ? 0.0f : (1.0f - wave) * 100.0f * 0.92f;
    /* Rear runs a little higher than front on this car. */
    g_vehicle.bse_rear_bar  = (1.0f - wave) * 62.0f;
    g_vehicle.bse_front_bar = (1.0f - wave) * 55.0f;
    g_vehicle.steering_pct  = 50.0f + (wave - 0.5f) * 60.0f;
    g_vehicle.steering_deg  = (wave - 0.5f) * 240.0f;
    VehicleData_MarkFresh(VD_GROUP_VCU_SENSOR2);
    VehicleData_MarkFresh(VD_GROUP_VCU_SENSOR1);

    g_vehicle.glv_soc     = 40.0f + (wave * 55.0f);
    g_vehicle.glv_voltage = 24.0f + wave * 2.0f;
    g_vehicle.glv_current = 3.0f + wave;
    VehicleData_MarkFresh(VD_GROUP_VCU_SYSTEM);

    g_vehicle.pack_voltage = 560.0f + wave * 40.0f;
    /* Negative under load, positive under regen, so the sign is exercised too. */
    g_vehicle.pack_current = (wave * -260.0f) + 30.0f;
    g_vehicle.pack_power   = g_vehicle.pack_current * g_vehicle.pack_voltage;
    g_vehicle.pack_soc     = 15.0f + wave * 80.0f;
    g_vehicle.temp_max     = 30.0f + wave * 25.0f;
    g_vehicle.temp_min     = 28.0f + wave * 20.0f;
    g_vehicle.temp_delta   = g_vehicle.temp_max - g_vehicle.temp_min;
    VehicleData_MarkFresh(VD_GROUP_AMS_STATUS);

    /*
     * Cells around a nominal 3.7 V, with one deliberately weak cell so the map
     * has something to point at. Without an outlier the page would look the
     * same whether the colouring worked or not.
     */
    for (uint16_t i = 0; i < VD_NUM_CELLS; i++) {
        const float ripple = (float)((i * 7u) % 23u) * 0.0009f;
        g_vehicle.cell_voltage[i] = 3.70f + ripple - (wave * 0.25f);
    }
    g_vehicle.cell_voltage[45] -= 0.11f;      /* the one that ends the run */
    g_vehicle.cell_voltage[46] -= 0.04f;

    g_vehicle.cell_v_min = 3.70f - (wave * 0.25f) - 0.11f;
    g_vehicle.cell_v_max = 3.70f + 0.0198f - (wave * 0.25f);
    g_vehicle.cell_v_delta = g_vehicle.cell_v_max - g_vehicle.cell_v_min;
    VehicleData_MarkFresh(VD_GROUP_AMS_CELLS);

    /* Spend most of the cycle ready so the green state is the common case,
     * with a stretch of N-RDY to check the red styling and the shorter text. */
    g_vehicle.rtd_active     = (phase > 0.2f);
    g_vehicle.cooling_active = (phase > 0.5f);
    /* Warm up for the first fifth of the scene, so the red WARM mode label and
     * the normal one both appear in a screenshot run. */
    g_vehicle.warmup_ready   = (phase > 0.2f);
    /* 30 seconds counting down across the warm-up stretch of the scene. */
    g_vehicle.warmup_countdown_s =
        (uint8_t)((phase < 0.2f) ? ((0.2f - phase) * 150.0f) : 0.0f);
    /* Cycle the drive mode so the label is exercised, not just one value. */
    g_vehicle.drive_mode     = (uint8_t)((now / 3000u) % 4u);
    VehicleData_MarkFresh(VD_GROUP_VCU_STATE);

    /*
     * The main indicator walks all six states, two seconds each, so a
     * screenshot run sees every word and every colour rather than whichever
     * one the car happened to be in.
     *
     * 1700 ms rather than a round number: at 2000 ms the sixth state landed
     * inside the bus dropout on every single cycle - 2000*(6k+5) is always
     * 10000 past a 12000 ms scene - and RESET could not be photographed at all.
     */
    g_vehicle.main_status = (uint8_t)((now / 1700u) % (uint32_t)VD_MAIN_STATUS_COUNT);
    VehicleData_MarkFresh(VD_GROUP_VCU_DASH);

    /*
     * Shutdown circuit: everything closed except one node, which walks the list
     * a step at a time.
     *
     * Walking rather than randomising is the point. Each node is open for one
     * known interval, so a bar wired to the wrong node shows its gap out of
     * step with its label - a mis-binding becomes something you can see rather
     * than something you have to reason about. Fifteen bars all quietly
     * tracking the same variable is exactly the bug this page shipped with.
     */
    const uint8_t open_node = (uint8_t)((now / 800u) % (uint8_t)VD_SDC_COUNT);

    g_vehicle.sdc_status = (uint16_t)~0u;
    g_vehicle.sdc_status &= (uint16_t)~(1u << open_node);
    VehicleData_MarkFresh(VD_GROUP_VCU_SDC);

    /*
     * ECU health, walked the same way and for the same reason - a bar on the
     * wrong node shows its gap out of step with its label.
     *
     * Two failure modes alternate rather than one, because the page collapses
     * them into a single indicator: on even passes a node drops offline, on odd
     * passes it stays online but raises a fault. Both must empty the bar, and
     * only exercising one of them would leave half the logic unproven.
     */
    static const uint8_t ecu_online_bits[7] = {
        VD_ONLINE_MCU1, VD_ONLINE_MCU2, VD_ONLINE_MCU3, VD_ONLINE_MCU4,
        VD_ONLINE_AMS,  VD_ONLINE_IMU,  VD_ONLINE_GPS,
    };
    /* GPS has no fault signal in the DBC, hence the 0. */
    static const uint8_t ecu_error_bits[7] = {
        VD_ERR_MCU1, VD_ERR_MCU2, VD_ERR_MCU3, VD_ERR_MCU4,
        VD_ERR_AMS,  VD_ERR_IMU,  0,
    };

    const uint8_t ecu_step = (uint8_t)((now / 900u) % 14u);
    const uint8_t bad_ecu = (uint8_t)(ecu_step % 7u);
    const bool    fault_mode = (ecu_step >= 7u);

    uint8_t online = 0;
    uint8_t errors = 0;
    for (uint8_t i = 0; i < 7u; i++) {
        online |= ecu_online_bits[i];
    }

    if (fault_mode) {
        /* On the GPS turn there is no fault bit to raise, so the bar correctly
         * stays green - the page cannot show a bad GPS, only a missing one. */
        errors = ecu_error_bits[bad_ecu];
    }
    else {
        online &= (uint8_t)~ecu_online_bits[bad_ecu];
    }

    g_vehicle.online_flags = online;
    g_vehicle.error_flags = errors;

    VehicleData_MarkFresh(VD_GROUP_VCU_ONLINE);
    VehicleData_MarkFresh(VD_GROUP_VCU_ERROR);

    /*
     * Inverters. Temperatures climb with the same wave as everything else, and
     * the fault set walks so the marquee is exercised at every length - one
     * fault sits still, several scroll, and that difference is the feature.
     */
    for (uint8_t i = 0; i < VD_INV_COUNT; i++) {
        g_vehicle.motor_temp[i] = 40.0f + (wave * 45.0f) + ((float)i * 3.0f);
        for (uint8_t p = 0; p < 3u; p++) {
            g_vehicle.gate_temp[i][p] = 35.0f + (wave * 50.0f) + ((float)p * 4.0f);
        }
        /* One phase on INV3 running away from the other two, which is the
         * case the range format exists to make visible. */
        if (i == 2u) {
            g_vehicle.gate_temp[i][1] += 22.0f;
        }
    }

    const uint8_t fault_step = (uint8_t)((now / 4000u) % 5u);
    g_vehicle.inv_faults = 0;
    for (uint8_t n = 0; n < fault_step; n++) {
        g_vehicle.inv_faults |= VD_INV_FAULT(n % VD_INV_COUNT, n % VD_INV_KINDS);
    }

    VehicleData_MarkFresh(VD_GROUP_VCU_MCU_STATUS);
}
