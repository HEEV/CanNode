#include "CanNode.h"
/**
 * Interpert a CanMessage as a signed 8 bit integer (will return error if incorrect)
 *
 * This function is intended to be used in the handler functions
 * (see CanNode_addFilter()) for extracting useful data from recieved messages.
 *
 * Example code - example uses uint16 function but still holds for any type
 *
 * ~~~~~~~~~~~~ {.c}
 * void nodeHandler(CanMessage* msg) {
 *  uint16_t data;
 *  if(CanNode_getData_uint16(msg, &data)==DATA_OK){
 *      //do something cool with the data like flash some lights
 *  }
 *  //the data is stored in data
 * ~~~~~~~~~~~~
 *
 * \param msg[in] Message recieved from someone else, should contain a int8
 * \param data[out] Place for the data extracted from the msg will be stored.
 *
 * \returns The function returns \ref DATA_ERROR if the message is null,
 * \ref INVALID_TYPE if the message doesn't contain the same type as the function,
 * or \ref DATA_OK if the function succeeded.
 */
CanState CanNode::getData_int8(const CanMessage *msg, int8_t *data) {

  if (msg == nullptr) {
    return DATA_ERROR;
  }

  CanState ret = DATA_OK;
  // check configuration byte
  if ( msg->len != 2 ||                      // not right length
      (msg->data[0] >> 5)   != CAN_INT8 ||   // not right type
      (msg->data[0] & 0x1F) != CAN_DATA) {   // not data

    ret = INVALID_TYPE;
  }

  // data
  *data = (int8_t)msg->data[1];

  return ret;
}

/**
 * Interpert a CanMessage as a unsigned 8 bit integer (will return error if incorrect)
 *
 * This function is intended to be used in the handler functions
 * (see CanNode_addFilter()) for extracting useful data from recieved messages.
 *
 * Example code - example uses uint16 function but still holds for any type
 *
 * ~~~~~~~~~~~~ {.c}
 * void nodeHandler(CanMessage* msg) {
 *  uint16_t data;
 *  if(CanNode_getData_uint16(msg, &data)==DATA_OK){
 *      //do something cool with the data like flash some lights
 *  }
 *  //the data is stored in data
 * ~~~~~~~~~~~~
 *
 * \param msg[in] Message recieved from someone else, should contain a int8
 * \param data[out] Place for the data extracted from the msg will be stored.
 *
 * \returns The function returns \ref DATA_ERROR if the message is null,
 * \ref INVALID_TYPE if the message doesn't contain the same type as the function,
 * or \ref DATA_OK if the function succeeded.
 *
 */
CanState CanNode::getData_uint8(const CanMessage *msg, uint8_t *data) {

  if (msg == nullptr) {
    return DATA_ERROR;
  }

  CanState ret = DATA_OK;
  // check configuration byte
  if ( msg->len != 2 ||                      // not right length
      (msg->data[0] >> 5)   != CAN_UINT8 ||  // not right type
      (msg->data[0] & 0x1F) != CAN_DATA) {   // not data

    ret = INVALID_TYPE;
  }

  // data
  *data = msg->data[1];

  return ret;
}

/**
 * Interpert a CanMessage as a signed 16 bit integer (will return error if incorrect)
 *
 * This function is intended to be used in the handler functions
 * (see CanNode_addFilter()) for extracting useful data from recieved messages.
 *
 * Example code - example uses uint16 function but still holds for any type
 *
 * ~~~~~~~~~~~~ {.c}
 * void nodeHandler(CanMessage* msg) {
 *  uint16_t data;
 *  if(CanNode_getData_uint16(msg, &data)==DATA_OK){
 *      //do something cool with the data like flash some lights
 *  }
 *  //the data is stored in data
 * ~~~~~~~~~~~~
 *
 * \param msg[in] Message recieved from someone else, should contain a int8
 * \param data[out] Place for the data extracted from the msg will be stored.
 *
 * \returns The function returns \ref DATA_ERROR if the message is null,
 * \ref INVALID_TYPE if the message doesn't contain the same type as the function,
 * or \ref DATA_OK if the function succeeded.
 *
 */
CanState CanNode::getData_int16(const CanMessage *msg, int16_t *data) {

  if (msg == nullptr) {
    return DATA_ERROR;
  }

  CanState ret = DATA_OK;
  // check configuration byte
  if ( msg->len != 3 ||                      // not right length
      (msg->data[0] >> 5)   != CAN_INT16 ||  // not right type
      (msg->data[0] & 0x1F) != CAN_DATA) {   // not data

    ret = INVALID_TYPE;
  }

  // data
  *data = (int16_t)msg->data[1];
  *data |= (int16_t)(msg->data[2] << 8);

  return ret;
}

