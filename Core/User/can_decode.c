/*
 * can_decode.c
 *
 *  Unpacks CAN frames into vehicle_data. This used to live in main.c and run
 *  inside the FDCAN ISR; it is now driven from the main loop, with frames
 *  arriving through the ring buffer in can_rx.c.
 */

#include "can_decode.h"
#include "can_rx.h"
#include "vehicle_data.h"
#include "ttr_can.h"

static void store_segment(uint8_t seg, const float *volts, const float *temps);
static void decode_one(const ttr_can_frame_t *frame);

/*
 * The eight AMS segment messages carry identical payloads and differ only in
 * type name and segment index. This was previously eight hand-expanded copies
 * totalling ~220 lines, where changing one field meant remembering all eight.
 *
 * Note the asymmetry: C0..C13 is 14 cells, T0..T6 is only 7 temperatures. Each
 * segment has 10 temperature slots but CAN only carries 7, so the last 3 stay
 * zero. The out-of-bounds bug fixed during the refactor came from sharing one
 * loop counter across both despite the different lengths.
 */
#define DECODE_AMS_MODULE(n, seg)                                              \
    case TTR_CAN_ID_AMS_AMS_MODULE_##n: {                                      \
        ttr_ams_ams_module_##n##_t m;                                          \
        ttr_ams_ams_module_##n##_unpack(&m, frame);                            \
        const float v[VD_CELLS_PER_SEG] = {                                    \
            m.C0, m.C1, m.C2,  m.C3,  m.C4,  m.C5,  m.C6,                      \
            m.C7, m.C8, m.C9,  m.C10, m.C11, m.C12, m.C13                      \
        };                                                                     \
        const float t[VD_TSENSORS_PER_SEG] = {                                 \
            m.T0, m.T1, m.T2, m.T3, m.T4, m.T5, m.T6, 0.0f, 0.0f, 0.0f         \
        };                                                                     \
        store_segment((seg), v, t);                                            \
        VehicleData_MarkFresh(VD_GROUP_AMS_CELLS);                             \
        break;                                                                 \
    }

void CAN_Poll(void)
{
    ttr_can_frame_t frame;

    /* Drain the whole queue each pass, so a slow iteration (full-screen
     * redraw, say) does not let the backlog grow. */
    while (CAN_RX_Dequeue(&frame)) {
        decode_one(&frame);
    }
}

