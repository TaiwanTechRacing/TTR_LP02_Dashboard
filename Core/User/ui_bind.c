/*
 * ui_bind.c
 *
 *  實作 EEZ Studio 在 vars.h 宣告的 get_var_xxx()。
 *
 *  EEZ 的 LVGL 專案在關閉 Flow 的模式下,只產生宣告、不產生實作 —— 值從哪來
 *  由我們決定。這就是整個 UI 的接縫:
 *
 *      vehicle_data  →  [ui_bind.c]  →  EEZ 產生的 screens.c
 *
 *  在 EEZ 裡搬動元件、改樣式、換字型都不會動到這個檔案。只有「新增或改名
 *  變數」才需要來這裡加對應的 getter。
 *
 *  重構前的作法是一個寫死幾百個 objects.xxx 名稱的 updatescreen() 大 switch,
 *  版面一重新產生就整片編不過。
 *
 *  ── 訊號過期的處理 ──
 *  每個 getter 都會先問 VehicleData_IsStale()。CAN 斷掉時畫面顯示 "---" 而不是
 *  凍結在最後一筆數值 —— 車手看到定住的 600V 會以為一切正常,這是安全問題。
 */

#include "vehicle_data.h"
#include <stdio.h>

/* 訊號過期時顯示的字串 */
#define STALE_TEXT "---"

/*
 * 每個 getter 各自持有回傳用的緩衝區。
 *
 * 不能用區域變數(回傳後就失效),也不共用一個緩衝區 —— LVGL 在同一次重繪
 * 裡會連續呼叫多個 getter,共用的話後面的會蓋掉前面的。
 */
static char s_speed_buf[8];
static char s_soc_buf[12];
static char s_lv_buf[16];
static char s_hv_buf[16];

/**
 * 車速。整數顯示,不補零 —— 前面補零在大字級時會佔掉版面寬度。
 */
const char *get_var_speed(void)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_SENSOR2, VD_DEFAULT_TIMEOUT_MS)) {
        return STALE_TEXT;
    }

    snprintf(s_speed_buf, sizeof(s_speed_buf), "%u", (unsigned)g_vehicle.car_speed_kph);
    return s_speed_buf;
}

/**
 * RTD 狀態。
 *
 * 這裡回傳的是要顯示的字,不是狀態碼 —— 顏色變化請在 EEZ 裡用樣式處理,
 * 韌體只負責內容。
 */
const char *get_var_ready(void)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_STATE, VD_DEFAULT_TIMEOUT_MS)) {
        return STALE_TEXT;
    }

    return g_vehicle.rtd_active ? "READY" : "NOT READY";
}

/** 高壓電池 SOC 百分比。 */
const char *get_var_label_soc_value(void)
{
    if (VehicleData_IsStale(VD_GROUP_AMS_STATUS, VD_DEFAULT_TIMEOUT_MS)) {
        return STALE_TEXT;
    }

    snprintf(s_soc_buf, sizeof(s_soc_buf), "%.0f%%", (double)g_vehicle.pack_soc);
    return s_soc_buf;
}

/** 低壓電池電壓。 */
const char *get_var_label_lv_value(void)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_SYSTEM, VD_DEFAULT_TIMEOUT_MS)) {
        return "LV:" STALE_TEXT;
    }

    snprintf(s_lv_buf, sizeof(s_lv_buf), "LV:%.1fV", (double)g_vehicle.glv_voltage);
    return s_lv_buf;
}

/** 高壓電池組電壓。 */
const char *get_var_label_hv_value(void)
{
    if (VehicleData_IsStale(VD_GROUP_AMS_STATUS, VD_DEFAULT_TIMEOUT_MS)) {
        return "HV:" STALE_TEXT;
    }

    snprintf(s_hv_buf, sizeof(s_hv_buf), "HV:%.0fV", (double)g_vehicle.pack_voltage);
    return s_hv_buf;
}

/**
 * SOC 長條圖的數值。
 *
 * EEZ 那邊 bar 的 min/max 目前設成 18/30(低壓電池的電壓範圍),但上面的標題
 * 寫 SOC。這裡先照變數名 lv 回傳低壓電壓 —— 如果那根 bar 其實是要顯示高壓
 * SOC,把 EEZ 的 min/max 改成 0/100,這裡改回傳 pack_soc 即可。
 *
 * 過期時回傳 min 值讓長條歸零,比停在最後一格容易看出異常。
 */
int32_t get_var_lv(void)
{
    if (VehicleData_IsStale(VD_GROUP_VCU_SYSTEM, VD_DEFAULT_TIMEOUT_MS)) {
        return 0;
    }

    return (int32_t)(g_vehicle.glv_voltage + 0.5f);
}
