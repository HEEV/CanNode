/**
 * CanNode.c
 * \brief implements functions for CanNode devices
 *
 * \author Samuel Ellicott
 * \date 6-20-16
 */
#include <stm32f0xx_hal.h>
#include "../Inc/CanNode.h"
#define UNUSED_FILTER 0xFFFF


static CanNode nodes[MAX_NODES] __attribute__((section(".can")));
static bool newMessage; 
static CanMessage tmpMsg;

static void CanNode_nodeHandler(CanNode* node, CanMessage* msg);

/**
 * Initilizes an empty CanNode structure to the values provided and saves it
 * to flash or retrieves a previously saved value from flash. This function has
 * two modes of operation determined by the force paramater. 
 *
 * -# force = false\n 
 *  *NOTE: You should use this mode for sensors*
 *		-# The function checks whether a node of the same id and type
 *		as those provided exist. If one does, it returns the address of it.
 *		-# The function tries to find an initilized node that has not been given
 *		out to another caller. If one exists, return its address.
 *		-# Try to initilize a node with the provded parameters if there is an 
 *		empty space.
 *		-# If the above fails, return NULL (this should never happen)
 * -# force = true\n
 *  *NOTE: You should use this mode for things that should not be overwritten in
 *  runtime. Using this mode reverts any changes back to what is in the compiled
 *  code at reset.*
 *		-# The function checks whether a node of the same id and type
 *		as those provided exist. If one does, it returns the address of it.
 *		-# Try to initilize a node with the provded parameters if there is an 
 *		empty space.
 *		-# If the above fails, return NULL 
 *
 * \param[in] type Gives the sensor type
 * \param[in] id CAN Address, use the CanNodeType type plus a constant.
 * \param[in] force Force the creation of a new node of the given paramaters 
 * if an old one is not found in flash memory.
 *
 * \returns the address of a \ref CanNode struct that stores the can information.
 * This information is necessary for using any of the sendData functions
 */
CanNode* CanNode_init(CanNodeType type, uint16_t id, bool force) {
	static bool has_run = false;
	static uint8_t usedNodes = 0;

	//if this is the first run clear list of nodes
	if(!has_run){
		can_init();
		can_set_bitrate(CAN_BITRATE_500K);
		can_enable();
		newMessage=false;
		has_run = true;
	}

	//check if a node of that type exists
	for(uint8_t i=0; i<MAX_NODES; ++i){
		//check for an id and type that matches the ones specified
		if(nodes[i].id == id && nodes[i].sensorType == type){
			//its a match!
			
			//add filters to hardware
			can_add_filter_id(id);
			for(uint8_t j=0; j<NUM_FILTERS; ++j){
				if(nodes[i].filters[j] != UNUSED_FILTER && 
				   nodes[i].filters[j] > 52 ){ //id's below 52 are reserved
					can_add_filter_id(nodes[i].filters[j]);
				}
			}

			//fill a spot in used nodes
			usedNodes |= 1<<i;
			return &nodes[i];
		}
	}
	if(!force) {
		
		for(uint8_t i=0; i<MAX_NODES; ++i){
			//check if the space has been used and was not given to another
			//caller
			if(nodes[i].id != 0xFFFF && (usedNodes & (1<<i)) == 0){
				
				//add filters to hardware
				can_add_filter_id(id);
				for(uint8_t j=0; j<NUM_FILTERS; ++j){
					if(nodes[i].filters[j] != UNUSED_FILTER && 
					   nodes[i].filters[j] > 52){ //id's below 52 are reserved
						can_add_filter_id(nodes[i].filters[j]);
					}
				}

				//fill a spot in used nodes
				usedNodes |= 1<<i;
				return &nodes[i];
			}
		}
	}

	//set node to NULL as a way of keeping track if the node has been saved.
	CanNode* node = NULL;

	//copy to ram while erasing
	CanNode nodebak[MAX_NODES];
	memcpy(nodebak, nodes, sizeof(CanNode)*MAX_NODES);

	flashUnlock();
	//erase flash
	flashErasePage((uint32_t) &nodes[0]);

	for(uint8_t i=0; i<MAX_NODES; ++i){
		//Find an open space and be sure we haven't stored our modified node yet.
		if((usedNodes & (1<<i)) == 0 && node == NULL){
			//found a happy place
			
			//sensor type
			flashWrite_16((uint32_t) &nodes[i].sensorType, type);

			//reset filters, filters should already be reset b/c flash erase
			//sets all values to 0xffff==UNUSED_FILTER.
			
			//add a filter so that the can hardware catches id's pertaining to 
			//the base id.
			can_add_filter_id(id);

			//set node to that id
			flashWrite_16((uint32_t) &nodes[i].id, id);

			//fill a spot in used nodes
			usedNodes |= 1<<i;
			node = &nodes[i];
		}
		//fill the rest of the nodes
		else if(usedNodes & (1<<i)) { 
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
 * Ids from 0 - 52 are reserved for the use of passing the return value of
 * can_add_filter_mask() to the function as a id. This allows for the use of 
 * id masks instead of identifier lists for filtering. It is adviseable to force
 * the reintilization of the node by passing true to the force paramater of 
 * CanNode_init().
 *
 * \param[in,out] node pointer to a node saved in flash memory
 * \param[in] filter id of the device that should be handled by handle
 * \param[in] handle function used to handle the filter
 *
 * \returns true if the filter was added, false if otherwise.
 *
 * \see CanNode_init()
 * \see can_add_filter_mask() for using mask filtering
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

			
			//if not a reseved address, add to hardware filtering
			if(filter > 52){
				can_add_filter_id(filter);
			}
			
			
			return true; //Sucess! Filter has been added
		}
	}

	return false; //no empty slots
}

