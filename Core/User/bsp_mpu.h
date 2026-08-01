/*
 * bsp_mpu.h
 *
 *  MPU 區域設定 + 開啟 CPU cache。
 */

#ifndef BSP_MPU_H
#define BSP_MPU_H

#include <stdint.h>

/**
 * 重新設定 MPU 並開啟 I-cache / D-cache。
 *
 * 呼叫時機:HAL_Init() 之後、SystemClock_Config() 之前。
 *
 * 這個函式會完全覆蓋掉 CubeMX 產生的 MPU_Config()。CubeMX 那份把
 * 0x60000000~0xDFFFFFFF 全設成 no-access,其中就包含 SDRAM 所在的
 * 0xC0000000 —— 這是之前 SDRAM 完全沒辦法用的直接原因。
 */
void BSP_MPU_ConfigAndEnableCache(void);

/**
 * 開放 QSPI 記憶體映射區域(0x90000000)為可讀、可快取。
 *
 * 由 BSP_QSPI_Init() 在讀到 JEDEC ID、確定容量之後呼叫。
 *
 * 為什麼要分兩段做:BSP_MPU_ConfigAndEnableCache() 會先把整個
 * 0x90000000 起始的 256MB 設成 no-access。記憶體映射的 QSPI 如果被當成
 * 一般記憶體,Cortex-M7 會做投機式預取,一旦預取到晶片實際容量之外,
 * QUADSPI 會等一個永遠不會來的回應而卡死。先全部擋掉、再只開實際存在
 * 的那幾 MB,就不會發生。
 *
 * @param size_bytes 實際容量,必須是 2 的冪次(8MB 或 16MB)。
 *                   傳 0 代表維持全部封鎖。
 */
void BSP_MPU_EnableQspiRegion(uint32_t size_bytes);

#endif /* BSP_MPU_H */
