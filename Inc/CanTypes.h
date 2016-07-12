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

typedef struct {
	uint16_t id;
	uint8_t len;
	bool rtr;
	uint8_t data[8];
} CanMessage;	

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

/** CanNode Data Type Enum.
 * Enum that contains various types of data that can be stored in a CanNode message.
 * Since the actual values for this enum are stored in a 3-bit value, there
 * can only be 8 individual types in this enum.
 */
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

/** CanNode Message Type Enum.
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

#endif //_CAN_TYPES_H_
