/*
 * sim_qspi.h
 *
 *  Loads a QSPI image built by tools/make_qspi_image.py into host memory so
 *  the simulator can render the animations that live on the flash part.
 */

#ifndef SIM_QSPI_H
#define SIM_QSPI_H

#include <stdbool.h>

/**
 * Load an image, or arrange for the simulator to behave like a blank part.
 *
 * @param path  file to load; NULL uses the TTR_QSPI_IMAGE environment variable
 *              and falls back to "qspi.bin" in the working directory.
 *
 * Returns false when nothing was loaded. That is not fatal - the firmware is
 * required to boot with a blank part, and the simulator reproduces that.
 */
bool SimQspi_Load(const char *path);

#endif /* SIM_QSPI_H */
