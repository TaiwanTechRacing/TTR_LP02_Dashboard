/*
 * bsp_mpu.h
 *
 *  MPU region setup and CPU cache enable.
 */

#ifndef BSP_MPU_H
#define BSP_MPU_H

/*
 * Framebuffer cache policy. 1 = write-back with write-allocate, 0 = the
 * write-through it used to be.
 *
 * Write-back lets the D-cache gather pixel stores into 32-byte bursts before
 * they reach SDRAM, which is what the software renderers (racer, tetris, cell
 * map) spend their frame doing - under write-through every store crossed the
 * FMC on its own. The cost is that the buffer must be cleaned out of the cache
 * before LTDC is pointed at it, which bsp_display.c does once per flip.
 *
 * Kept as a switch rather than just changed, because which one wins is a
 * question about access patterns and can only be settled by measuring on the
 * bench. Build both and compare the sysmon CPU figure on the racer page.
 */
#ifndef BSP_FB_WRITE_BACK
#define BSP_FB_WRITE_BACK 1
#endif

#include <stdint.h>

/**
 * Reconfigure the MPU and enable I-cache and D-cache.
 *
 * Call after HAL_Init() and before SystemClock_Config().
 *
 * This fully overrides the CubeMX-generated MPU_Config(), which marks
 * 0x60000000..0xDFFFFFFF as no-access - a range that contains the SDRAM at
 * 0xC0000000. That was the direct reason the SDRAM was unusable.
 */
void BSP_MPU_ConfigAndEnableCache(void);

/**
 * Open the memory-mapped QSPI window at 0x90000000 as readable and cacheable.
 *
 * Called by BSP_QSPI_Init() once the JEDEC ID has been read and the capacity
 * is known.
 *
 * Why this is two steps: BSP_MPU_ConfigAndEnableCache() first marks the whole
 * 256 MB starting at 0x90000000 as no-access. Memory-mapped QSPI treated as
 * ordinary memory invites speculative prefetch, and a prefetch past the end of
 * the real chip leaves QUADSPI waiting for a response that never comes, hanging
 * the CPU. Blocking everything and then opening only the megabytes that exist
 * avoids that.
 *
 * @param size_bytes actual capacity, must be a power of two (8 MB or 16 MB).
 *                   Passing 0 leaves the whole window blocked.
 */
void BSP_MPU_EnableQspiRegion(uint32_t size_bytes);

#endif /* BSP_MPU_H */
