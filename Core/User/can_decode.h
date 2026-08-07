/*
 * can_decode.h
 */

#ifndef CAN_DECODE_H
#define CAN_DECODE_H

/**
 * Unpack every CAN message queued by can_rx into vehicle_data.
 * Called once per main loop iteration.
 */
void CAN_Poll(void);

#endif /* CAN_DECODE_H */
