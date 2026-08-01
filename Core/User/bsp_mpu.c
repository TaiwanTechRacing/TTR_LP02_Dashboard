/*
 * bsp_mpu.c
 *
 *  Cortex-M7 MPU and cache configuration.
 *
 *  This is the foundation of the performance work. With caches off, an M7
 *  fetches instructions from flash and touches SRAM several times slower than
 *  it should, and LVGL's software rendering takes the brunt of it.
 */

#include "bsp_mpu.h"
#include "bsp_sdram.h"
#include "stm32h7xx_hal.h"
#include <stdbool.h>

/* Base of the memory-mapped QSPI window. Same value as QSPI_BASE_ADDR in
 * bsp_qspi.h, duplicated here so the MPU setup does not depend on the QSPI
 * driver in the other direction. */
#define QSPI_MAPPED_BASE  0x90000000UL

static void mpu_disable_region(uint8_t number);
static bool mpu_size_code(uint32_t size_bytes, uint8_t *code);

void BSP_MPU_ConfigAndEnableCache(void)
{
    MPU_Region_InitTypeDef mpu = {0};

    HAL_MPU_Disable();

    /*
     * Region 0: the whole 32 MB SDRAM, write-back with write-allocate.
     * The LVGL heap lives in here, and WB is fastest for data structures that
     * are read and written repeatedly.
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
    mpu.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;   /* data only, never executed */
    mpu.SubRegionDisable = 0x00;
    HAL_MPU_ConfigRegion(&mpu);

    /*
     * Region 1: the two framebuffers (512 KB), write-through. It overlaps
     * region 0, and on the M7 the higher-numbered region wins, so these 512 KB
     * end up WT.
     *
     * Why the framebuffer must be write-through: LTDC fetches pixels straight
     * from SDRAM and never looks at the D-cache. Under write-back, freshly
     * drawn pixels can still be sitting dirty in cache, showing up as tearing
     * or garbage on screen. Write-through pushes every write out to SDRAM, so
     * no manual SCB_CleanDCache is needed - and no bug from forgetting one.
     */
    mpu.Number           = MPU_REGION_NUMBER1;
    mpu.BaseAddress      = SDRAM_FB0_ADDR;
    mpu.Size             = MPU_REGION_SIZE_512KB;
    mpu.TypeExtField     = MPU_TEX_LEVEL0;      /* TEX=000 C=1 B=0 -> write-through */
    mpu.IsCacheable      = MPU_ACCESS_CACHEABLE;
    mpu.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&mpu);

    /*
     * Region 2: block the entire 256 MB QSPI window at 0x90000000.
     *
     * Memory-mapped QSPI treated as ordinary memory invites speculative
     * prefetch from the M7. A prefetch past the end of the real chip leaves
     * QUADSPI waiting for a response that never arrives and the CPU hangs. So
     * block it all by default; BSP_MPU_EnableQspiRegion() opens only the real
     * capacity once BSP_QSPI_Init() has determined it.
     */
    mpu.Number           = MPU_REGION_NUMBER2;
    mpu.BaseAddress      = QSPI_MAPPED_BASE;
    mpu.Size             = MPU_REGION_SIZE_256MB;
    mpu.AccessPermission = MPU_REGION_NO_ACCESS;
    mpu.TypeExtField     = MPU_TEX_LEVEL0;
    mpu.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    mpu.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&mpu);

    /* CubeMX only used region 0, but disable the rest anyway so no stale
     * configuration survives. */
    for (uint8_t i = 3U; i < 8U; i++) {
        mpu_disable_region(i);
    }

    /*
     * Enable the MPU while keeping the default background map (PRIVDEFENA), so
     * flash at 0x08000000, internal SRAM at 0x24000000 and the peripheral space
     * keep ARM's default attributes: flash cacheable, internal SRAM write-back,
     * peripherals device memory. All of which is what we want, so no extra
     * regions are needed.
     */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

    SCB_EnableICache();
    SCB_EnableDCache();
}

void BSP_MPU_EnableQspiRegion(uint32_t size_bytes)
{
    uint8_t size_code;

    if (!mpu_size_code(size_bytes, &size_code)) {
        return;   /* implausible size - leave the window blocked */
    }

    MPU_Region_InitTypeDef mpu = {0};

    HAL_MPU_Disable();

    /*
     * Region 3 sits on top of region 2 (no-access). The higher-numbered region
     * wins on the M7, so the megabytes that actually exist become readable
     * while everything beyond stays blocked by region 2.
     *
     * Read-only is deliberate: this is memory-mapped flash, so any write is a
     * logic error and it is better for the MPU to catch it than to silently
     * discard it.
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

/* Convert a byte count to the MPU size code. Powers of two only, minimum 256. */
static bool mpu_size_code(uint32_t size_bytes, uint8_t *code)
{
    if (size_bytes < 256U) {
        return false;
    }

    if ((size_bytes & (size_bytes - 1U)) != 0U) {
        return false;   /* not a power of two */
    }

    uint8_t exp = 0;
    while ((size_bytes >> exp) > 1U) {
        exp++;
    }

    /* MPU SIZE field is defined as: region size = 2^(SIZE+1) */
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
