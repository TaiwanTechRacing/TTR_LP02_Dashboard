/*
 * sim_app.h
 *
 *  The parts of main.c's loop that are not hardware: the splash-to-main
 *  hand-off and the periodic UI refresh.
 *
 *  Duplicating this in each simulator entry point would let it drift from the
 *  firmware, and a simulator that drifts stops being evidence. Keeping the
 *  constants and the sequence in one place at least confines the duplication
 *  to a single file that sits next to main.c in review.
 */

#ifndef SIM_APP_H
#define SIM_APP_H

#include <stdbool.h>
#include <stdint.h>

/* Mirrors UI_UPDATE_PERIOD_MS in Core/Src/main.c */
#define SIM_UI_PERIOD_MS 25u

/** Reset the sequencer. Call after ui_init(). */
void SimApp_Reset(void);

/** Advance one iteration of the firmware's main loop, minus the hardware. */
void SimApp_Step(uint32_t now);

/**
 * Choose which page the welcome screen hands over to, indexed the same as the
 * page list in Core/User/nav.c. Defaults to 1 (main), matching the firmware.
 *
 * A shortcut for the headless screenshot tool, which has nowhere to press a
 * button. Interactively, use SimApp_SetButtons() instead.
 */
void SimApp_ShowPage(uint8_t index);

/**
 * Current state of the two dashboard buttons, true while held.
 *
 * Fed to the same Nav_Scan() the firmware calls, so debounce, the wrap at both
 * ends of the page list and the both-held overlay gesture behave exactly as
 * they do in the car - including needing a real hold to trigger.
 */
void SimApp_SetButtons(bool button1_pressed, bool button2_pressed);

#endif /* SIM_APP_H */
