/**
 * CanNode.c
 * \brief implements functions for CanNode devices
 *
 * \author Samuel Ellicott
 * \date 6-20-16
 */
#include "CanNode.h"

CanNode *CanNode::nodes[MAX_NODES] = {nullptr};
bool CanNode::newMessage = false;
CanMessage CanNode::tmpMsg;

/**
 * Initilizes an empty CanNode structure to the values provided.
 *
 * This function basicaly uses the integer value of the \ref CanNodeType enum
 * passed to it to populate the id field of an internal CanNode structure. It
 * also populates the RTR callback from the provided function. Additional callbacks
 * are added by using the \ref CanNode_addFilter() function.
 *
 * \param[in] id CAN Address, use the \ref CanNodeType type.
 * \param[in] rtrHandle function pointer to a handler function for rtr requests.
 * \param[in] force (depricated) Force the creation of a new node of the given paramaters
 * if an old one is not found in flash memory.
 *
 * \returns the address of a \ref CanNode struct that stores the can information.
 * This information is necessary for using any of the sendData functions
 */
CanNode::CanNode(CanNodeType id, filterHandler rtrHandle) {
  static bool has_run = false;
  static uint64_t usedNodes = 0;

  // if this is the first run clear list of nodes
  if (!has_run) {
    can_init();
    can_set_bitrate(CAN_BITRATE_500K);
    can_enable();
    has_run = true;
  }

  // check if a node of that type exists
  for (uint8_t i = 0; i < MAX_NODES; ++i) {
    // check if spot is used
    if (nodes[i] != nullptr ) {
      continue;
    }
    // otherwise its open

    for(int j = 0; j < NUM_FILTERS; j++){
        this->filters[j] = 0;
        this->handle[j] = nullptr;
    }

    // add id etc
    nodes[i] = this;

    //clear the name and info pointers
    nameStr=NULL;
    infoStr=NULL;

    this->id = id;
    // add filters to hardware
    // default filters
    can_add_filter_id(id);     // rtr filter
    can_add_filter_id(id + 1); // get name filter
    can_add_filter_id(id + 2); // get info filter
    can_add_filter_id(id + 3); // configuration filter

    // fill a spot in used nodes
    usedNodes |= 1 << i;
    break;
  }
}

/**
 * Saves a filter id and a handler to a node local to the library. The function also
 * accepts a function which gets called if a message from that id is avalible.
 *
 * Ids from 0 - 52 are reserved for the use of passing the return value of
 * can_add_filter_mask() to the function as a id. This allows for the use of
 * id masks instead of identifier lists for filtering.
 *
 * Example code
 *
 * ~~~~~~~~~~~~ {.c}
 * uint16_t id = can_add_filter_mask(id_to_filter, id_mask);
 * CanNode_addFilter(id, handler);
 * ~~~~~~~~~~~~
 *
 * \param node [in,out] pointer to a node that was initilized with CanNode_init()
 * \param filter [in] id of the device that should be handled by handle
 * \param handle [in] function used to handle the filter
 *
 * \returns true if the filter was added, false if otherwise.
 *
 * \see can_add_filter_mask() for using mask filtering
 */
bool CanNode::addFilter(uint16_t filter, filterHandler handle) {
  if (filter > 0x7FF || handle == NULL) {
    return false;
  }

  // add to the end of the list of filters... If there's room.
  for (uint8_t i = 0; i < NUM_FILTERS; ++i) {
    if (this->filters[i] == 0) {
      // save the filter id
      this->filters[i] = filter;
      // save a pointer to the handler function
      this->handle[i] = handle;

      /*
       * If not a reseved address, add to hardware filtering
       * aka. It's assumed that the id was already added to the
       * hardware filtering if the id is below 52.
       */
      if (filter > 52) {
        can_add_filter_id(filter);
      }

      return true; // Sucess! Filter has been added
    }
  }

  return false; // no empty slots
}

