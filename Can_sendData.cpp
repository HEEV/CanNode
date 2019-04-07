#include "CanNode.h"

/** \ingroup CanNode_SendData_Functions
 *
 * \param node [in] Specifies the node to send data from (basically an id)
 * \param data [in] Data to send
 *
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_int32()
 * \see CanNode_sendData_uint32()
 *
 * \see CanNode_sendDataArr_int8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_int16()
 */
void CanNode::sendData_int8(int8_t data) const {
  CanMessage msg;
  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_INT8) << 5) | (0x1F & CAN_DATA);
  // data
  msg.data[1] = (uint8_t)data;
  // set other odds and ends
  msg.rtr = false;
  msg.len = 2;
  msg.id = this->id;
  can_tx(&msg, 5);
}

/**
 * \param[in] node Specifies the node to send data from (basically an id)
 * \param[in] data Data to send
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_int32()
 * \see CanNode_sendData_uint32()
 *
 * \see CanNode_sendDataArr_int8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_int16()
 */
void CanNode::sendData_uint8(uint8_t data) const {
  CanMessage msg;
  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_UINT8) << 5) | (0x1F & CAN_DATA);
  // data
  msg.data[1] = data;
  // set other odds and ends
  msg.len = 2;
  msg.rtr = false;
  msg.id = this->id;
  can_tx(&msg, 5);
}

/**
 * \param[in] node Specifies the node to send data from (basically an id)
 * \param[in] data Data to send
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_int32()
 * \see CanNode_sendData_uint32()
 *
 * \see CanNode_sendDataArr_int8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_int16()
 */
void CanNode::sendData_int16(int16_t data) const {
  CanMessage msg;
  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_INT16) << 5) | (0x1F & CAN_DATA);
  // data
  msg.data[1] = (uint8_t)(data & 0x00ff);
  msg.data[2] = (uint8_t)((data & 0xff00) >> 8);
  // set other odds and ends
  msg.len = 3;
  msg.rtr = false;
  msg.id = this->id;
  can_tx(&msg, 5);
}

/**
 * \param[in] node Specifies the node to send data from (basically an id)
 * \param[in] data Data to send
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_int32()
 * \see CanNode_sendData_uint32()
 *
 * \see CanNode_sendDataArr_int8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_int16()
 */
void CanNode::sendData_uint16(uint16_t data) const {
  CanMessage msg;
  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_UINT16) << 5) | (0x1F & CAN_DATA);
  // data
  msg.data[1] = (uint8_t) (data & 0x00ff);
  msg.data[2] = (uint8_t)((data & 0xff00) >> 8);
  // set other odds and ends
  msg.len = 3;
  msg.rtr = false;
  msg.id = this->id;
  can_tx(&msg, 5);
}

/**
 * \param[in] node Specifies the node to send data from (basically an id)
 * \param[in] data Data to send
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_uint32()
 *
 * \see CanNode_sendDataArr_int8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_int16()
 */
void CanNode::sendData_int32(int32_t data) const {
  CanMessage msg;
  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_INT32) << 5) | (0x1F & CAN_DATA);
  // data
  msg.data[1] = (uint8_t) (data & 0x000000ff);
  msg.data[2] = (uint8_t)((data & 0x0000ff00) >> 8);
  msg.data[3] = (uint8_t)((data & 0x00ff0000) >> 16);
  msg.data[4] = (uint8_t)((data & 0xff000000) >> 24);
  // set other odds and ends
  msg.len = 5;
  msg.rtr = false;
  msg.id = this->id;
  can_tx(&msg, 5);
}

/**
 * \param[in] node Specifies the node to send data from (basically an id)
 * \param[in] data Data to send
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_int32()
 *
 * \see CanNode_sendDataArr_int8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_int16()
 */
void CanNode::sendData_uint32(uint32_t data) const {
  CanMessage msg;
  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_UINT32) << 5) | (0x1F & CAN_DATA);
  // data
  msg.data[1] = (uint8_t) (data & 0x000000ff);
  msg.data[2] = (uint8_t)((data & 0x0000ff00) >> 8);
  msg.data[3] = (uint8_t)((data & 0x00ff0000) >> 16);
  msg.data[4] = (uint8_t)((data & 0xff000000) >> 24);
  // set other odds and ends
  msg.len = 5;
  msg.rtr = false;
  msg.id = this->id;
  can_tx(&msg, 5);
}

void CanNode::sendData_float(float data) const {
  CanMessage msg;
  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_FLOAT) << 5) | (0x1F & CAN_DATA);

  // set up a pointer to the float to pack it into the data structure
  uint8_t* flt_ptr = (uint8_t*) &data;
  // data
  msg.data[1] =  *flt_ptr;
  msg.data[2] = *(flt_ptr+1);
  msg.data[3] = *(flt_ptr+2);
  msg.data[4] = *(flt_ptr+3);
  // set other odds and ends
  msg.len = 5;
  msg.rtr = false;
  msg.id = this->id;
  can_tx(&msg, 5);
}

/**
 * This function will send a CanMessage from a particular CanNode No type checking byte will be
 * used. You can send whatever data you want with this, but you will have to know how to reconstruct
 * it on the recieveing end. Use the other sendData functions for type checking.
 *
 * \param[in] Pointer to a CanMessage with the data, len, and rtr fields filled. 
 *
 * \see sendData_int8()
 * \see sendData_uint8()
 * \see sendData_int16()
 * \see sendData_uint16()
 * \see sendData_int32()
 * \see sendData_uint32()
 *
 * \see sendDataArr_int8()
 * \see sendDataArr_uint8()
 * \see sendDataArr_uint8()
 * \see sendDataArr_int16()
 */
