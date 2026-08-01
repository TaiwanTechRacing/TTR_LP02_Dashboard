/*
 * vehicle_data.c
 */

#include "vehicle_data.h"
#include "stm32h7xx_hal.h"
#include <string.h>

vehicle_data_t g_vehicle;

/* 0 代表「開機到現在都還沒收到過」。HAL_GetTick() 從 0 開始,理論上第一個
 * tick 也是 0,但那只有開機瞬間的 1ms,不影響判斷。 */
static uint32_t s_last_update[VD_GROUP_COUNT];

void VehicleData_Init(void)
{
    memset(&g_vehicle, 0, sizeof(g_vehicle));
    memset(s_last_update, 0, sizeof(s_last_update));
}

void VehicleData_MarkFresh(vd_group_t group)
{
    if (group < VD_GROUP_COUNT) {
        s_last_update[group] = HAL_GetTick();
    }
}

bool VehicleData_IsStale(vd_group_t group, uint32_t timeout_ms)
{
    if (group >= VD_GROUP_COUNT) {
        return true;
    }

    if (s_last_update[group] == 0u) {
        return true;    /* 從來沒收到過 */
    }

    return (HAL_GetTick() - s_last_update[group]) > timeout_ms;
}

uint8_t VehicleData_WarnFlags(void)
{
    uint8_t flags = 0;

    if (g_vehicle.tebppc_active) {
        flags |= VD_WARN_TEBPPC;
    }
    if (g_vehicle.cell_over_temp) {
        flags |= VD_WARN_CELL_OVERTEMP;
    }

    return flags;
}

bool VehicleData_SdcNode(vd_sdc_node_t node)
{
    if (node >= VD_SDC_COUNT) {
        return false;
    }

    return (g_vehicle.sdc_status & (1u << (uint8_t)node)) != 0u;
}
