#ifndef _CAN_H
#define _CAN_H

#include <stm32f0xx_hal.h>
#include <stdbool.h>
#include "CanTypes.h"

void can_init(void);
void can_enable(void);
void can_disable(void);
void can_set_bitrate(can_bitrate bitrate);
void can_set_silent(uint8_t silent);
can_bus_state can_tx(CanMessage *tx_msg, uint32_t timeout);
uint32_t can_rx(CanMessage *rx_msg, uint32_t timeout);
uint8_t is_can_msg_pending(uint8_t fifo);

#endif // _CAN_H
