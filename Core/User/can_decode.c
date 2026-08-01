/*
 * can_decode.c
 *
 *  把 CAN frame 解包寫進 vehicle_data。原本這段在 main.c 裡、而且是跑在
 *  FDCAN 中斷內;現在由主迴圈呼叫(frame 來自 can_rx.c 的環形佇列)。
 */

#include "can_decode.h"
#include "can_rx.h"
#include "vehicle_data.h"
#include "ttr_can.h"

static void store_segment(uint8_t seg, const float *volts, const float *temps);
static void decode_one(const ttr_can_frame_t *frame);

/*
 * AMS 的八段訊息內容完全一樣,只有型別名稱和段號不同。原本是八段各自展開的
 * 複製貼上,合計約 220 行 —— 改一個欄位要記得改八個地方。
 *
 * 注意 C0~C13 有 14 個(電芯),T0~T6 只有 7 個。溫度那邊每段有 10 個位置,
 * 但 CAN 只送 7 個,剩下 3 個維持 0。重構前那個會踩記憶體的 bug 就是因為
 * 兩邊長度不同卻共用同一個迴圈計數。
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

    /* 一次清空佇列。即使某一圈因為重繪整頁而變慢,也不會讓佇列越積越多。 */
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

        /* 原本這裡是 12 行 read-modify-write 的 bit 搬移,而且位移量從 4 開始
         * (歷史遺留的偏移)。現在 bit 0 就是第一個節點,對齊 vd_sdc_node_t。 */
        uint16_t sdc = 0;
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
        g_vehicle.bse_rear_pu = s.BSE_REAR_PU;
        VehicleData_MarkFresh(VD_GROUP_VCU_SENSOR1);
        break;
    }

    case TTR_CAN_ID_VCU_VCU_SENSOR2: {
        ttr_vcu_vcu_sensor2_t s;
        ttr_vcu_vcu_sensor2_unpack(&s, frame);
        /* 方向盤 -180~+180 度換算成 0~100,而且左右反過來(畫面上的圓弧
         * 是順時針增加,方向盤是逆時針為正)。 */
        g_vehicle.steering_pct  = 100.0f - (s.STEERING_ANGLE + 180.0f) / 360.0f * 100.0f;
        g_vehicle.apps1_pu      = s.APPS1_PU;
        g_vehicle.car_speed_kph = s.CAR_SPEED;
        VehicleData_MarkFresh(VD_GROUP_VCU_SENSOR2);
        break;
    }

    case TTR_CAN_ID_VCU_VCU_SENSOR3: {
        ttr_vcu_vcu_sensor3_t s;
        ttr_vcu_vcu_sensor3_unpack(&s, frame);
        g_vehicle.glv_voltage = s.GLV_VOLTAGE;
        VehicleData_MarkFresh(VD_GROUP_VCU_SENSOR3);
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
            | (s.AMS_ERR  ? VD_ERR_AMS  : 0));
        VehicleData_MarkFresh(VD_GROUP_VCU_ERROR);
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

    case TTR_CAN_ID_AMS_AMS_STATUS0: {
        ttr_ams_ams_status0_t s;
        ttr_ams_ams_status0_unpack(&s, frame);
        g_vehicle.pack_voltage   = s.PACK_VOLTAGE;
        g_vehicle.pack_soc       = s.PACK_SOC;
        g_vehicle.temp_max       = s.TEMPERATURE_MAX;
        g_vehicle.temp_min       = s.TEMPERATURE_MIN;
        g_vehicle.temp_delta     = s.TEMPERATURE_DELTA;
        g_vehicle.cell_over_temp = s.CELL_OVER_TEMP_ERR;
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
        /* 濾波器理論上只放行上面這些 ID,收到別的就忽略。 */
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