//getter and setter functions -------------------------------------------------

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
void CanNode::sendData(int8_t data) const {
  CanMessage msg;
  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_INT8) << 5) | (0x1F & CAN_DATA);
  // data
  msg.data[1] = (uint8_t)data;
  // set other odds and ends
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
void CanNode::sendData(uint8_t data) const {
  CanMessage msg;
  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_UINT8) << 5) | (0x1F & CAN_DATA);
  // data
  msg.data[1] = data;
  // set other odds and ends
  msg.len = 2;
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
void CanNode::sendData(int16_t data) const {
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
void CanNode::sendData(uint16_t data) const {
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
void CanNode::sendData(int32_t data) const {
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
void CanNode::sendData(uint32_t data) const {
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
CanState CanNode::sendData(int8_t *data, uint8_t len) const {
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
CanState CanNode::sendData(uint8_t *data, uint8_t len) const {
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
CanState CanNode::sendData(int16_t *data, uint8_t len) const {
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
CanState CanNode::sendData(uint16_t *data, uint8_t len) const {
  CanMessage msg;
  // check if valid
  if (len > 2) {
    return DATA_OVERFLOW;
  }

  // configuration byte
  msg.data[0] = (uint8_t)((0x7 & CAN_UINT16) << 5) | (0x1F & CAN_DATA);
  // data
  for (uint8_t i = 0; i < len; ++i) {
    msg.data[i * 2 + 1] = (uint8_t)(data[i] & 0x00ff);
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
 *
 * \see CanNode_getData_uint8()
 * \see CanNode_getData_int16()
 * \see CanNode_getData_int16()
 * \see CanNode_getData_uint16()
 * \see CanNode_getData_int32()
 * \see CanNode_getData_uint32()
 */
CanState CanNode::getData(const CanMessage *msg, int8_t *data) {

  if (msg == NULL) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ((msg->data[0] >> 5) != CAN_INT8 ||   // not right type
      msg->len != 2 ||                     // not right length
      (msg->data[0] & 0x1F) != CAN_DATA) { // not data

    return INVALID_TYPE;
  }

  // data
  *data = (int8_t)msg->data[1];

  return DATA_OK;
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
 * \see CanNode_getData_int8()
 * \see CanNode_getData_int16()
 * \see CanNode_getData_int16()
 * \see CanNode_getData_uint16()
 * \see CanNode_getData_int32()
 * \see CanNode_getData_uint32()
 */
CanState CanNode::getData(const CanMessage *msg, uint8_t *data) {

  if (msg == NULL) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ((msg->data[0] >> 5) != CAN_UINT8 ||  // not right type
      msg->len != 2 ||                     // not right length
      (msg->data[0] & 0x1F) != CAN_DATA) { // not data

    return INVALID_TYPE;
  }

  // data
  *data = msg->data[1];

  return DATA_OK;
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
 * \see CanNode_getData_int8()
 * \see CanNode_getData_uint8()
 * \see CanNode_getData_uint16()
 * \see CanNode_getData_int32()
 * \see CanNode_getData_uint32()
 */
CanState CanNode::getData(const CanMessage *msg, int16_t *data) {

  if (msg == NULL) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ((msg->data[0] >> 5) != CAN_INT16 ||  // not right type
      msg->len != 3 ||                     // not right length
      (msg->data[0] & 0x1F) != CAN_DATA) { // not data

    return INVALID_TYPE;
  }

  // data
  *data = (int16_t)msg->data[1];
  *data |= (int16_t)(msg->data[2] << 8);

  return DATA_OK;
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
CanState CanNode::getData(const CanMessage *msg, uint16_t *data) {

  if (msg == NULL) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ((msg->data[0] >> 5) != CAN_UINT16 || // not right type
      msg->len != 3 ||                     // not right length
      (msg->data[0] & 0x1F) != CAN_DATA) { // not data

    return INVALID_TYPE;
  }

  // data
  *data = (uint16_t)msg->data[1];
  *data |= (uint16_t)(msg->data[2] << 8);

  return DATA_OK;
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
CanState CanNode::getData(const CanMessage *msg, int32_t *data) {

  if (msg == NULL) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ((msg->data[0] >> 5) != CAN_INT32 ||  // not right type
      msg->len != 5 ||                     // not right length
      (msg->data[0] & 0x1F) != CAN_DATA) { // not data

    return INVALID_TYPE;
  }

  // data
  *data = (uint32_t)msg->data[1];
  *data |= (uint32_t)(msg->data[2] << 8);
  *data |= (uint32_t)(msg->data[3] << 16);
  *data |= (uint32_t)(msg->data[4] << 24);

  return DATA_OK;
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
CanState CanNode::getData(const CanMessage *msg, uint32_t *data) {

  if (msg == NULL) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ((msg->data[0] >> 5) != CAN_UINT32 || // not right type
      msg->len != 5 ||                     // not right length
      (msg->data[0] & 0x1F) != CAN_DATA) { // not data

    return INVALID_TYPE;
  }

  // data
  *data = (uint32_t)msg->data[1];
  *data |= (uint32_t)(msg->data[2] << 8);
  *data |= (uint32_t)(msg->data[3] << 16);
  *data |= (uint32_t)(msg->data[4] << 24);

  return DATA_OK;
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
CanState CanNode::getData(const CanMessage *msg, int8_t data[7], uint8_t *len) {
  if (msg == NULL) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ((msg->data[0] >> 5) != CAN_INT8 ||   // not right type
      msg->len > 1 ||                      // not right length
      (msg->data[0] & 0x1F) != CAN_DATA) { // not data

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
CanState CanNode::getData(const CanMessage *msg, uint8_t data[7],
                          uint8_t *len) {
  if (msg == NULL) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ((msg->data[0] >> 5) != CAN_UINT8 ||  // not right type
      msg->len > 1 ||                      // not right length
      (msg->data[0] & 0x1F) != CAN_DATA) { // not data

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
CanState CanNode::getData(const CanMessage *msg, int16_t data[2],
                                  uint8_t *len) {
  if (msg == NULL) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ((msg->data[0] >> 5) != CAN_INT16 ||  // not right type
      msg->len > 3 ||                      // not right length
      (msg->data[0] & 0x1F) != CAN_DATA) { // not data

    return INVALID_TYPE;
  }

  *len = msg->len - 1;
  // data
  for (int i = 0; i < *len; i += 2) {
    data[i] = (int16_t)msg->data[i + 1];
    data[i] |= (int16_t)(msg->data[i + 2] << 8);
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
CanState CanNode::getData(const CanMessage *msg, uint16_t data[2],
                          uint8_t *len) {
  if (msg == NULL) {
    return DATA_ERROR;
  }

  // check configuration byte
  if ((msg->data[0] >> 5) != CAN_UINT16 || // not right type
      msg->len > 3 ||                      // not right length
      (msg->data[0] & 0x1F) != CAN_DATA) { // not data

    return INVALID_TYPE;
  }

  *len = msg->len - 1;
  // data
  for (int i = 0; i < *len; i += 2) {
    data[i] = (uint16_t)msg->data[i + 1];
    data[i] |= (uint16_t)(msg->data[i + 2] << 8);
  }

  // b/c 16 bit ints are twice as long as 8 bit ones
  *len /= 2;
  return DATA_OK;
}

/**
 * Function that should be called from within the main loop. It calls handler
 * functions for each stored node.
 *
 * Because of the unknown length of the handler
 * functions this function call could take a very long time. In order to keep
 * this function call to take a reasonable ammount of time, be sure to make
 * handler functions short. If that is impossible it is recommeded to use
 * interrupts for time-sensative components.
 *
 * This function will call an intrinsic handler (CanNode_nodeHandler())
 * if the message has the id of one of the stored nodes and the calling node
 * is not sending a request frame.
 */
void CanNode::checkForMessages() {
  // pc code should check if a new message is avalible
  // TODO stm32 uses an interrupt to put the newest message in a struct

  // if there are no new messages don't do anything
  if (!is_can_msg_pending()) {
    return;
  }

  can_rx(&tmpMsg, 5);
  // loop through nodes
  for (uint8_t i = 0; i < MAX_NODES; ++i) {

    // CanNode takes over if the caller asks for a reserved id
    // rtr request for node data
    if (nodes[i] != nullptr && tmpMsg.id == nodes[i]->id && tmpMsg.rtr) {
      nodes[i]->rtrHandle(&tmpMsg);
    }
    // get name id if asked with an rtr
    else if (nodes[i] != nullptr && tmpMsg.id == nodes[i]->id + 1 &&
             tmpMsg.rtr) {
      nodes[i]->sendName();
    }
    // get info id
    else if (nodes[i] != nullptr && tmpMsg.id == nodes[i]->id + 2 &&
             tmpMsg.rtr) {
      nodes[i]->sendInfo();
    }
    // configuration id
    //else if (nodes[i] != nullptr && tmpMsg.id == nodes[i]->id + 3) {
      // CanNode_nodeHandler(&nodes[i], &tmpMsg);
    else {
      // call callbacks for the user defined filters
      for (uint8_t j = 0; j < NUM_FILTERS; ++j) {
        if (nodes[i] != nullptr && tmpMsg.id == nodes[i]->filters[j] &&
            nodes[i]->handle[j] != nullptr) {

          // call handler function
          nodes[i]->handle[j](&tmpMsg);
        }
        // check if the filter match equals a filter id
        else if (nodes[i] != nullptr &&
                 tmpMsg.fmi == nodes[i]->filters[j] && // filter matches
                 nodes[i]->handle[j] != nullptr) {

          // call handler function
          nodes[i]->handle[j](&tmpMsg);
        }
      }
    }
  }

  // clear new message flag
  newMessage = false;
}

void CanNode::setName(const char *name) {
    this->nameStr = name;
}

void CanNode::setInfo(const char *info) {
    this->infoStr = info;
}

void CanNode::getString(uint16_t id, char *buff, uint8_t len,
                        uint8_t timeout) {
  CanMessage msg;
  uint32_t tickStart;
  // send a request to the specified CanNode and query its get name address
  msg.id = id;
  msg.len = 1;
  msg.rtr = true;
  msg.data[0] = CAN_GET_NAME | (CAN_INT8 << 5);
  can_tx(&msg, 5);

  msg.id = 0;
  // get start time
  tickStart = HAL_GetTick();

  int badMessages = 0;

  char *str = buff;
  // keep collecting data until a null character is reached, buffer is full,
  // or a timeout condition is reached.
  while (str - buff < len && HAL_GetTick() - tickStart < timeout) {
    // wait for message or timeout
    while (!is_can_msg_pending() && HAL_GetTick() - tickStart < timeout)
      ;
    // get the next buffer
    can_rx(&msg, 5);
    // check if it is from our id
    if (msg.id != id || (msg.data[0] & 0x1F) != CAN_NAME_INFO) {
      badMessages++;
      if (badMessages > 10) {
        msg.id = id;
        msg.len = 1;
        msg.rtr = true;
        msg.data[0] = CAN_GET_NAME | (CAN_INT8 << 5);
        can_tx(&msg, 5);
        msg.id = 0;
      }
      HAL_Delay(50);
      continue;
    }
    // get all the data from this buffer
    for (uint8_t i = 1; i < msg.len && str - buff < len; ++str, ++i) {
      *str = msg.data[i];
    }
  }

  // this won't hurt anything and if the function timeouted this will make sure
  // that the string is null terminated
  *(buff + len - 1) = '\0';
}

/**
 * Get the name string from the node of the given id and put it in a character
 * buffer.
 *
 * \param id id of the node that you want the name of
 * \param buff character buffer to put the name into
 * \param len length of the character buffer
 * \param timeout length in mili-seconds before giving up the message
 *
 * \see CanNode_requestInfo()
 */
void CanNode::requestName(CanNodeType id, char *buff, uint8_t len,
                          uint16_t timeout) {
  getString(id + 1, buff, len, timeout);
}

/**
 * Get the info string from the node of the given id and put it in a character
 * buffer. The function will never deliver more than \ref MAX_INFO_LEN.
 *
 * \param id id of the node that you want the information string of
 * \param info character buffer to put the info string into
 * \param buff_len length of the character buffer
 * \param timeout length in mili-seconds before giving up the message
 *
 * \see CanNode_requestName()
 */
void CanNode::requestInfo(CanNodeType id, char *buff, uint8_t len,
                          uint16_t timeout) {
  getString(id + 2, buff, len, timeout);
}

void CanNode::sendString(uint16_t id, const char *str) {
  CanMessage msg;
  msg.id = id;
  msg.rtr = false;
  msg.data[0] = CAN_NAME_INFO | CAN_INT8 << 5;

  bool msgFinished = false;

  //fill buffers and send them 
  const char *namePtr = str;
  //check that there is valid data to transmit
  if(namePtr == nullptr){
      return;
  }
  //loop while the string is valid
  while (!msgFinished) {

    // break if end of name has been reached
    for (msg.len = 1; msg.len < 8; msg.len++, namePtr++) {

      // set data
      msg.data[msg.len] = *namePtr;
      if(*namePtr == '\0'){
          msgFinished = true;
          msg.len++;
          break;
      }

    }
    //transmit data with 5ms timeout
    can_tx(&msg, 5);
    HAL_Delay(50);
  }
}

void CanNode::sendName() {
    sendString(this->id+1, this->nameStr);
}

void CanNode::sendInfo() {
    sendString(this->id+2, this->infoStr);
}
