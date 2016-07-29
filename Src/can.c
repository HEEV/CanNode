/* can.c -- shamelessly stolen from the CANtact development pageL_CAN_ConfigFilter(&hcan, &filter);
 * implementations of can functions defined in can.h
 */
#include <stm32f0xx_hal.h>
#include "mxconstants.h"
#include "../Inc/can.h"

CAN_HandleTypeDef hcan;
CAN_FilterConfTypeDef filter;
static uint32_t prescaler;
static CanState bus_state;

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
	bus_state = BUS_OFF;
}

void can_enable(void) {
	if (bus_state == BUS_OFF) {
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
		bus_state = BUS_OK;
	}
}

void can_set_bitrate(canBitrate bitrate) {
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

/**
 * \param id id to filter on 
 *
 * \returns the filter number of the added filter returns \ref CAN_FILTER_ERROR
 * if the function was unable to add a filter.
 */
uint16_t can_add_filter_id(uint16_t id) {
	uint8_t i, fltr_num;
	const int MAX_FILTER = 12;
	//enable filter modification
	CAN->FMR |= CAN_FMR_FINIT;
	
	//loop through filter banks to find an empty filter register
	for(fltr_num=i=0; i<MAX_FILTER; i++){
		//this is a register currently in use, check if there is any openings
		if(CAN->FA1R & (1<<i)){ 
			//check if bank is using a mask or a id list
			if(CAN->FM1R & (1<<i)){
				//id list
				//set FLTR to point at the first filter register
				uint16_t* FLTR = (uint16_t*) &CAN->sFilterRegister[i].FR1;
				for(uint8_t j=0; j<4; j++, fltr_num++, FLTR++){
					if((*FLTR & (0x7FF<<5)) == (id<<5)){ //same as an old filter
						//do nothing
						return fltr_num;
					}
					else if((*FLTR & (0x7FF<<5)) == (0x7FF<<5)){ //unused
						*FLTR=0;//reset filter
						//add id
						*FLTR = (id<<5);

						//enable filter running
						CAN->FMR &= ~CAN_FMR_FINIT;
						return fltr_num;
					}
				}
			}
			else {
				//add the number of filters in an id-mask to the filter number
				fltr_num+=2;
			}
		}
		else {
			//register not in use, set it up.
			CAN->FA1R |= (1<<i); //using bank
			CAN->FM1R |= (1<<i); //id list

			//set the first postition to the id and the rest to unused
			CAN->sFilterRegister[i].FR1=(id<<5)    | (0x7FF<<21);
			CAN->sFilterRegister[i].FR2=(0x7FF<<5) | (0x7FF<<21);
			
			//enable filter running
			CAN->FMR &= ~CAN_FMR_FINIT;

			//return the filter number
			return fltr_num;
		}
	}
	
	return CAN_FILTER_ERROR;
}

/**
 * *NOTE:
 * This function takes some finagleing in order for it to work correctly with
 * the %CanNode library.
 * For it to work correctly the returned value from this function should be 
 * passed to CanNode_addFilter() as the id. This lets CanNode_checkForMessages() 
 * know what handler to call if a message using this filter is recieved.*
 * 
 * Example code
 *
 * ~~~~~~~~~~~~ {.c}
 * uint16_t id = can_add_filter_mask(id_to_filter, id_mask);
 * CanNode_addFilter(id, handler);
 * ~~~~~~~~~~~~
 * 
 * \param id base id of the filter mask
 * \param mask mask on top of the base id, 0's are don't cares
 *
 * \returns the filter number of the added filter returns \ref CAN_FILTER_ERROR
 * if the function was unable to add a filter.
 */
uint16_t can_add_filter_mask(uint16_t id, uint16_t mask) {
	uint8_t i, fltr_num;
	const int MAX_FILTER = 12;
	//enable filter modification
	CAN->FMR |= CAN_FMR_FINIT;
	
	//loop through filter banks to find an empty filter register
	for(fltr_num=i=0; i<MAX_FILTER; i++){
		//this is a register currently in use, check if there is any openings
		if(CAN->FA1R & (1<<i)){ 
			//check if bank is using a mask or a id list
			if((CAN->FM1R & (1<<i)) == 0){
				//mask
				//set FLTR to point at the first filter register
				uint32_t* FLTR = (uint32_t*) &CAN->sFilterRegister[i].FR1;
				for(uint8_t j=0; j<2; j++, fltr_num++, FLTR++){
					if((*FLTR & (0x7FF<<5)) == (0x7FF<<5)){ //unused
						*FLTR=0;//reset filter
						//add id
						*FLTR = (id<<5) | (mask<<21);

						//enable filter running
						CAN->FMR &= ~CAN_FMR_FINIT;
						return fltr_num;
					}
				}
			}
			else {
				//add the number of filters in an id-list to the filter number
				fltr_num+=4;
			}
		}
		else {
			//register not in use, set it up.
			CAN->FA1R |= (1<<i); //using bank
			CAN->FM1R &= ~(1<<i); //id mask 

			//set the first postition to the id and the rest to unused
			CAN->sFilterRegister[i].FR1=(id<<5)    | (mask <<21);
			CAN->sFilterRegister[i].FR2=(0x7FF<<5) | (0x7FF<<21);
			
			//enable filter running
			CAN->FMR &= ~CAN_FMR_FINIT;

			//return the filter number
			return fltr_num;
		}
	}

	return CAN_FILTER_ERROR;
}

CanState can_tx(CanMessage *tx_msg, uint32_t timeout) {
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
		return BUS_BUSY;
	}

	//add data to register
	CAN->sTxMailBox[mailbox].TIR = (uint32_t) tx_msg->id << 21;
	if(tx_msg->rtr){
		CAN->sTxMailBox[mailbox].TIR |= CAN_TI0R_RTR;
	}
	
	//set message length
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

	return BUS_OK;
}

CanState can_rx(CanMessage *rx_msg, uint32_t timeout) {
	uint8_t fifoNum = 0;

	//check if there is data in fifo0
	if((CAN->RF0R & CAN_RF0R_FMP0) == 0){ //if there is no data
		return NO_DATA;
	}

	//get data from regisers
	//get the id field
	rx_msg->id = (uint16_t) (CAN->sFIFOMailBox[fifoNum].RIR >> 21);

	//check if it is a rtr message
	rx_msg->rtr = false;
	if(CAN->sFIFOMailBox[fifoNum].RIR & CAN_RI0R_RTR){ 
		rx_msg->rtr = true;
	}
	
	//get data length
	rx_msg->len = (uint8_t) (CAN->sFIFOMailBox[fifoNum].RDTR & CAN_RDT0R_DLC);
	
	//get filter mask index
	rx_msg->fmi = (uint8_t) (CAN->sFIFOMailBox[fifoNum].RDTR >> 8);

	//get the data
    for(uint8_t i=0; i<4; ++i) {
		rx_msg->data[i+4] = (uint8_t) (CAN->sFIFOMailBox[fifoNum].RDHR >> (8*i));
		rx_msg->data[i]   = (uint8_t) (CAN->sFIFOMailBox[fifoNum].RDLR >> (8*i));
	}

	//clear fifo
	CAN->RF0R |= CAN_RF0R_RFOM0;

	return BUS_OK;
}

uint8_t is_can_msg_pending(uint8_t fifo) {
	if (bus_state == BUS_OFF) {
		return 0;
	}
	return (__HAL_CAN_MSG_PENDING(&hcan, fifo) > 0);
}
