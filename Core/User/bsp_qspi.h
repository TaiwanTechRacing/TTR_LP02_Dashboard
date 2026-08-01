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

/*
 * Results of the last BSP_QSPI_Init(), exposed as globals so they can be read
 * straight from a debugger watch window - no file qualifier, no accessor call.
 *
 *   0xEF4017 = W25Q64  (8 MB)
 *   0xEF4018 = W25Q128 (16 MB)
 *   0 or 0xFFFFFF     = the chip did not answer
 */
extern uint32_t g_qspi_jedec_id;
extern uint32_t g_qspi_flash_size;

/* W25Q64 geometry. Erase works in 4 KB sectors; programming cannot cross a
 * 256-byte page boundary, which BSP_QSPI_Program() handles internally. */
#define QSPI_SECTOR_SIZE  4096u
#define QSPI_PAGE_SIZE     256u

/**
 * Erase one 4 KB sector. @p addr may be anywhere inside it.
 *
 * Erasing sets bytes to 0xFF; programming can only clear bits, so a sector must
 * be erased before it is rewritten. Typically a few tens of milliseconds, but
 * the datasheet allows up to 400 ms.
 */
bool BSP_QSPI_EraseSector(uint32_t addr);

/**
 * Erase the whole chip. Can take tens of seconds - the datasheet allows 100 s
 * for this part - so it blocks for a long time.
 */
bool BSP_QSPI_EraseChip(void);

/**
 * Program @p len bytes at @p addr. Splits across page boundaries as needed.
 *
 * The target must already be erased. This does not check, because verifying
 * beforehand would double the time and the caller normally just erased it.
 */
bool BSP_QSPI_Program(uint32_t addr, const uint8_t *data, uint32_t len);

/**
 * Read @p len bytes from @p addr through the memory-mapped window.
 */
bool BSP_QSPI_Read(uint32_t addr, uint8_t *data, uint32_t len);

/**
 * Erase, program and read back a pattern in the last sector of the chip.
 *
 * The last sector is used because it is the least likely to hold anything
 * wanted. Running this proves the whole write path - erase, program, status
 * polling, and cache invalidation on the mapped window - before anything
 * depends on it.
 *
 * @return true if every byte read back matches.
 */
bool BSP_QSPI_SelfTestWrite(void);

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
