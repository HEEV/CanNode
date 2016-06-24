/*
 * CanNode.c - provides functions for using CanNode devices
 *
 * Author: Samuel Ellicott
 * Date: 6-20-16
 */
#include <stdlib.h>
#include <stdint.h>
#include "../Inc/CanNode.h"

#define MAX_NODES 6
static CanNode* nodes[MAX_NODES];
static bool newMessage; 
static CanMessage tmpMsg;

static void CanNode_nodeHandler(CanNode* node, CanMessage* msg);

uint16_t CanNode_init(CanNode* node, CanNodeType type, CanNodePriority pri) {
	bool has_run = false;

	//if this is the first run clear list of nodes
	if(!has_run){
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

	//TODO
	//find id closest to the specified priority.
	//set node to that id

	//add node to list of nodes... If there's room
	for(uint8_t i=0; i<MAX_NODES; ++i){
		if(nodes[i]==NULL){
			nodes[i]=node;
			return node->id; //Sucess! Return the id of the node
		}
	}
	return 0;//no open slots
}

bool CanNode_addFilter(CanNode* node, uint16_t filter) {
	if(filter > 0x7FF){
		return false;
	}

	//add to the end of the list of filters... If there's room.
	for(uint8_t i=0; i<NUM_FILTERS; ++i){
		if(node->filters[i] != 0){
			node->filters[i] = filter;
			return true; //Sucess! Filter has been added
		}
	}

	return false; //no empty slots
}

void CanNode_setFilterHandler(CanNode* node, filterHandler handle) {
	if(node == NULL || handle == NULL){
		return;
	}

	node->handle = handle;
}

//getter and setter functions -------------------------------------------------
void CanNode_setData_int8(CanMessage* msg, int8_t data) {
	//configuration byte
	msg->data[0] = (uint8_t) ((0x7 & CAN_INT8) << 5) | (0x1F & CAN_DATA);
	//data
	msg->data[1] = (uint8_t) data;
	//set other odds and ends
	msg->len = 2;
	msg->rtr = false;
}

void CanNode_setData_uint8(CanMessage* msg, uint8_t data) {
	//configuration byte
	msg->data[0] = (uint8_t) ((0x7 & CAN_UINT8) << 5) | (0x1F & CAN_DATA);
	//data
	msg->data[1] = data;
	//set other odds and ends
	msg->len = 2;
	msg->rtr = false;
}

void CanNode_setData_int16(CanMessage* msg, int16_t data) {
	//configuration byte
	msg->data[0] = (uint8_t) ((0x7 & CAN_INT16) << 5) | (0x1F & CAN_DATA);
	//data
	msg->data[1] = (uint8_t) (data & 0x00ff);
	msg->data[2] = (uint8_t) (data & 0xff00) >> 8;
	//set other odds and ends
	msg->len = 3;
	msg->rtr = false;
}

void CanNode_setData_uint16(CanMessage* msg, uint16_t data) {
	//configuration byte
	msg->data[0] = (uint8_t) ((0x7 & CAN_UINT16) << 5) | (0x1F & CAN_DATA);
	//data
	msg->data[1] = (uint8_t) (data & 0x00ff);
	msg->data[2] = (uint8_t) (data & 0xff00) >> 8;
	//set other odds and ends
	msg->len = 3;
	msg->rtr = false;
}

void CanNode_setData_int32(CanMessage* msg, int32_t data) {
	//configuration byte
	msg->data[0] = (uint8_t) ((0x7 & CAN_INT32) << 5) | (0x1F & CAN_DATA);
	//data
	msg->data[1] = (uint8_t) (data & 0x000000ff);
	msg->data[2] = (uint8_t) (data & 0x0000ff00) >> 8;
	msg->data[3] = (uint8_t) (data & 0x00ff0000) >> 16;
	msg->data[4] = (uint8_t) (data & 0xff000000) >> 24;
	//set other odds and ends
	msg->len = 5;
	msg->rtr = false;
}

void CanNode_setData_uint32(CanMessage* msg, uint32_t data) {
	//configuration byte
	msg->data[0] = (uint8_t) ((0x7 & CAN_UINT32) << 5) | (0x1F & CAN_DATA);
	//data
	msg->data[1] = (uint8_t) (data & 0x000000ff);
	msg->data[2] = (uint8_t) (data & 0x0000ff00) >> 8;
	msg->data[3] = (uint8_t) (data & 0x00ff0000) >> 16;
	msg->data[4] = (uint8_t) (data & 0xff000000) >> 24;
	//set other odds and ends
	msg->len = 5;
	msg->rtr = false;
}

CanNodeFmtError CanNode_setDataArr_int8(CanMessage* msg, int8_t* data, uint8_t len) {
	//check if valid
	if(len>7){
		return DATA_OVERFLOW;
	}

	//configuration byte
	msg->data[0] = (uint8_t) ((0x7 & CAN_INT8) << 5) | (0x1F & CAN_DATA);
	//data
	for(uint8_t i=0; i<len && i<7; ++i){
		msg->data[i+1] = (uint8_t) data[i];
	}

	//set other odds and ends
	msg->len = len+1;
	msg->rtr = false;
	return DATA_OK;
}

CanNodeFmtError CanNode_setDataArr_uint8(CanMessage* msg, uint8_t* data, uint8_t len) {
	//check if valid
	if(len>7){
		return DATA_OVERFLOW;
	}

	//configuration byte
	msg->data[0] = (uint8_t) ((0x7 & CAN_UINT8) << 5) | (0x1F & CAN_DATA);
	//data
	for(uint8_t i=0; i<len && i<7; ++i){
		msg->data[i+1] = data[i];
	}

	//set other odds and ends
	msg->len = len+1;
	msg->rtr = false;
	return DATA_OK;
}

CanNodeFmtError CanNode_setDAtaArr_int16(CanMessage* msg, int16_t* data, uint8_t len) {
	//check if valid
	if(len>2){
		return DATA_OVERFLOW;
	}

	//configuration byte
	msg->data[0] = (uint8_t) ((0x7 & CAN_INT16) << 5) | (0x1F & CAN_DATA);
	//data
	for(uint8_t i=0; i<len && i<7; ++i){
		msg->data[i+1] = (uint8_t) (data[i] & 0x00ff);
		msg->data[i+2] = (uint8_t) (data[i] & 0xff00) >> 8;
	}

	//set other odds and ends
	msg->len = len*2 +1;
	msg->rtr = false;
	return DATA_OK;
}

CanNodeFmtError CanNode_setDataArr_uint16(CanMessage* msg, uint16_t* data, uint8_t len) {
	//check if valid
	if(len>2){
		return DATA_OVERFLOW;
	}

	//configuration byte
	msg->data[0] = (uint8_t) ((0x7 & CAN_UINT16) << 5) | (0x1F & CAN_DATA);
	//data
	for(uint8_t i=0; i<len && i<7; ++i){
		msg->data[i+1] = (uint8_t) (data[i] & 0x00ff);
		msg->data[i+2] = (uint8_t) (data[i] & 0xff00) >> 8;
	}

	//set other odds and ends
	msg->len = len*2 +1;
	msg->rtr = false;
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

	//loop through nodes
	for(uint8_t i=0; i<MAX_NODES && nodes[i] != NULL; ++i){

		//CanNode takes over if the caller asks for your id
		if(tmpMsg.id == nodes[i]->id){
			CanNode_nodeHandler(nodes[i], &tmpMsg);
		}
		
		//call callbacks for the user defined filters
		for(uint8_t j=0; j<NUM_FILTERS; ++j){ 
			if(tmpMsg.id == nodes[i]->filters[j]){
				nodes[i]->handle(&tmpMsg);
			}
		}
	}

	//clear new message flag
	newMessage = false;
}

bool CanNode_sendMessage(CanNode* node, CanMessage* msg) {
	msg->id = node->id;
	can_tx(msg, 5);
	return true;
}

//TODO
void CanNode_nodeHandler(CanNode* node, CanMessage* msg) {
}
