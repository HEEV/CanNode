/**
 * CanNode.c
 * \brief implements functions for CanNode devices
 *
 * \author Samuel Ellicott
 * \date 6-20-16
 */
#include "CanNode.h"

CanNode *CanNode::nodes[MAX_NODES] = {nullptr};
uint8_t CanNode::filter_bank = 0;
bool CanNode::newMessage = false;
CanMessage CanNode::tmpMsg;

/// \brief Initilize a CanNode from given parameters.
CanNode::CanNode(uint16_t id, filterHandler rtrHandle) {
  static bool has_run = false;
  static uint64_t usedNodes = 0;

  // if this is the first run clear list of nodes
  if (!has_run) {
    can_init();
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
        this->filters[j] = UNUSED_FILTER;
        this->handle[j] = nullptr;
    }

    // add id etc
    nodes[i] = this;

    //clear the name and info pointers
    this->nameStr=nullptr;
    this->infoStr=nullptr;
    this->rtrHandle = rtrHandle;

    this->id = id;
    // add filters to hardware

    // default filters
    auto fmi = can_add_filter_id(
      {static_cast<uint16_t>(id),   true}, 
      {static_cast<uint16_t>(id+1), true},
      {static_cast<uint16_t>(id+2), true},
      {static_cast<uint16_t>(id+3), false},
      filter_bank
    );
    filter_bank++;

    // setup the filters and handlers
    this->filters[0] = fmi.id1_fmi;
    this->filters[1] = fmi.id2_fmi;
    this->filters[2] = fmi.id3_fmi;
    this->filters[3] = fmi.id4_fmi;

    this->handle[0] = rtrHandle;

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
bool CanNode::addFilter_id(
                    filter_id_t id1,
                    filter_id_t id2,
                    filter_id_t id3,
                    filter_id_t id4,
                    filterHandler handle1,
                    filterHandler handle2,
                    filterHandler handle3,
                    filterHandler handle4
                  )
{
  const uint16_t MAX_ID = 0x7FF;
  if (id1.id > MAX_ID || handle1 == nullptr) {
    return false;
  }
  // check the ids of the other filters
  id2.id = (id2.id > MAX_ID) ? 0 : id2.id;
  id3.id = (id2.id > MAX_ID) ? 0 : id3.id;
  id4.id = (id4.id > MAX_ID) ? 0 : id4.id;

  if (handle2 == nullptr || handle3 == nullptr || handle4 == nullptr){
    handle2 = handle3 = handle4 = handle1;
  }

  // add to the end of the list of filters... If there's room.
  for (uint8_t i = 0; i < NUM_FILTERS; ++i) {
    // check that the filter is open and there are enough
    if (this->filters[i] == UNUSED_FILTER && i+4 < NUM_FILTERS) {
      // add the filters
      auto ret = can_add_filter_id(id1, id2, id3, id4, filter_bank);
      filter_bank++;

      // save the filter id
      this->filters[i]   = ret.id1_fmi;
      this->filters[i+1] = ret.id2_fmi;
      this->filters[i+2] = ret.id3_fmi;
      this->filters[i+3] = ret.id4_fmi;
      // save a pointer to the handler function
      this->handle[i]   = handle1;
      this->handle[i+1] = handle2;
      this->handle[i+2] = handle3;
      this->handle[i+3] = handle4;

      return true; // Sucess! Filter has been added
    }
  }

  return false; // no empty slots
}

bool CanNode::addFilter_mask(
                      filter_id_mask_t id1,
                      filter_id_mask_t id2,
                      filterHandler handle1,
                      filterHandler handle2
                    )
{
  const uint16_t MAX_ID = 0x7FF;
  if (id1.filter_id.id > MAX_ID || handle1 == nullptr) {
    return false;
  }
  // check the ids of the other filters
  id2.filter_id.id = (id2.filter_id.id > MAX_ID) ? 0 : id2.filter_id.id;

  if (handle2 == nullptr){
    handle2 = handle1;
  }

  // add to the end of the list of filters... If there's room.
  for (uint8_t i = 0; i < NUM_FILTERS; ++i) {
    // check that the filter is open and there are enough
    if (this->filters[i] == UNUSED_FILTER && i+2 < NUM_FILTERS) {
      // add the filters
      auto ret = can_add_filter_mask(id1, id2, filter_bank);
      filter_bank++;

      // save the filter id
      this->filters[i]   = ret.id1_fmi;
      this->filters[i+1] = ret.id2_fmi;
      // save a pointer to the handler function
      this->handle[i]   = handle1;
      this->handle[i+1] = handle2;

      return true; // Sucess! Filter has been added
    }
  }

  return false; // no empty slots
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
  if (!newMessage) {
    return;
  }

  // loop through nodes
  for (uint8_t i = 0; i < MAX_NODES; ++i) {

    // CanNode takes over if the caller asks for a reserved id
    // rtr request for node data
    if (nodes[i] == nullptr) {
        continue;
    }
    if (tmpMsg.fmi == nodes[i]->filters[0]) {
      nodes[i]->rtrHandle(&tmpMsg);
    }
    // get name id if asked with an rtr
    else if (tmpMsg.fmi == nodes[i]->filters[1]) {
      nodes[i]->sendName();
    }
    // get info id
    else if (tmpMsg.fmi == nodes[i]->filters[2]) {
      nodes[i]->sendInfo();
    }
    else {
      // call callbacks for the user defined filters
      for (uint8_t j = 4; j < NUM_FILTERS; ++j) {
        if (tmpMsg.fmi == nodes[i]->filters[j]) {

          // call handler function
          nodes[i]->handle[j](&tmpMsg);
        }
        // check if the filter match equals a filter id
        else if ( tmpMsg.fmi == nodes[i]->filters[j] ) { // filter matches

          // call handler function
          nodes[i]->handle[j](&tmpMsg);
        }
      }
    }
  }

  // clear new message flag
  newMessage = false;
}

bool CanNode::updateMessage(CanMessage* msg)
{
  if (!newMessage)
  {
    tmpMsg = *msg;
    return true;
  }
  return false;
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
