/*
 * gif_pages.c
 */

#include "gif_pages.h"
#include "bsp_qspi.h"
#include "screens.h"

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

/** Screen index (as used by screens[] in main.c) showing each animation. */
static uint8_t screen_for(uint8_t index)
{
    return (uint8_t)(index + 2u);   /* 0 welcome, 1 main, 2..4 debug1..3 */
}

void GifPages_Init(void)
{
    s_count = 0;

    if (BSP_QSPI_GetFlashSize() == 0u) {
        return;     /* the part never answered; nothing to read */
    }

    const qspi_image_header_t *header = (const qspi_image_header_t *)QSPI_BASE_ADDR;

    /*
     * A blank device reads 0xFF everywhere, so without this check the decoder
     * would be handed 8 MB of 0xFF and asked to make an animation of it.
     */
    if (header->magic != QSPI_IMAGE_MAGIC || header->count == 0u) {
        return;
    }

    const qspi_image_entry_t *entries =
        (const qspi_image_entry_t *)(QSPI_BASE_ADDR + sizeof(qspi_image_header_t));

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
        s_dsc[i].data = (const uint8_t *)(QSPI_BASE_ADDR + offset);

        lv_obj_t *gif = lv_gif_create(parent);
        if (gif == NULL) {
            continue;
        }

        lv_gif_set_src(gif, &s_dsc[i]);
        lv_obj_center(gif);

        s_gif[i] = gif;
        s_count++;
    }

    /* Nothing is on screen yet, so leave every animation paused. */
    GifPages_SetVisiblePage(0u);
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

        if (screen_for(i) == screen_index) {
            lv_gif_resume(s_gif[i]);
        }
        else {
            lv_gif_pause(s_gif[i]);
        }
    }
}
