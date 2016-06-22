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
static CanNodeMessage tmpMsg;

static void CanNode_nodeHandler(CanNode* node, CanNodeMessage* msg);

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

//TODO
bool CanNode_sendMessage(CanNode* node, const CanNodeMessage* msg) {
	return true;
}

//TODO
void CanNode_nodeHandler(CanNode* node, CanNodeMessage* msg) {
}
