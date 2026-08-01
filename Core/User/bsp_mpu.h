/*
 * bsp_mpu.h
 *
 *  MPU 區域設定 + 開啟 CPU cache。
 */

#ifndef BSP_MPU_H
#define BSP_MPU_H

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

#endif /* BSP_MPU_H */
