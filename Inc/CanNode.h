/*
 * CanNode.h 
 * Interface for using CanNode devices. Provides high-level functions for 
 * protocol implementation; driver level functions in can.h. Goal to be easily
 * portable to a PC application. 
 *
 * Author: Samuel Ellicott
 * Date: 6-20-16
 */
#ifndef _CAN_NODE_H_
#define _CAN_NODE_H_

#include <stdint.h>
#include <stdbool.h>
#include "can.h"

#ifndef MAX_NODES
	#define MAX_NODES 3
#endif

#ifndef NUM_FILTERS
	#define NUM_FILTERS 4
#endif

//This typedef contains various types of possible sensors and their CAN ids
typedef enum {
	ANALOG      = 1200,
	RELAY       = 1264,
	LED	        = 1392,
	MEGASQUIRT  = 1520,
	SWITCH      = 1584,
	PRESSURE    = 1648,
	TEMPURATURE = 1664,
	UNCONFIG    = 1984
} CanNodeType;

//This typedef contains the various types of data 
//that can be stored in a message
typedef enum {
	CAN_UINT8,
	CAN_INT8,
	CAN_UINT16,
	CAN_INT16,
	CAN_UINT32,
	CAN_INT32,
	CAN_BIT_FIELD,
	CAN_CUSTOM
} CanNodeDataType;

//Types of commands that can be sent
typedef enum {
	CAN_DATA,           //Normal operation, device is sending data to other nodes
	CAN_DATA_MODE,      //Sent by master to enter data mode (default mode)
	CAN_CONFIG_MODE,    //Sent by master to enter config mode
	CAN_SET_ID,	        //Sent by master to change the id of the node
	CAN_ID_SET_ERROR,   //Error sent by node if the new id is not availible
	CAN_CONFIG_ERROR,   //General configuration error
	CAN_GET_NAME,       //Ask a node for its name
	CAN_GET_INFO,       //Ask a node for its info
	CAN_NAME_INFO       //message is part of a name/info message
} CanNodeMsgType;

//types of errors that can be returned by functions
typedef enum {
	DATA_OK,
	DATA_ERROR,
	INVALID_TYPE,
	DATA_OVERFLOW,
} CanNodeFmtError;

typedef void (*filterHandler)(CanMessage* data);

typedef struct {
	uint16_t id;
	uint8_t status;
	uint16_t filters[NUM_FILTERS];
	filterHandler handle[NUM_FILTERS];
	CanNodeType sensorType;
	char* nodeName;
	uint8_t nodeNameLen;
	char* nodeInfo;
	uint16_t nodeInfoLen;
} CanNode;

uint16_t CanNode_init(CanNode* node, CanNodeType type, uint16_t id);
bool CanNode_addFilter(CanNode* node, uint16_t filter, filterHandler handle);
void CanNode_checkForMessages();

//NOTE: these functions are blocking and any inportant information
//dilivered in the period using these functions will be lost. 
void CanNode_getName(uint16_t id, char* name, uint8_t buff_len, uint32_t timeout);
void CanNode_getInfo(uint16_t id, char* info, uint16_t buff_len, uint32_t timeout);

//functions for setting data
void CanNode_sendData_int8   (const CanNode* node,   int8_t data);
void CanNode_sendData_uint8  (const CanNode* node,  uint8_t data);
void CanNode_sendData_int16  (const CanNode* node,  int16_t data);
void CanNode_sendData_uint16 (const CanNode* node, uint16_t data);
void CanNode_sendData_int32  (const CanNode* node,  int32_t data);
void CanNode_sendData_uint32 (const CanNode* node, uint32_t data);

CanNodeFmtError CanNode_sendDataArr_int8   (const CanNode* node, int8_t* data, uint8_t len);
CanNodeFmtError CanNode_sendDataArr_uint8  (const CanNode* node, uint8_t* data, uint8_t len);
CanNodeFmtError CanNode_sendDataArr_int16  (const CanNode* node, int16_t* data, uint8_t len);
CanNodeFmtError CanNode_sendDataArr_uint16 (const CanNode* node, uint16_t* data, uint8_t len);

//functions for getting data
CanNodeFmtError CanNode_getData_int8   (const CanMessage* msg,   int8_t* data);
CanNodeFmtError CanNode_getData_uint8  (const CanMessage* msg,  uint8_t* data);
CanNodeFmtError CanNode_getData_int16  (const CanMessage* msg,  int16_t* data);
CanNodeFmtError CanNode_getData_uint16 (const CanMessage* msg, uint16_t* data);
CanNodeFmtError CanNode_getData_int32  (const CanMessage* msg,  int32_t* data);
CanNodeFmtError CanNode_getData_uint32 (const CanMessage* msg, uint32_t* data);

CanNodeFmtError CanNode_getDataArr_int8   (const CanMessage* msg, int8_t data[7]);
CanNodeFmtError CanNode_getDataArr_uint8  (const CanMessage* msg, uint8_t data[7]);
CanNodeFmtError CanNode_getDataArr_int16  (const CanMessage* msg, int16_t data[2]);
CanNodeFmtError CanNode_getDataArr_uint16 (const CanMessage* msg, uint16_t data[2]);

#endif //_CAN_NODE_H_
