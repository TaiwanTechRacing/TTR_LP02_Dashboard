/*
 * gif_pages.c
 */

#include "gif_pages.h"
#include "bsp_qspi.h"
#include "screens.h"
#include "nav.h"

#include "lvgl.h"

#include <string.h>

#define GIF_PAGES_MAX 3u

/* Must match tools/make_qspi_image.py */
#define QSPI_IMAGE_MAGIC  0x51525454u   /* 'TTRQ' little-endian */

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t count;
    uint32_t reserved;
} qspi_image_header_t;

typedef struct {
    uint32_t offset;
    uint32_t size;
    uint32_t reserved;
} qspi_image_entry_t;

/*
 * LVGL wants an image descriptor rather than a raw pointer. The data stays in
 * QSPI - only these few bytes of descriptor live in RAM, and .data points
 * straight into the memory-mapped window.
 */
static lv_image_dsc_t s_dsc[GIF_PAGES_MAX];
static lv_obj_t      *s_gif[GIF_PAGES_MAX];
static uint8_t        s_count;

/** The container each animation belongs in, indexed the same as the entries. */
static lv_obj_t *container_for(uint8_t index)
{
    switch (index) {
    case 0: return objects.gif;
    case 1: return objects.gif_1;
    case 2: return objects.gif_2;
    default: return NULL;
    }
}

/**
 * Which page each animation lives on, asked rather than assumed.
 *
 * This used to be a constant offset into nav.c's page list, and it went wrong
 * three times as pages were added ahead of the debug ones - quietly, because a
 * wrong index pauses the animation being looked at and runs two nobody can
 * see. Returns -1 when the debug pages are compiled out.
 */
static int8_t screen_for(uint8_t index)
{
    switch (index) {
    case 0: return Nav_PageIndexOf(SCREEN_ID_DEBUG1);
    case 1: return Nav_PageIndexOf(SCREEN_ID_DEBUG2);
    case 2: return Nav_PageIndexOf(SCREEN_ID_DEBUG3);
    default: return -1;
    }
}

/*
 * Showcase: one widget in the corner of the game page that plays every
 * animation in turn.
 *
 * Reuses the descriptors built above rather than opening the files again, so
 * the only extra cost is one decoder's frame buffer.
 */
/* The slot EEZ lays out on the game page. */
#define SHOWCASE_W  136
#define SHOWCASE_H  102
#define SHOWCASE_HOLD_MS 5000u

static lv_obj_t *s_showcase;
static uint8_t   s_showcase_index;
static uint32_t  s_showcase_since;
static bool      s_showcase_active;

static void showcase_show(uint8_t index)
{
    if (s_showcase == NULL || index >= s_count) {
        return;
    }

    s_showcase_index = index;
    lv_gif_set_color_format(s_showcase, LV_COLOR_FORMAT_RGB565);
    lv_gif_set_src(s_showcase, &s_dsc[index]);

    /*
     * Deliberately not scaled to fit.
     *
     * lv_image_set_scale() renders these as a sparse scatter of stray pixels -
     * the software transform produces garbage for a GIF's frame buffer, and it
     * is not the ARGB8888 support flag this time, since enabling that changes
     * nothing. Unscaled the same animation draws perfectly and the slot simply
     * crops it, which for a 200 px cat in a 136x102 window reads as a close-up
     * rather than a fault. If you try scaling again, look at the slot before
     * believing it worked.
     */
    lv_obj_center(s_showcase);
}

static void showcase_create(void)
{
    if (s_count == 0u || objects.place_gif == NULL) {
        return;     /* nothing loaded, or the page has no slot for it */
    }

    s_showcase = lv_gif_create(objects.place_gif);
    if (s_showcase == NULL) {
        return;
    }

    showcase_show(0u);
}

void GifPages_SetShowcaseActive(bool active)
{
    s_showcase_active = active;

    if (s_showcase == NULL) {
        return;
    }

    if (active) {
        s_showcase_since = 0;   /* ShowcaseService() restarts the clock */
        lv_gif_resume(s_showcase);
    }
    else {
        lv_gif_pause(s_showcase);
    }
}

