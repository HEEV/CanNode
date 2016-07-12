/**
 * \file CanTypes.h
 * \brief Type definitions for CanNode functions.
 *
 * Defines various structs and enums for storing states of CanNodes.
 * \author Samuel Ellicott
 * \date 7-12-16
 */
#ifndef _CAN_TYPES_H_
#define _CAN_TYPES_H_

#include <stdint.h>
#include <stdbool.h>

#ifndef MAX_NODES
	#define MAX_NODES 3
#endif

#ifndef NUM_FILTERS
	#define NUM_FILTERS 4
#endif

typedef enum {
	CAN_BITRATE_10K,
	CAN_BITRATE_20K,
	CAN_BITRATE_50K,
	CAN_BITRATE_100K,
	CAN_BITRATE_125K,
	CAN_BITRATE_250K,
	CAN_BITRATE_500K,
	CAN_BITRATE_750K,
	CAN_BITRATE_1000K,
} can_bitrate;

typedef enum {
	BUS_OK,
	BUS_BUSY,
	NO_DATA,
	BUS_OFF
} can_bus_state;


/**
 * \enum CanNodeType
 * \brief Defines sensor type and base id
 *
 */
typedef enum {
	ANALOG      = 1200, ///< Analog sensor (depricated)
	RELAY       = 1264, ///< Relay
	LED	        = 1392, ///< LEDs
	MEGASQUIRT  = 1520, ///< Megasquirt EFI controller
	SWITCH      = 1584, ///< All types of switches and potentiometers
	PRESSURE    = 1648, ///< Pressure sensors
	TEMPURATURE = 1664, ///< Tempurature sensors
	UNCONFIG    = 1984  ///< Unconfigured nodes
} CanNodeType;

/** 
 * \enum CanNodeDataType
 * \brief CanNode Data Type Enum.
 *
 * Enum that contains various types of data that can be stored in a CanNode message.
 * Since the actual values for this enum are stored in a 3-bit value, there
 * can only be 8 individual types in this enum.
 */
typedef enum {
	CAN_UINT8,     ///< Unsigned 8-bit integer
	CAN_INT8,      ///< Signed 8-bit integer
	CAN_UINT16,    ///< Unsigned 16-bit integer
	CAN_INT16,     ///< Signed 16-bit integer
	CAN_UINT32,    ///< Unsigned 32-bit integer
	CAN_INT32,     ///< Signed 32-bit integer
	CAN_BIT_FIELD, ///< Each bit defines the state of value
	CAN_CUSTOM     ///< Catch all
} CanNodeDataType;

/** 
 * \enum CanNodeMsgType
 * \brief CanNode Message Type Enum.
 *
 * Enum for command that can be sent and recieved from a CanNode.
 * Since the actual values for this enum are sent in a 5-bit value, there can
 * only be 32 individual types in this enum.
 */
typedef enum {
	CAN_DATA,           ///< Normal operation, device is sending data to other nodes
	CAN_DATA_MODE,      ///< Sent by master to enter data mode (default mode)
	CAN_CONFIG_MODE,    ///< Sent by master to enter config mode
	CAN_SET_ID,	        ///< Sent by master to change the id of the node
	CAN_ID_SET_ERROR,   ///< Error sent by node if the new id is not availible
	CAN_CONFIG_ERROR,   ///< General configuration error
	CAN_GET_NAME,       ///< Ask a node for its name
	CAN_GET_INFO,       ///< Ask a node for its info
	CAN_NAME_INFO       ///< Message is part of a name/info message
} CanNodeMsgType;

//types of errors that can be returned by functions
typedef enum {
	DATA_OK,
	DATA_ERROR,
	INVALID_TYPE,
	DATA_OVERFLOW,
} CanNodeFmtError;


/**
 * \struct CanMessage
 * \brief Stucture for holding a CANBus message.
 *
 */
typedef struct {
	uint16_t id;     ///< ID of the sender
	uint8_t len;     ///< length of the message
	bool rtr;        ///< Asking for data (true) or sending data (false)
	uint8_t data[8]; ///< Data
} CanMessage;	

/**
 * \typedef filterHandler
 * \brief Function pointer to a function that accepts a CanMessage pointer
 *
 * A function of this type should look like 
 *
 * <code> void foo(CanMessage* msg) </code>
 *
 * Functions of this type can be used to handle filter matches. These functions
 * are added set to handle filters with the CanNode_addFilter() function. They are
 * called by the CanNode_checkForMessages() function
 *
 * \see CanNode_addFilter
 * \see CanNode_checkForMessages
 */
typedef void (*filterHandler)(CanMessage* data);

/**
 * \struct CanNode
 * \brief Stucture for holding information about a CanNode
 *
 * Members should never be accessed directly, instead use functions in
 * CanNode.h
 *
 * \see CanNode.h
 * \see CanNode_init
 * \see CanNode_addFilter
 * \see CanNode_setName
 * \see CanNode_setInfo
 */
typedef struct {
	uint16_t id;                       ///< id of the node
	uint8_t status;                    ///< status of the node (not currently used)
	uint16_t filters[NUM_FILTERS];     ///< array of id's to handle
	filterHandler handle[NUM_FILTERS]; ///< array of function pointers to call
	                                      ///< when a id in filters is found
	CanNodeType sensorType;            ///< Type of sensor
	char* nodeName;                    ///< Name of the node 
                                          ///< (points to an address in flash)
	uint8_t nodeNameLen;               ///< Length of the string
	char* nodeInfo;                    ///< Information about the sensor
	                                      ///< (points to an area in flash)
	uint16_t nodeInfoLen;              ///< Length of the string
} CanNode;

#endif //_CAN_TYPES_H_
