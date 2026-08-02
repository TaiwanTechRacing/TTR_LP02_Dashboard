/*
 * sim_shot.c
 *
 *  Headless screenshot mode for the simulator.
 *
 *  Renders the UI into a plain memory buffer - no window, no Win32 backend -
 *  and writes it out as a BMP. Useful for looking at the layout over a remote
 *  session, for attaching a picture to a discussion, and for catching layout
 *  regressions in CI without anyone having to look at a screen.
 *
 *  Usage:
 *      dashboard_shot <out.bmp> [scene_ms] [page]
 *
 *  scene_ms fast-forwards the synthetic data generator, so a specific moment
 *  can be captured: the startup sweep, a mid-range reading, or the stale
 *  window where everything falls back to "---".
 *
 *  page indexes screens[] in Core/Src/main.c and defaults to 1 (main). 5, 6
 *  and 7 are the debug pages, which is where the QSPI animations appear.
 */

#include "lvgl.h"
#include "ui.h"
#include "ui_bind.h"
#include "vehicle_data.h"
#include "sim_data.h"
#include "sim_app.h"
#include "nav.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHOT_WIDTH   480
#define SHOT_HEIGHT  272

/*
 * The generator in sim_main.c is driven by HAL_GetTick(). Here the clock is
 * ours to set, which is what makes a specific frame reproducible.
 */
static uint32_t s_virtual_tick;

uint32_t HAL_GetTick(void)
{
    return s_virtual_tick;
}

static uint16_t s_framebuffer[SHOT_WIDTH * SHOT_HEIGHT];

static void shot_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
}

/*
 * 24-bit BMP. Rows are written bottom-up because that is what the format
 * expects, and RGB565 is expanded with bit replication so mid greys do not
 * come out slightly dark.
 */
static bool write_bmp(const char *path, const uint16_t *pixels)
{
    const uint32_t row_bytes = ((SHOT_WIDTH * 3u) + 3u) & ~3u;
    const uint32_t image_size = row_bytes * SHOT_HEIGHT;
    const uint32_t file_size = 54u + image_size;

    FILE *f = fopen(path, "wb");
    if (f == NULL) {
        return false;
    }

    uint8_t header[54] = {0};
    header[0] = 'B'; header[1] = 'M';
    memcpy(&header[2], &file_size, 4);
    const uint32_t offset = 54u;   memcpy(&header[10], &offset, 4);
    const uint32_t dib = 40u;      memcpy(&header[14], &dib, 4);
    const int32_t w = SHOT_WIDTH;  memcpy(&header[18], &w, 4);
    const int32_t h = SHOT_HEIGHT; memcpy(&header[22], &h, 4);
    const uint16_t planes = 1u;    memcpy(&header[26], &planes, 2);
    const uint16_t bpp = 24u;      memcpy(&header[28], &bpp, 2);
    memcpy(&header[34], &image_size, 4);
    fwrite(header, 1, sizeof(header), f);

    uint8_t *row = calloc(1, row_bytes);
    if (row == NULL) {
        fclose(f);
        return false;
    }

    for (int y = SHOT_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < SHOT_WIDTH; x++) {
            const uint16_t p = pixels[y * SHOT_WIDTH + x];
            const uint8_t r5 = (uint8_t)((p >> 11) & 0x1F);
            const uint8_t g6 = (uint8_t)((p >> 5)  & 0x3F);
            const uint8_t b5 = (uint8_t)(p         & 0x1F);
            row[x * 3 + 0] = (uint8_t)((b5 << 3) | (b5 >> 2));   /* BMP is BGR */
            row[x * 3 + 1] = (uint8_t)((g6 << 2) | (g6 >> 4));
            row[x * 3 + 2] = (uint8_t)((r5 << 3) | (r5 >> 2));
        }
        fwrite(row, 1, row_bytes, f);
    }

    free(row);
    fclose(f);
    return true;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <out.bmp> [scene_ms]\n", argv[0]);
        return 2;
    }

    const uint32_t scene_ms = (argc >= 3) ? (uint32_t)strtoul(argv[2], NULL, 10) : 5000u;
    const uint8_t  page     = (argc >= 4) ? (uint8_t)strtoul(argv[3], NULL, 10) : 1u;

    s_virtual_tick = 0;

    lv_init();
    lv_tick_set_cb(HAL_GetTick);

    lv_display_t *disp = lv_display_create(SHOT_WIDTH, SHOT_HEIGHT);
    lv_display_set_buffers(disp, s_framebuffer, NULL,
                           sizeof(s_framebuffer),
                           LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_flush_cb(disp, shot_flush_cb);

    VehicleData_Init();
    ui_init();
    SimApp_Reset();
    SimApp_ShowPage(page);

    /*
     * Step the virtual clock to the requested moment, letting LVGL run at each
     * step. Stepping rather than jumping matters: loadScreen() uses a fade
     * animation and the startup sweep is time based, so a single jump would
     * land on a half-finished transition.
     *
     * The step matches the button scan period so nav.c sees the same number of
     * samples per press that it would in the car. A coarser step silently
     * doubles the debounce and the hold gesture.
     */
    for (uint32_t t = 0; t <= scene_ms; t += NAV_SCAN_PERIOD_MS) {
        s_virtual_tick = t;
        SimApp_Step(t);
    }

    /* One last refresh so the final state is definitely in the buffer. */
    lv_refr_now(disp);

    if (!write_bmp(argv[1], s_framebuffer)) {
        fprintf(stderr, "failed to write %s\n", argv[1]);
        return 1;
    }

    printf("wrote %s at t=%u ms\n", argv[1], (unsigned)scene_ms);
    return 0;
}