void GifPages_ShowcaseService(uint32_t now_ms)
{
    if (!s_showcase_active || s_showcase == NULL || s_count <= 1u) {
        return;
    }

    if (s_showcase_since == 0u) {
        s_showcase_since = now_ms;
        return;
    }

    if ((now_ms - s_showcase_since) < SHOWCASE_HOLD_MS) {
        return;
    }

    s_showcase_since = now_ms;
    showcase_show((uint8_t)((s_showcase_index + 1u) % s_count));
    lv_gif_resume(s_showcase);
}

void GifPages_Init(void)
{
    s_count = 0;

    if (BSP_QSPI_GetFlashSize() == 0u) {
        return;     /* the part never answered; nothing to read */
    }

    const uint8_t *base = BSP_QSPI_GetMappedBase();
    if (base == NULL) {
        return;
    }

    const qspi_image_header_t *header = (const qspi_image_header_t *)base;

    /*
     * A blank device reads 0xFF everywhere, so without this check the decoder
     * would be handed 8 MB of 0xFF and asked to make an animation of it.
     */
    if (header->magic != QSPI_IMAGE_MAGIC || header->count == 0u) {
        return;
    }

    const qspi_image_entry_t *entries =
        (const qspi_image_entry_t *)(base + sizeof(qspi_image_header_t));

    uint32_t count = header->count;
    if (count > GIF_PAGES_MAX) {
        count = GIF_PAGES_MAX;
    }

    for (uint32_t i = 0u; i < count; i++) {
        lv_obj_t *parent = container_for((uint8_t)i);
        if (parent == NULL) {
            continue;
        }

        const uint32_t offset = entries[i].offset;
        const uint32_t size = entries[i].size;

        if (size == 0u || (offset + size) > BSP_QSPI_GetFlashSize()) {
            continue;   /* entry points outside the device - ignore it */
        }

        /*
         * LV_COLOR_FORMAT_RAW tells LVGL the bytes are an encoded file rather
         * than pixels, which is what makes the GIF decoder take them.
         */
        s_dsc[i].header.magic = LV_IMAGE_HEADER_MAGIC;
        s_dsc[i].header.cf = LV_COLOR_FORMAT_RAW;
        s_dsc[i].header.w = 0;
        s_dsc[i].header.h = 0;
        s_dsc[i].data_size = size;
        s_dsc[i].data = base + offset;

        lv_obj_t *gif = lv_gif_create(parent);
        if (gif == NULL) {
            continue;
        }

        /*
         * Decode straight to RGB565. This is not an optimisation, it is what
         * makes the animation appear at all: the decoder defaults to
         * ARGB8888, and lv_conf.h leaves LV_DRAW_SW_SUPPORT_ARGB8888 off to
         * save flash. The software renderer then declines to draw the frame
         * and does so silently - correct pixels sit in the buffer, the widget
         * is laid out and visible, and the screen stays black.
         *
         * Must come before lv_gif_set_src(): the format is read when the file
         * is opened. Setting it afterwards works too but re-opens the file.
         *
         * RGB565 also matches the panel, so no conversion happens per blend,
         * and the frame buffer drops from w*h*5 to w*h*3 bytes. None of the
         * three animations uses transparency, so nothing is lost by giving up
         * the alpha channel.
         */
        lv_gif_set_color_format(gif, LV_COLOR_FORMAT_RGB565);

        lv_gif_set_src(gif, &s_dsc[i]);
        lv_obj_center(gif);

        s_gif[i] = gif;
        s_count++;
    }

    showcase_create();

    /* Nothing is on screen yet, so leave every animation paused. */
    GifPages_SetVisiblePage(0u);
    GifPages_SetShowcaseActive(false);
}

uint8_t GifPages_Count(void)
{
    return s_count;
}

void GifPages_SetVisiblePage(uint8_t screen_index)
{
    for (uint8_t i = 0u; i < GIF_PAGES_MAX; i++) {
        if (s_gif[i] == NULL) {
            continue;
        }

        const int8_t page = screen_for(i);

        if (page >= 0 && (uint8_t)page == screen_index) {
            lv_gif_resume(s_gif[i]);
        }
        else {
            lv_gif_pause(s_gif[i]);
        }
    }
}
