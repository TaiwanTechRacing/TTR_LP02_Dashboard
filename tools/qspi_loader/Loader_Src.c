/*
 * Loader_Src.c
 *
 *  STM32CubeProgrammer external loader for the W25Q64 on QUADSPI.
 *
 *  CubeProgrammer downloads this into the target's RAM, halts the CPU, and
 *  calls the functions below directly - there is no main(), no startup code and
 *  no interrupts. Everything the peripheral needs must therefore be set up
 *  inside Init().
 *
 *  The flash operations themselves are not reimplemented here: Core/User/
 *  bsp_qspi.c is compiled into the loader as-is. A second implementation would
 *  drift from the firmware's, and a loader that writes differently from the
 *  code that reads is a bad place for a discrepancy to hide.
 *
 *  Return convention is CubeProgrammer's: 1 on success, 0 on failure.
 */

#include "bsp_qspi.h"
#include "stm32h7xx_hal.h"

#include <stdint.h>
#include <stddef.h>

/* ------------------------------------------------------------------------- */
/* Freestanding fill-ins                                                      */
/*                                                                            */
/* Linked with -nostdlib and without the HAL's own startup, so the handful of  */
/* symbols the reused sources expect have to be supplied here.                */
/* ------------------------------------------------------------------------- */

/* GCC emits calls to these for struct initialisers like "= {0}" regardless of
 * -nostdlib, so they must exist even though nothing calls them by name. */
void *memset(void *dst, int value, size_t len)
{
    uint8_t *p = (uint8_t *)dst;
    while (len--) {
        *p++ = (uint8_t)value;
    }
    return dst;
}

void *memcpy(void *dst, const void *src, size_t len)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    while (len--) {
        *d++ = *s++;
    }
    return dst;
}

/* HAL_RCC_ClockConfig() reconfigures the tick on success. The loader has no
 * tick to reconfigure - HAL_GetTick() below reads the cycle counter directly -
 * so this only has to succeed. */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    (void)TickPriority;
    return HAL_OK;
}

/*
 * bsp_qspi.c hands the detected capacity to the MPU so the firmware can open
 * just that much of the mapped window. The loader disabled the MPU outright in
 * Init(), so there is nothing to open.
 */
void BSP_MPU_EnableQspiRegion(uint32_t size_bytes)
{
    (void)size_bytes;
}

/*
 * There is no SysTick here - no interrupts run - so the timebase that
 * bsp_qspi.c relies on comes from the Cortex-M7 cycle counter instead.
 */
static uint32_t s_cycles_per_ms;

uint32_t HAL_GetTick(void)
{
    return (s_cycles_per_ms != 0u) ? (DWT->CYCCNT / s_cycles_per_ms) : 0u;
}

void HAL_Delay(uint32_t ms)
{
    const uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < ms) {
        /* spin */
    }
}

static void enable_cycle_counter(uint32_t sysclk_hz)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    s_cycles_per_ms = sysclk_hz / 1000u;
}

/**
 * Called once before anything else. Must leave the part readable at
 * 0x90000000, because CubeProgrammer verifies by reading through the mapping.
 */
int Init(void)
{
    /*
     * The MPU is whatever the previous firmware left configured, and that
     * firmware deliberately marks the QSPI window no-access until its capacity
     * is known. Turning the MPU off entirely is simpler and safer here than
     * reproducing that logic: the loader is the only thing running.
     */
    HAL_MPU_Disable();
    /* CubeProgrammer holds the part in reset before loading this image, so the
     * caches are already disabled. CMSIS' set/way cache-maintenance routine
     * is unsafe here because no Reset_Handler/SystemInit cache setup ran. */

    /* CubeProgrammer enters Init directly; unlike normal firmware startup,
     * neither Reset_Handler nor main() has called HAL_Init(). Give it a
     * working polling timebase at the reset-clock rate. The loader stays on
     * the 64 MHz HSI, making the fixed QSPI divider a conservative 16 MHz. */
    enable_cycle_counter(64000000u);
    if (HAL_Init() != HAL_OK) {
        return 0;
    }

    BSP_QSPI_Init();

    /* A wrong or absent JEDEC ID means there is no point continuing - better to
     * fail here than to report a successful erase of nothing. */
    return (g_qspi_flash_size == 0x00800000u) ? 1 : 0;
}

/**
 * Erase one sector, addressed by absolute address in the mapped window.
 */
int SectorErase(uint32_t start_address, uint32_t end_address)
{
    if (start_address < QSPI_BASE_ADDR) {
        return 0;
    }

    uint32_t addr = (start_address - QSPI_BASE_ADDR) & ~(QSPI_SECTOR_SIZE - 1u);
    const uint32_t end = end_address - QSPI_BASE_ADDR;

    while (addr <= end) {
        if (!BSP_QSPI_EraseSector(addr)) {
            return 0;
        }
        addr += QSPI_SECTOR_SIZE;
    }

    return 1;
}

int MassErase(void)
{
    return BSP_QSPI_EraseChip() ? 1 : 0;
}

int Write(uint32_t address, uint32_t size, uint8_t *buffer)
{
    if (address < QSPI_BASE_ADDR) {
        return 0;
    }

    return BSP_QSPI_Program(address - QSPI_BASE_ADDR, buffer, size) ? 1 : 0;
}

/**
 * CubeProgrammer usually reads through the memory mapping and never calls this,
 * but the entry point has to exist.
 */
int Read(uint32_t address, uint32_t size, uint8_t *buffer)
{
    if (address < QSPI_BASE_ADDR) {
        return 0;
    }

    return BSP_QSPI_Read(address - QSPI_BASE_ADDR, buffer, size) ? 1 : 0;
}

/**
 * Optional verify hook. Returning the end address signals success; on mismatch
 * CubeProgrammer expects the failing address in the low word and the byte read
 * there in the high word.
 */
uint64_t Verify(uint32_t memory_addr, uint32_t rambuffer, uint32_t size, uint32_t missalignement)
{
    const uint8_t *flash = (const uint8_t *)memory_addr;
    const uint8_t *ram = (const uint8_t *)rambuffer;

    (void)missalignement;

    for (uint32_t i = 0u; i < size; i++) {
        if (flash[i] != ram[i]) {
            return (uint64_t)(memory_addr + i) | ((uint64_t)flash[i] << 32);
        }
    }

    return (uint64_t)(memory_addr + size);
}
