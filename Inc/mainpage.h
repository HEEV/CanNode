/**
 * \mainpage %CanNode library documentation
 *
 * \tableofcontents
 *
 * \section About About The CanNode Library
 *
 * The %CanNode library attempts to provide an easy to use interface for 
 * connecting CANBus devices together over a network. The library targets stm32 
 * devices with a CANBus hardware built in; although porting to a PC application 
 * should be possible. 
 * 
 * The library also attempts to provide a method of updating simple (ie. sensor)
 * nodes without reprogramming. For more information on that see the 
 * \ref CanNodeProtocol page. 
 *
 * The CanNode library was written by Samuel Ellicott for the Cedarville 
 * University Shell Eco Marathon team. 
 *
 *
 * \section Useage
 *
 * Nodes are created by calling CanNode_init() which returns a pointer to
 * an initilized CanNode struct saved in flash memory on the device. 
 * This pointer is then used to send data over the CANBus by means of the 
 * \ref sendData functions. The CanNode structure also provides for the storage
 * of a name and information string which can be retrieved and modified by the 
 * [info functions](\ref infoFunctions). Information is saved through power
 * downs.
 *
 * a basic implementation for a device using the %CanNode library would be as 
 * follows
 *
 * ~~~~~~~~~~~~~ {.c}
 * #include "CanNode.h"
 * #define LISTEN_ID 1520 //Id to be listening for
 *
 * int main();
 * // prototypes for handler function
 * void nodeHandler(CanMessage* msg);
 *
 * int main() {
 *     CanNode* node;
 *     uint16_t data_to_send;
 *     const char[] NAME = "Tempurature sensor 3";
 *     const char[] INFO = "Tempurature sensor for the blah blah blah. This can be up MAX_INFO_LEN characters long."
 *
 *     //Analog sensor node 3. The boolean value makes the library search for
 *     //old values before putting in the specified configuration
 *     node = CanNode_init(TEMPURATURE, TEMPURATURE+3, false); 
 *
 *     //set name and information strings
 *     CanNode_setName(node, NAME,(uint8_t) sizeof(NAME));
 *     CanNode_setInfo(node, INFO,(uint8_t) sizeof(INFO));
 *
 *     //add a filter to listen for other nodes, handle it with a function
 *     CanNode_addFilter(node, LISTEN_ID, nodeHandler);
 *
 *     for(;;) {
 *	       //get data to send and put it in varible
 *         CanNode_sendData_uint16(node, data_to_send);
 *     }
 * }
 *
 * void nodeHandler(CanMessage* msg) {
 *     uint16_t data;
 *
 *     if(CanNode_getData_uint16(msg, &data)==DATA_OK){
 *	       //do something cool with the data like flash some lights
 *     }
 * }
 *
 * ~~~~~~~~~~~~~
 * 
 * Of course code for clock and GPIO initilization would also be necessary.
 * 
 * For more information about the underlying protocol, read \ref CanNodeProtocol
 * .
 *
 * \subsection Overriding Overriding Other Nodes
 *
 * For devices which want to override data from another device on the bus. eg.
 * a computer that wants to start the engine, overriding a start engine button.
 * It is a good idea to just make a node in ram instead of using CanNode_init().
 * This is so that calls to CanNode_getName() or CanNode_getInfo() do not result
 * in conflicting or corrupted data to the caller. 
 *
 * The main problem (and benefit) of making a node in ram is that these nodes are not 
 * handled in CanNode_checkForMessages(). This means that the default %CanNode 
 * handleing code will not run for this node. Therefore filters and handlers added
 * with CanNode_addFilter() will not run either.
 *
 * Example code
 *
 * ~~~~~~~~~~~~~ {.c}
 * uint16_t data; //place to put data you want to write
 * CanNode node;  //temporary node in ram, changes will not be saved after restart 
 * node.id = id_to_override; //id of the node to override
 *
 * CanNode_sendData_uint16(node, data); //send the data
 * ~~~~~~~~~~~~~
 *
 * \section Files Important Files
 *
 * The most important file is CanNode.h it provides functions for reading and
 * writing to devices on the CANBus. 
 *
 * - - -
 *
 * \par Copyright:
 * Copyright 2016 Samuel Ellicott. All rights reserved.
 *
 * \par License:
 * Unless otherwise specified all code in the %CanNode library is 
 * released under a [BSD 3-Clause License](https://opensource.org/licenses/BSD-3-Clause)
 */


 /**
 * \page CanNodeProtocol %CanNode Protocol Implementation Details
 * \tableofcontents
 *
 * \section General General Information About the Protocol
 * The goal of the %CanNode protocol is to provide a uniform and safe method for
 * passing data between CAN devices. It provides an interface to pass various
 * integer types (see \ref CanNodeDataType). The protocol also provides a 
 * standard set of messages to transmit between CAN devices (see 
 * \ref CanNodeMsgType).
 *
 * \section Implementation Implementation Details
 * Data is sent over the CANbus in messages containing an 11-bit id and up to 8 
 * bytes of data. Messages can either request information (RTR) for a certain 
 * id, or provide data to other nodes on the bus. The id gives information about
 * the data contained in the packet, so that receiving nodes can filter out 
 * unwanted packets. See \ref CANFiltering for more information
 *
 * \subsection ConfigurationByte Configuration Byte
 * The first byte of the CAN message is the configuration byte. It allows for 
 * two way communication between a master node and the devices on the CANbus. 
 * Since the CANbus is not intended to function in this way, this feature 
 * should be used sparingly (if at all). My intended use of this feature is to 
 * allow the setup or re-setup the devices on the node. What this does is 
 * allows a computer to reconfigure one node like a temperature sensor into a 
 * pressure sensor. The first five bits are used to store the message type, the 
 * last three bits are used for the data type. 
 *
 * |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
 * |:-----:|:-----:|:-----:|:-----:|:-----:|:-----:|:-----:|:-----:|
 * | FMT2  | FMT1  | FMT0  | MSG4  | MSG3  | MSG2  | MSG1  | MSG0  |
 *
 * #### MSG[4:0] ####
 * Defines the type of message being transmitted. It an be one of the types 
 * defined in \ref CanNodeMsgType.
 *
 * Most of the time data (\ref CAN_DATA) is sent and only type information is
 * necessary. However, in certain circumstances other message types are 
 * required. Most of the other types are used internally in the %CanNode 
 * Library.
 *
 * #### FMT[2:0] ####
 * Defines how the data is stored in the Data portion of the message. 
 * It can be one of types defined in \ref CanNodeDataType.
 *
 * \subsection Addressing  CanNode Addressing
 * *NOTE: This section is tentative and subject to change*
 *
 * Devices are addressed using a base address for the 0th sensor of that type, 
 * then a number of subsequent addresses are used for other sensors of the same 
 * type. This is the method employed by Megasquirt.
 *
 *  ### List of Addresses ###
 *  * 1264 - 1391: Relays
 *  * 1392 - 1519: LEDs
 *  * 1520 - 1583: Megasqurit
 *  * 1584 - 1647: Switches
 *  * 1648 - 1663: Pressure Sensors
 *  * 1664 - 1728: Temperature Sensors
 *  * 1984 - 2047: Unconfigured Nodes
 *
 * \subsection Data Data
 * The data format is specified from the CanNodeDataType enum and it determines
 * the size of the message sent. Data is sent in little-endian format (less
 * significant bytes appearing in lower adresses.) For example 16-bit numbers
 * have the lower 8 bits stored in CanMessage::data[1] and the upper bytes 
 * stored in CanMessage::data[2] (data[0] is used for the configuration byte).
 */
