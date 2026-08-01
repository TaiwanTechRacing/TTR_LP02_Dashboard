/*
 * bsp_sdram.h
 *
 *  核心板上的 W9825G6KH SDRAM(接在 FMC SDRAM Bank1)。
 *
 *  重要:FMC 沒有配置在 .ioc 裡面,是手寫的。如果之後有人用 CubeMX 重新產生
 *  程式碼,CubeMX 不知道下面這些腳位已經被佔用,可能會把它們配給別的周邊。
 *  改 .ioc 之前請先確認這張表:
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

/* W9825G6KH:13 row bits x 9 column bits x 4 banks x 16 bit = 32 MB */
#define SDRAM_BASE_ADDR       0xC0000000UL
#define SDRAM_SIZE_BYTES      (32U * 1024U * 1024U)

/*
 * SDRAM 分配表。改這裡的話 lv_conf.h 的 LV_MEM_ADR / LV_MEM_SIZE 要跟著改。
 *
 *   0xC0000000  256 KB  framebuffer 0   ┐ 這 512 KB 在 MPU 設成 write-through,
 *   0xC0040000  256 KB  framebuffer 1   ┘ LTDC 才不會讀到還留在 D-cache 裡的資料
 *   0xC0080000  512 KB  (保留)
 *   0xC0100000    2 MB  LVGL heap       ← write-back,CPU 存取快
 *   0xC0300000   29 MB  (未使用)
 */
#define SDRAM_FB0_ADDR        (SDRAM_BASE_ADDR + 0x00000000UL)
#define SDRAM_FB1_ADDR        (SDRAM_BASE_ADDR + 0x00040000UL)
#define SDRAM_FB_REGION_SIZE  0x00080000UL   /* 兩張 framebuffer 合起來 = 512 KB */

#define SDRAM_LVGL_HEAP_ADDR  (SDRAM_BASE_ADDR + 0x00100000UL)
#define SDRAM_LVGL_HEAP_SIZE  (2U * 1024U * 1024U)

#define SDRAM_FREE_ADDR       (SDRAM_BASE_ADDR + 0x00300000UL)
#define SDRAM_FREE_SIZE       (SDRAM_SIZE_BYTES - 0x00300000UL)

/**
 * 初始化 FMC 與 SDRAM。必須在 SystemClock_Config() 之後、任何會碰到
 * 0xC0000000 的程式(包含 LTDC 初始化)之前呼叫。
 */
void BSP_SDRAM_Init(void);

/**
 * 寫入再讀回一小段資料,確認 SDRAM 真的通了。
 * 上車前的自我檢查用;回傳 false 代表 SDRAM 沒接好或時序不對。
 */
bool BSP_SDRAM_SelfTest(void);

#endif /* BSP_SDRAM_H */
