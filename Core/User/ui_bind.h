/*
 * ui_bind.h
 *
 *  Most of ui_bind.c is the get_var_xxx() functions EEZ declares in vars.h and
 *  calls on its own; only the entry point below needs declaring.
 */

#ifndef UI_BIND_H
#define UI_BIND_H

#include <stdbool.h>

/**
 * Apply the parts of the UI state that cannot be expressed as a bound variable.
 * Call once per UI update, right after ui_tick().
 */
void UIBind_ApplyDynamicStyles(void);

/**
 * Arm the one-shot 0 -> 150 -> 0 sweep of the speed readout.
 *
 * Call when the main screen is first shown. The sweep does not start until a
 * speed frame has actually arrived - starting it on a silent bus made the
 * readout animate and then drop to "---", which reads as a fault rather than
 * a self-test.
 */
void UIBind_ArmStartupSweep(void);

/**
 * Whether the splash screen is finished and the main screen should take over.
 *
 * True once the name reveal has played out and either every signal the main
 * screen shows has arrived or the bus has been given long enough to prove it
 * is not coming up. Poll it from the main loop.
 */
bool UIBind_BootComplete(void);

#endif /* UI_BIND_H */
