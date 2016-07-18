/**
 * CanNode.c
 * \brief implements functions for CanNode devices
 *
 * \author Samuel Ellicott
 * \date 6-20-16
 */
#include "../Inc/CanNode.h"
#define UNUSED_FILTER 0xFFFF


static CanNode nodes[MAX_NODES] __attribute__((section(".can")));
static bool newMessage; 
static CanMessage tmpMsg;

static void CanNode_nodeHandler(CanNode* node, CanMessage* msg);

/**
 * Initilizes an empty CanNode structure to the values provided and saves it
 * to flash. If a CanNode of the same type and id was previously saved to flash
 * that will be retrieved instead of making a new one. 
 * 
 * \param[in] type Gives the sensor type
 * \param[in] id CAN Address, use the CanNodeType type plus a constant.
 * \param[in] force Force the creation of a new node, if an old one is not found 
 * in flash memory create it. 
 *
 * \returns the address of a \ref CanNode struct that stores the can information.
 * This information is necessary for using any of the sendData functions
 */
CanNode* CanNode_init(CanNodeType type, uint16_t id, bool force) {
	static bool has_run = false;
	static uint8_t usedNodes = 0;

	//if this is the first run clear list of nodes
	if(!has_run){
		can_set_bitrate(CAN_BITRATE_500K);
		can_init();
		can_enable();
		newMessage=false;
		has_run = true;
	}

	//check if a node of that type exists
	for(uint8_t i=0; i<MAX_NODES; ++i){
		//check for an id and type that matches the ones specified
		if(nodes[i].id == id && nodes[i].sensorType == type){
			//its a match!
			//fill a spot in used nodes
			usedNodes |= 1<<i;
			return &nodes[i];
		}
	}
	if(!force) {
		//did not find anything
		return NULL;
	}

	//did we find a space?
	CanNode* node = NULL;
	//copy to ram while erasing
	CanNode nodebak[MAX_NODES];
	memcpy(nodebak, nodes, sizeof(CanNode)*MAX_NODES);

	flashUnlock();
	//erase flash
	flashErasePage((uint32_t) &nodes[0]);

	for(uint8_t i=0; i<MAX_NODES; ++i){
		//Found open space and we haven't stored our modified node yet.
		if((usedNodes & (1<<i)) == 0 && node == NULL){
			//found a happy place
			
			//sensor type
			flashWrite_16((uint32_t) &nodes[i].sensorType, type);

			//reset filters, filters should already be reset b/c flash erase
			//sets all values to 0xffff==UNUSED_FILTER.

			//set node to that id
			flashWrite_16((uint32_t) &nodes[i].id, id);

			//fill a spot in used nodes
			usedNodes |= 1<<i;
			node = &nodes[i];
		}
		else if(usedNodes & (1<<i)) { //fill the rest of the nodes
			//set id
			flashWrite_16((uint32_t) &nodes[i].id, nodebak[i].id);
			//set sensor type
			flashWrite_16((uint32_t) &nodes[i].sensorType, nodebak[i].sensorType);

			//set filters and handlers
			for(uint8_t j=0; j<NUM_FILTERS; ++j){
				flashWrite_16((uint32_t) &nodes[i].filters[j], nodebak[i].filters[j]);
				//write the filter handler
				flashWrite_32((uint32_t) &nodes[i].handle[j], 
						      (uint32_t) nodebak[i].handle[j]);
			}

			//copy name and info
			CanNode_setName(&nodes[i], &nodebak[i].nodeInfoBuff[0], MAX_NAME_LEN);
			CanNode_setInfo(&nodes[i], &nodebak[i].nodeInfoBuff[MAX_NAME_LEN], MAX_INFO_LEN);
		}
	}

	flashLock();
	return node;//returns null if no empty spots found
}

/**
 * Saves a filter id and a handler to a node in flash memory. The function also
 * accepts a function which gets called if a message from that id is avalible.
 *
 * \param[in,out] node pointer to a node saved in flash memory
 * \param[in] filter id of the device that should be handled by handle
 * \param[in] handle function used to handle the filter
 *
 * \returns true if the filter was added, false if otherwise.
 *
 * \see CanNode_init()
 */
bool CanNode_addFilter(CanNode* node, uint16_t filter, filterHandler handle) {
	if(filter > 0x7FF || handle == NULL){
		return false;
	}

	//add to the end of the list of filters... If there's room.
	for(uint8_t i=0; i<NUM_FILTERS; ++i){
		if(node->filters[i] == UNUSED_FILTER){
			//save the filter id
			flashWrite_16((uint32_t) &node->filters[i], filter);
			//save a pointer to the handler function
			flashWrite_32((uint32_t) &node->handle[i], (uint32_t) handle);
			
			//check if write worked
			if(node->handle[i] == handle){
				return true;
			}
			else{
				return false;
			}
			//return true; //Sucess! Filter has been added
		}
	}

	return false; //no empty slots
}

