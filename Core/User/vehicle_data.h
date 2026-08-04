/*
 * vehicle_data.h
 *
 *  Vehicle data model - the single interface between CAN decoding and the display.
 *
 *  Before the refactor, a hundred-odd volatile globals were scattered through
 *  main.c and pushed to the screen by an updatescreen() switch that hard-coded
 *  several hundred objects.xxx widget names. Any layout change broke the data
 *  layer. The code is now split into three layers:
 *
 *      can_decode.c  ->  vehicle_data  ->  ui_bind.c  ->  EEZ-generated screens
 *
 *  Redesigning the layout only touches ui_bind.c; this layer and can_decode.c
 *  stay untouched.
 *
 *  Thread safety: not needed. CAN decoding moved out of the ISR into the main
 *  loop (see can_rx.h), so writes and reads happen on the same thread. No
 *  volatile, no critical sections.
 */

#ifndef VEHICLE_DATA_H
#define VEHICLE_DATA_H

#include <stdbool.h>
#include <stdint.h>

#define VD_NUM_CELLS        112u    /* total cells */
#define VD_NUM_TSENSORS      80u    /* total temperature sense points */
#define VD_NUM_SEGMENTS       8u    /* AMS is split into 8 segments */
#define VD_CELLS_PER_SEG    (VD_NUM_CELLS / VD_NUM_SEGMENTS)      /* 14 */
#define VD_TSENSORS_PER_SEG (VD_NUM_TSENSORS / VD_NUM_SEGMENTS)   /* 10 */

/* Drive mode reported by the VCU */
typedef enum {
    VD_DRIVE_MODE_OFF   = 0,
    VD_DRIVE_MODE_EDIFF = 1,
    VD_DRIVE_MODE_RATIO = 2,
    VD_DRIVE_MODE_DYC   = 3,
} vd_drive_mode_t;

/*
 * The main status indicator, straight out of VCU_DASH.
 *
 * The VCU decides this, not the dashboard. It knows the precharge contactors,
 * the fault latches and where the RTD sequence has got to; the dashboard sees
 * none of that and would have to guess at it from SDC bits and RTD_ACTIVE. Two
 * implementations of the same state machine that disagree is worse than no
 * indicator at all, so this one is a display of someone else's decision.
 *
 * Values are fixed by the DBC (VAL_ 1074 MAIN_STATUS_INDICATOR).
 */
typedef enum {
    VD_MAIN_STATUS_RTD = 0,   /* driving; the pedal works */
    VD_MAIN_STATUS_READY,     /* precharged, waiting for the RTD sequence */
    VD_MAIN_STATUS_PRCHG,     /* SDC closed, waiting on precharge */
    VD_MAIN_STATUS_N_RDY,     /* not ready and not resettable */
    VD_MAIN_STATUS_FAULT,     /* faulted or a node is missing */
    VD_MAIN_STATUS_RESET,     /* faulted, but the reset procedure will clear it */
    VD_MAIN_STATUS_COUNT
} vd_main_status_t;

/*
 * Bit positions of each shutdown circuit node within sdc_status.
 *
 * These used to be bare numbers scattered through the code (sdcStatus bits
 * 4..15), with a second table in the UI layer listing them in a different
 * order that had to be kept in sync by hand. This is now the only definition.
 */
