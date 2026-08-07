/*
 * sim_sdram.c
 *
 *  Host stand-in for the SDRAM allocator in Core/User/bsp_sdram.c, which is all
 *  FMC register work and cannot be built for the host.
 *
 *  Only BSP_SDRAM_Alloc() is provided, because that is the only part of the
 *  driver that anything above the BSP layer calls.
 */

#include "bsp_sdram.h"

#include <stdlib.h>

void *BSP_SDRAM_Alloc(uint32_t size_bytes)
{
    /*
     * Never freed, matching the target: the callers take their buffer once at
     * start-up and hold it for the life of the program.
     */
    return calloc(1, size_bytes);
}
