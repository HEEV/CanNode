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
 *     const char[] INFO = "Tempurature sensor for the blah blah blah. This can be up 170 characters long."
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
