/**
 * \mainpage %CanNode library documentation
 *
 * \tableofcontents
 *
 * \section About The CanNode Library
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
 * \section Important Files
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
