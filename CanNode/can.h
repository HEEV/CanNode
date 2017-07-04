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

#ifndef _CAN_DRV_H
#define _CAN_DRV_H


#include "CanTypes.h"

#define HAL_Delay(ms_delay) (usleep(ms_delay * 1000))
uint32_t HAL_GetTick();

/// \brief Initilize CAN hardware.
void can_init(void);
/// \brief Enable CAN hardware.
void can_enable(void);
/// \brief Put CAN hardware to sleep.
void can_sleep(void);
/// \brief Set the speed of the CANBus.
void can_set_bitrate(canBitrate bitrate);

/// \brief Add a filter to the can hardware with an id
uint16_t can_add_filter_id(uint16_t id);
/// \brief Add a filter to the can hardware with a mask
uint16_t can_add_filter_mask(uint16_t id, uint16_t mask);

/// \brief Send a CanMessage over the bus.
CanState can_tx(CanMessage *tx_msg, uint32_t timeout);
/// \brief Get a CanMessage from the hardware if it is availible.
CanState can_rx(CanMessage *rx_msg, uint32_t timeout);
/// \brief Check if a new message is avalible.
bool is_can_msg_pending();

#endif // _CAN_H
