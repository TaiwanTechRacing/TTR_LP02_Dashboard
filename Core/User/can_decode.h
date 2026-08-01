/*
 * can_decode.h
 */

#ifndef CAN_DECODE_H
#define CAN_DECODE_H

/**
 * 把 can_rx 佇列裡累積的所有 CAN 訊息解包寫進 vehicle_data。
 * 主迴圈每圈呼叫一次。
 */
void CAN_Poll(void);

#endif /* CAN_DECODE_H */
