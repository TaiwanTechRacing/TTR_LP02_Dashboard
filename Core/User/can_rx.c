/*
 * can_rx.c
 *
 *  Single-producer (FDCAN ISR) / single-consumer (main loop) ring buffer.
 *
 *  No locking or interrupt masking is required: each side only advances its
 *  own index, and both indices are 32-bit aligned volatiles, which the
 *  Cortex-M7 reads and writes atomically.
 */

#include "can_rx.h"
#include "stm32h7xx_hal.h"
#include <string.h>

static ttr_can_frame_t   s_queue[CAN_RX_QUEUE_LEN];
static volatile uint32_t s_head;              /* next write slot, ISR only */
static volatile uint32_t s_tail;              /* next read slot, main loop only */
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
        /* Full. Drop the new frame rather than overwrite older data the main
         * loop has not consumed yet. */
        s_overflow_count++;
        return false;
    }

    s_queue[head] = *frame;

    /* Publish the slot only after it is fully written, so the consumer never
     * sees a half-filled entry. */
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
    /* Nothing received since boot counts as unhealthy too. */
    if (s_last_frame_tick == 0u) {
        return true;
    }

    return (HAL_GetTick() - s_last_frame_tick) > timeout_ms;
}
