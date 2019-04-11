/* can.c -- shamelessly stolen from the CANtact development pageL_CAN_ConfigFilter(&hcan, &filter);
 * implementations of can functions defined in can.h
 * Modified extensively by Samuel Ellicott to use hardware registers instead
 * of HAL library.
 */


#include "CanNode.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/can.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>
#include <unistd.h>

static int s;
static CanState bus_state;
static uint8_t num_msg;
static struct can_frame *tmp_msg;

static void frame_to_message(CanMessage *out, struct can_frame *in);
static void message_to_frame(struct can_frame *out, CanMessage *in);


void can_init(void) {
  // default to kbit/s
  struct sockaddr_can addr;
  struct ifreq ifr;
  tmp_msg = NULL;

  s = socket(PF_CAN, SOCK_RAW, CAN_RAW);

  strcpy(ifr.ifr_name, "can0");
  ioctl(s, SIOCGIFINDEX, &ifr);

  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;

  bind(s, (struct sockaddr *)&addr, sizeof(addr));

  int flags = fcntl(s, F_GETFL, 0);
  fcntl(s, F_SETFL, flags | O_NONBLOCK);
}

void can_enable(void) {
}

void can_set_bitrate(canBitrate bitrate) {
}

/**
 * \param id id to filter on 
 *
 * \returns the filter number of the added filter returns \ref CAN_FILTER_ERROR
 * if the function was unable to add a filter.
 */
uint16_t can_add_filter_id(uint16_t id) {
    return 0;
}

/**
 * *NOTE:
 * This function takes some finagleing in order for it to work correctly with
 * the %CanNode library.
 * For it to work correctly the returned value from this function should be 
 * passed to CanNode_addFilter() as the id. This lets CanNode_checkForMessages() 
 * know what handler to call if a message using this filter is recieved.*
 * 
 * Example code
 *
 * ~~~~~~~~~~~~ {.c}
 * uint16_t id = can_add_filter_mask(id_to_filter, id_mask);
 * CanNode_addFilter(id, handler);
 * ~~~~~~~~~~~~
 * 
 * \param id base id of the filter mask
 * \param mask mask on top of the base id, 0's are don't cares
 *
 * \returns the filter number of the added filter returns \ref CAN_FILTER_ERROR
 * if the function was unable to add a filter.
 */
uint16_t can_add_filter_mask(uint16_t id, uint16_t mask) {
    return 0;
}

CanState can_tx(CanMessage *tx_msg, uint32_t timeout) { 
    struct can_frame msg;
    message_to_frame(&msg, tx_msg);
    write(s, &msg, sizeof(struct can_frame));
    HAL_Delay(10);

    return DATA_OK;
}

CanState can_rx(CanMessage *rx_msg, uint32_t timeout) {

  if (is_can_msg_pending()) {
    // convert a can_frame into a CanMessage
    frame_to_message(rx_msg, tmp_msg);
    free(tmp_msg);
    tmp_msg = NULL;
    HAL_Delay(10);
    return DATA_OK;
  }

  return NO_DATA;
}

bool is_can_msg_pending() {
  if (tmp_msg == NULL) {
    tmp_msg = (can_frame*) malloc(sizeof(struct can_frame));
  } 
  else {
    return true;
  }

  int nbytes = read(s, tmp_msg, sizeof(struct can_frame));
  if (nbytes > 0) {
    return true;
  }

  free(tmp_msg);
  tmp_msg = NULL;

  if (errno = EAGAIN) {
    return false;
  }
  if (nbytes < 0) {
    perror("can raw socket read");
  } 

  return false;
}

void frame_to_message(CanMessage *out, struct can_frame *in){
    out->id = (uint16_t) in->can_id & 0x7FF;
    out->len = in->can_dlc;
    out->rtr = (in->can_id & CAN_RTR_FLAG) ? true : false;
    memcpy(out->data, in->data, 8);
}

void message_to_frame(struct can_frame *out, CanMessage *in){
    out->can_id = in->id;
    out->can_id |= in->rtr ? CAN_RTR_FLAG: 0;
    out->can_dlc = in->len;
    memcpy(out->data, in->data, 8);
}
