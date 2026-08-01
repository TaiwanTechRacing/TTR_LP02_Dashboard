/*
 * sim/shim/stm32h7xx_hal.h
 *
 *  Minimal stand-in for the STM32 HAL header, used only by the PC simulator.
 *
 *  ui_bind.c and vehicle_data.c include "stm32h7xx_hal.h" for HAL_GetTick().
 *  Rather than littering those files with #ifdefs, the simulator puts this
 *  directory first on the include path and supplies the one function they
 *  actually use. The firmware sources therefore compile for the host without
 *  a single change, which is the whole point - the simulator has to run the
 *  same code as the car or it is not telling you anything.
 */

#ifndef SIM_STM32H7XX_HAL_H
#define SIM_STM32H7XX_HAL_H

#include <stdint.h>

/** Milliseconds since the simulator started. */
uint32_t HAL_GetTick(void);

#endif /* SIM_STM32H7XX_HAL_H */
