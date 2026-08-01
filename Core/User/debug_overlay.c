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
     * hide it explicitly. Off by default is deliberate: during a race the
     * driver wants a clean instrument display.
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

#else /* overlay compiled out */

void DebugOverlay_Init(void) { }
void DebugOverlay_Toggle(void) { }
bool DebugOverlay_IsVisible(void) { return false; }

#endif
