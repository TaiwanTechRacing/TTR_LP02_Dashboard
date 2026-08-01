/*
 * debug_overlay.c
 *
 *  用 LVGL 內建的 sysmon 元件,不自己畫。它的 FPS 是直接從 LVGL 的
 *  refresh 流程統計出來的,比自己在主迴圈數圈數準確 —— 主迴圈跑得快
 *  不等於畫面真的有更新。
 */

#include "debug_overlay.h"
#include "lvgl.h"

#if LV_USE_SYSMON && LV_USE_PERF_MONITOR

static bool s_visible;

void DebugOverlay_Init(void)
{
    /*
     * LVGL 在建立 display 的時候就會把效能標籤顯示出來,所以這裡要主動關掉。
     * 預設關閉是刻意的:比賽時車手需要的是乾淨的儀表畫面。
     */
    lv_sysmon_hide_performance(lv_display_get_default());
    s_visible = false;
}

void DebugOverlay_Toggle(void)
{
    lv_display_t *disp = lv_display_get_default();

    if (s_visible) {
        lv_sysmon_hide_performance(disp);
    }
    else {
        lv_sysmon_show_performance(disp);
    }

    s_visible = !s_visible;
}

bool DebugOverlay_IsVisible(void)
{
    return s_visible;
}

#else /* 疊層被編譯掉 */

void DebugOverlay_Init(void) { }
void DebugOverlay_Toggle(void) { }
bool DebugOverlay_IsVisible(void) { return false; }

#endif
