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

#include <stdint.h>

/* Mirrors WELCOME_HOLD_MS in Core/Src/main.c */
#define SIM_WELCOME_HOLD_MS 3000u

/* Mirrors UI_UPDATE_PERIOD_MS in Core/Src/main.c */
#define SIM_UI_PERIOD_MS 25u

/** Reset the sequencer. Call after ui_init(). */
void SimApp_Reset(void);

/** Advance one iteration of the firmware's main loop, minus the hardware. */
void SimApp_Step(uint32_t now);

#endif /* SIM_APP_H */
