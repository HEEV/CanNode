/*
 * CanNode.c - provides functions for using CanNode devices
 *
 * Author: Samuel Ellicott
 * Date: 6-20-16
 */
#include <stdlib.h>
#include <stdint.h>
#include "../Inc/CanNode.h"
#define UNUSED_FILTER 0xFFFF


static CanNode* nodes[MAX_NODES];
static bool newMessage; 
static CanMessage tmpMsg;

static void CanNode_nodeHandler(CanNode* node, CanMessage* msg);

uint16_t CanNode_init(CanNode* node, CanNodeType type, uint16_t id) {
	static bool has_run = false;

	//if this is the first run clear list of nodes
	if(!has_run){
		can_init();
		can_enable();
		for(uint8_t i=0; i<MAX_NODES; ++i){
			nodes[i]=NULL;
		}
		newMessage=false;
		has_run = true;
	}

	//check if input is valid
	if(node == NULL){
		return 0;//error
	}
	node->sensorType = type;
	//reset filters
	for(uint8_t i=0; i<NUM_FILTERS; ++i){
		node->filters[i] = UNUSED_FILTER;
	}

	//check if the id is taken if it is taken return an error, if not assign id
	//TODO check id
	//tmpMsg.rtr = false;
	//can_rtr(&tmpMsg, id, 5);
	//if(!tmpMsg.rtr){
	//	return 0; //slot is taken
	//}
	//set node to that id
	node->id = id;

	//add node to list of nodes... If there's room
	for(uint8_t i=0; i<MAX_NODES; ++i){
		if(nodes[i]==NULL){
			nodes[i]=node;
			return node->id; //Sucess! Return the id of the node
		}
	}
	return 0;//no open slots
}

bool CanNode_addFilter(CanNode* node, uint16_t filter, filterHandler handle) {
	if(filter > 0x7FF || handle == NULL){
		return false;
	}

	//add to the end of the list of filters... If there's room.
	for(uint8_t i=0; i<NUM_FILTERS; ++i){
		if(node->filters[i] == UNUSED_FILTER){
			node->filters[i] = filter;
			node->handle[i] = handle;
			return true; //Sucess! Filter has been added
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
		if(msg.id != id || (msg.data[0] & 0x05) != CAN_NAME_INFO){
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
	msg.data[0] = CAN_GET_NAME | (CAN_INT8 << 5);
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
		if(msg.id != id || (msg.data[0] & 0x05) != CAN_NAME_INFO){
			continue;
		}
		//get all the data from this buffer
		for(uint8_t i=0; i<msg.len-1 && namePtr-info<buff_len; ++namePtr, ++i){
			*namePtr = msg.data[i];
		}
	}
}

//getter and setter functions -------------------------------------------------

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

CanNodeFmtError CanNode_sendDataArr_int8(const CanNode* node, int8_t* data, uint8_t len) {
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

CanNodeFmtError CanNode_sendDataArr_uint8(const CanNode* node, uint8_t* data, uint8_t len) {
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

CanNodeFmtError CanNode_sendDataArr_int16(const CanNode* node, int16_t* data, uint8_t len) {
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

CanNodeFmtError CanNode_sendDataArr_uint16(const CanNode* node, uint16_t* data, uint8_t len) {
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

CanNodeFmtError CanNode_getData_int8(const CanMessage* msg, int8_t* data) {

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

CanNodeFmtError CanNode_getData_uint8(const CanMessage* msg, uint8_t* data) {

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

CanNodeFmtError CanNode_getData_int16(const CanMessage* msg, int16_t* data) {

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

CanNodeFmtError CanNode_getData_uint16(const CanMessage* msg, uint16_t* data) {

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
	for(uint8_t i=0; i<MAX_NODES && nodes[i] != NULL; ++i){

		//CanNode takes over if the caller asks for your id
		if(tmpMsg.id == nodes[i]->id){
			CanNode_nodeHandler(nodes[i], &tmpMsg);
		}
		
		//call callbacks for the user defined filters
		for(uint8_t j=0; j<NUM_FILTERS; ++j){ 
			if(tmpMsg.id == nodes[i]->filters[j]){
				nodes[i]->handle[j](&tmpMsg);
			}
		}
	}

	//clear new message flag
	newMessage = false;
}

//TODO
void CanNode_nodeHandler(CanNode* node, CanMessage* msg) {
}