static void decode_one(const ttr_can_frame_t *frame)
{
    switch (frame->id) {

    case TTR_CAN_ID_VCU_VCU_STATE: {
        ttr_vcu_vcu_state_t s;
        ttr_vcu_vcu_state_unpack(&s, frame);
        g_vehicle.rtd_active     = s.RDY_TO_DRIVE_ACTIVE;
        g_vehicle.cooling_active = s.COOLING_SYSTEM_ACTIVE;
        g_vehicle.drive_mode     = s.SYS_DRIVE_MODE;
        g_vehicle.tebppc_active  = s.TEBPPC_ACTIVE;
        g_vehicle.ams_ready      = s.AMS_RDY;
        VehicleData_MarkFresh(VD_GROUP_VCU_STATE);
        break;
    }

    case TTR_CAN_ID_VCU_VCU_SDC: {
        ttr_vcu_vcu_sdc_t s;
        ttr_vcu_vcu_sdc_unpack(&s, frame);

        /* This was 12 lines of read-modify-write bit shuffling starting at
         * bit 4 for historical reasons. Bit 0 is now the first node, matching
         * vd_sdc_node_t. */
        uint16_t sdc = 0;
        sdc |= (uint16_t)((s.IMD_STATUS    & 1u) << VD_SDC_IMD);
        sdc |= (uint16_t)((s.AMS_STATUS    & 1u) << VD_SDC_AMS);
        sdc |= (uint16_t)((s.BSPD_STATUS   & 1u) << VD_SDC_BSPD);
        sdc |= (uint16_t)((s.PDOC_STATUS   & 1u) << VD_SDC_PDOC);
        sdc |= (uint16_t)((s.CSB_STATUS    & 1u) << VD_SDC_CSB);
        sdc |= (uint16_t)((s.LSB_STATUS    & 1u) << VD_SDC_LSB);
        sdc |= (uint16_t)((s.RSB_STATUS    & 1u) << VD_SDC_RSB);
        sdc |= (uint16_t)((s.INRT_STATUS   & 1u) << VD_SDC_INRT);
        sdc |= (uint16_t)((s.BOTS_STATUS   & 1u) << VD_SDC_BOTS);
        sdc |= (uint16_t)((s.MCU_IL_STATUS & 1u) << VD_SDC_MCU_IL);
        sdc |= (uint16_t)((s.M1_IL_STATUS  & 1u) << VD_SDC_M1_IL);
        sdc |= (uint16_t)((s.M2_IL_STATUS  & 1u) << VD_SDC_M2_IL);
        sdc |= (uint16_t)((s.M3_IL_STATUS  & 1u) << VD_SDC_M3_IL);
        sdc |= (uint16_t)((s.M4_IL_STATUS  & 1u) << VD_SDC_M4_IL);
        sdc |= (uint16_t)((s.TSMS_STATUS   & 1u) << VD_SDC_TSMS);
        sdc |= (uint16_t)((s.MSD_STATUS    & 1u) << VD_SDC_MSD);
        g_vehicle.sdc_status = sdc;

        VehicleData_MarkFresh(VD_GROUP_VCU_SDC);
        break;
    }

    case TTR_CAN_ID_VCU_VCU_SENSOR1: {
        ttr_vcu_vcu_sensor1_t s;
        ttr_vcu_vcu_sensor1_unpack(&s, frame);
        g_vehicle.bse_rear_pu  = s.BSE_REAR_PU;
        g_vehicle.bse_front_pu = s.BSE_FRONT_PU;
        VehicleData_MarkFresh(VD_GROUP_VCU_SENSOR1);
        break;
    }

    case TTR_CAN_ID_VCU_VCU_SENSOR2: {
        ttr_vcu_vcu_sensor2_t s;
        ttr_vcu_vcu_sensor2_unpack(&s, frame);
        /* Map steering -180..+180 deg onto 0..100 and invert it: the on-screen
         * arc grows clockwise while positive steering angle is anticlockwise. */
        g_vehicle.steering_pct  = 100.0f - (s.STEERING_ANGLE + 180.0f) / 360.0f * 100.0f;
        g_vehicle.apps1_pu      = s.APPS1_PU;
        g_vehicle.apps2_pu      = s.APPS2_PU;

        /* The sensor page's arc is symmetrical over -180..180, so it wants the
         * angle itself rather than the 0..100 mapping above. */
        g_vehicle.steering_deg  = s.STEERING_ANGLE;
        g_vehicle.car_speed_kph = (uint16_t)(s.CAR_SPEED + 0.5f);   /* float in the new DBC */
        VehicleData_MarkFresh(VD_GROUP_VCU_SENSOR2);
        break;
    }

    /* Low voltage battery. Was VCU_SENSOR3 in the old DBC, moved to
     * VCU_SYSTEM_STATUS in the new one. */
    case TTR_CAN_ID_VCU_VCU_SYSTEM_STATUS: {
        ttr_vcu_vcu_system_status_t s;
        ttr_vcu_vcu_system_status_unpack(&s, frame);
        g_vehicle.glv_voltage = s.GLV_VOLTAGE;
        g_vehicle.glv_current = s.GLV_CURRENT;
        VehicleData_MarkFresh(VD_GROUP_VCU_SYSTEM);
        break;
    }

    case TTR_CAN_ID_VCU_VCU_ERROR: {
        ttr_vcu_vcu_error_t s;
        ttr_vcu_vcu_error_unpack(&s, frame);
        g_vehicle.error_flags = (uint8_t)(
              (s.MCU1_ERR ? VD_ERR_MCU1 : 0)
            | (s.MCU2_ERR ? VD_ERR_MCU2 : 0)
            | (s.MCU3_ERR ? VD_ERR_MCU3 : 0)
            | (s.MCU4_ERR ? VD_ERR_MCU4 : 0)
            | (s.AMS_ERR  ? VD_ERR_AMS  : 0)
            | (s.IMU_ERR  ? VD_ERR_IMU  : 0));
        VehicleData_MarkFresh(VD_GROUP_VCU_ERROR);
        break;
    }

    case TTR_CAN_ID_VCU_VCU_ONLINE: {
        ttr_vcu_vcu_online_t s;
        ttr_vcu_vcu_online_unpack(&s, frame);
        g_vehicle.online_flags = (uint8_t)(
              (s.MCU1_ONLINE ? VD_ONLINE_MCU1 : 0)
            | (s.MCU2_ONLINE ? VD_ONLINE_MCU2 : 0)
            | (s.MCU3_ONLINE ? VD_ONLINE_MCU3 : 0)
            | (s.MCU4_ONLINE ? VD_ONLINE_MCU4 : 0)
            | (s.AMS_ONLINE  ? VD_ONLINE_AMS  : 0)
            | (s.IMU_ONLINE  ? VD_ONLINE_IMU  : 0)
            | (s.GPS_ONLINE  ? VD_ONLINE_GPS  : 0));
        VehicleData_MarkFresh(VD_GROUP_VCU_ONLINE);
        break;
    }

    case TTR_CAN_ID_VCU_VCU_GPS: {
        ttr_vcu_vcu_gps_t s;
        ttr_vcu_vcu_gps_unpack(&s, frame);
        g_vehicle.latitude  = s.LATITUDE;
        g_vehicle.longitude = s.LONGTITUDE;
        VehicleData_MarkFresh(VD_GROUP_VCU_GPS);
        break;
    }

    /* High voltage battery. The old DBC had a single AMS_STATUS0; the new one
     * splits it into BASIC + LIMIT, and everything the dashboard shows is in
     * BASIC. */
    case TTR_CAN_ID_AMS_AMS_STATUS_BASIC: {
        ttr_ams_ams_status_basic_t s;
        ttr_ams_ams_status_basic_unpack(&s, frame);
        g_vehicle.pack_voltage    = s.PACK_VOLTAGE;
        g_vehicle.pack_soc        = s.PACK_SOC;
        g_vehicle.pack_current    = s.PACK_CURRENT;
        g_vehicle.pack_power      = s.PACK_POWER;
        g_vehicle.temp_max        = s.TEMPERATURE_MAX;
        g_vehicle.temp_min        = s.TEMPERATURE_MIN;
        g_vehicle.temp_delta      = s.TEMPERATURE_DELTA;
        g_vehicle.cell_v_min      = s.CELL_V_MIN;
        g_vehicle.cell_v_max      = s.CELL_V_MAX;
        g_vehicle.cell_v_delta    = s.CELL_V_DELTA;
        g_vehicle.ams_state       = s.AMS_STATE;
        g_vehicle.cell_over_temp  = s.CELL_OVER_TEMP_ERR;
        g_vehicle.cell_over_volt  = s.CELL_OVER_VOLT_ERR;
        g_vehicle.cell_under_volt = s.CELL_UNDER_VOLT_ERR;
        VehicleData_MarkFresh(VD_GROUP_AMS_STATUS);
        break;
    }

    DECODE_AMS_MODULE(1, 0)
    DECODE_AMS_MODULE(2, 1)
    DECODE_AMS_MODULE(3, 2)
    DECODE_AMS_MODULE(4, 3)
    DECODE_AMS_MODULE(5, 4)
    DECODE_AMS_MODULE(6, 5)
    DECODE_AMS_MODULE(7, 6)
    DECODE_AMS_MODULE(8, 7)

    default:
        /* The hardware filter should only admit the IDs above; ignore anything else. */
        break;
    }
}

static void store_segment(uint8_t seg, const float *volts, const float *temps)
{
    if (seg >= VD_NUM_SEGMENTS) {
        return;
    }

    for (uint8_t i = 0; i < VD_CELLS_PER_SEG; i++) {
        g_vehicle.cell_voltage[seg * VD_CELLS_PER_SEG + i] = volts[i];
    }

    for (uint8_t i = 0; i < VD_TSENSORS_PER_SEG; i++) {
        g_vehicle.cell_temp[seg * VD_TSENSORS_PER_SEG + i] = temps[i];
    }
}
