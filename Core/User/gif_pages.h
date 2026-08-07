/*
 * gif_pages.h
 *
 *  Puts the three animations from QSPI into the containers on Debug1..Debug3.
 *
 *  The GIF data does not live in internal flash - 3.6 MB of it does not fit
 *  alongside the firmware - so it is programmed into the W25Q64 and read
 *  through the memory-mapped window. See tools/make_qspi_image.py for the
 *  layout and how to flash it.
 */

#ifndef GIF_PAGES_H
#define GIF_PAGES_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Create the GIF widgets, if the QSPI image is present.
 *
 * Safe to call when nothing has been programmed: an unwritten part reads 0xFF
 * everywhere, which the header check rejects rather than handing to the
 * decoder. Call after ui_init().
 */
void GifPages_Init(void);

/**
 * Number of animations found in the QSPI image. 0 means none were loaded -
 * either the part is blank or the header did not match.
 */
uint8_t GifPages_Count(void);

/**
 * Run only the animation on the page currently shown.
 *
 * EEZ creates every screen at startup, so all three GIF widgets exist from the
 * first frame and LVGL would decode all of them continuously - burning CPU on
 * two animations nobody can see. On a dashboard that cost lands on the frame
 * rate of whatever page the driver is actually looking at.
 *
 * Call whenever the visible page changes.
 */
void GifPages_SetVisiblePage(uint8_t screen_index);

/**
 * Run the showcase in the corner of the game page, cycling every animation in
 * turn. Pass false when leaving the page so it stops decoding.
 */
void GifPages_SetShowcaseActive(bool active);

/**
 * Advance the showcase to the next animation when its turn is up. Call from the
 * main loop; does nothing while the showcase is stopped.
 */
void GifPages_ShowcaseService(uint32_t now_ms);

#endif /* GIF_PAGES_H */
