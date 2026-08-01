/*
 * Dev_Inf.c
 *
 *  The one descriptor STM32CubeProgrammer reads before it will touch the chip.
 *
 *  Values come from the part actually fitted, confirmed on hardware by reading
 *  the JEDEC ID: 0xEF4017 is a Winbond W25Q64, 8 MB.
 */

#include "Dev_Inf.h"

/*
 * Must live in .Dev_Info - CubeProgrammer locates this by section, not by
 * address. "used" keeps it through --gc-sections, which would otherwise drop
 * it because nothing in the loader references it.
 */
__attribute__((section(".Dev_Info"), used))
const struct StorageInfo StorageInfo = {
    "W25Q64_TTR_LP02_Dashboard",
    SPI_FLASH,
    0x90000000u,          /* where the part appears once memory-mapped */
    0x00800000u,          /* 8 MB */
    0x00000100u,          /* 256-byte page: the largest single program */
    0xFFu,                /* erased state */

    /*
     * 2048 uniform 4 KB sectors. W25Q parts also support 32 KB and 64 KB block
     * erases, which are faster per byte, but the loader only implements the
     * 4 KB sector erase and this table has to describe what it can actually do.
     */
    {
        { 0x00000800u, 0x00001000u },
        { 0x00000000u, 0x00000000u },   /* terminator */
    }
};
