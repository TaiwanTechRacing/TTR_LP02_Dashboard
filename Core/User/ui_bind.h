/*
 * ui_bind.h
 *
 *  Most of ui_bind.c is the get_var_xxx() functions EEZ declares in vars.h and
 *  calls on its own; only the entry point below needs declaring.
 */

#ifndef UI_BIND_H
#define UI_BIND_H

/**
 * Apply the parts of the UI state that cannot be expressed as a bound variable.
 * Call once per UI update, right after ui_tick().
 */
void UIBind_ApplyDynamicStyles(void);

/**
 * Start the one-shot 0 -> 150 -> 0 sweep of the speed readout.
 * Call when the main screen is first shown.
 */
void UIBind_StartStartupSweep(void);

#endif /* UI_BIND_H */