void CanNode::sendData_custom(CanMessage* msg) const {
    msg->id = this->id;
    can_tx(msg, 5);
}

/**
 * Sends an array of data over the CANBus.
 * Maximum size for the aray is 7 bytes.
 *
 * \param node Node to send data from (basically an id)
 * \param data An array of data
 * \param len  Length of the data to be sent. Maximum length of 7
 *
 * \returns \ref DATA_OVERFLOW if len > 7, \ref DATA_OK otherwise
 *
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_int16()
 * \see CanNode_sendDataArr_uint16()
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_int32()
 * \see CanNode_sendData_uint32()
 */
CanState CanNode::sendDataArr_int8(int8_t *data, uint8_t len) const {
  CanMessage msg;
  // check if valid
  if (len > 7) {
    return DATA_OVERFLOW;
  }

  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_INT8) << 5) | (0x1F & CAN_DATA);
  // data
  for (uint8_t i = 0; i < len; ++i) {
    msg.data[i + 1] = (uint8_t)data[i];
  }

  // set other odds and ends
  msg.len = len + 1;
  msg.rtr = false;
  msg.id = this->id;
  can_tx(&msg, 5);
  return DATA_OK;
}

/**
 * Sends an array of data over the CANBus.
 * Maximum size for the aray is 7 bytes.
 *
 * \param node Pointer to a CanNode
 * \param data An array of data
 * \param len  Length of the data to be sent. Maximum length of 7
 *
 * \returns \ref DATA_OVERFLOW if len > 7, \ref DATA_OK otherwise
 *
 * \see CanNode_sendDataArr_int8()
 * \see CanNode_sendDataArr_int16()
 * \see CanNode_sendDataArr_uint16()
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_int32()
 * \see CanNode_sendData_uint32()
 */
CanState CanNode::sendDataArr_uint8(uint8_t *data, uint8_t len) const {
  CanMessage msg;
  // check if valid
  if (len > 7) {
    return DATA_OVERFLOW;
  }

  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_UINT8) << 5) | (0x1F & CAN_DATA);
  // data
  for (uint8_t i = 0; i < len; ++i) {
    msg.data[i + 1] = data[i];
  }

  // set other odds and ends
  msg.len = len + 1;
  msg.rtr = false;
  msg.id = this->id;
  can_tx(&msg, 5);
  return DATA_OK;
}

/**
 * Sends an array of data over the CANBus.
 * Maximum size for the aray is 2 integers.
 *
 * \param node Pointer to a CanNode
 * \param data An array of data
 * \param len  Length of the data to be sent. Maximum length of 2
 *
 * \returns \ref DATA_OVERFLOW if len > 2, \ref DATA_OK otherwise
 *
 * \see CanNode_sendDataArr_int8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_uint16()
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_int32()
 * \see CanNode_sendData_uint32()
 */
CanState CanNode::sendDataArr_int16(int16_t *data, uint8_t len) const {
  CanMessage msg;
  // check if valid
  if (len > 2) {
    return DATA_OVERFLOW;
  }

  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_INT16) << 5) | (0x1F & CAN_DATA);
  // data
  for (uint8_t i = 0; i < len; ++i) {
    msg.data[i * 2 + 1] = (uint8_t) (data[i] & 0x00ff);
    msg.data[i * 2 + 2] = (uint8_t)((data[i] & 0xff00) >> 8);
  }

  // set other odds and ends
  msg.len = len * 2 + 1;
  msg.rtr = false;
  msg.id = this->id;
  can_tx(&msg, 5);
  return DATA_OK;
}

/**
 * Sends an array of data over the CANBus.
 * Maximum size for the aray is 2 integers.
 *
 * \param node Pointer to a CanNode
 * \param data An array of data
 * \param len  Length of the data to be sent. Maximum length of 2
 *
 * \returns \ref DATA_OVERFLOW if len > 2, \ref DATA_OK otherwise
 *
 * \see CanNode_sendDataArr_int8()
 * \see CanNode_sendDataArr_uint8()
 * \see CanNode_sendDataArr_int16()
 *
 * \see CanNode_sendData_int8()
 * \see CanNode_sendData_uint8()
 * \see CanNode_sendData_int16()
 * \see CanNode_sendData_uint16()
 * \see CanNode_sendData_int32()
 * \see CanNode_sendData_uint32()
 */
CanState CanNode::sendDataArr_uint16(uint16_t *data, uint8_t len) const {
  CanMessage msg;
  // check if valid
  if (len >= 3) {
    return DATA_OVERFLOW;
  }

  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_UINT16) << 5) | (0x1F & CAN_DATA);
  // data
  for (uint8_t i = 0; i < len; ++i) {
    msg.data[2*i + 1] = (uint8_t)(data[i] & 0x00ff);
    msg.data[2*i + 2] = (uint8_t)((data[i] & 0xff00) >> 8);
  }

  // set other odds and ends
  msg.len = len * 2 + 1;
  msg.rtr = false;
  msg.id = this->id;
  can_tx(&msg, 5);
  return DATA_OK;
}