/*
 * bsp_qspi.h
 *
 *  核心板上的 W25Qxx QSPI Flash,設定成記憶體映射模式。
 *
 *  用途:內部 2MB Flash 幾乎被 UI 圖片佔滿(5 張全螢幕背景就吃掉 1.25MB)。
 *  把圖片資料放到這顆外部 Flash,LVGL 可以像讀一般記憶體一樣直接從
 *  0x90000000 取用,內部 Flash 就只留給程式碼。
 *
 *  重要:FMC/SDRAM 沒有配置在 .ioc 裡,QSPI 也一樣。CubeMX 不知道下面這些腳
 *  已經被佔用:
 *
 *    PB2  QUADSPI_CLK      (AF9)
 *    PB6  QUADSPI_BK1_NCS  (AF10)
 *    PF6  QUADSPI_BK1_IO3  (AF9)
 *    PF7  QUADSPI_BK1_IO2  (AF9)
 *    PF8  QUADSPI_BK1_IO0  (AF10)
 *    PF9  QUADSPI_BK1_IO1  (AF10)
 *
 *  這幾支和 SDRAM 不衝突 —— FMC 用的是 PF0~PF5 與 PF11~PF15。
 */

#ifndef BSP_QSPI_H
#define BSP_QSPI_H

#include <stdbool.h>
#include <stdint.h>

/** 記憶體映射之後,Flash 內容出現在這個位址。 */
#define QSPI_BASE_ADDR   0x90000000UL

/**
 * 初始化 QUADSPI、讀取 JEDEC ID 判斷容量、開啟 Quad 模式,
 * 最後切到記憶體映射。
 *
 * 失敗不會停機。這顆 Flash 目前是備援用途,沒有它儀表照樣能跑,所以
 * 這裡刻意不呼叫 Error_Handler() —— 不值得為了一顆還沒用到的晶片
 * 讓整個儀表板黑掉。所有等待都有逾時保護。
 *
 * @return 成功並完成映射時回傳 true。
 */
bool BSP_QSPI_Init(void);

/**
 * 偵測到的 Flash 容量(bytes)。尚未初始化或偵測失敗時回傳 0。
 * W25Q64 = 8MB,W25Q128 = 16MB。
 */
uint32_t BSP_QSPI_GetFlashSize(void);

/**
 * 讀到的 JEDEC ID(0xEF40xx 之類)。除錯用。
 */
uint32_t BSP_QSPI_GetJedecId(void);

#endif /* BSP_QSPI_H */
