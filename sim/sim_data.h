/*
 * sim_data.h
 *
 *  Synthetic vehicle data, shared by the windowed simulator and the headless
 *  screenshot tool so both show the same scene at the same timestamp.
 */

#ifndef SIM_DATA_H
#define SIM_DATA_H

#include <stdint.h>

/* One full pass of the generator, including the stale window at the end. */
#define SIM_SCENE_PERIOD_MS 12000u

/**
 * Populate g_vehicle for the given moment on the simulated clock.
 * Deterministic: the same "now" always produces the same scene.
 */
void SimData_Feed(uint32_t now);

#endif /* SIM_DATA_H */
