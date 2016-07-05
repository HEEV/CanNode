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

can_bus_state can_tx(CanMessage *tx_msg, uint32_t timeout) {
	uint8_t mailbox;

	//find an empty mailbox
	for(mailbox=0; mailbox<3; ++mailbox){
		//check the status
		if(CAN->sTxMailBox[mailbox].TIR & CAN_TI0R_TXRQ) {
		   	continue; 
		}
		else {
			break;//found open mailbox
		}
	}

	//if there are no open mailboxes
	if(mailbox == 3) {
		return BUSY_BUS;
	}

	//add data to register
	CAN->sTxMailBox[mailbox].TIR = (uint32_t) tx_msg->id << 21;
	if(tx_msg->rtr){
		CAN->sTxMailBox[mailbox].TIR |= CAN_TI0R_RTR;
	}
	
	CAN->sTxMailBox[mailbox].TDTR = tx_msg->len & 0x0F;

	//clear mailbox and add new data
	CAN->sTxMailBox[mailbox].TDHR = 0;
	CAN->sTxMailBox[mailbox].TDLR = 0;
    for(uint8_t i=0; i<4; ++i){
		CAN->sTxMailBox[mailbox].TDHR |= tx_msg->data[i+4] << (8*i);
		CAN->sTxMailBox[mailbox].TDLR |= tx_msg->data[i] << (8*i);
	}
	//transmit can frame
	CAN->sTxMailBox[mailbox].TIR |= CAN_TI0R_TXRQ;

	return ON_BUS;
}

uint32_t can_rx(CanMessage *rx_msg, uint32_t timeout) {
	//TODO write code to check in both fifos
	uint8_t fifoNum = 0;

	//check if there is data in fifo0
	if((CAN->RF0R & CAN_RF0R_FMP0) == 0){ //if there is no data
		return NO_DATA;
	}

	//get data from regisers
	//get the id field
	rx_msg->id = (uint16_t) CAN->sFIFOMailBox[fifoNum].RIR >> 21;

	//check if it is a rtr message
	rx_msg->rtr = false;
	if(CAN->sFIFOMailBox[fifoNum].RIR & CAN_RI0R_RTR){ 
		rx_msg->rtr = true;
	}
	
	//get data length
	rx_msg->len = (uint8_t) CAN->sFIFOMailBox[fifoNum].RDTR & CAN_RDT0R_DLC;

	//get the data
    for(uint8_t i=0; i<4; ++i){
		rx_msg->data[i+4] = (uint8_t) CAN->sFIFOMailBox[fifoNum].RDHR >> (8*i);
		rx_msg->data[i]   = (uint8_t) CAN->sFIFOMailBox[fifoNum].RDLR >> (8*i);
	}
	//clear fifo
	CAN->RF0R |= CAN_RF0R_RFOM0;

	return ON_BUS;
}

uint8_t is_can_msg_pending(uint8_t fifo) {
	if (bus_state == OFF_BUS) {
		return 0;
	}
	return (__HAL_CAN_MSG_PENDING(&hcan, fifo) > 0);
}
