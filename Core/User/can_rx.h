/*
 * can_rx.h
 *
 *  CAN receive queue.
 *
 *  The original code unpacked entire messages inside the FDCAN ISR and wrote
 *  the results straight into a hundred-odd float globals. Two problems with
 *  that:
 *
 *   1. FDCAN2 runs at interrupt priority 0 (the highest) and unpacking is not
 *      short, so every arriving frame preempted whatever rendering was in
 *      flight. Under heavy bus traffic the display visibly stuttered.
 *   2. The only synchronisation between ISR and main loop was volatile, with
 *      no critical section. For a multi-field message like sensor2 the main
 *      loop could observe a mix of old and new field values.
 *
 *  The ISR now only pushes the frame into this queue, which takes a few dozen
 *  cycles. Unpacking happens in the main loop.
 */

#ifndef CAN_RX_H
#define CAN_RX_H

#include "ttr_can.h"
#include <stdbool.h>
#include <stdint.h>

/* 32 frames of headroom. At the current bus load the main loop drains this
 * every pass; the depth is only there to absorb bursts. */
#define CAN_RX_QUEUE_LEN 32u

/**
 * Push one frame. Called from the ISR only.
 * When the queue is full the new frame is dropped, the overflow counter is
 * incremented and false is returned.
 */
bool CAN_RX_Enqueue(const ttr_can_frame_t *frame);

/**
 * Pop one frame. Called from the main loop only. Returns false when empty.
 */
bool CAN_RX_Dequeue(ttr_can_frame_t *frame);

/**
 * Cumulative overflow count. A number that keeps climbing means the main loop
 * is too slow or the queue too shallow - a useful diagnostic.
 */
uint32_t CAN_RX_OverflowCount(void);

/**
 * HAL_GetTick() value when any CAN frame last arrived.
 */
uint32_t CAN_RX_LastFrameTick(void);

/**
 * Whether more than timeout_ms has passed since the last frame.
 *
 * This exists for a safety reason: after a CAN dropout the screen would keep
 * showing the last values received, so the RTD state, SDC status and battery
 * temperatures the driver sees are all stale. With this the UI can switch to
 * "---" or raise a warning instead.
 */
bool CAN_RX_IsLinkStale(uint32_t timeout_ms);

#endif /* CAN_RX_H */
