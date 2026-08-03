/*
 * nav.h
 *
 *  Page navigation and the two dashboard buttons.
 *
 *  Split out of main.c so the PC simulator runs the same code rather than an
 *  imitation of it. Debounce timing, the wrap-around at both ends of the page
 *  list and the both-held gesture are exactly what the car does, which is what
 *  makes trying a page order on the desktop worth anything.
 *
 *  Nav_Scan() takes the two button states as plain booleans instead of reading
 *  GPIO itself. That is the whole seam: main.c passes HAL_GPIO_ReadPin()
 *  results, the simulator passes key states.
 */

#ifndef NAV_H
#define NAV_H

#include <stdbool.h>
#include <stdint.h>

#include "screens.h"

/** Button sampling period. Nav_Scan() assumes it is called this often. */
#define NAV_SCAN_PERIOD_MS 5U

/**
 * Whether the three animation pages are built in.
 *
 * They are for looking at on a bench, not for a driver paging past them on the
 * way to the battery screen, so whether they exist is a build decision rather
 * than something to navigate around. Set to 0 and they leave the page cycle
 * entirely - and the animations stop being created, since there is nowhere to
 * put them.
 */
#ifndef NAV_DEBUG_PAGES
#define NAV_DEBUG_PAGES 1
#endif

/**
 * Set up the page list and show the splash screen. Call after ui_init().
 */
void Nav_Init(void);

/**
 * Show a page by its index in the cycle.
 *
 * 0 is the splash screen, shown at boot and excluded from the button cycle.
 * 1 is main, which is where the splash hands over. Out-of-range values are
 * ignored.
 */
void Nav_ShowPage(uint8_t index);

/** Index of the page currently on screen. */
uint8_t Nav_CurrentPage(void);

/**
 * Where a screen sits in the page cycle, or -1 if it is not built in.
 *
 * Anything that needs to know a page's position should ask rather than count
 * the list itself. The animation pages have moved three times as pages were
 * added ahead of them, and each time a hard-coded index went quietly wrong.
 */
int8_t Nav_PageIndexOf(enum ScreensEnum id);

/**
 * Sample the buttons and act on them. Call every NAV_SCAN_PERIOD_MS.
 *
 * @param now_ms           milliseconds, for the gestures that are held
 * @param button1_pressed  page back
 * @param button2_pressed  page forward
 *
 * Debounce counts consecutive samples, because that is what filters contact
 * bounce. Everything longer is measured in milliseconds instead: the caller
 * cannot guarantee the sampling rate - a busy frame delays the main loop on the
 * car exactly as rendering does in the simulator - and a one second hold that
 * quietly becomes three because the loop got slower is not a hold anyone can
 * perform.
 */
void Nav_Scan(uint32_t now_ms, bool button1_pressed, bool button2_pressed);

#endif /* NAV_H */
