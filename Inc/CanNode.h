/*
 * CanNode.h 
 * Interface for using CanNode devices. Provides high-level functions for 
 * protocol implementation; driver level functions in can.h. Goal to be easily
 * portable to a PC application. 
 *
 * Author: Samuel Ellicott
 * Date: 6-20-16
 */
#include <stdint.h>
#include "can.h"


enum CanNodeType {
	RELAY		= 1264,
	LED			= 1392,
	MEGASQUIRT	= 1520,
	SWITCH		= 1584,
	PRESSURE	= 1648,
	TEMPURATURE = 1664,
	UNCONFIG	= 1984
};

enum CanNodePriority {
	CAN_LOW_PRIORITY,
	CAN_MED_PRIORITY,
	CAN_HIGH_PRIORITY
};

enum CanNodeDataType {
	CAN_UINT8,
	CAN_INT8,
	CAN_UINT16,
	CAN_INT16,
	CAN_UINT32,
	CAN_INT32,
	CAN_BIT_FIELD,
	CAN_CUSTOM
};

enum CanNodeMsgType {
	CAN_DATA,			//Normal operation, device is sending data to other nodes
	CAN_DATA_MODE,		//Sent by master to enter data mode (default mode)
	CAN_CONFIG_MODE,	//Sent by master to enter config mode
	CAN_SET_ID,			//Sent by master to change the id of the node
	CAN_ID_SET_ERROR,	//Error sent by node if the new id is not availible
	CAN_CONFIG_ERROR	//General configuration error
};


union CanNodeData {
	//8bit data
	uint8_t uint8;
	int8_t int8;
	//16bit data
	uint16_t uint16;
	int16_t int16;
	//32bit data
	uint32_t uint32;
	int32_t int32;
	//arrays of data
	uint8_t uint8_arr[7];
	int8_t int8_arr[7];
	uint16_t uint16_arr[3];
	int16_t int16_arr[3];
};

struct CanNodeMessage {
	uint16_t id;
	uint8_t len;
	CanNodeDataType type;
	CanNodeData data;
};

typedef void (*filterHandler)(CanNodeData* data);

#define NUM_FILTERS 4
struct CanNode {
	uint16_t id;
	uint8_t status;
	uint16_t filters[NUM_FILTERS];
	CanNodeType sensorType;
	filterHandler handle;
};

uint16_t CanNode_Init(CanNode* node, CanNodeType type, CanNodePriority pri);
bool CanNode_addFilter(uint16_t filter);
void CanNode_setFilterHandler(CanNode* node, filterHandler handle);
bool CanNode_sendMessage(CanNode* node, const CanNodeMessage* msg);
