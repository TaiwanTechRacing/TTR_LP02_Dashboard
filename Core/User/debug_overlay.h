/*
 * debug_overlay.h
 *
 *  Debug overlay: frame rate and LVGL CPU share, drawn top-left.
 *
 *  Compile-time only: set DEBUG_OVERLAY_VISIBLE to 1 to have it on from boot.
 *  There is no gesture for it.
 *
 *  It used to be a one second hold of both buttons, which shared its gesture
 *  with getting into the games and so had to be undone again when that armed.
 *  Two features on one hold, one of them only useful on a bench, is not worth
 *  the state it takes to keep them apart - and a driver who reaches it by
 *  accident gets a box of numbers over the instruments mid-session.
 *
 *  The overlay lives on LVGL's sysmon layer rather than on a page, so it stays
 *  visible across screen changes with no per-page handling.
 *
 *  To compile it out entirely, set LV_USE_SYSMON back to 0 in Drivers/lv_conf.h.
 *  These functions then become empty and callers need no changes.
 */

#ifndef DEBUG_OVERLAY_H
#define DEBUG_OVERLAY_H

#include <stdbool.h>

/*
 * Off by default. During a race nothing should sit on top of the instrument
 * display, so turning it on is a deliberate act at build time - either here or
 * with -DDEBUG_OVERLAY_VISIBLE=1.
 */
#ifndef DEBUG_OVERLAY_VISIBLE
#define DEBUG_OVERLAY_VISIBLE 0
#endif

/**
 * Show or hide the overlay according to DEBUG_OVERLAY_VISIBLE.
 * Must be called after BSP_Display_Init().
 */
void DebugOverlay_Init(void);

/** Whether the overlay is currently shown. */
bool DebugOverlay_IsVisible(void);

#endif /* DEBUG_OVERLAY_H */
