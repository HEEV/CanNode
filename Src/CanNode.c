/**
 * CanNode.c
 * \brief implements functions for CanNode devices
 *
 * \author Samuel Ellicott
 * \date 6-20-16
 */
#include <CanNode.h>

#define UNUSED_FILTER 0xFFFF


static CanNode nodes[MAX_NODES];
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
 *		-# The function checks whether a node of the same id 
 *		as the one provided exist. If one does, it returns the address of it.
 *		-# The function tries to find an initilized node that has not been given
 *		out to another caller. If one exists, return its address.
 *		-# Try to initilize a node with the provded parameters if there is an 
 *		empty space.
 *		-# If the above fails, return NULL (this should never happen)
 * -# force = true\n
 *  *NOTE: You should use this mode for things that should not be overwritten in
 *  runtime. Using this mode reverts any changes back to what is in the compiled
 *  code at reset.*
 *		-# The function checks whether a node of the same id 
 *		as the one provided exist. If one does, it returns the address of it.
 *		-# Try to initilize a node with the provded parameters if there is an 
 *		empty space.
 *		-# If the above fails, return NULL 
 *
 * \param[in] id CAN Address, use the CanNodeType type plus a constant.
 * \param[in] rtrHandle function pointer to a handler function for rtr requests.
 * \param[in] force Force the creation of a new node of the given paramaters 
 * if an old one is not found in flash memory.
 *
 * \returns the address of a \ref CanNode struct that stores the can information.
 * This information is necessary for using any of the sendData functions
 */
CanNode* CanNode_init(CanNodeType id, filterHandler rtrHandle, bool force) {
	static bool has_run = false;
	static uint8_t usedNodes = 0;
	CanNode* node = NULL;

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
		//check if spot is used
		if(nodes[i].id != 0){
			continue;
		}
		//otherwise its open

		//add id etc
		node = &nodes[i];

		node->id = id;
		//add filters to hardware
		//default filters
		can_add_filter_id(id);  //rtr filter
		can_add_filter_id(id+1);//get name filter
		can_add_filter_id(id+2);//get info filter
		can_add_filter_id(id+3);//configuration filter
		//fill a spot in used nodes
		usedNodes |= 1<<i;
		break;
	}
	return node;//returns null if no empty spots found
}

void CanNode_saveNode(CanNode* flashNode, CanNode* newNode){
	//clear the flash data for the current node

	//copy to ram while erasing
	CanNode nodebak[MAX_NODES];
	memcpy(nodebak, nodes, sizeof(CanNode)*MAX_NODES);

	//unlock the flash for writing
	flashUnlock();
	//erase flash
	flashErasePage((uint32_t) &nodes[0]);

	//copy all the nodes that are not the node we are changing
	for(uint8_t i=0; i<MAX_NODES; ++i) {
		if(flashNode != &nodes[i]) {
			//copy the node
			flashWriteMemBlock((uint32_t) &nodes[i],
					(uint8_t*) &nodes[0], sizeof(CanNode));
		}
	}
	//copy over the new node
	flashWriteMemBlock((uint32_t) flashNode, (uint8_t*) newNode, sizeof(CanNode));

	//relock the flash
	flashLock();
}

/**
 * Saves a filter id and a handler to a node in flash memory. The function also
 * accepts a function which gets called if a message from that id is avalible.
 *
 * Ids from 0 - 52 are reserved for the use of passing the return value of
 * can_add_filter_mask() to the function as a id. This allows for the use of
 * id masks instead of identifier lists for filtering. It is adviseable to force * the reintilization of the node by passing true to the force paramater of
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
			node->filters[i]=filter;
			//save a pointer to the handler function
			node->handle[i]=handle;


			//if not a reseved address, add to hardware filtering
			if(filter > 52){
				can_add_filter_id(filter);
			}

			return true; //Sucess! Filter has been added
		}
	}

	return false; //no empty slots
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

		//CanNode takes over if the caller asks for a reserved id
		//rtr request for node data
		if(tmpMsg.id == nodes[i].id && tmpMsg.rtr){
			nodes[i].rtrHandle(&tmpMsg);
		}
		//get name id if asked with an rtr
		else if(tmpMsg.id == nodes[i].id+1 && tmpMsg.rtr){
			//CanNode_sendName(&nodes[i], tmpMsg.id);
		}
		//get info id
		else if(tmpMsg.id == nodes[i].id+2 && tmpMsg.rtr){
			//CanNode_sendInfo(&nodes[i], tmpMsg.id);
		}
		//configuration id
		else if(tmpMsg.id == nodes[i].id+3){
			//CanNode_nodeHandler(&nodes[i], &tmpMsg);
		}
		else {
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
	}

	//clear new message flag
	newMessage = false;
}

/**
 * \brief handles configuration functionality of the CanNode
 */
/*
void CanNode_nodeHandler(CanNode* node, CanMessage* msg) {
	//get the type of message
	CanNodeMsgType type = msg->data[0] & 0x1F;
	const int TIMEOUT = 1000;
	static bool configMode = false;
	static int tickStart = 0;

	if(configMode && tickStart<HAL_GetTick()) {
		configMode = false;
	}

	switch(type){
		case CAN_CONFIG_MODE:
			//enter configuration mode and start a 1 second exit timer
			tickStart=HAL_GetTick()+TIMEOUT;
			configMode=true;
			break;

		//following cases only work after configuration mode has been entered.
		case CAN_SET_ID:
			if(!configMode) break; //Hey! What's that clown doing?

			//copy the node we are modifying into ram
			CanNode tempNode;
			memcpy(&tempNode, node, sizeof(CanNode));

			//get the id from the message
			tempNode.id  = (uint16_t)  msg->data[1];
			tempNode.id |= (uint16_t) (msg->data[2] << 8);

			//save the node
			//CanNode_saveNode(node, &tempNode);
			break;
		case CAN_SET_NAME:
			if(!configMode) break;
			//get the name info from the master node. It is sending this
			//information to us by sending it on our address, so we should listen
			//to that address.
			//get the name string with a 50ms timeout
			char name[MAX_NAME_LEN];
			CanNode_getName(node->id, name, MAX_NAME_LEN, 50);
			CanNode_setName(node, name, MAX_NAME_LEN);
			break;
		case CAN_SET_INFO:
			if(!configMode) break;
			//same as above
			char info[MAX_INFO_LEN];
			CanNode_getInfo(node->id, info, MAX_INFO_LEN, 100);
			CanNode_setInfo(node, info, MAX_INFO_LEN);
			break;
		default:
			break;
	}
}
*/