/**
 * Get the name string from the node of the given id and put it in a character
 * buffer. The function will never deliver more than \ref MAX_NAME_LEN.
 *
 * \param id id of the node that you want the name of
 * \param name character buffer to put the name into
 * \param buff_len length of the character buffer
 * \param timeout length in mili-seconds before giving up the message
 *
 * \see CanNode_getInfo()
 */
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

/**
 * Get the info string from the node of the given id and put it in a character
 * buffer. The function will never deliver more than \ref MAX_INFO_LEN.
 *
 * \param id id of the node that you want the information string of
 * \param info character buffer to put the info string into
 * \param buff_len length of the character buffer
 * \param timeout length in mili-seconds before giving up the message
 *
 * \see CanNode_getName()
 */
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

/**
 * Set the provided CanNode's name string attribute. This will be stored in flash
 * and will fail silently if it has already been set. 
 *
 * \param node CanNode to set name string of. Should be one initilized by 
 * CanNode_init(), eg. stored in flash. If passed a struct that is not in flash
 * this function may do unexspected things. 
 *
 * \param name String containing the info string, if it is longer than 
 * \ref MAX_INFO_LEN it will be truncated and a null charater added.
 *
 * \param buff_len length of the name string
 *
 * \see CanNode_setInfo()
 * \see CanNode_init()
 */
void CanNode_setName(const CanNode* node, const char* name, uint8_t buff_len) {
	//store the data in buffer into the space pointed to by node->name
	//this address space resides in flash so a special process is taken
	uint8_t i;
	for(i=0; //continued on the next line
		*name!='\0' &&      //is it a null character?
		*name!= '\377' &&   //is it empty flash?
		i<MAX_NAME_LEN-1 && //are we out of range of the total buffer
		i<buff_len-1;       //are we out of range of the passed buffer
	    i+=2){              //writing in 16 bit incriments, advance by 2 chars

		//archatecture is little-endian (i think)
		uint16_t chars = name[i] | (name[i+1] << 8);
		flashWrite_16((uint32_t) &node->nodeInfoBuff[i], chars);
	}
	//if odd pad last byte with a null character
	if(buff_len & 1){
		uint16_t chars = name[i] | ('\0' << 8);
		flashWrite_16((uint32_t) &node->nodeInfoBuff[i], chars);
	}
	else {
		//add null character
		flashWrite_16((uint32_t) &node->nodeInfoBuff[i], 0);
	}
}	

