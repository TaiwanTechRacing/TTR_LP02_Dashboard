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
#include <stdbool.h>

/* QSPI 記憶體映射的起始位址。與 bsp_qspi.h 的 QSPI_BASE_ADDR 相同,
 * 這裡另外定義是為了讓 MPU 設定不必反過來相依於 QSPI 驅動。 */
#define QSPI_MAPPED_BASE  0x90000000UL

static void mpu_disable_region(uint8_t number);
static bool mpu_size_code(uint32_t size_bytes, uint8_t *code);

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

    /*
     * Region 2:把 QSPI 映射視窗(0x90000000 起 256MB)整段設成 no-access。
     *
     * 記憶體映射的 QSPI 如果被當成一般記憶體,M7 會做投機式預取。一旦預取
     * 到晶片實際容量之外,QUADSPI 會等一個永遠不會來的回應,整顆 CPU 就卡
     * 在那裡。所以預設全部擋掉,等 BSP_QSPI_Init() 讀到實際容量之後,再由
     * BSP_MPU_EnableQspiRegion() 只開放真正存在的那幾 MB。
     */
    mpu.Number           = MPU_REGION_NUMBER2;
    mpu.BaseAddress      = QSPI_MAPPED_BASE;
    mpu.Size             = MPU_REGION_SIZE_256MB;
    mpu.AccessPermission = MPU_REGION_NO_ACCESS;
    mpu.TypeExtField     = MPU_TEX_LEVEL0;
    mpu.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    mpu.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&mpu);

    /* CubeMX 只用了 region 0,但保險起見把其餘的關掉,避免殘留設定。 */
    for (uint8_t i = 3U; i < 8U; i++) {
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

void BSP_MPU_EnableQspiRegion(uint32_t size_bytes)
{
    uint8_t size_code;

    if (!mpu_size_code(size_bytes, &size_code)) {
        return;   /* 容量不合理,維持全部封鎖 */
    }

    MPU_Region_InitTypeDef mpu = {0};

    HAL_MPU_Disable();

    /*
     * Region 3 疊在 region 2(no-access)上面。M7 的規則是編號大的贏,
     * 所以實際存在的這幾 MB 變成可讀,範圍外仍然被 region 2 擋著。
     *
     * 設成唯讀是刻意的:這是記憶體映射的 Flash,任何寫入都是程式邏輯錯誤,
     * 讓 MPU 直接抓出來比默默忽略好。
     */
    mpu.Enable           = MPU_REGION_ENABLE;
    mpu.Number           = MPU_REGION_NUMBER3;
    mpu.BaseAddress      = QSPI_MAPPED_BASE;
    mpu.Size             = size_code;
    mpu.AccessPermission = MPU_REGION_PRIV_RO_URO;
    mpu.TypeExtField     = MPU_TEX_LEVEL0;      /* TEX=000 C=1 B=0 -> write-through */
    mpu.IsCacheable      = MPU_ACCESS_CACHEABLE;
    mpu.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    mpu.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    mpu.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    mpu.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&mpu);

    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/* 把容量換成 MPU 的 size 代碼。只接受 2 的冪次,而且至少 256 bytes。 */
static bool mpu_size_code(uint32_t size_bytes, uint8_t *code)
{
    if (size_bytes < 256U) {
        return false;
    }

    if ((size_bytes & (size_bytes - 1U)) != 0U) {
        return false;   /* 不是 2 的冪次 */
    }

    uint8_t exp = 0;
    while ((size_bytes >> exp) > 1U) {
        exp++;
    }

    /* MPU 的 SIZE 欄位定義:region 大小 = 2^(SIZE+1) */
    *code = (uint8_t)(exp - 1U);
    return true;
}

static void mpu_disable_region(uint8_t number)
{
    MPU_Region_InitTypeDef mpu = {0};

    mpu.Enable = MPU_REGION_DISABLE;
    mpu.Number = number;
    HAL_MPU_ConfigRegion(&mpu);
}
