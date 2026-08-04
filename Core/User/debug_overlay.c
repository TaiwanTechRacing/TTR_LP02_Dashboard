/*
 * debug_overlay.c
 *
 *  Uses LVGL's built-in sysmon component rather than drawing our own. Its FPS
 *  comes from LVGL's refresh pipeline, which is more honest than counting main
 *  loop iterations: a fast loop does not mean the screen actually updated.
 */

#include "debug_overlay.h"
#include "lvgl.h"

#if LV_USE_SYSMON && LV_USE_PERF_MONITOR

static bool s_visible;

void DebugOverlay_Init(void)
{
    /*
     * LVGL shows the performance label as soon as the display is created, so
     * the hidden case has to be asked for explicitly - it is not the default.
     */
    lv_display_t *disp = lv_display_get_default();

#if DEBUG_OVERLAY_VISIBLE
    lv_sysmon_show_performance(disp);
    s_visible = true;
#else
    lv_sysmon_hide_performance(disp);
    s_visible = false;
#endif
}

bool DebugOverlay_IsVisible(void)
{
    return s_visible;
}

#else /* overlay compiled out */

void DebugOverlay_Init(void) { }
bool DebugOverlay_IsVisible(void) { return false; }

#endif
