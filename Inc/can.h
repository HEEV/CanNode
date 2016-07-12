/** 
 * \file can.h
 * \brief Low level functions for CAN
 * 
 * Provides low level functions for sending and reciving CanMessages.
 * These functions directly access stm32 hardware and should be reimplemented for
 * a PC/tablet application.
 *
 * \author Samuel Ellicott
 * \date 6-20-16
 */

#ifndef _CAN_H
#define _CAN_H

#include <stm32f0xx_hal.h>
#include <stdbool.h>
#include "CanTypes.h"

/// \brief Initilize CAN hardware.
void can_init(void);
/// \brief Enable CAN hardware.
void can_enable(void);
/// \brief Disable CAN hardware.
void can_disable(void);
/// \brief Set the speed of the CANBus.
void can_set_bitrate(canBitrate bitrate);
/// \brief make the CAN hardware silent on the bus.
void can_set_silent(uint8_t silent);
/// \brief Send a CanMessage over the bus.
CanState can_tx(CanMessage *tx_msg, uint32_t timeout);
/// \brief Get a CanMessage from the hardware if it is availible.
CanState can_rx(CanMessage *rx_msg, uint32_t timeout);
/// \brief Check if a new message is avalible.
uint8_t is_can_msg_pending(uint8_t fifo);

#endif // _CAN_H