/**
 * Set the provided CanNode's info string attribute. This will be stored in flash
 * and will fail silently if it has already been set. 
 *
 * \param node CanNode to set name string of. Should be one initilized by 
 * CanNode_init(), eg. stored in flash. If passed a struct that is not in flash
 * this function may do unexspected things. 
 *
 * \param name String containing the info string, if it is longer than 
 * \ref MAX_INFO_LEN it will be truncated and a null charater added.
 *
 * \param buff_len length of the name string
 *
 * \see CanNode_setInfo()
 * \see CanNode_init()
 */
void CanNode_setInfo(const CanNode* node, const char* info, uint8_t buff_len) {
	//store the data in buffer into the space pointed to by node->name
	//this address space resides in flash so a special process is taken
	
	uint8_t i;
	for(i=0;                //continued on the next line
	    *info!='\0' &&      //is it a null character?
	    *info!= '\377' &&   //is it empty flash?
		i<MAX_INFO_LEN-1 && //are we out of range of the total buffer
		i<buff_len-1;       //are we out of range of the passed buffer
	    i+=2){              //writing in 16 bit incriments, advance by 2 chars

		uint16_t chars = info[i] | (info[i+1] << 8);
		flashWrite_16((uint32_t) &node->nodeInfoBuff[i+MAX_NAME_LEN], chars);
	}
	//if odd pad last byte with a null character
	if(buff_len & 1){
		uint16_t chars = info[i] | ('\0' << 8);
		flashWrite_16((uint32_t) &node->nodeInfoBuff[i], chars);
	}
	else {
		//add null character
		flashWrite_16((uint32_t) &node->nodeInfoBuff[i], 0);
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
	   (msg->data[0] & 0x1F) != CAN_DATA ){  //not data

		return INVALID_TYPE;
	}

	//data
	*data  = (uint16_t)  msg->data[1];
	*data |= (uint16_t) (msg->data[2] << 8);

	return DATA_OK;
}

/**
 * Function that should be called from within the main loop. It calls handler
 * functions for each stored node. Because of the unknown length of the handler
 * functions this function call could take a very long time. In order to keep 
 * this function call to take a reasonable ammount of time, be sure to make
 * handler functions short. If that is impossible it is recommeded to use
 * interrupts for time-sensative components.
 *
 * This function will call an intrinsic handler if the message has the id
 * of one of the stored nodes and the calling node is not sending a request
 * frame. 
 */
void CanNode_checkForMessages() {
	//pc code should check if a new message is avalible
	//TODO stm32 uses an interrupt to put the newest message in a struct
	
	//if there are no new messages don't do anything
	if(!is_can_msg_pending()){
		return;
	}

	can_rx(&tmpMsg, 5);
	//loop through nodes
	for(uint8_t i=0; i<MAX_NODES; ++i){

		//CanNode takes over if the caller asks for your id (and it isn't rtr)
		if(tmpMsg.id == nodes[i].id && !tmpMsg.rtr){
			CanNode_nodeHandler(&nodes[i], &tmpMsg);
		}
		
		//call callbacks for the user defined filters
		for(uint8_t j=0; j<NUM_FILTERS; ++j){ 
			if(tmpMsg.id == nodes[i].filters[j] && 
			   nodes[i].handle[j] != NULL){

				//call handler function
				nodes[i].handle[j](&tmpMsg);
			}
			//check if the filter match equals a filter id
			else if(tmpMsg.fmi == nodes[i].filters[j] && //filter matches 
					nodes[i].handle[j] != NULL){

				//call handler function
				nodes[i].handle[j](&tmpMsg);
			}
		}
	}

	//clear new message flag
	newMessage = false;
}

//TODO
void CanNode_nodeHandler(CanNode* node, CanMessage* msg) {
	//get the type of message
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