void CanNode_getName(uint16_t id, char* name, uint8_t buff_len, uint32_t timeout){
	CanMessage msg;
	uint32_t tickStart;
	//send a request to the specified id
	msg.id = id;
	msg.len = 1;
	msg.rtr = false;
	msg.data[0] = CAN_GET_NAME | (CAN_INT8 << 5);
	can_tx(&msg, 5);

	//get start time
	tickStart = HAL_GetTick();
	//TODO check if node is even there with an rtr
	
	char* namePtr = name;
	//keep collecting data until a null character is reached, buffer is full,
	//or a timeout condition is reached.
	while(namePtr-name < buff_len && HAL_GetTick()-tickStart < timeout){
		//wait for message or timeout
		while(!is_can_msg_pending(CAN_FIFO0) && HAL_GetTick()-tickStart < timeout);
		//get the next buffer
		can_rx(&msg, 5);
		//check if it is from our id
		if(msg.id != id || (msg.data[0] & 0x1F) != CAN_NAME_INFO){
			continue;
		}
		//get all the data from this buffer
		for(uint8_t i=0; i<msg.len-1 && namePtr-name<buff_len; ++namePtr, ++i){
			*namePtr = msg.data[i];
		}
	}
}

void CanNode_getInfo(uint16_t id, char* info, uint16_t buff_len, uint32_t timeout){
	CanMessage msg;
	uint32_t tickStart;
	//send a request to the specified id
	msg.id = id;
	msg.len = 1; msg.rtr = false;
	msg.data[0] = CAN_GET_INFO | (CAN_INT8 << 5);
	can_tx(&msg, 5);

	//get start time
	tickStart = HAL_GetTick();
	//TODO check if node is even there with an rtr
	
	char* namePtr = info;
	//keep collecting data until a null character is reached, buffer is full,
	//or a timeout condition is reached.
	while(namePtr-info< buff_len && HAL_GetTick()-tickStart < timeout){
		//wait for message or timeout
		while(!is_can_msg_pending(CAN_FIFO0) && HAL_GetTick()-tickStart < timeout);
		//get the next buffer
		can_rx(&msg, 5);
		//check if it is from our id
		if(msg.id != id || (msg.data[0] & 0x1F) != CAN_NAME_INFO){
			continue;
		}
		//get all the data from this buffer
		for(uint8_t i=0; i<msg.len-1 && namePtr-info<buff_len; ++namePtr, ++i){
			*namePtr = msg.data[i];
		}
	}
}

void CanNode_setName(const CanNode* node, const char* name, uint8_t buff_len) {
	//store the data in buffer into the space pointed to by node->name
	//this address space resides in flash so a special process is taken
	
	for(uint8_t i=0; 
		*name!='\0' && //is it a null character?
		*name!= '\377' && //is it empty flash?
		i<MAX_NAME_LEN && //are we out of range of the total buffer
		i<buff_len; //are we out of range of the passed buffer
	    i+=2){

		uint16_t chars = name[i] | (name[i+1] << 8);
		flashWrite_16((uint32_t) &node->nodeInfoBuff[i], chars);
	}
}	

void CanNode_setInfo(const CanNode* node, const char* info, uint8_t buff_len) {
	//store the data in buffer into the space pointed to by node->name
	//this address space resides in flash so a special process is taken
	
	
	for(uint8_t i=0; 
	    *info!='\0' && //is it a null character?
	    *info!= '\377' && //is it empty flash?
		i<MAX_INFO_LEN && //are we out of range of the total buffer
		i<buff_len; //are we out of range of the passed buffer
	    i+=2){

		uint16_t chars = info[i] | (info[i+1] << 8);
		flashWrite_16((uint32_t) &node->nodeInfoBuff[i+MAX_NAME_LEN], chars);
	}
}


