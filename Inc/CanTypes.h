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
	/// Maximum number of nodes stored in flash. Can be overwriten by redefinition
	#define MAX_NODES 4
#endif

#ifndef NUM_FILTERS
	/// Number of filters each node can have. Can be overwriten by redefinition
	#define NUM_FILTERS 4
#endif

/// Key to unlokck stm32 flash memory
#define FLASH_FKEY1 0x45670123
/// Key to unlokck stm32 flash memory
#define FLASH_FKEY2 0xCDEF89AB

/// Total length of the name/info string
#define TOTAL_INFO_LEN 200
/// Maximum length of a name string for the CanNode_getName()
#define MAX_NAME_LEN 30
/// Maximum length of a info string for the CanNode_getInfo()
#define MAX_INFO_LEN TOTAL_INFO_LEN - MAX_NAME_LEN

/**
 * \enum canBitrate
 * \brief Defines various avalible bitrates for the CANBus
 *
 */
typedef enum {
	CAN_BITRATE_10K,  ///< 10k baud
	CAN_BITRATE_20K,  ///< 20k baud
	CAN_BITRATE_50K,  ///< 50k baud
	CAN_BITRATE_100K, ///< 100k baud
	CAN_BITRATE_125K, ///< 125k baud
	CAN_BITRATE_250K, ///< 250k baud
	CAN_BITRATE_500K, ///< 500k baud
	CAN_BITRATE_750K, ///< 750k baud
	CAN_BITRATE_1000K,///< 1M baud
} canBitrate;

/**
 * \enum CanState 
 * \brief Defines various errors and states of the CANBus 
 *
 */
typedef enum {
	BUS_OK = 0,    ///< Good state
	DATA_OK = 0,   ///< Good state
	DATA_ERROR,    ///< Catch all data error
	NO_DATA,       ///< No data availible on the bus
	INVALID_TYPE,  ///< The bus has data but of a different type then asked for
	DATA_OVERFLOW, ///< Too much data was was put in the message
	BUS_BUSY,      ///< The bus is working with someone else right now
	BUS_OFF        ///< The bus is off - call can_init() and can_enable()
} CanState;

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
	char nodeInfoBuff[TOTAL_INFO_LEN]; ///< (points to an address in flash)
} CanNode;

#endif //_CAN_TYPES_H_
