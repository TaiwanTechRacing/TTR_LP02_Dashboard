/*
 * bsp_display.c
 *
 *  顯示層。原本的做法是:LVGL 畫進一塊 1/10 螢幕大的小 buffer,再用 DMA2D
 *  搬到 framebuffer,而且是 CPU 空轉輪詢等 DMA2D 做完。一張畫面要重複這個
 *  流程十幾次,CPU 全部浪費在等待上,而且畫面會撕裂。
 *
 *  現在改成:兩張整頁 framebuffer 放在 SDRAM,LVGL 直接畫在上面(DIRECT
 *  模式),畫完只要換 LTDC 的讀取位址就好,一次記憶體搬移都不用。換頁排在
 *  垂直空白期,所以不會撕裂。
 */

#include "bsp_display.h"
#include "bsp_sdram.h"
#include "stm32h7xx_hal.h"

/* 換頁沒完成的容忍上限。60 Hz 一張畫面 16.7 ms,50 ms 代表中斷真的出事了。 */
#define FLUSH_TIMEOUT_MS 50U

static lv_display_t *s_disp;
static volatile bool s_flush_pending;
static volatile uint32_t s_flush_start_tick;

static void lcd_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

void BSP_Display_Init(void)
{
    lv_init();
    lv_tick_set_cb(HAL_GetTick);

    s_disp = lv_display_create(LCD_WIDTH, LCD_HEIGHT);

    /*
     * DIRECT 模式 + 兩張整頁 buffer。LVGL 會輪流畫在這兩張上面,並且自己
     * 記住最近兩張畫面各自髒掉的區域,所以兩張的內容會保持一致。
     */
    lv_display_set_buffers(s_disp,
                           (void *)SDRAM_FB0_ADDR,
                           (void *)SDRAM_FB1_ADDR,
                           LCD_FB_SIZE,
                           LV_DISPLAY_RENDER_MODE_DIRECT);

    lv_display_set_flush_cb(s_disp, lcd_flush_cb);

    /*
     * 開 LTDC 的 register reload 中斷。優先權刻意設得比 FDCAN2(0)低,
     * 畫面換頁絕對不該延誤 CAN 收包。
     */
    LTDC->ICR = LTDC_ICR_CRRIF;
    HAL_NVIC_SetPriority(LTDC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(LTDC_IRQn);
}

void BSP_Display_Service(void)
{
    if (!s_flush_pending) {
        return;
    }

    if ((HAL_GetTick() - s_flush_start_tick) < FLUSH_TIMEOUT_MS) {
        return;
    }

    /* 中斷沒來。放棄等待,讓 LVGL 繼續跑。 */
    LTDC->IER &= ~LTDC_IER_RRIE;
    s_flush_pending = false;
    lv_display_flush_ready(s_disp);
}

static void lcd_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);

    /*
     * DIRECT 模式下 LVGL 已經把像素寫進整頁 buffer 了,每個髒區域都會呼叫一次
     * 這裡。只有最後一次才需要動作 —— 前面幾次直接回報完成就好。
     */
    if (!lv_display_flush_is_last(disp)) {
        lv_display_flush_ready(disp);
        return;
    }

    /*
     * framebuffer 那塊 MPU 設成 write-through,所以像素此刻已經在 SDRAM 裡,
     * 不需要 SCB_CleanDCache_by_Addr。
     *
     * 把 LTDC 指到剛畫好的這張,並要求在下一個垂直空白期生效。在那之前螢幕
     * 顯示的還是舊的那張,所以不會看到畫到一半的畫面。
     */
    s_flush_start_tick = HAL_GetTick();
    s_flush_pending = true;

    LTDC_Layer1->CFBAR = (uint32_t)px_map;
    LTDC->IER |= LTDC_IER_RRIE;
    LTDC->SRCR = LTDC_SRCR_VBR;

    /* 這裡不呼叫 lv_display_flush_ready() —— 等 LTDC_IRQHandler 確認換頁
     * 真的生效之後再放行,LVGL 才不會提早去畫還在顯示中的那張。 */
}

/**
 * LTDC 全域中斷。CubeMX 沒有產生這個 handler(.ioc 裡沒開 LTDC 中斷),
 * 所以在這裡定義,蓋掉 startup 檔裡的 weak 版本。
 */
void LTDC_IRQHandler(void)
{
    if ((LTDC->ISR & LTDC_ISR_RRIF) == 0U) {
        return;
    }

    LTDC->ICR = LTDC_ICR_CRRIF;
    LTDC->IER &= ~LTDC_IER_RRIE;

    if (s_flush_pending) {
        s_flush_pending = false;
        lv_display_flush_ready(s_disp);
    }
}