typedef enum {
    VD_SDC_IMD = 0,       /* new DBC only */
    VD_SDC_AMS,           /* new DBC only */
    VD_SDC_BSPD,          /* new DBC only */
    VD_SDC_PDOC,          /* new DBC only */
    VD_SDC_CSB,
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

/* Bits of error_flags */
#define VD_ERR_MCU1  (1u << 0)
#define VD_ERR_MCU2  (1u << 1)
#define VD_ERR_MCU3  (1u << 2)
#define VD_ERR_MCU4  (1u << 3)
#define VD_ERR_AMS   (1u << 4)
#define VD_ERR_IMU   (1u << 5)

/*
 * Inverter faults, four kinds for each of the four inverters, packed into one
 * word. VD_INV_FAULT(inv, kind) with inv 0..3.
 */
#define VD_INV_COUNT     4u
#define VD_INV_GATE      0u   /* gate driver */
#define VD_INV_ENC       1u   /* encoder */
#define VD_INV_OTP       2u   /* over temperature */
#define VD_INV_OCP       3u   /* over current */
#define VD_INV_KINDS     4u

#define VD_INV_FAULT(inv, kind) ((uint16_t)(1u << (((inv) * VD_INV_KINDS) + (kind))))

/*
 * Bits of online_flags, from VCU_ONLINE.
 *
 * The VCU reports which nodes it can currently hear. This is not the same as
 * the dashboard hearing them: the dashboard's CAN filter admits only a handful
 * of IDs, so it has no way to observe an MCU directly and has to take the VCU's
 * word for it.
 */
#define VD_ONLINE_MCU1 (1u << 0)
#define VD_ONLINE_MCU2 (1u << 1)
#define VD_ONLINE_MCU3 (1u << 2)
#define VD_ONLINE_MCU4 (1u << 3)
#define VD_ONLINE_AMS  (1u << 4)
#define VD_ONLINE_IMU  (1u << 5)
#define VD_ONLINE_GPS  (1u << 6)

/* Bits of the value returned by VehicleData_WarnFlags() */
#define VD_WARN_TEBPPC        (1u << 0)
#define VD_WARN_CELL_OVERTEMP (1u << 1)

/*
 * Signal groups. Each maps to one CAN message and carries its own last-update
 * timestamp.
 *
 * Per-group rather than one timestamp for the whole bus: losing AMS and losing
 * the VCU are different failures and the driver needs to know which. With a
 * single bus-level timestamp, an AMS that dies while the VCU keeps transmitting
 * would leave the battery readings frozen at their last value, indistinguishable
 * from healthy data.
 */
typedef enum {
    VD_GROUP_VCU_STATE = 0,
    VD_GROUP_VCU_SDC,
    VD_GROUP_VCU_SENSOR1,
    VD_GROUP_VCU_SENSOR2,
    VD_GROUP_VCU_SYSTEM,
    VD_GROUP_VCU_ERROR,
    VD_GROUP_VCU_ONLINE,
    VD_GROUP_VCU_MCU_STATUS,
    VD_GROUP_VCU_DASH,
    VD_GROUP_VCU_GPS,
    VD_GROUP_AMS_STATUS,
    VD_GROUP_AMS_CELLS,
    VD_GROUP_COUNT
} vd_group_t;

/** How long without an update before a signal counts as stale. */
#define VD_DEFAULT_TIMEOUT_MS 500u

typedef struct {
    /* --- VCU_STATE --- */
    bool     rtd_active;
    bool     cooling_active;
    bool     tebppc_active;
    bool     ams_ready;
    bool     warmup_ready;        /* the warm-up timer has run out */
    uint8_t  drive_mode;          /* vd_drive_mode_t */

    /* --- VCU_DASH --- */
    uint8_t  main_status;         /* vd_main_status_t */

    /* --- VCU_SDC --- */
    uint16_t sdc_status;          /* bit positions per vd_sdc_node_t */

    /* --- VCU_SENSOR1 --- */
    float    bse_rear_pu;         /* rear brake pressure, 0..100 */
    float    bse_front_pu;        /* front brake pressure, 0..100 */
    float    bse_rear_bar;        /* rear brake line pressure, bar */
    float    bse_front_bar;       /* front brake line pressure, bar */

    /* --- VCU_SENSOR2 --- */
    float    steering_pct;        /* steering angle mapped to 0..100 */
    float    apps1_pu;            /* throttle pedal channel 1, 0..100 */
    float    apps2_pu;            /* throttle pedal channel 2, 0..100 */
    float    steering_deg;        /* steering angle, -180..180 */
    uint16_t car_speed_kph;

    /* --- VCU_SYSTEM_STATUS --- */
    float    glv_voltage;         /* low voltage battery */
    float    glv_soc;             /* %, low voltage battery */
    float    glv_current;

    /* --- VCU_ERROR --- */
    uint8_t  error_flags;         /* VD_ERR_* */

    /* --- VCU_ONLINE --- */
    uint8_t  online_flags;        /* VD_ONLINE_* */

    /* --- VCU_MCU_STATUS --- */
    float    motor_temp[VD_INV_COUNT];      /* C */
    float    gate_temp[VD_INV_COUNT][3];    /* C, phases U V W */
    uint16_t inv_faults;                    /* VD_INV_FAULT(inv, kind) */

    /* --- VCU_GPS --- */
    uint8_t  latitude;
    uint8_t  longitude;

    /* --- AMS_STATUS_BASIC --- */
    float    pack_voltage;        /* high voltage pack */
    float    pack_soc;            /* 0..100 */
    float    pack_current;
    float    pack_power;
    float    temp_max;
    float    temp_min;
    float    temp_delta;
    float    cell_v_min;
    float    cell_v_max;
    float    cell_v_delta;
    uint8_t  ams_state;
    bool     cell_over_temp;
    bool     cell_over_volt;
    bool     cell_under_volt;

    /* --- AMS_MODULE_1..8 --- */
    float    cell_voltage[VD_NUM_CELLS];
    float    cell_temp[VD_NUM_TSENSORS];
} vehicle_data_t;

/** The one instance. Written by can_decode.c, read by ui_bind.c. */
extern vehicle_data_t g_vehicle;

/** Clear everything and mark all groups as never received. Call at startup. */
void VehicleData_Init(void);

/** Called by can_decode.c after a successful unpack to refresh the timestamp. */
void VehicleData_MarkFresh(vd_group_t group);

/**
 * Whether the group has gone longer than timeout_ms without an update.
 * Never having received the message also counts as stale - the display should
 * treat "never arrived" the same as "arrived then stopped".
 */
bool VehicleData_IsStale(vd_group_t group, uint32_t timeout_ms);

/** Derive the warning flags (VD_WARN_*) from the current state. */
uint8_t VehicleData_WarnFlags(void);

/** Read one shutdown circuit node. */
bool VehicleData_SdcNode(vd_sdc_node_t node);

#endif /* VEHICLE_DATA_H */
