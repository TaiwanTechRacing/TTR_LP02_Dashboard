/*
 * bsp_mpu.c
 *
 *  Cortex-M7 的 MPU 與 cache 設定。
 *
 *  這裡是整個效能改善的地基:M7 不開 cache 的話,從 Flash 抓指令和存取 SRAM
 *  都會慢好幾倍,LVGL 的軟體繪圖首當其衝。
 */

#include "bsp_mpu.h"
#include "bsp_sdram.h"
#include "stm32h7xx_hal.h"

static void mpu_disable_region(uint8_t number);

void BSP_MPU_ConfigAndEnableCache(void)
{
    MPU_Region_InitTypeDef mpu = {0};

    HAL_MPU_Disable();

    /*
     * Region 0:整塊 32 MB SDRAM,write-back + write-allocate。
     * LVGL 的 heap 放在這裡面,WB 對於反覆讀寫的資料結構最快。
     */
    mpu.Enable           = MPU_REGION_ENABLE;
    mpu.Number           = MPU_REGION_NUMBER0;
    mpu.BaseAddress      = SDRAM_BASE_ADDR;
    mpu.Size             = MPU_REGION_SIZE_32MB;
    mpu.AccessPermission = MPU_REGION_FULL_ACCESS;
    mpu.TypeExtField     = MPU_TEX_LEVEL1;      /* TEX=001 C=1 B=1 -> write-back, write-allocate */
    mpu.IsCacheable      = MPU_ACCESS_CACHEABLE;
    mpu.IsBufferable     = MPU_ACCESS_BUFFERABLE;
    mpu.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    mpu.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;   /* 資料區,不放程式 */
    mpu.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&mpu);

    /*
     * Region 1:兩張 framebuffer(512 KB),write-through。
     * 位址和 region 0 重疊,M7 的規則是編號大的贏,所以這 512 KB 會是 WT。
     *
     * 為什麼 framebuffer 一定要 WT:LTDC 是自己直接去 SDRAM 抓畫面的,不會看
     * D-cache。如果用 write-back,CPU 畫好的像素可能還躺在 cache 裡沒寫回去,
     * 螢幕就會出現殘影或花屏。WT 讓寫入直接穿透到 SDRAM,完全不需要手動
     * SCB_CleanDCache,也就不會有忘記清 cache 的 bug。
     */
    mpu.Number           = MPU_REGION_NUMBER1;
    mpu.BaseAddress      = SDRAM_FB0_ADDR;
    mpu.Size             = MPU_REGION_SIZE_512KB;
    mpu.TypeExtField     = MPU_TEX_LEVEL0;      /* TEX=000 C=1 B=0 -> write-through */
    mpu.IsCacheable      = MPU_ACCESS_CACHEABLE;
    mpu.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&mpu);

    /* CubeMX 只用了 region 0,但保險起見把其餘的關掉,避免殘留設定。 */
    for (uint8_t i = 2U; i < 8U; i++) {
        mpu_disable_region(i);
    }

    /*
     * 開啟 MPU,並保留背景的預設記憶體映射(PRIVDEFENA)。
     * 這樣 Flash(0x08000000)、內部 SRAM(0x24000000)、周邊暫存器都沿用
     * ARM 預設屬性 —— Flash 是 cacheable、內部 SRAM 是 write-back、周邊是
     * device memory,剛好都是我們要的,不需要額外開 region。
     */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

    SCB_EnableICache();
    SCB_EnableDCache();
}

static void mpu_disable_region(uint8_t number)
{
    MPU_Region_InitTypeDef mpu = {0};

    mpu.Enable = MPU_REGION_DISABLE;
    mpu.Number = number;
    HAL_MPU_ConfigRegion(&mpu);
}