void CanNode_sendName(const CanNode* node, uint16_t id){
	CanMessage msg;
	msg.id = id;
	msg.rtr = false;
	msg.data[0] = CAN_NAME_INFO | CAN_INT8 << 5;

	//fill buffers and send them
	uint8_t i=0;
	while(node->nodeInfoBuff[i] != '\0' && 
		  node->nodeInfoBuff[i] != '\377') {

		//break if end of name has been reached
		for(msg.len=1; msg.len<8 && i<MAX_NAME_LEN; ++msg.len, ++i){
			//set data
			msg.data[msg.len] = node->nodeInfoBuff[i];

			//check data
			if(msg.data[msg.len] == '\0' || msg.data[msg.len] == 0xff){
				break;
			}
		}
		if(msg.len>8){
			msg.len=8;
		}
		can_tx(&msg, 5);
	}
}
void CanNode_sendInfo(const CanNode* node, uint16_t id) {
	CanMessage msg;
	msg.id = id;
	msg.rtr = false;
	msg.data[0] = CAN_NAME_INFO | CAN_INT8 << 5;

	//fill buffers and send them
	uint8_t i=MAX_NAME_LEN;
	while(node->nodeInfoBuff[i] != '\0' && 
		  node->nodeInfoBuff[i] != '\377') {

		//break if end of name has been reached
		for(msg.len=1; msg.len<8 && i<TOTAL_INFO_LEN; ++msg.len, ++i){
			//set data
			msg.data[msg.len] = node->nodeInfoBuff[i];

			//check data
			if(msg.data[msg.len] == '\0' || msg.data[msg.len] == 0xff){
				break;
			}
		}
		if(msg.len>8){
			msg.len=8;
		}
		can_tx(&msg, 5);
	}
}

//getter and setter functions -------------------------------------------------

/**
 * \param node pointer to a CanNode
 * \param data data to send
 *
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_int32()
 * \see CanNode_sendData_uint32()
 */
void CanNode_sendData_int8(const CanNode* node, int8_t data) {
	CanMessage msg;
	//configuration byte
	msg.data[0] = (uint8_t) ((0x7 & CAN_INT8) << 5) | (0x1F & CAN_DATA);
	//data
	msg.data[1] = (uint8_t) data;
	//set other odds and ends
	msg.len = 2;
	msg.rtr = false;
	msg.id = node->id;
	can_tx(&msg, 5); 
}
/**
 * \param node pointer to a CanNode
 * \param data data to send
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_int32()
 * \see CanNode_sendData_uint32()
 */
void CanNode_sendData_uint8(const CanNode* node, uint8_t data) {
	CanMessage msg;
	//configuration byte
	msg.data[0] = (uint8_t) ((0x7 & CAN_UINT8) << 5) | (0x1F & CAN_DATA);
	//data
	msg.data[1] = data;
	//set other odds and ends
	msg.len = 2;
	msg.rtr = false;
	msg.id = node->id;
	can_tx(&msg, 5); 
}

/**
 * \param node pointer to a CanNode
 * \param data data to send
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_int32()
 * \see CanNode_sendData_uint32()
 */
void CanNode_sendData_int16(const CanNode* node, int16_t data) {
	CanMessage msg;
	//configuration byte
	msg.data[0] = (uint8_t) ((0x7 & CAN_INT16) << 5) | (0x1F & CAN_DATA);
	//data
	msg.data[1] = (uint8_t)  (data & 0x00ff);
	msg.data[2] = (uint8_t) ((data & 0xff00) >> 8);
	//set other odds and ends
	msg.len = 3;
	msg.rtr = false;
	msg.id = node->id;
	can_tx(&msg, 5); 
}

/**
 * \param node pointer to a CanNode
 * \param data data to send
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_int32()
 * \see CanNode_sendData_uint32()
 */
void CanNode_sendData_uint16(const CanNode* node, uint16_t data) {
	CanMessage msg;
	//configuration byte
	msg.data[0] = (uint8_t) ((0x7 & CAN_UINT16) << 5) | (0x1F & CAN_DATA);
	//data
	msg.data[1] = (uint8_t)  (data & 0x00ff);
	msg.data[2] = (uint8_t) ((data & 0xff00) >> 8);
	//set other odds and ends
	msg.len = 3;
	msg.rtr = false;
	msg.id = node->id;
	can_tx(&msg, 5); 
}

/**
 * \param node pointer to a CanNode
 * \param data data to send
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_uint32()
 */
void CanNode_sendData_int32(const CanNode* node, int32_t data) {
	CanMessage msg;
	//configuration byte
	msg.data[0] = (uint8_t) ((0x7 & CAN_INT32) << 5) | (0x1F & CAN_DATA);
	//data
	msg.data[1] = (uint8_t)  (data & 0x000000ff);
	msg.data[2] = (uint8_t) ((data & 0x0000ff00) >> 8);
	msg.data[3] = (uint8_t) ((data & 0x00ff0000) >> 16);
	msg.data[4] = (uint8_t) ((data & 0xff000000) >> 24);
	//set other odds and ends
	msg.len = 5;
	msg.rtr = false;
	msg.id = node->id;
	can_tx(&msg, 5); 
}

