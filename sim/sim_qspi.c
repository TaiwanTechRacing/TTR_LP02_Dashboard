/*
 * sim_qspi.c
 */

#include "sim_qspi.h"
#include "bsp_qspi.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Stands in for Core/User/bsp_qspi.c, which is all register work and cannot be
 * built for the host. Only the two calls gif_pages.c makes are provided; the
 * erase and program paths have no meaning here, so anything reaching for them
 * fails at link time instead of silently doing nothing.
 */
static const uint8_t *g_sim_qspi_image;
static uint32_t       g_sim_qspi_size;

const uint8_t *BSP_QSPI_GetMappedBase(void)
{
    return g_sim_qspi_image;
}

uint32_t BSP_QSPI_GetFlashSize(void)
{
    return g_sim_qspi_size;
}

bool SimQspi_Load(const char *path)
{
    g_sim_qspi_image = NULL;
    g_sim_qspi_size = 0;

    if (path == NULL) {
        path = getenv("TTR_QSPI_IMAGE");
    }
    if (path == NULL) {
        path = "qspi.bin";
    }

    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, "qspi: no image at %s - simulating a blank part\n", path);
        return false;
    }

    fseek(f, 0, SEEK_END);
    const long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        fprintf(stderr, "qspi: %s is empty\n", path);
        return false;
    }

    uint8_t *buf = malloc((size_t)size);
    if (buf == NULL) {
        fclose(f);
        fprintf(stderr, "qspi: out of memory for %ld bytes\n", size);
        return false;
    }

    const size_t got = fread(buf, 1, (size_t)size, f);
    fclose(f);

    if (got != (size_t)size) {
        free(buf);
        fprintf(stderr, "qspi: short read on %s\n", path);
        return false;
    }

    g_sim_qspi_image = buf;
    g_sim_qspi_size = (uint32_t)size;

    fprintf(stderr, "qspi: loaded %s, %lu bytes\n", path, (unsigned long)size);
    return true;
}
