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
#include "platform.h"

uint32_t HAL_GetTick();

/// \brief Initilize CAN hardware.
void can_init(void);

/// \brief Add a filter to the can hardware with an id
fmi_ret_t can_add_filter_id(filter_id_t id1,
                            filter_id_t id2,
                            filter_id_t id3,
                            filter_id_t id4,
                            uint8_t filter_bank,
                            uint8_t fifo_num = 0);

/// \brief Add a filter to the can hardware with a mask
fmi_ret_t can_add_filter_mask(filter_id_mask_t id1,
                              filter_id_mask_t id2,
                              uint8_t filter_bank,
                              uint8_t fifo_num = 1);

/// \brief Send a CanMessage over the bus.
CanState can_tx(CanMessage *tx_msg, uint32_t timeout);

/// \brief Get a CanMessage from the hardware if it is availible.
can_rx_ret_t can_rx(CanMessage *rx_msg, uint32_t timeout);

/// \brief Check if a new message is avalible.
int is_can_msg_pending();

void enable_notifications(bool en);

#endif // _CAN_H
