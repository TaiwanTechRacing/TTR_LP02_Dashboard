/*
 * cell_map.c
 */

#include "cell_map.h"

#include "screens.h"
#include "vehicle_data.h"

#include "lvgl.h"

#include <string.h>

/*
 * Grid geometry. One row per segment, one column per cell within it, which is
 * how the AMS packs them - so a whole row going dark means a segment stopped
 * reporting rather than fourteen cells failing at once.
 *
 * The EEZ canvas must be set to match:  cellMapCanvas  392 x 128
 */
#define CELL_W 28
#define CELL_H 16

#define MAP_W (VD_CELLS_PER_SEG * CELL_W)     /* 14 x 28 = 392 */
#define MAP_H (VD_NUM_SEGMENTS * CELL_H)      /* 8 x 16 = 128 */

static uint16_t s_buf[MAP_W * MAP_H] __attribute__((aligned(32)));

static bool     s_active;
static uint32_t s_last_draw;

#define REDRAW_MS 100u

/*
 * Below this the pack is even enough that the differences are not worth
 * looking at, and the whole map stays green.
 *
 * Without it a healthy pack sitting within a few millivolts would be painted
 * as a full red-to-green spread, because the scale is relative - which would
 * read as a problem every single time the page was opened.
 */
#define SPREAD_FLOOR_V 0.030f

#define COLOUR_NO_DATA 0x2104u   /* faint grey */

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(((r & 0xF8u) << 8) | ((g & 0xFCu) << 3) | (b >> 3));
}

/** Red at the bottom of the pack, yellow in the middle, green at the top. */
static uint16_t heat_colour(float t)
{
    if (t < 0.0f) {
        t = 0.0f;
    }
    if (t > 1.0f) {
        t = 1.0f;
    }

    if (t < 0.5f) {
        const float k = t * 2.0f;
        return rgb565(255u, (uint8_t)(k * 255.0f), 0u);
    }

    const float k = (t - 0.5f) * 2.0f;
    return rgb565((uint8_t)((1.0f - k) * 255.0f), 255u, 0u);
}

static void fill_rect(int x, int y, int w, int h, uint16_t colour)
{
    for (int r = 0; r < h; r++) {
        uint16_t *row = s_buf + ((y + r) * MAP_W) + x;
        for (int c = 0; c < w; c++) {
            row[c] = colour;
        }
    }
}

static void draw(void)
{
    /*
     * Scale from the array itself rather than the AMS's own min and max. The
     * two come from different messages, and a map coloured by one while the
     * numbers underneath report the other would disagree on screen for no
     * visible reason.
     */
    float lo = 0.0f, hi = 0.0f;
    bool any = false;

    for (uint16_t i = 0; i < VD_NUM_CELLS; i++) {
        const float v = g_vehicle.cell_voltage[i];
        if (v <= 0.0f) {
            continue;       /* never received */
        }
        if (!any) {
            lo = hi = v;
            any = true;
        }
        else if (v < lo) {
            lo = v;
        }
        else if (v > hi) {
            hi = v;
        }
    }

    const float spread = hi - lo;
    const bool  stale = VehicleData_IsStale(VD_GROUP_AMS_CELLS,
                                            VD_DEFAULT_TIMEOUT_MS);

    for (uint8_t seg = 0; seg < VD_NUM_SEGMENTS; seg++) {
        for (uint8_t i = 0; i < VD_CELLS_PER_SEG; i++) {
            const uint16_t index = (uint16_t)((seg * VD_CELLS_PER_SEG) + i);
            const float v = g_vehicle.cell_voltage[index];

            uint16_t colour;
            if (!any || stale || v <= 0.0f) {
                colour = COLOUR_NO_DATA;
            }
            else if (spread < SPREAD_FLOOR_V) {
                colour = heat_colour(1.0f);
            }
            else {
                colour = heat_colour((v - lo) / spread);
            }

            const int x = i * CELL_W;
            const int y = seg * CELL_H;

            fill_rect(x, y, CELL_W, CELL_H, colour);

            /* A one pixel gap on two sides, so a block of similar cells still
             * reads as individual cells rather than a wash of colour. */
            fill_rect(x, y, CELL_W, 1, 0x0000u);
            fill_rect(x, y, 1, CELL_H, 0x0000u);
        }
    }
}

void CellMap_Init(void)
{
    lv_canvas_set_buffer(objects.cell_map_canvas, s_buf,
                         MAP_W, MAP_H, LV_COLOR_FORMAT_RGB565);
    draw();
    lv_obj_invalidate(objects.cell_map_canvas);
}

void CellMap_SetActive(bool active)
{
    s_active = active;

    if (active) {
        draw();
        lv_obj_invalidate(objects.cell_map_canvas);
        s_last_draw = 0;
    }
}

void CellMap_Service(uint32_t now_ms)
{
    if (!s_active) {
        return;
    }

    /*
     * Ten times a second. The cells arrive far faster than that, but nobody
     * reads a 112 square heat map at 60 Hz and redrawing it costs 50k pixels
     * each time.
     */
    if (s_last_draw != 0u && (now_ms - s_last_draw) < REDRAW_MS) {
        return;
    }

    s_last_draw = now_ms;
    draw();
    lv_obj_invalidate(objects.cell_map_canvas);
}
