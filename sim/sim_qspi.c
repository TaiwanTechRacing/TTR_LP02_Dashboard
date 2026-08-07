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

/*
 * Where to look when no path is given.
 *
 * The bare name only works if the simulator happens to be run from the
 * directory holding the image, which is easy to get wrong and looks exactly
 * like a blank flash part when it happens. SIM_REPO_ROOT is baked in by CMake
 * so the two places the image is actually built into are found from anywhere.
 */
static const char *const s_default_paths[] = {
    "qspi.bin",
    SIM_REPO_ROOT "/qspi.bin",
    SIM_REPO_ROOT "/dashboard_layout/gif/qspi.bin",
};

#define SIM_DEFAULT_PATH_COUNT \
    (sizeof(s_default_paths) / sizeof(s_default_paths[0]))

static FILE *open_image(const char *path, const char **opened)
{
    if (path == NULL) {
        path = getenv("TTR_QSPI_IMAGE");
    }

    if (path != NULL) {
        FILE *f = fopen(path, "rb");
        if (f == NULL) {
            fprintf(stderr, "qspi: cannot open %s\n", path);
        }
        *opened = path;
        return f;
    }

    for (size_t i = 0; i < SIM_DEFAULT_PATH_COUNT; i++) {
        FILE *f = fopen(s_default_paths[i], "rb");
        if (f != NULL) {
            *opened = s_default_paths[i];
            return f;
        }
    }

    fprintf(stderr, "qspi: no image found - simulating a blank part.\n"
                    "      Looked in:\n");
    for (size_t i = 0; i < SIM_DEFAULT_PATH_COUNT; i++) {
        fprintf(stderr, "        %s\n", s_default_paths[i]);
    }
    fprintf(stderr, "      Build one with tools/make_qspi_image.py, or point\n"
                    "      TTR_QSPI_IMAGE at an existing image.\n");

    *opened = NULL;
    return NULL;
}

bool SimQspi_Load(const char *path)
{
    g_sim_qspi_image = NULL;
    g_sim_qspi_size = 0;

    const char *opened = NULL;
    FILE *f = open_image(path, &opened);
    if (f == NULL) {
        return false;
    }

    path = opened;

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
