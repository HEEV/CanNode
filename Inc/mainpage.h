/**
 * \mainpage \ref CanNode_Module library documentation
 *
 * \tableofcontents
 *
 * \section About About The CanNode Library
 *
 * The [CanNode library](\ref CanNode_Module) attempts to provide an easy to use
 * interface for connecting CANBus devices together over a network. The library
 * targets stm32 devices with a CANBus hardware built in.
 * \ref CanNodeProtocol page.
 *
 * The \ref CanNode_Module library was written by Samuel Ellicott for the
 * Cedarville University Shell Eco Marathon team.
 *
 * \section Useage
 *
 * Nodes are created by calling CanNode_init() which returns a pointer to
 * an initilized CanNode.
 * This pointer is then used to send data over the CANBus by means of the
 * \ref sendData functions. The CanNode structure also provides for the storage
 * of a name and information string which can be retrieved and modified by the
 * [info functions](\ref infoFunctions). 
 *
 * a basic implementation for a device using the %CanNode library would be as
 * follows
 *
 * ~~~~~~~~~~~~~ {.c}
 * #include "CanNode.h"
 * #define LISTEN_ID 1520 //Id to be listening for
 *
 * //this is global so that the handler functions can send messages
 * CanNode* node;
 *
 * int main();
 * // prototypes for handler function
 * void nodeHandler(CanMessage* msg);
 * void tempuratureHandler(CanMessage* msg);
 *
 * int main() {
 *     uint16_t data_to_send;
 *     const char[] NAME = "Tempurature sensor 3";
 *     const char[] INFO = "Tempurature sensor for the blah blah blah.";
 *
 *     //Analog sensor node 3. The boolean value makes the library search for
 *     //old values before putting in the specified configuration
 *     //ENGINE_TEMP is a uint16_t constant defined in the CanNodeType enum
 *     node = CanNode_init(ENGINE_TEMP, tempuratureRTR);
 *
 *     //set name and information strings
 *     CanNode_setName(node, NAME);
 *     CanNode_setInfo(node, INFO);
 *
 *     //add a filter to listen for other nodes, handle it with a function
 *     CanNode_addFilter(node, LISTEN_ID, nodeHandler);
 *
 *     //infinate loop
 *     for(;;) {
 *         //necessary for handler functions to work
 *         CanNode_checkForMessages();
 *	       //get data to send and put it in varible
 *         CanNode_sendData_uint16(node, data_to_send);
 *     }
 * }
 *
 * //RTR messages are when a caller wants a particular node to provide some
 * //data back. Therefore the RTR handler function should either send data back
 * //or flag the main process to send data back.
 * void tempuratureRTR(CanMessage* msg) {
 *     //get data
 *     uint16_t data;
 *     data = getTempurature();
 *     //send data back
 *     CanNode_sendData_uint16(node, data);
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
 * For more information about the underlying protocol, read \ref
 *CanNodeProtocol.
 *
 * \subsection Overriding Overriding Other Nodes
 *
 * For devices which want to override data from another device on the bus. eg.
 * a computer that wants to start the engine, overriding a start engine button.
 * It is a good idea to just make a node in ram instead of using CanNode_init().
 * This is so that calls to CanNode_getName() or CanNode_getInfo() do not result
 * in conflicting or corrupted data to the caller.
 *
 * The main problem (and benefit) of making a node in ram is that these nodes
 *are not
 * handled in CanNode_checkForMessages(). This means that the default %CanNode
 * handleing code will not run for this node. Therefore filters and handlers
 *added
 * with CanNode_addFilter() will not run either.
 *
 * Example code
 *
 * ~~~~~~~~~~~~~ {.c}
 * uint16_t data; //place to put data you want to write
 * CanNode node;  //temporary node in ram, changes will not be saved after
 *                //restart
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
 * released under a [BSD 3-Clause
 *License](https://opensource.org/licenses/BSD-3-Clause)
 */

/**
* \page CanNodeProtocol %CanNode Protocol Implementation Details
* \tableofcontents
*
* This page is about my higher level CAN protocol, for more about the underlying
* protocol see the [wikipedia article](https://en.wikipedia.org/wiki/CAN_bus).
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
* unwanted packets.
*
* \subsection ConfigurationByte Configuration Byte
* The first byte of the CAN message is the configuration byte. This allows for
* the recipient of the message to know what type of message and data the sender
* broadcast. Since the CANbus is not intended to function in this way, this
* feature should be used sparingly (if at all). My intended use of this
* feature is to allow the setup or re-setup the devices on the node. What this
* does is allows a computer to reconfigure one node like a temperature sensor
* into a pressure sensor. The first five bits are used to store the message
* type, the last three bits are used for the data type.
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
*
* Each CanNode reserves for itself four ids. The first of these ids (base id)
* is used to broadcast data on. It can also be called by rtr and the latest
* data collected from it. The second id is for getting the Name string from the
* node. The third id is for getting the info string. Finally the fourth id is
* used for configuring the node at runtime.
*
* Devices are addressed using a base address for the 0th sensor of that type,
* then a number of subsequent addresses are used for other sensors of the same
* type. This is the method employed by Megasquirt.
*
* For a list of devices currently planned for the Supermileage CanBus see
* \ref CanNodeType
*
*  ### Example List of Addresses ###
*  * 1264 - 1391: Relays
*   * 1264 - 1267: Starter Relay
*   * 1268 - 1271: Brights
*   * 1272 - 1275: Left Turn Signal
*   * 1276 - 1280: Right Turn Signal
*  * 1392 - 1519: LEDs
*  * 1520 - 1583: Megasqurit
*  * 1584 - 1647: Switches
*   * 1584 - 1587: Engine Stop Switch
*   * 1588 - 1592: Engine Start Switch
*  * 1648 - 1663: Pressure Sensors
*   * 1648 - 1651: Pitot Tube
*  * 1664 - 1728: Temperature Sensors
*
* \subsection Data Data
* The data format is specified from the CanNodeDataType enum and it determines
* the size of the message sent. Data is sent in little-endian format (less
* significant bytes appearing in lower adresses.) For example 16-bit numbers
* have the lower 8 bits stored in CanMessage::data[1] and the upper bytes
* stored in CanMessage::data[2] (data[0] is used for the configuration byte).
*
* For 8-bit types (int8_t and uint8_t) a CanNodeMessage is two bytes long, for
* 16-bit types (int16_t and uint16_t) messages are 3 bytes long. 32-bit types
* are 5 bytes. 64-bit types are not supported.
*
* - - -
*
* \par Copyright:
* Copyright 2016 Samuel Ellicott. All rights reserved.
*
* \par License:
* Unless otherwise specified all code in the %CanNode library is
* released under a [BSD 3-Clause
* License](https://opensource.org/licenses/BSD-3-Clause)
*/
