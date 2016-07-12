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
#include "CanTypes.h"
#include "can.h"

/// \brief Initilize a CanNode from given parameters.
uint16_t CanNode_init(CanNode* node, CanNodeType type, uint16_t id);
/// \brief Add a filter and handler to a given CanNode.
bool CanNode_addFilter(CanNode* node, uint16_t filter, filterHandler handle);
/// \brief Check all initilized CanNodes for messages and call callbacks.
void CanNode_checkForMessages();

//NOTE: these functions are blocking and any inportant information
//dilivered in the period using these functions will be lost. 
//These functions get the Name and Info strings from the device with the given id

/// \brief Get an name string from a CAN id.
void CanNode_getName(uint16_t id, char* name, uint8_t buff_len, uint32_t timeout);
/// \brief Get an info string from a CAN id.
void CanNode_getInfo(uint16_t id, char* info, uint16_t buff_len, uint32_t timeout);

//These functions set the name and info strings into flash memory of the chip
/// \brief Set the name string for a CanNode into flash.
void CanNode_setName(const CanNode* node, const char* name, uint8_t buff_len);
/// \brief Set the info string for a CanNode into flash.
void CanNode_setInfo(const CanNode* node, const char* info, uint16_t buff_len);

//functions for setting data
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

/// \brief Send an array of uinsigned 8-bit integers.
CanNodeFmtError CanNode_sendDataArr_int8   (const CanNode* node, int8_t* data, uint8_t len);
/// \brief Send an array of signed 8-bit integers.
CanNodeFmtError CanNode_sendDataArr_uint8  (const CanNode* node, uint8_t* data, uint8_t len);
/// \brief Send an array of uinsigned 16-bit integers.
CanNodeFmtError CanNode_sendDataArr_int16  (const CanNode* node, int16_t* data, uint8_t len);
/// \brief Send an array of signed 16-bit integers.
CanNodeFmtError CanNode_sendDataArr_uint16 (const CanNode* node, uint16_t* data, uint8_t len);

//functions for getting data
/// \brief Get a signed 8-bit integer from a CanMessage.
CanNodeFmtError CanNode_getData_int8   (const CanMessage* msg,   int8_t* data);
/// \brief Get an unsigned 8-bit integer from a CanMessage.
CanNodeFmtError CanNode_getData_uint8  (const CanMessage* msg,  uint8_t* data);
/// \brief Get a signed 16-bit integer from a CanMessage.
CanNodeFmtError CanNode_getData_int16  (const CanMessage* msg,  int16_t* data);
/// \brief Get an unsigned 16-bit integer from a CanMessage.
CanNodeFmtError CanNode_getData_uint16 (const CanMessage* msg, uint16_t* data);
/// \brief Get a signed 32-bit integer from a CanMessage.
CanNodeFmtError CanNode_getData_int32  (const CanMessage* msg,  int32_t* data);
/// \brief Get an unsigned 32-bit integer from a CanMessage.
CanNodeFmtError CanNode_getData_uint32 (const CanMessage* msg, uint32_t* data);

/// \brief Get an array of signed 8-bit integers from a CanMessage.
CanNodeFmtError CanNode_getDataArr_int8   (const CanMessage* msg, int8_t data[7]);
/// \brief Get an array of unsigned 8-bit integers from a CanMessage.
CanNodeFmtError CanNode_getDataArr_uint8  (const CanMessage* msg, uint8_t data[7]);
/// \brief Get an array of signed 16-bit integers from a CanMessage.
CanNodeFmtError CanNode_getDataArr_int16  (const CanMessage* msg, int16_t data[2]);
/// \brief Get an array of unsigned 16-bit integers from a CanMessage.
CanNodeFmtError CanNode_getDataArr_uint16 (const CanMessage* msg, uint16_t data[2]);

#endif //_CAN_NODE_H_