/**
 * \param node pointer to a CanNode
 * \param data data to send
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_int32()
 */
void CanNode_sendData_uint32(const CanNode* node, uint32_t data) {
	CanMessage msg;
	//configuration byte
	msg.data[0] = (uint8_t) ((0x7 & CAN_UINT32) << 5) | (0x1F & CAN_DATA);
	//data
	msg.data[1] = (uint8_t)  (data & 0x000000ff);
	msg.data[2] = (uint8_t) ((data & 0x0000ff00) >> 8);
	msg.data[3] = (uint8_t) ((data & 0x00ff0000) >> 16);
	msg.data[4] = (uint8_t) ((data & 0xff000000) >> 24);
	//set other odds and ends
	msg.len = 5;
	msg.rtr = false;
	msg.id = node->id;
	can_tx(&msg, 5); 
}

/**
 * Sends an array of data over the CANBus.
 * Maximum size for the aray is 7 bytes.
 *
 * \param node Pointer to a CanNode
 * \param data An array of data
 * \param len  Length of the data to be sent. Maximum length of 7
 *
 * \returns \ref DATA_OVERFLOW if len > 7, \ref DATA_OK otherwise
 *
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_int16()
 * \see CanNode_sendDataArr_uint16()
 */
CanState CanNode_sendDataArr_int8(const CanNode* node, int8_t* data, uint8_t len) {
	CanMessage msg;
	//check if valid
	if(len>7){
		return DATA_OVERFLOW;
	}

	//configuration byte
	msg.data[0] = (uint8_t) ((0x7 & CAN_INT8) << 5) | (0x1F & CAN_DATA);
	//data
	for(uint8_t i=0; i<len; ++i){
		msg.data[i+1] = (uint8_t) data[i];
	}

	//set other odds and ends
	msg.len = len+1;
	msg.rtr = false;
	msg.id = node->id;
	can_tx(&msg, 5); 
	return DATA_OK;
}

/**
 * Sends an array of data over the CANBus.
 * Maximum size for the aray is 7 bytes.
 *
 * \param node Pointer to a CanNode
 * \param data An array of data
 * \param len  Length of the data to be sent. Maximum length of 7
 *
 * \returns \ref DATA_OVERFLOW if len > 7, \ref DATA_OK otherwise
 *
 * \see CanNode_sendDataArr_int8()
 * \see CanNode_sendDataArr_int16()
 * \see CanNode_sendDataArr_uint16()
 */
CanState CanNode_sendDataArr_uint8(const CanNode* node, uint8_t* data, uint8_t len) {
	CanMessage msg;
	//check if valid
	if(len>7){
		return DATA_OVERFLOW;
	}

	//configuration byte
	msg.data[0] = (uint8_t) ((0x7 & CAN_UINT8) << 5) | (0x1F & CAN_DATA);
	//data
	for(uint8_t i=0; i<len; ++i){
		msg.data[i+1] = data[i];
	}

	//set other odds and ends
	msg.len = len+1;
	msg.rtr = false;
	msg.id = node->id;
	can_tx(&msg, 5); 
	return DATA_OK;
}

/**
 * Sends an array of data over the CANBus.
 * Maximum size for the aray is 2 integers.
 *
 * \param node Pointer to a CanNode
 * \param data An array of data
 * \param len  Length of the data to be sent. Maximum length of 2
 *
 * \returns \ref DATA_OVERFLOW if len > 2, \ref DATA_OK otherwise
 *
 * \see CanNode_sendDataArr_int8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_uint16()
 */
CanState CanNode_sendDataArr_int16(const CanNode* node, int16_t* data, uint8_t len) {
	CanMessage msg;
	//check if valid
	if(len>2){
		return DATA_OVERFLOW;
	}

	//configuration byte
	msg.data[0] = (uint8_t) ((0x7 & CAN_INT16) << 5) | (0x1F & CAN_DATA);
	//data
	for(uint8_t i=0; i<len; ++i){
		msg.data[i*2+1] = (uint8_t)  (data[i] & 0x00ff);
		msg.data[i*2+2] = (uint8_t) ((data[i] & 0xff00) >> 8);
	}

	//set other odds and ends
	msg.len = len*2 +1;
	msg.rtr = false;
	msg.id = node->id;
	can_tx(&msg, 5); 
	return DATA_OK;
}

