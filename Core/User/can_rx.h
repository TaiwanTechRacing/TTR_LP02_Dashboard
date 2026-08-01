/*
 * can_rx.h
 *
 *  CAN 接收佇列。
 *
 *  原本的寫法是在 FDCAN 中斷裡直接把整包解開、寫進上百個 float 全域變數。
 *  這有兩個問題:
 *
 *   1. FDCAN2 的中斷優先權是 0(最高),解包又不短,等於每來一包 CAN 就把
 *      正在跑的繪圖工作打斷一次。訊務量大的時候畫面會明顯卡頓。
 *   2. 中斷和主迴圈之間只靠 volatile,沒有任何臨界區。像 sensor2 那種一包
 *      帶好幾個欄位的訊息,主迴圈可能讀到「一半舊、一半新」的組合。
 *
 *  現在中斷只負責把 frame 丟進這個佇列(幾十個 cycle 就結束),解包搬到主
 *  迴圈做。
 */

#ifndef CAN_RX_H
#define CAN_RX_H

#include "ttr_can.h"
#include <stdbool.h>
#include <stdint.h>

/* 32 包的緩衝。以目前的 CAN 訊務量,主迴圈每圈都會清空,這個深度只是給
 * 突發流量留餘裕。 */
#define CAN_RX_QUEUE_LEN 32u

/**
 * 把一包 frame 放進佇列。只會從中斷呼叫。
 * 佇列滿的話丟棄最新這包並回傳 false(同時累加 overflow 計數)。
 */
bool CAN_RX_Enqueue(const ttr_can_frame_t *frame);

/**
 * 取出一包。只會從主迴圈呼叫。沒有資料時回傳 false。
 */
bool CAN_RX_Dequeue(ttr_can_frame_t *frame);

/**
 * 佇列溢位的累計次數。持續增加代表主迴圈跑太慢或佇列太淺,
 * 是很有用的診斷數字。
 */
uint32_t CAN_RX_OverflowCount(void);

/**
 * 最後一次收到任何 CAN 訊息的時間(HAL_GetTick 的毫秒數)。
 */
uint32_t CAN_RX_LastFrameTick(void);

/**
 * 距離上一包 CAN 是否已經超過 timeout_ms。
 *
 * 這是為了解決一個安全性問題:CAN 斷線之後,螢幕會一直顯示斷線前的最後一組
 * 數值,車手看到的 RTD、SDC 狀態、電池溫度全都是假的。有了這個判斷,UI 就
 * 可以把過期的數值改成 "---" 或跳警告。
 */
bool CAN_RX_IsLinkStale(uint32_t timeout_ms);

#endif /* CAN_RX_H */
