/*
 * can_rx.c
 *
 *  單一生產者(FDCAN 中斷)、單一消費者(主迴圈)的環形佇列。
 *
 *  因為生產者和消費者各自只動一個索引,而且兩個索引都是 32-bit 對齊的
 *  volatile 變數(在 Cortex-M7 上讀寫是原子的),所以不需要關中斷或加鎖。
 */

#include "can_rx.h"
#include "stm32h7xx_hal.h"
#include <string.h>

static ttr_can_frame_t   s_queue[CAN_RX_QUEUE_LEN];
static volatile uint32_t s_head;              /* 下一個要寫入的位置,只有 ISR 會改 */
static volatile uint32_t s_tail;              /* 下一個要讀出的位置,只有主迴圈會改 */
static volatile uint32_t s_overflow_count;
static volatile uint32_t s_last_frame_tick;

static inline uint32_t next_index(uint32_t i)
{
    return (i + 1u) % CAN_RX_QUEUE_LEN;
}

bool CAN_RX_Enqueue(const ttr_can_frame_t *frame)
{
    const uint32_t head = s_head;
    const uint32_t next = next_index(head);

    s_last_frame_tick = HAL_GetTick();

    if (next == s_tail) {
        /* 佇列滿了。寧可丟掉新的一包,也不要覆寫主迴圈還沒處理完的舊資料。 */
        s_overflow_count++;
        return false;
    }

    s_queue[head] = *frame;

    /* 資料寫完之後才推進 head,消費者才不會看到只寫了一半的格子。 */
    __DMB();
    s_head = next;

    return true;
}

bool CAN_RX_Dequeue(ttr_can_frame_t *frame)
{
    const uint32_t tail = s_tail;

    if (tail == s_head) {
        return false;
    }

    *frame = s_queue[tail];

    __DMB();
    s_tail = next_index(tail);

    return true;
}

uint32_t CAN_RX_OverflowCount(void)
{
    return s_overflow_count;
}

uint32_t CAN_RX_LastFrameTick(void)
{
    return s_last_frame_tick;
}

bool CAN_RX_IsLinkStale(uint32_t timeout_ms)
{
    /* 開機到現在都還沒收過任何一包,也算不健康。 */
    if (s_last_frame_tick == 0u) {
        return true;
    }

    return (HAL_GetTick() - s_last_frame_tick) > timeout_ms;
}
