/*
 * debug_overlay.h
 *
 *  Debug overlay: frame rate and LVGL CPU share, drawn top-left.
 *
 *  Toggled by holding both buttons for about one second. Off at boot - nothing
 *  should sit on top of the instrument display during a race.
 *
 *  The overlay lives on LVGL's sysmon layer rather than on a page, so it stays
 *  visible across screen changes with no per-page handling.
 *
 *  To compile it out entirely for a race build, set LV_USE_SYSMON back to 0 in
 *  Drivers/lv_conf.h. These functions then become empty and callers need no
 *  changes.
 */

#ifndef DEBUG_OVERLAY_H
#define DEBUG_OVERLAY_H

#include <stdbool.h>

/**
 * Initialise and make sure the overlay starts hidden.
 * Must be called after BSP_Display_Init().
 */
void DebugOverlay_Init(void);

/** Toggle visibility. */
void DebugOverlay_Toggle(void);

/** Whether the overlay is currently shown. */
bool DebugOverlay_IsVisible(void);

#endif /* DEBUG_OVERLAY_H */