/**
 * Interpert a CanMessage as a unsigned 16 bit integer (will return error if incorrect)
 *
 * This function is intended to be used in the handler functions
 * (see CanNode_addFilter()) for extracting useful data from recieved messages.
 *
 * Example code - example uses uint16 function but still holds for any type
 *
 * ~~~~~~~~~~~~ {.c}
 * void nodeHandler(CanMessage* msg) {
 *  uint16_t data;
 *  if(CanNode_getData_uint16(msg, &data)==DATA_OK){
 *      //do something cool with the data like flash some lights
 *  }
 *  //the data is stored in data
 * ~~~~~~~~~~~~
 *
 * \param msg[in] Message recieved from someone else, should contain a int8
 * \param data[out] Place for the data extracted from the msg will be stored.
 *
 * \returns The function returns \ref DATA_ERROR if the message is null,
 * \ref INVALID_TYPE if the message doesn't contain the same type as the function,
 * or \ref DATA_OK if the function succeeded.
 *
 * \see CanNode_getData_int8()
 * \see CanNode_getData_uint8()
 * \see CanNode_getData_int16()
 * \see CanNode_getData_int32()
 * \see CanNode_getData_uint32()
 */
CanState CanNode::getData_uint16(const CanMessage *msg, uint16_t *data) {

  if (msg == nullptr) {
    return DATA_ERROR;
  }

  CanState ret = DATA_OK;
  // check configuration byte
  if ( msg->len != 3 ||                      // not right length
      (msg->data[0] >> 5)   != CAN_UINT16 || // not right type
      (msg->data[0] & 0x1F) != CAN_DATA) {   // not data

    ret = INVALID_TYPE;
  }

  // data
  *data = (uint16_t)msg->data[1];
  *data |= (uint16_t)(msg->data[2] << 8);

  return ret;
}

/**
 * Interpert a CanMessage as a signed 32 bit integer (will return error if incorrect)
 *
 * This function is intended to be used in the handler functions
 * (see CanNode_addFilter()) for extracting useful data from recieved messages.
 *
 * Example code - example uses uint16 function but still holds for any type
 *
 * ~~~~~~~~~~~~ {.c}
 * void nodeHandler(CanMessage* msg) {
 *  uint16_t data;
 *  if(CanNode_getData_uint16(msg, &data)==DATA_OK){
 *      //do something cool with the data like flash some lights
 *  }
 *  //the data is stored in data
 * ~~~~~~~~~~~~
 *
 * \param msg[in] Message recieved from someone else, should contain a int8
 * \param data[out] Place for the data extracted from the msg will be stored.
 *
 * \returns The function returns \ref DATA_ERROR if the message is null,
 * \ref INVALID_TYPE if the message doesn't contain the same type as the function,
 * or \ref DATA_OK if the function succeeded.
 *
 * \see CanNode_getData_int8()
 * \see CanNode_getData_uint8()
 * \see CanNode_getData_int16()
 * \see CanNode_getData_uint16()
 * \see CanNode_getData_uint32()
 */
CanState CanNode::getData_int32(const CanMessage *msg, int32_t *data) {

  if (msg == nullptr) {
    return DATA_ERROR;
  }

  CanState ret = DATA_OK;
  // check configuration byte
  if ( msg->len != 5 ||                      // not right length
      (msg->data[0] >> 5)   != CAN_INT32 || // not right type
      (msg->data[0] & 0x1F) != CAN_DATA) {   // not data

    ret = INVALID_TYPE;
  }

  // data
  *data =  (uint32_t) msg->data[1];
  *data |= (uint32_t)(msg->data[2] << 8);
  *data |= (uint32_t)(msg->data[3] << 16);
  *data |= (uint32_t)(msg->data[4] << 24);

  return ret;
}

/**
 * Interpert a CanMessage as a signed 32 bit integer (will return error if incorrect)
 *
 * This function is intended to be used in the handler functions
 * (see CanNode_addFilter()) for extracting useful data from recieved messages.
 *
 * Example code - example uses uint16 function but still holds for any type
 *
 * ~~~~~~~~~~~~ {.c}
 * void nodeHandler(CanMessage* msg) {
 *  uint16_t data;
 *  if(CanNode_getData_uint16(msg, &data)==DATA_OK){
 *      //do something cool with the data like flash some lights
 *  }
 *  //the data is stored in data
 * ~~~~~~~~~~~~
 *
 * \param msg[in] Message recieved from someone else, should contain a int8
 * \param data[out] Place for the data extracted from the msg will be stored.
 *
 * \returns The function returns \ref DATA_ERROR if the message is null,
 * \ref INVALID_TYPE if the message doesn't contain the same type as the function,
 * or \ref DATA_OK if the function succeeded.
 *
 * \see CanNode_getData_int8()
 * \see CanNode_getData_uint8()
 * \see CanNode_getData_int16()
 * \see CanNode_getData_uint16()
 * \see CanNode_getData_int32()
 */