/**
 * Sends an array of data over the CANBus.
 * Maximum size for the aray is 2 integers.
 *
 * \param node Pointer to a CanNode
 * \param data An array of data
 * \param len  Length of the data to be sent. Maximum length of 2
 *
 * \returns \ref DATA_OVERFLOW if len > 2, \ref DATA_OK otherwise
 *
 * \see CanNode_sendDataArr_int8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_int16()
 */
CanState CanNode_sendDataArr_uint16(const CanNode* node, uint16_t* data, uint8_t len) {
	CanMessage msg;
	//check if valid
	if(len>2){
		return DATA_OVERFLOW;
	}

	//configuration byte
	msg.data[0] = (uint8_t) ((0x7 & CAN_UINT16) << 5) | (0x1F & CAN_DATA);
	//data
	for(uint8_t i=0; i<len; ++i){
		msg.data[i*2+1] = (uint8_t)  (data[i] & 0x00ff);
		msg.data[i*2+2] = (uint8_t) ((data[i] & 0xff00) >> 8);
	}

	//set other odds and ends
	msg.len = len*2 +1;
	msg.rtr = false;
	msg.id = node->id;
	can_tx(&msg, 5); 
	return DATA_OK;
}

CanState CanNode_getData_int8(const CanMessage* msg, int8_t* data) {

	if(msg == NULL){
		return DATA_ERROR;
	}

	//check configuration byte
	if((msg->data[0] >> 5) != CAN_INT8 ||   //not right type
	    msg->len != 2                  ||   //not right length
	   (msg->data[0] & 0x1F) != CAN_DATA ){ //not data

		return INVALID_TYPE;
	}

	//data
	*data = (int8_t) msg->data[1];

	return DATA_OK;
}

CanState CanNode_getData_uint8(const CanMessage* msg, uint8_t* data) {

	if(msg == NULL){
		return DATA_ERROR;
	}

	//check configuration byte
	if((msg->data[0] >> 5) != CAN_UINT8 ||  //not right type
	    msg->len != 2                   ||  //not right length
	   (msg->data[0] & 0x1F) != CAN_DATA ){ //not data

		return INVALID_TYPE;
	}

	//data
	*data = msg->data[1];

	return DATA_OK;
}

CanState CanNode_getData_int16(const CanMessage* msg, int16_t* data) {

	if(msg == NULL){
		return DATA_ERROR;
	}

	//check configuration byte
	if((msg->data[0] >> 5) != CAN_INT16 ||  //not right type
	    msg->len != 3                   ||  //not right length
	   (msg->data[0] & 0x1F) != CAN_DATA ){ //not data

		return INVALID_TYPE;
	}

	//data
	*data  = (int16_t)  msg->data[1];
	*data |= (int16_t) (msg->data[2] << 8);

	return DATA_OK;
}

CanState CanNode_getData_uint16(const CanMessage* msg, uint16_t* data) {

	if(msg == NULL){
		return DATA_ERROR;
	}

	//check configuration byte
	if((msg->data[0] >> 5) != CAN_UINT16 ||  //not right type
	    msg->len != 3                    ||  //not right length
	   (msg->data[0] & 0x1F) != CAN_DATA ){ //not data

		return INVALID_TYPE;
	}

	//data
	*data  = (uint16_t)  msg->data[1];
	*data |= (uint16_t) (msg->data[2] << 8);

	return DATA_OK;
}

void CanNode_checkForMessages() {
	//pc code should check if a new message is avalible
	//stm32 uses an interrupt to put the newest message in a struct
	
	//temporary use of polling
	
	//if there are no new messages don't do anything
	if(!is_can_msg_pending(CAN_FIFO0)){
		return;
	}

	can_rx(&tmpMsg, 5);
	//loop through nodes
	for(uint8_t i=0; i<MAX_NODES; ++i){

		//CanNode takes over if the caller asks for your id
		if(tmpMsg.id == nodes[i].id){
			CanNode_nodeHandler(&nodes[i], &tmpMsg);
		}
		
		//call callbacks for the user defined filters
		for(uint8_t j=0; j<NUM_FILTERS; ++j){ 
			if(tmpMsg.id == nodes[i].filters[j]){
				nodes[i].handle[j](&tmpMsg);
			}
		}
	}

	//clear new message flag
	newMessage = false;
}

//TODO
void CanNode_nodeHandler(CanNode* node, CanMessage* msg) {
	CanNodeMsgType type = msg->data[0] & 0x1F;

	switch(type){
		case CAN_GET_NAME:
			CanNode_sendName(node, msg->id);
			break;
		case CAN_GET_INFO:
			CanNode_sendInfo(node, msg->id);
			break;
		default:
			break;
	}
}
