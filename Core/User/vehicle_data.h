/*
 * vehicle_data.h
 *
 *  整車資料模型 —— 這份檔案是 CAN 解包和畫面顯示之間唯一的介面。
 *
 *  重構前的作法是把一百多個 volatile 全域變數散在 main.c,再由一個寫死幾百個
 *  objects.xxx 名稱的 updatescreen() 大 switch 推到畫面上。後果是版面一改,
 *  資料層就跟著爆炸。現在拆成三層:
 *
 *      can_decode.c  →  vehicle_data  →  ui_bind.c  →  EEZ 產生的畫面
 *
 *  改版面只會動到 ui_bind.c,這一層和 can_decode.c 完全不用碰。
 *
 *  執行緒安全:不需要。CAN 解包已經從中斷搬到主迴圈(見 can_rx.h),
 *  寫入和讀取都在同一個執行緒,所以這裡不需要 volatile,也不需要臨界區。
 */

#ifndef VEHICLE_DATA_H
#define VEHICLE_DATA_H

#include <stdbool.h>
#include <stdint.h>

#define VD_NUM_CELLS        112u    /* 電芯總數 */
#define VD_NUM_TSENSORS      80u    /* 溫度感測點總數 */
#define VD_NUM_SEGMENTS       8u    /* AMS 分成 8 段 */
#define VD_CELLS_PER_SEG    (VD_NUM_CELLS / VD_NUM_SEGMENTS)      /* 14 */
#define VD_TSENSORS_PER_SEG (VD_NUM_TSENSORS / VD_NUM_SEGMENTS)   /* 10 */

/* VCU 回報的駕駛模式 */
typedef enum {
    VD_DRIVE_MODE_OFF   = 0,
    VD_DRIVE_MODE_EDIFF = 1,
    VD_DRIVE_MODE_RATIO = 2,
    VD_DRIVE_MODE_DYC   = 3,
} vd_drive_mode_t;

/*
 * Shutdown circuit 各節點在 sdc_status 裡的 bit 位置。
 *
 * 重構前這些是散落在程式裡的裸數字(sdcStatus 的 bit 4~15),而且 UI 那邊還有
 * 一份順序不同的對照表,兩邊要人工同步。現在只有這一份定義。
 */
typedef enum {
    VD_SDC_CSB = 0,
    VD_SDC_LSB,
    VD_SDC_RSB,
    VD_SDC_INRT,
    VD_SDC_BOTS,
    VD_SDC_MCU_IL,
    VD_SDC_M1_IL,
    VD_SDC_M2_IL,
    VD_SDC_M3_IL,
    VD_SDC_M4_IL,
    VD_SDC_TSMS,
    VD_SDC_MSD,
    VD_SDC_COUNT
} vd_sdc_node_t;

/* error_flags 的 bit 定義 */
#define VD_ERR_MCU1  (1u << 0)
#define VD_ERR_MCU2  (1u << 1)
#define VD_ERR_MCU3  (1u << 2)
#define VD_ERR_MCU4  (1u << 3)
#define VD_ERR_AMS   (1u << 4)

/* warn_flags 的 bit 定義 */
#define VD_WARN_TEBPPC        (1u << 0)
#define VD_WARN_CELL_OVERTEMP (1u << 1)

/*
 * 訊號群組。每個群組對應一則 CAN 訊息,各自記錄最後更新時間。
 *
 * 為什麼要分群組而不是整條匯流排一個時間戳:AMS 掉線和 VCU 掉線是兩件事,
 * 車手需要知道是哪一邊沒了。只看「有沒有 CAN」的話,VCU 還在傳但 AMS 死掉時
 * 電池數值會靜靜地停在最後一筆,看起來像正常值。
 */
typedef enum {
    VD_GROUP_VCU_STATE = 0,
    VD_GROUP_VCU_SDC,
    VD_GROUP_VCU_SENSOR1,
    VD_GROUP_VCU_SENSOR2,
    VD_GROUP_VCU_SENSOR3,
    VD_GROUP_VCU_ERROR,
    VD_GROUP_VCU_GPS,
    VD_GROUP_AMS_STATUS,
    VD_GROUP_AMS_CELLS,
    VD_GROUP_COUNT
} vd_group_t;

/** 訊號多久沒更新就視為過期。 */
#define VD_DEFAULT_TIMEOUT_MS 500u

typedef struct {
    /* --- VCU_STATE --- */
    bool     rtd_active;
    bool     cooling_active;
    bool     tebppc_active;
    bool     ams_ready;
    uint8_t  drive_mode;          /* vd_drive_mode_t */

    /* --- VCU_SDC --- */
    uint16_t sdc_status;          /* bit 位置見 vd_sdc_node_t */

    /* --- VCU_SENSOR1 --- */
    float    bse_rear_pu;         /* 煞車踏板 0~100 */

    /* --- VCU_SENSOR2 --- */
    float    steering_pct;        /* 方向盤角度換算成 0~100 */
    float    apps1_pu;            /* 油門踏板 0~100 */
    uint16_t car_speed_kph;

    /* --- VCU_SENSOR3 --- */
    float    glv_voltage;         /* 低壓電池電壓 */

    /* --- VCU_ERROR --- */
    uint8_t  error_flags;         /* VD_ERR_* */

    /* --- VCU_GPS --- */
    uint8_t  latitude;
    uint8_t  longitude;

    /* --- AMS_STATUS0 --- */
    float    pack_voltage;        /* 高壓電池組電壓 */
    float    pack_soc;            /* 0~100 */
    float    temp_max;
    float    temp_min;
    float    temp_delta;
    bool     cell_over_temp;

    /* --- AMS_MODULE_1..8 --- */
    float    cell_voltage[VD_NUM_CELLS];
    float    cell_temp[VD_NUM_TSENSORS];
} vehicle_data_t;

/** 全車唯一的資料實例。can_decode.c 寫,ui_bind.c 讀。 */
extern vehicle_data_t g_vehicle;

/** 開機時清空,並把所有群組標成尚未收到資料。 */
void VehicleData_Init(void);

/** 由 can_decode.c 在成功解包後呼叫,更新該群組的時間戳。 */
void VehicleData_MarkFresh(vd_group_t group);

/**
 * 該群組是否已經超過 timeout_ms 沒更新。
 * 開機後從未收到過也算過期 —— 顯示上要和「收過但斷了」一樣處理。
 */
bool VehicleData_IsStale(vd_group_t group, uint32_t timeout_ms);

/** 綜合目前狀態算出警告旗標(VD_WARN_*)。 */
uint8_t VehicleData_WarnFlags(void);

/** 取某一節點的 shutdown circuit 狀態。 */
bool VehicleData_SdcNode(vd_sdc_node_t node);

#endif /* VEHICLE_DATA_H */
