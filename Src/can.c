/* can.c -- shamelessly stolen from the CANtact development page
 * implementations of can functions defined in can.h
 */
#include <stm32f0xx_hal.h>
#include "mxconstants.h"
#include "can.h"

CAN_HandleTypeDef hcan;
CAN_FilterConfTypeDef filter;
uint32_t prescaler;
can_bus_state bus_state;

void can_init(void) {
	filter.FilterIdHigh = 0;
	filter.FilterIdLow = 0;
	filter.FilterMaskIdHigh = 0;
	filter.FilterMaskIdLow = 0;
	filter.FilterMode = CAN_FILTERMODE_IDMASK;
	filter.FilterScale = CAN_FILTERSCALE_32BIT;
	filter.FilterNumber = 0;
	filter.FilterFIFOAssignment = CAN_FIFO0;
	filter.BankNumber = 0;
	filter.FilterActivation = ENABLE;

	// default to 125 kbit/s
	prescaler = 48;
	hcan.Instance = CAN;
	bus_state = OFF_BUS;
}

void can_enable(void) {
	if (bus_state == OFF_BUS) {
		hcan.Init.Prescaler = prescaler;
		hcan.Init.Mode = CAN_MODE_NORMAL;
		hcan.Init.SJW = CAN_SJW_1TQ;
		hcan.Init.BS1 = CAN_BS1_4TQ;
		hcan.Init.BS2 = CAN_BS2_3TQ;
		hcan.Init.TTCM = DISABLE;
		hcan.Init.ABOM = DISABLE;
		hcan.Init.AWUM = DISABLE;
		hcan.Init.NART = DISABLE;
		hcan.Init.RFLM = DISABLE;
		hcan.Init.TXFP = DISABLE;
		hcan.pTxMsg = NULL;
		HAL_CAN_Init(&hcan);
		HAL_CAN_ConfigFilter(&hcan, &filter);
		bus_state = ON_BUS;
	}
}

void can_set_bitrate(can_bitrate bitrate) {
	switch(bitrate) {
		case CAN_BITRATE_10K:
			prescaler = 600;
			break;
		case CAN_BITRATE_20K:
			prescaler = 300;
			break;
		case CAN_BITRATE_50K:
			prescaler = 120;
			break;
		case CAN_BITRATE_100K:
			prescaler = 60;
			break;
		case CAN_BITRATE_125K:
			prescaler = 48;
			break;
		case CAN_BITRATE_250K:
			prescaler = 24;
			break;
		case CAN_BITRATE_500K:
			prescaler = 12;
			break;
		case CAN_BITRATE_750K:
			prescaler = 8;
			break;
		case CAN_BITRATE_1000K:
			prescaler = 6;
			break;
	}
}

void can_set_silent(uint8_t silent) {
	if (bus_state == ON_BUS) {
		// cannot set silent mode while on bus
		return;
	}
	if (silent) {
		hcan.Init.Mode = CAN_MODE_SILENT;
	} else {
		hcan.Init.Mode = CAN_MODE_NORMAL;
	}
}

uint32_t can_tx(CanMessage *tx_msg, uint32_t timeout) {
	//TODO do hardware stuff here
	CanTxMsgTypeDef temp;
	uint32_t status;

	temp.StdId = tx_msg->id;
	temp.ExtId = 0;
	temp.RTR = (tx_msg->rtr) ? CAN_RTR_REMOTE : CAN_RTR_DATA;
	temp.IDE = CAN_ID_STD;
	temp.DLC = tx_msg->len;
    for(uint8_t i=0; i<8; ++i){
		temp.Data[i] = tx_msg->data[i];
	}
	// transmit can frame
	hcan.pTxMsg = &temp;
	status = HAL_CAN_Transmit(&hcan, timeout);

	return status;
}

uint32_t can_rx(CanMessage *rx_msg, uint32_t timeout) {
    CanRxMsgTypeDef temp;
	uint32_t status;
	hcan.pRxMsg = &temp;

	status = HAL_CAN_Receive(&hcan, CAN_FIFO0, timeout);

	//transfer to struct
	rx_msg->id = temp.StdId; 
	rx_msg->rtr = temp.RTR==CAN_RTR_REMOTE;
	rx_msg->len = temp.DLC;

    for(uint8_t i=0; i<8; ++i){
		rx_msg->data[i] = temp.Data[i];
	}

	return status;
}

uint8_t is_can_msg_pending(uint8_t fifo) {
	if (bus_state == OFF_BUS) {
		return 0;
	}
	return (__HAL_CAN_MSG_PENDING(&hcan, fifo) > 0);
}
