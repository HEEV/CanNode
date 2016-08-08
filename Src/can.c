/* can.c -- shamelessly stolen from the CANtact development pageL_CAN_ConfigFilter(&hcan, &filter);
 * implementations of can functions defined in can.h
 * Modified extensively by Samuel Ellicott to use hardware registers instead
 * of HAL library.
 */
#include "mxconstants.h"
#include "../Inc/can.h"

static CAN_HandleTypeDef hcan;
static uint32_t prescaler;
static CanState bus_state;
static uint8_t num_msg;

void can_init(void) {
	// default to kbit/s
	can_set_bitrate(CAN_BITRATE_125K);
	hcan.Instance = CAN; //this is for convinience debugging
	num_msg = 0;
	bus_state = BUS_OFF;
}

static inline void can_io_init() {
	//enable gpioa clock
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	/* CAN GPIO Configuration    
	 * PA11     ------> CAN_RX
	 * PA12     ------> CAN_TX 
	 */
	//altranate function pins
	GPIOA->MODER |= GPIO_MODER_MODER12_1 | GPIO_MODER_MODER11_1;
	//high speed pins
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR12 | GPIO_OSPEEDR_OSPEEDR11; 
	//AF4 (CAN)	
	GPIOA->AFR[1] |= 4 << 16 | 4 << 12;
}

void can_enable(void) {
	if (bus_state == BUS_OFF) {

		//enable CAN clock
		RCC->APB1ENR |= RCC_APB1ENR_CANEN;
		can_io_init();

		//Enter CAN init mode to write the configuration
		CAN->MCR |= CAN_MCR_INRQ;
		// Wait for the hardware to initilize
		while ((CAN->MSR & CAN_MSR_INAK) != CAN_MSR_INAK);
		//Exit sleep mode
		CAN->MCR &= ~CAN_MCR_SLEEP;
		// Setup timing: BS1 = 4 time quanta (3+1), BS2 = 3 time quanta (2+1).
		// The prescalar is set to whatever it was set to from can_set_bitrate()
		CAN->BTR =  2 << 20 | 3 << 16 | (prescaler-1);

		CAN->MCR &= ~CAN_MCR_INRQ;/* Leave init mode */
		/* Wait the init mode leaving */
		while ((CAN->MSR & CAN_MSR_INAK) == CAN_MSR_INAK);

		/* Set FIFO0 message pending IT enable */
		//CAN->IER |= CAN_IER_FMPIE0;

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

	CAN_FilterConfTypeDef filter;
	uint8_t bank_num, fltr_num;
	const int MAX_FILTER = 12;

	//mostly setup filter
	filter.FilterIdLow = 0;
	filter.FilterIdHigh = 0;
	filter.FilterMaskIdLow = 0;
	filter.FilterMaskIdHigh = 0;
	filter.FilterMode = CAN_FILTERMODE_IDLIST;
	filter.FilterScale = CAN_FILTERSCALE_16BIT;
	filter.FilterFIFOAssignment = CAN_FIFO0;
	filter.BankNumber = 0;
	filter.FilterActivation = ENABLE;

	//loop through filter banks to find an empty filter register
	for(fltr_num=bank_num=0; bank_num<MAX_FILTER; bank_num++){
		//this is a register currently in use, check if there is any openings
		if(CAN->FA1R & (1<<bank_num)){ 
			//check if bank is using a mask or a id list
			if(CAN->FM1R & (1<<bank_num)){
				//id list

				filter.FilterNumber = bank_num;
				//if we are here then the first slot has already been filled
				filter.FilterIdLow = CAN->sFilterRegister[bank_num].FR1;
				++fltr_num;

				//check if second slot has been filled
				if(CAN->sFilterRegister[bank_num].FR1>>16){
					filter.FilterIdHigh = CAN->sFilterRegister[bank_num].FR1>>16;
					++fltr_num;
				}
				else {
					filter.FilterIdHigh = (id << 5);
					HAL_CAN_ConfigFilter(&hcan, &filter);
					return fltr_num;
				}

				if(CAN->sFilterRegister[bank_num].FR2){
					filter.FilterMaskIdLow = CAN->sFilterRegister[bank_num].FR2;
					++fltr_num;
				}
				else{
					filter.FilterMaskIdLow = (id << 5);
					HAL_CAN_ConfigFilter(&hcan, &filter);
					return fltr_num;
				}

				if((CAN->sFilterRegister[bank_num].FR2>>16)==0){
					//last slot empty
					filter.FilterMaskIdHigh = (id << 5);
					HAL_CAN_ConfigFilter(&hcan, &filter);
					return fltr_num;
				}
				//no empty slots incriment number, should be 4 greater than when
				//started
				++fltr_num;

			}
			else {
				//add the number of filters in an id-mask to the filter number
				fltr_num+=2;
			}
		}
		else {

			filter.FilterIdLow = (id << 5);
			filter.FilterNumber = bank_num;

			HAL_CAN_ConfigFilter(&hcan, &filter);
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
	CAN_FilterConfTypeDef filter;
	uint8_t bank_num, fltr_num;
	const int MAX_FILTER = 12;

	//mostly setup filter
	filter.FilterIdLow = 0;
	filter.FilterIdHigh = 0;
	filter.FilterMaskIdLow = 0;
	filter.FilterMaskIdHigh = 0;
	filter.FilterMode = CAN_FILTERMODE_IDMASK;
	filter.FilterScale = CAN_FILTERSCALE_16BIT;
	filter.FilterFIFOAssignment = CAN_FIFO0;
	filter.BankNumber = 0;
	filter.FilterActivation = ENABLE;

	//loop through filter banks to find an empty filter register
	for(fltr_num=bank_num=0; bank_num<MAX_FILTER; bank_num++){
		//this is a register currently in use, check if there is any openings
		if(CAN->FA1R & (1<<bank_num)){ 
			//check if bank is using a mask or a id list
			if((CAN->FM1R & (1<<bank_num)) == 0){
				//id mask 
			
				//if we are here then the first slot has been taken. fill it
				//from the registers
				filter.FilterIdLow = CAN->sFilterRegister[bank_num].FR1;
				filter.FilterIdHigh = (CAN->sFilterRegister[bank_num].FR1>>16);

				//incriment filter index
				++fltr_num;

				//fill the new slot
				filter.FilterMaskIdLow = (id << 5);
				filter.FilterMaskIdHigh = (mask << 5);

				//configure it
				HAL_CAN_ConfigFilter(&hcan, &filter);
			}
			else {
				//add the number of filters in an id-list to the filter number
				fltr_num += 4;
			}
		}
		else {

			filter.FilterIdLow = (id << 5);
			filter.FilterIdHigh = (mask << 5);
			filter.FilterNumber = bank_num;

			HAL_CAN_ConfigFilter(&hcan, &filter);
			//return the filter number
			return fltr_num;
		}
	}

	return CAN_FILTER_ERROR;
}

CanState can_tx(CanMessage *tx_msg, uint32_t timeout) { uint8_t mailbox; //find an empty mailbox
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
	--num_msg;

	return BUS_OK;
}

bool is_can_msg_pending() {
	return ((CAN->RF0R & CAN_RF0R_FMP0) > 0); //if there is no data
}
