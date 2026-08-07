/*
 * bsp_display.h
 *
 *  LVGL display layer: double framebuffer in SDRAM, page flip on LTDC vblank.
 */

#ifndef BSP_DISPLAY_H
#define BSP_DISPLAY_H

#include "lvgl.h"

#define LCD_WIDTH        480
#define LCD_HEIGHT       272
#define LCD_BYTES_PER_PX 2
#define LCD_FB_SIZE      ((uint32_t)LCD_WIDTH * LCD_HEIGHT * LCD_BYTES_PER_PX)

/**
 * lv_init(), create the display, attach the two SDRAM framebuffers.
 * Must be called after BSP_SDRAM_Init() and MX_LTDC_Init().
 */
void BSP_Display_Init(void);

/**
 * Call once per main loop iteration.
 *
 * Normally does nothing - the LTDC interrupt is what tells LVGL the flip
 * completed. This is purely a safety net: if that interrupt never arrives
 * (NVIC disabled, or someone breaks the interrupt setup later), LVGL waits
 * forever on flush_ready and the screen freezes. Better to force it through
 * after a timeout so the display keeps moving than to leave the driver
 * staring at a frozen instrument panel.
 */
void BSP_Display_Service(void);

#endif /* BSP_DISPLAY_H */