CanState CanNode::getData_uint32(const CanMessage *msg, uint32_t *data) {

  if (msg == nullptr) {
    return DATA_ERROR;
  }

  CanState ret = DATA_OK;
  // check configuration byte
  if ( msg->len != 5 ||                      // not right length
      (msg->data[0] >> 5)   != CAN_UINT32 || // not right type
      (msg->data[0] & 0x1F) != CAN_DATA) {   // not data

    ret = INVALID_TYPE;
  }

  // data
  *data =  (uint32_t) msg->data[1];
  *data |= (uint32_t)(msg->data[2] << 8);
  *data |= (uint32_t)(msg->data[3] << 16);
  *data |= (uint32_t)(msg->data[4] << 24);

  return ret;
}

CanState CanNode::getData_float(const CanMessage *msg, float *data)
{
  if (msg == nullptr) {
    return DATA_ERROR;
  }

  CanState ret = DATA_OK;
  // check configuration byte
  if ( msg->len != 5 ||                     // not right length
      (msg->data[0] >> 5)   != CAN_FLOAT || // not right type
      (msg->data[0] & 0x1F) != CAN_DATA) {  // not data

    ret = INVALID_TYPE;
  }

  // recast the float into a uint8 to do pull the data
  // out of the structure
  uint8_t* flt_ptr = (uint8_t*)data;

  // data
  *flt_ptr      =  msg->data[1];
  *(flt_ptr+1) |= (msg->data[2] << 8);
  *(flt_ptr+2) |= (msg->data[3] << 16);
  *(flt_ptr+3) |= (msg->data[4] << 24);

  return ret;
}

/**
 * Interpert a CanMessage as a signed 8 bit array (will return error if
 * incorrect)
 *
 * This function is intended to be used in the handler functions
 * (see CanNode_addFilter()) for extracting useful data from recieved messages.
 *
 * Example code
 *
 * ~~~~~~~~~~~~ {.c}
 * void nodeHandler(CanMessage* msg) {
 *  uint8_t data[7];
 *  if(CanNode_getDataArr_uint8(msg, data)==DATA_OK){
 *      //do something cool with the data like flash some lights
 *  }
 *  //the data is stored in data
 * ~~~~~~~~~~~~
 *
 * \param msg[in] Message recieved from someone else, should contain a int8
 * \param data[out] Place for the data extracted from the msg will be stored.
 * \param len[out] length of the recieved data
 *
 * \returns The function returns \ref DATA_ERROR if the message is null,
 * \ref INVALID_TYPE if the message doesn't contain the same type as the
 * function,
 * or \ref DATA_OK if the function succeeded.
 *
 * \see CanNode_getDataArr_uint8()
 * \see CanNode_getDataArr_int16()
 * \see CanNode_getDataArr_uint16()
 *
 * \see CanNode_getData_int8()
 * \see CanNode_getData_uint8()
 * \see CanNode_getData_int16()
 * \see CanNode_getData_uint16()
 * \see CanNode_getData_int32()
 * \see CanNode_getData_uint32()
 */
CanState CanNode::getDataArr_int8(const CanMessage *msg, int8_t data[7], uint8_t *len) {
  if (msg == nullptr) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ( (msg->data[0] & 0x1F) != CAN_DATA) { // not data

    return INVALID_TYPE;
  }

  *len = msg->len - 1;
  // data
  for (int i = 0; i < *len; i++) {
    data[i] = (int8_t)msg->data[i + 1];
  }

  return DATA_OK;
}

/**
 * Interpert a CanMessage as a unsigned 8 bit array (will return error if
 * incorrect)
 *
 * This function is intended to be used in the handler functions
 * (see CanNode_addFilter()) for extracting useful data from recieved messages.
 *
 * Example code
 *
 * ~~~~~~~~~~~~ {.c}
 * void nodeHandler(CanMessage* msg) {
 *  uint8_t data[7];
 *  if(CanNode_getDataArr_uint8(msg, data)==DATA_OK){
 *      //do something cool with the data like flash some lights
 *  }
 *  //the data is stored in data
 * ~~~~~~~~~~~~
 *
 * \param msg[in] Message recieved from someone else, should contain a int8
 * \param data[out] Place for the data extracted from the msg will be stored.
 * \param len[out] length of the recieved data
 *
 * \returns The function returns \ref DATA_ERROR if the message is null,
 * \ref INVALID_TYPE if the message doesn't contain the same type as the
 * function,
 * or \ref DATA_OK if the function succeeded.
 *
 * \see CanNode_getDataArr_int8()
 * \see CanNode_getDataArr_int16()
 * \see CanNode_getDataArr_uint16()
 *
 * \see CanNode_getData_int8()
 * \see CanNode_getData_uint8()
 * \see CanNode_getData_int16()
 * \see CanNode_getData_uint16()
 * \see CanNode_getData_int32()
 * \see CanNode_getData_uint32()
 */
