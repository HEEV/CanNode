
/** 
 * \file CanNode.h
 * \brief Interface for using CanNode devices.
 * 
 * CanNode.h provides high-level functions for protocol implementation.
 * Driver level functions are in can.h. This compilation unit has a goal to be 
 * easily portable to a PC application. 
 *
 * \author Samuel Ellicott
 * \date 6-20-16
 */

#ifndef _CAN_NODE_H_
#define _CAN_NODE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "CanTypes.h"
#include "can.h"

/** \addtogroup CanNode_Module CanNode
 * \brief Library to provide a higher level protocol for CAN communication.
 * Specifically for stm32 microcontrollers
 *
 * A CanNode should be initilized with CanNode_init() this will return a pointer to
 * an initilized node to be used with the other functions. Callbacks to occur on
 * messages from a particular id can be added to a node with the CanNode_addFilter()
 * function. To translate [CanMessages](\ref CanMessage) to useful data, use the
 * getData functions. Data can be sent from a node with the sendData functions.
 *
 * ### Initilizing a node ###
 *
 * In order to send data and use filter callbacks you need to initilize a node.
 * to do this call CanNode_init(), passing in a CanNodeType (acts like an id), a
 * filterHandler for handling RTR callbacks for that node,
 * and a boolean value (it doesn't matter what) this returns a CanNode pointer to
 * a newly initilized CanNode struct. You can initilize up to \ref MAX_NODES nodes
 * in this way.
 *
 * Example code
 * ~~~~~~~~~~~~ {.c}
 *
 * void pitotRTR(CanMessage* msg);
 *
 * void main(){
 *	CanNode* newNode;
 *	newNode = CanNode_init(PITOT, pitotRTR, false);
 *	//other stuff here
 * }
 *
 * void pitotRTR(CanMessage* msg){
 *	//continue to do what needs to be done.
 * }
 * ~~~~~~~~~~~~
 *
 * Another often usefull thing to do is add filters
 *
 * ### Adding filters ###
 *
 * Example code
 *
 * ~~~~~~~~~~~~ {.c}
 * uint16_t id = can_add_filter_mask(id_to_filter, id_mask);
 * CanNode_addFilter(id, handler);
 * ~~~~~~~~~~~~
 *@{
 */

/**
 * \defgroup CanNode_Functions CanNode Public Functions
 *
 * \brief Functions for interfacing with the CanNode struct, reciving and sending
 * CAN messages.
 *
 *@{
 */

/// \brief Initilize a CanNode from given parameters.
CanNode* CanNode_init(CanNodeType id, filterHandler rtrHandle, bool force);
/// \brief Add a filter and handler to a given CanNode.
bool CanNode_addFilter(CanNode* node, uint16_t filter, filterHandler handle);
/// \brief Check all initilized CanNodes for messages and call callbacks.
void CanNode_checkForMessages();

/**
 * \anchor sendData
 * \name sendData Functions
 * These functions that send data over the CANBus and support various integer
 * types of data. They are non-blocking.
 * @{
 */
/// \brief Send a signed 8-bit integer.
void CanNode_sendData_int8   (const CanNode* node,   int8_t data);
/// \brief Send an unsigned 8-bit integer.
void CanNode_sendData_uint8  (const CanNode* node,  uint8_t data);
/// \brief Send a signed 16-bit integer.
void CanNode_sendData_int16  (const CanNode* node,  int16_t data);
/// \brief Send an unsigned 16-bit integer.
void CanNode_sendData_uint16 (const CanNode* node, uint16_t data);
/// \brief Send a signed 32-bit integer.
void CanNode_sendData_int32  (const CanNode* node,  int32_t data);
/// \brief Send an unsigned 32-bit integer.
void CanNode_sendData_uint32 (const CanNode* node, uint32_t data);
//@}

/**
 * \name sendDataArr Functions
 * These functions that send an array of data over the CANBus. They support
 * various integer types of data. They are non-blocking.
 *@{
 */
/// \brief Send an array of uinsigned 8-bit integers.
CanState CanNode_sendDataArr_int8   (const CanNode* node, int8_t* data, uint8_t len);
/// \brief Send an array of signed 8-bit integers.
CanState CanNode_sendDataArr_uint8  (const CanNode* node, uint8_t* data, uint8_t len);
/// \brief Send an array of uinsigned 16-bit integers.
CanState CanNode_sendDataArr_int16  (const CanNode* node, int16_t* data, uint8_t len);
/// \brief Send an array of signed 16-bit integers.
CanState CanNode_sendDataArr_uint16 (const CanNode* node, uint16_t* data, uint8_t len);
//@}

/**
 * \anchor getData
 * \name getData Functions
 * These functions get data of various integer types from a CanMessage. They are
 * non-blocking. If the data is not of the same type as the called function
 * \ref INVALID_TYPE is returned.
 *
 * These functions are useful for data parsing in a handler function of the type
 * passed to CanNode_addFilter() where a CanMessage pointer is passed to the
 * handler as input.
 * @{
 */
/// \brief Get a signed 8-bit integer from a CanMessage.
CanState CanNode_getData_int8   (const CanMessage* msg,   int8_t* data);
/// \brief Get an unsigned 8-bit integer from a CanMessage.
CanState CanNode_getData_uint8  (const CanMessage* msg,  uint8_t* data);
/// \brief Get a signed 16-bit integer from a CanMessage.
CanState CanNode_getData_int16  (const CanMessage* msg,  int16_t* data);
/// \brief Get an unsigned 16-bit integer from a CanMessage.
CanState CanNode_getData_uint16 (const CanMessage* msg, uint16_t* data);
/// \brief Get a signed 32-bit integer from a CanMessage.
CanState CanNode_getData_int32  (const CanMessage* msg,  int32_t* data);
/// \brief Get an unsigned 32-bit integer from a CanMessage.
CanState CanNode_getData_uint32 (const CanMessage* msg, uint32_t* data);
//@}

/**
 * \name getDataArr Functions
 * These functions get arrays of various integer types from a CanMessage. They are
 * non-blocking. If the data is not of the same type as the called function
 * \ref INVALID_TYPE is returned.
 * @{
 */
/// \brief Get an array of signed 8-bit integers from a CanMessage.
CanState CanNode_getDataArr_int8   (const CanMessage* msg, int8_t data[7], uint8_t* len);
/// \brief Get an array of unsigned 8-bit integers from a CanMessage.
CanState CanNode_getDataArr_uint8  (const CanMessage* msg, uint8_t data[7], uint8_t* len);
/// \brief Get an array of signed 16-bit integers from a CanMessage.
CanState CanNode_getDataArr_int16  (const CanMessage* msg, int16_t data[2], uint8_t* len);
/// \brief Get an array of unsigned 16-bit integers from a CanMessage.
CanState CanNode_getDataArr_uint16 (const CanMessage* msg, uint16_t data[2], uint8_t* len);
//@}

/*@}*/

/*@}*/
#endif //_CAN_NODE_H_
