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


typedef enum {
	RELAY       = 1264,
	LED	        = 1392,
	MEGASQUIRT  = 1520,
	SWITCH	    = 1584,
	PRESSURE    = 1648,
	TEMPURATURE = 1664,
	UNCONFIG    = 1984
} CanNodeType;

typedef enum {
	CAN_LOW_PRIORITY,
	CAN_MED_PRIORITY,
	CAN_HIGH_PRIORITY
} CanNodePriority;

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

typedef enum {
	CAN_DATA,           //Normal operation, device is sending data to other nodes
	CAN_DATA_MODE,      //Sent by master to enter data mode (default mode)
	CAN_CONFIG_MODE,    //Sent by master to enter config mode
	CAN_SET_ID,	        //Sent by master to change the id of the node
	CAN_ID_SET_ERROR,   //Error sent by node if the new id is not availible
	CAN_CONFIG_ERROR    //General configuration error
} CanNodeMsgType;

typedef enum {
	DATA_OK,
	INVALID_TYPE,
	DATA_OVERFLOW
} CanNodeFmtError;

typedef void (*filterHandler)(CanMessage* data);

#define NUM_FILTERS 4
typedef struct {
	uint16_t id;
	uint8_t status;
	uint16_t filters[NUM_FILTERS];
	CanNodeType sensorType;
	filterHandler handle;
} CanNode;

uint16_t CanNode_init(CanNode* node, CanNodeType type, CanNodePriority pri);
bool CanNode_addFilter(CanNode* node, uint16_t filter);
void CanNode_setFilterHandler(CanNode* node, filterHandler handle);

//functions for setting data
void CanNode_setData_int8   (CanMessage* msg,   int8_t data);
void CanNode_setData_uint8  (CanMessage* msg,  uint8_t data);
void CanNode_setData_int16  (CanMessage* msg,  int16_t data);
void CanNode_setData_uint16 (CanMessage* msg, uint16_t data);
void CanNode_setData_int32  (CanMessage* msg,  int32_t data);
void CanNode_setData_uint32 (CanMessage* msg, uint32_t data);

CanNodeFmtError CanNode_setDAtaArr_int8(CanMessage* msg, int8_t* data, uint8_t len);
CanNodeFmtError CanNode_setDataArr_uint8(CanMessage* msg, uint8_t* data, uint8_t len);
CanNodeFmtError CanNode_setDAtaArr_int16(CanMessage* msg, int16_t* data, uint8_t len);
CanNodeFmtError CanNode_setDataArr_uint16(CanMessage* msg, uint16_t* data, uint8_t len);

//functions for getting data
CanNodeFmtError CanNode_getData_int8   (CanMessage* msg,   int8_t* data);
CanNodeFmtError CanNode_getData_uint8  (CanMessage* msg,  uint8_t* data);
CanNodeFmtError CanNode_getData_int16  (CanMessage* msg,  int16_t* data);
CanNodeFmtError CanNode_getData_uint16 (CanMessage* msg, uint16_t* data);
CanNodeFmtError CanNode_getData_int32  (CanMessage* msg,  int32_t* data);
CanNodeFmtError CanNode_getData_uint32 (CanMessage* msg, uint32_t* data);

CanNodeFmtError CanNode_getDataArr_int8(CanMessage* msg, int8_t data[7]);
CanNodeFmtError CanNode_getDataArr_uint8(CanMessage* msg, uint8_t data[7]);
CanNodeFmtError CanNode_getDataArr_int16(CanMessage* msg, int16_t data[2]);
CanNodeFmtError CanNode_getDataArr_uint16(CanMessage* msg, uint16_t data[2]);


void CanNode_CheckForMessages();
bool CanNode_sendMessage(CanNode* node, CanMessage* msg);

#endif //_CAN_NODE_H_
