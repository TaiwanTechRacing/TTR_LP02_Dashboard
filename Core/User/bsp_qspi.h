/*
 * bsp_qspi.h
 *
 *  W25Qxx QSPI flash on the core board, configured for memory-mapped reads.
 *
 *  Why: the internal 2 MB flash was nearly full of UI artwork - five
 *  full-screen backgrounds alone accounted for 1.25 MB. With image data on
 *  this external chip, LVGL can read it like ordinary memory at 0x90000000
 *  and the internal flash is left for code.
 *
 *  Important: like FMC/SDRAM, QSPI is not configured in the .ioc. CubeMX does
 *  not know these pins are taken:
 *
 *    PB2  QUADSPI_CLK      (AF9)
 *    PB6  QUADSPI_BK1_NCS  (AF10)
 *    PF6  QUADSPI_BK1_IO3  (AF9)
 *    PF7  QUADSPI_BK1_IO2  (AF9)
 *    PF8  QUADSPI_BK1_IO0  (AF10)
 *    PF9  QUADSPI_BK1_IO1  (AF10)
 *
 *  None of these clash with the SDRAM, which uses PF0..PF5 and PF11..PF15.
 */

#ifndef BSP_QSPI_H
#define BSP_QSPI_H

#include <stdbool.h>
#include <stdint.h>

/** Once mapped, the flash contents appear at this address. */
#define QSPI_BASE_ADDR   0x90000000UL

/**
 * Bring up QUADSPI, read the JEDEC ID to determine capacity, enable quad mode
 * and switch to memory-mapped reads.
 *
 * Failure is not fatal. This chip is currently a fallback and the dashboard
 * runs fine without it, so Error_Handler() is deliberately not called - it is
 * not worth blanking the whole display over a chip nothing uses yet. Every
 * wait is bounded by a timeout.
 *
 * @return true if the chip was found and mapping succeeded.
 */
bool BSP_QSPI_Init(void);

/**
 * Detected capacity in bytes, or 0 if not initialised or detection failed.
 * W25Q64 is 8 MB, W25Q128 is 16 MB.
 */
uint32_t BSP_QSPI_GetFlashSize(void);

/**
 * The JEDEC ID that was read (0xEF40xx and similar). For debugging.
 */
uint32_t BSP_QSPI_GetJedecId(void);

#endif /* BSP_QSPI_H */
