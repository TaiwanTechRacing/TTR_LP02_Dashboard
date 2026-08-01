/*
 * bsp_sdram.h
 *
 *  W9825G6KH SDRAM on the core board, wired to FMC SDRAM bank 1.
 *
 *  Important: FMC is NOT configured in the .ioc - it is hand-written. CubeMX
 *  does not know these pins are taken and may hand them to another peripheral
 *  when regenerating. Check this table before touching the .ioc:
 *
 *    PC0  SDNWE    PD0  D2    PE0  NBL0   PF0  A0   PG0  A10
 *    PC2  SDNE0    PD1  D3    PE1  NBL1   PF1  A1   PG1  A11
 *    PC3  SDCKE0   PD8  D13   PE7  D4     PF2  A2   PG2  A12
 *                  PD9  D14   PE8  D5     PF3  A3   PG4  BA0
 *                  PD10 D15   PE9  D6     PF4  A4   PG5  BA1
 *                  PD14 D0    PE10 D7     PF5  A5   PG8  SDCLK
 *                  PD15 D1    PE11 D8     PF11 SDNRAS
 *                             PE12 D9     PF12 A6   PG15 SDNCAS
 *                             PE13 D10    PF13 A7
 *                             PE14 D11    PF14 A8
 *                             PE15 D12    PF15 A9
 */

#ifndef BSP_SDRAM_H
#define BSP_SDRAM_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/* W9825G6KH: 13 row bits x 9 column bits x 4 banks x 16 bit = 32 MB */
#define SDRAM_BASE_ADDR       0xC0000000UL
#define SDRAM_SIZE_BYTES      (32U * 1024U * 1024U)

/*
 * SDRAM map. Changing this means changing LV_MEM_ADR / LV_MEM_SIZE in
 * lv_conf.h to match.
 *
 *   0xC0000000  256 KB  framebuffer 0   | these 512 KB are write-through in the
 *   0xC0040000  256 KB  framebuffer 1   | MPU so LTDC never reads stale D-cache
 *   0xC0080000  512 KB  (reserved)
 *   0xC0100000    2 MB  LVGL heap       - write-back, fast for the CPU
 *   0xC0300000   29 MB  (unused)
 */
#define SDRAM_FB0_ADDR        (SDRAM_BASE_ADDR + 0x00000000UL)
#define SDRAM_FB1_ADDR        (SDRAM_BASE_ADDR + 0x00040000UL)
#define SDRAM_FB_REGION_SIZE  0x00080000UL   /* both framebuffers = 512 KB */

#define SDRAM_LVGL_HEAP_ADDR  (SDRAM_BASE_ADDR + 0x00100000UL)
#define SDRAM_LVGL_HEAP_SIZE  (2U * 1024U * 1024U)

#define SDRAM_FREE_ADDR       (SDRAM_BASE_ADDR + 0x00300000UL)
#define SDRAM_FREE_SIZE       (SDRAM_SIZE_BYTES - 0x00300000UL)

/**
 * Bring up FMC and the SDRAM. Must run after SystemClock_Config() and before
 * anything touches 0xC0000000, LTDC initialisation included.
 */
void BSP_SDRAM_Init(void);

/**
 * Write then read back a few locations to confirm the SDRAM responds.
 * A startup sanity check; false means bad wiring or wrong timings.
 */
bool BSP_SDRAM_SelfTest(void);

#endif /* BSP_SDRAM_H */