CanState CanNode::getDataArr_uint8(const CanMessage *msg, uint8_t data[7],
                          uint8_t *len) {
  if (msg == nullptr) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ((msg->data[0] & 0x1F) != CAN_DATA) { // not data

    return INVALID_TYPE;
  }

  *len = msg->len - 1;
  // data
  for (int i = 0; i < *len; i++) {
    data[i] = (uint8_t)msg->data[i + 1];
  }

  return DATA_OK;
}

/**
 * Interpert a CanMessage as a signed 16 bit array (will return error if
 * incorrect)
 *
 * This function is intended to be used in the handler functions
 * (see CanNode_addFilter()) for extracting useful data from recieved messages.
 *
 * Example code
 *
 * ~~~~~~~~~~~~ {.c}
 * void nodeHandler(CanMessage* msg) {
 *  uint16_t data[2];
 *  if(CanNode_getDataArr_uint16(msg, data)==DATA_OK){
 *      //do something cool with the data like flash some lights
 *  }
 *  //the data is stored in data
 * ~~~~~~~~~~~~
 *
 * \param msg[in] Message recieved from someone else, should contain a int8
 * \param data[out] Place for the data extracted from the msg will be stored.
 * \param len[out] length of the recieved data
 *
 * \returns The function returns \ref DATA_ERROR if the message is null,
 * \ref INVALID_TYPE if the message doesn't contain the same type as the
 * function,
 * or \ref DATA_OK if the function succeeded.
 *
 * \see CanNode_getDataArr_int8()
 * \see CanNode_getDataArr_uint8()
 * \see CanNode_getDataArr_uint16()
 *
 * \see CanNode_getData_int8()
 * \see CanNode_getData_uint8()
 * \see CanNode_getData_int16()
 * \see CanNode_getData_uint16()
 * \see CanNode_getData_int32()
 * \see CanNode_getData_uint32()
 */
CanState CanNode::getDataArr_int16(const CanMessage *msg, int16_t data[3],
                                  uint8_t *len) {
  if (msg == nullptr) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ((msg->data[0] & 0x1F) != CAN_DATA) { // not data

    return INVALID_TYPE;
  }

  *len = msg->len - 1;
  // data
  for (int i = 0; i < *len; i++ ) {
    // grab the lower and upper bytes
    data[i] = (int16_t)msg->data[2*i + 1];
    data[i] |= (int16_t)(msg->data[2*i + 2] << 8);
  }

  // b/c 16 bit ints are twice as long as 8 bit ones
  *len /= 2;
  return DATA_OK;
}

/**
 * Interpert a CanMessage as a unsigned 16 bit array (will return error if
 * incorrect)
 *
 * This function is intended to be used in the handler functions
 * (see CanNode_addFilter()) for extracting useful data from recieved messages.
 *
 * Example code
 *
 * ~~~~~~~~~~~~ {.c}
 * void nodeHandler(CanMessage* msg) {
 *  uint16_t data[2];
 *  if(CanNode_getDataArr_uint16(msg, data)==DATA_OK){
 *      //do something cool with the data like flash some lights
 *  }
 *  //the data is stored in data
 * ~~~~~~~~~~~~
 *
 * \param msg[in] Message recieved from someone else, should contain a int8
 * \param data[out] Place for the data extracted from the msg will be stored.
 * \param len[out] length of the recieved data
 *
 * \returns The function returns \ref DATA_ERROR if the message is null,
 * \ref INVALID_TYPE if the message doesn't contain the same type as the
 * function,
 * or \ref DATA_OK if the function succeeded.
 *
 * \see CanNode_getDataArr_int8()
 * \see CanNode_getDataArr_uint8()
 * \see CanNode_getDataArr_int16()
 *
 * \see CanNode_getData_int8()
 * \see CanNode_getData_uint8()
 * \see CanNode_getData_int16()
 * \see CanNode_getData_uint16()
 * \see CanNode_getData_int32()
 * \see CanNode_getData_uint32()
 */
CanState CanNode::getDataArr_uint16(const CanMessage *msg, uint16_t data[3],
                          uint8_t *len) {
  if (msg == nullptr) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ((msg->data[0] & 0x1F) != CAN_DATA) { // not data

    return INVALID_TYPE;
  }

  *len = msg->len - 1;
  // data
  for (int i = 0; i < *len; i++ ) {
    // grab the lower and upper bytes
    data[i] = (uint16_t)msg->data[2*i + 1];
    data[i] |= (uint16_t)(msg->data[2*i + 2] << 8);
  }

  // b/c 16 bit ints are twice as long as 8 bit ones
  *len /= 2;
  return DATA_OK;
}