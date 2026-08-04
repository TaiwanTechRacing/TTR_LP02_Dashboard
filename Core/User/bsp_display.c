/*
 * bsp_display.c
 *
 *  Display layer. The original approach had LVGL render into a buffer one
 *  tenth of the screen, copy it to the framebuffer with DMA2D, and busy-wait
 *  on the CPU until DMA2D finished. A full frame repeated that a dozen-odd
 *  times, so the CPU spent most of its time waiting - and the picture tore.
 *
 *  Now: two full-page framebuffers in SDRAM with LVGL drawing straight into
 *  them (DIRECT mode). Presenting a frame is just pointing LTDC at the other
 *  buffer, with no memory copy at all. The swap is scheduled for the vertical
 *  blanking interval, so there is no tearing.
 */

#include "bsp_display.h"
#include "bsp_mpu.h"
#include "bsp_sdram.h"
#include "stm32h7xx_hal.h"

/* How long to tolerate an unfinished flip. A 60 Hz frame is 16.7 ms, so 50 ms
 * means the interrupt genuinely failed. */
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
     * DIRECT mode with two full-page buffers. LVGL alternates between them and
     * tracks which areas went dirty in each of the last two frames, so both
     * buffers stay consistent.
     */
    lv_display_set_buffers(s_disp,
                           (void *)SDRAM_FB0_ADDR,
                           (void *)SDRAM_FB1_ADDR,
                           LCD_FB_SIZE,
                           LV_DISPLAY_RENDER_MODE_DIRECT);

    lv_display_set_flush_cb(s_disp, lcd_flush_cb);

    /*
     * Enable the LTDC register-reload interrupt. Its priority is deliberately
     * lower than FDCAN2 (which is 0): presenting a frame must never delay CAN
     * reception.
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

    /* The interrupt never came. Give up waiting and let LVGL continue. */
    LTDC->IER &= ~LTDC_IER_RRIE;
    s_flush_pending = false;
    lv_display_flush_ready(s_disp);
}

static void lcd_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(area);

    /*
     * In DIRECT mode LVGL has already written the pixels into the full-page
     * buffer and calls this once per dirty area. Only the last call needs to do
     * anything; the earlier ones just report completion.
     */
    if (!lv_display_flush_is_last(disp)) {
        lv_display_flush_ready(disp);
        return;
    }

#if BSP_FB_WRITE_BACK
    /*
     * Push the frame out of the D-cache before LTDC is told to read it. Under
     * write-back the pixels can still be sitting dirty in cache, and LTDC would
     * scan out whatever SDRAM happened to hold - a frame of garbage or a mix of
     * two frames.
     *
     * The whole cache rather than SCB_CleanDCache_by_Addr over the buffer: the
     * range is 255 KB against a 16 KB D-cache, so cleaning by address would
     * walk eight thousand lines to clean at most five hundred. Set/way is both
     * cheaper and simpler here. This is the one place it is needed, and it is
     * on the same code path as the flip so it cannot be forgotten.
     */
    SCB_CleanDCache();
#endif

    /*
     * Point LTDC at the buffer just rendered and ask for the change to take
     * effect at the next vertical blanking. Until then the old buffer stays on
     * screen, so a half-drawn frame is never visible.
     */
    s_flush_start_tick = HAL_GetTick();
    s_flush_pending = true;

    LTDC_Layer1->CFBAR = (uint32_t)px_map;
    LTDC->IER |= LTDC_IER_RRIE;
    LTDC->SRCR = LTDC_SRCR_VBR;

    /* Deliberately no lv_display_flush_ready() here. LTDC_IRQHandler releases
     * it once the swap has actually taken effect, so LVGL never starts drawing
     * into the buffer still on screen. */
}

/**
 * LTDC global interrupt. CubeMX does not generate this handler because the
 * .ioc has no LTDC interrupt enabled, so it is defined here, overriding the
 * weak symbol in the startup file.
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
