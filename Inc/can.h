#ifndef _CAN_H
#define _CAN_H

#include <stm32f0xx_hal.h>
#include <stdbool.h>

typedef enum {
	CAN_BITRATE_10K,
	CAN_BITRATE_20K,
	CAN_BITRATE_50K,
	CAN_BITRATE_100K,
	CAN_BITRATE_125K,
	CAN_BITRATE_250K,
	CAN_BITRATE_500K,
	CAN_BITRATE_750K,
	CAN_BITRATE_1000K,
} can_bitrate;

typedef enum {
	BUS_OK,
	BUS_BUSY,
	NO_DATA,
	BUS_OFF
} can_bus_state;

typedef struct {
	uint16_t id;
	uint8_t len;
	bool rtr;
	uint8_t data[8];
} CanMessage;	

void can_init(void);
void can_enable(void);
void can_disable(void);
void can_set_bitrate(can_bitrate bitrate);
void can_set_silent(uint8_t silent);
can_bus_state can_tx(CanMessage *tx_msg, uint32_t timeout);
uint32_t can_rx(CanMessage *rx_msg, uint32_t timeout);
uint8_t is_can_msg_pending(uint8_t fifo);

#endif // _CAN_H
