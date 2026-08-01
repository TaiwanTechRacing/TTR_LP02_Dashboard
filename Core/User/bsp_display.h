/*
 * bsp_display.h
 *
 *  LVGL 顯示層:SDRAM 雙 framebuffer + LTDC 垂直空白期換頁。
 */

#ifndef BSP_DISPLAY_H
#define BSP_DISPLAY_H

#include "lvgl.h"

#define LCD_WIDTH        480
#define LCD_HEIGHT       272
#define LCD_BYTES_PER_PX 2
#define LCD_FB_SIZE      ((uint32_t)LCD_WIDTH * LCD_HEIGHT * LCD_BYTES_PER_PX)

/**
 * lv_init() + 建立 display + 掛上兩張 SDRAM framebuffer。
 * 必須在 BSP_SDRAM_Init() 和 MX_LTDC_Init() 之後呼叫。
 */
void BSP_Display_Init(void);

/**
 * 主迴圈每圈呼叫一次。
 *
 * 正常情況下什麼事都不做 —— 換頁完成是靠 LTDC 中斷通知 LVGL 的。
 * 這裡純粹是保險:萬一 LTDC 中斷沒進來(NVIC 被關掉、或是之後有人改壞了
 * 中斷設定),LVGL 會一直等 flush_ready 而整個畫面凍住。與其讓車手看到
 * 一個定格的儀表,不如超時後強制放行,至少畫面還會動。
 */
void BSP_Display_Service(void);

#endif /* BSP_DISPLAY_H */
