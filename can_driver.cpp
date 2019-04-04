/* can.c -- shamelessly stolen from the CANtact development pageL_CAN_ConfigFilter(&hcan, &filter);
 * implementations of can functions defined in can.h
 * Modified extensively by Samuel Ellicott to use hardware registers instead
 * of HAL library.
 */


#include "CanNode.h"
#include "can.h"

static uint16_t prescaler;
static uint8_t bs1;
static uint8_t bs2;

void can_init(void) {
  MX_CAN_Init();
  HAL_CAN_ActivateNotification(&hcan, CAN_IER_FOVIE0 | CAN_IER_FOVIE1);
  HAL_CAN_Start(&hcan);
}

void can_set_bitrate(canBitrate bitrate) {
  // all these values were calculated from the equation given in the reference
  // manual for
  // finding the baudrate. They are calculated from an LibreOffice Calc
  // spreadsheet for a 16MHZ clock
#ifndef STM32F3
  switch (bitrate) {
  case CAN_BITRATE_10K:
    prescaler = 159;
    bs1 = 3;
    bs2 = 4;
    break;
  case CAN_BITRATE_20K:
    prescaler = 99;
    bs1 = 2;
    bs2 = 3;
    break;
  case CAN_BITRATE_50K:
    prescaler = 31;
    bs1 = 4;
    bs2 = 3;
    break;
  case CAN_BITRATE_100K:
    prescaler = 19;
    bs1 = 3;
    bs2 = 2;
    break;
  case CAN_BITRATE_125K:
    prescaler = 7;
    bs1 = 7;
    bs2 = 6;
    break;
  case CAN_BITRATE_250K:
    prescaler = 7;
    bs1 = 1;
    bs2 = 4;
    break;

  default:
  case CAN_BITRATE_500K:
    prescaler = 1;
    bs1 = 7;
    bs2 = 6;
    break;
  case CAN_BITRATE_750K:
    prescaler = 2;
    bs1 = 2;
    bs2 = 2;
    break;
  case CAN_BITRATE_1000K:
    prescaler = 1;
    bs1 = 2;
    bs2 = 3;
    break;
  }
#endif
// used the same spreadsheet for a 24MHz clock
#ifdef STM32F3
  switch (bitrate) {
  case CAN_BITRATE_10K:
    prescaler = 479;
    bs1 = 2;
    bs2 = 0;
    break;
  case CAN_BITRATE_20K:
    prescaler = 149;
    bs1 = 4;
    bs2 = 1;
    break;
  case CAN_BITRATE_50K:
    prescaler = 95;
    bs1 = 2;
    bs2 = 0;
    break;
  case CAN_BITRATE_100K:
    prescaler = 23;
    bs1 = 4;
    bs2 = 3;
    break;
  case CAN_BITRATE_125K:
    prescaler = 31;
    bs1 = 3;
    bs2 = 0;
    break;
  case CAN_BITRATE_250K:
    prescaler = 23;
    bs1 = 0;
    bs2 = 1;
    break;

  default:
  case CAN_BITRATE_500K:
    prescaler = 5;
    bs1 = 2;
    bs2 = 3;
    break;
  case CAN_BITRATE_750K:
    prescaler = 3;
    bs1 = 4;
    bs2 = 1;
    break;
  case CAN_BITRATE_1000K:
    prescaler = 2;
    bs1 = 5;
    bs2 = 0;
    break;
  }
#endif
}

// get the starting fmi (filter mask index) of the bank spefied
// warning, direct hardware access ahead
uint8_t get_fmi_cnt(uint8_t filter_bank, uint8_t fifo_num)
{

  uint8_t filter_cnt = 0;
  for (int i = 0; i < filter_bank; i++)
  {
    // check if the filter is enabled and 
    // belongs to the correct fifo
    if( CAN->FA1R & (1 << i) && CAN->FFA1R & (1 << i) )
    {
      filter_cnt += (CAN->FM1R & (1 << i)) ?
        4 : 2;
    }
  }
  
  return filter_cnt;
}

/**
 * \param id id to filter on 
 *
 * \returns the filter number of the added filter returns \ref CAN_FILTER_ERROR
 * if the function was unable to add a filter.
 */
/// \brief Add a filter to the can hardware with an id
fmi_ret_t can_add_filter_id(filter_id_t id1,
                            filter_id_t id2,
                            filter_id_t id3,
                            filter_id_t id4,
                            uint8_t filter_bank,
                            uint8_t fifo_num)
{

  CAN_FilterTypeDef filter;
  const int MAX_FILTER = 13;
  const uint16_t BAD_FMI = 0xF;
  if (filter_bank > MAX_FILTER || fifo_num > 1) {
    return {
      BAD_FMI, 
      BAD_FMI, 
      BAD_FMI, 
      BAD_FMI 
    };
  }

  // setup the filters as specified in the datasheet
  auto id_to_hw = [](filter_id_t &id) -> uint32_t 
  {
    return (id.id << 5) | ( (id.id_rtr ? 1 : 0) << 4 );
  };

  // mostly setup filter
  filter.FilterIdLow      = id_to_hw(id1);
  filter.FilterMaskIdLow  = id_to_hw(id2);
  filter.FilterIdHigh     = id_to_hw(id3);
  filter.FilterMaskIdHigh = id_to_hw(id4);
  filter.FilterMode = CAN_FILTERMODE_IDLIST;
  filter.FilterScale = CAN_FILTERSCALE_16BIT;
  filter.FilterFIFOAssignment = fifo_num;
  filter.FilterBank = filter_bank;
  filter.FilterActivation = ENABLE;

  HAL_CAN_ConfigFilter(&hcan, &filter);

  // TODO use the 
  auto base_ret = get_fmi_cnt(filter_bank, fifo_num);
  return {
    static_cast<uint16_t>(base_ret), 
    static_cast<uint16_t>(base_ret+1), 
    static_cast<uint16_t>(base_ret+2), 
    static_cast<uint16_t>(base_ret+3) 
  };
}

/// \brief Add a filter to the can hardware with a mask
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
fmi_ret_t can_add_filter_mask(filter_id_mask_t id1,
                              filter_id_mask_t id2,
                              uint8_t filter_bank,
                              uint8_t fifo_num)
{

  CAN_FilterTypeDef filter;
  const int MAX_FILTER = 13;
  const uint16_t BAD_FMI = 0xF;

  if (filter_bank > MAX_FILTER || fifo_num > 1) {
    return {
      BAD_FMI, 
      BAD_FMI, 
      BAD_FMI, 
      BAD_FMI 
    };
  }

  // setup the filters as specified in the datasheet
  auto id_to_hw = [](filter_id_t &id) -> uint32_t 
  {
    return (id.id << 5) | ( (id.id_rtr ? 1 : 0) << 4 );
  };

  // mostly setup filter
  filter.FilterIdLow      = id_to_hw(id1.filter_id);
  filter.FilterMaskIdLow  = id_to_hw(id1.mask_id);
  filter.FilterIdHigh     = id_to_hw(id2.filter_id);
  filter.FilterMaskIdHigh = id_to_hw(id2.mask_id);
  filter.FilterMode = CAN_FILTERMODE_IDMASK;
  filter.FilterScale = CAN_FILTERSCALE_16BIT;
  filter.FilterFIFOAssignment = fifo_num;
  filter.FilterBank = filter_bank;
  filter.FilterActivation = ENABLE;

  HAL_CAN_ConfigFilter(&hcan, &filter);
  
  // TODO use the 
  auto base_ret = get_fmi_cnt(filter_bank, fifo_num);

  return {
    static_cast<uint16_t>(base_ret), 
    static_cast<uint16_t>(base_ret+1), 
    static_cast<uint16_t>(BAD_FMI), 
    static_cast<uint16_t>(BAD_FMI) 
  };
}

CanState can_tx(CanMessage *tx_msg, uint32_t timeout) { 
  UNUSED(timeout);

  CAN_TxHeaderTypeDef tx_header = {
    tx_msg->id, 0,
    CAN_ID_STD,
    tx_msg->rtr ? CAN_RTR_REMOTE : CAN_RTR_DATA,
    tx_msg->len, 
    DISABLE
  };
  uint32_t can_mailbox;

  HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_msg->data, &can_mailbox);

  return BUS_OK;
}

can_rx_ret_t can_rx(CanMessage *rx_msg, uint32_t timeout) {
  UNUSED(timeout);

  can_rx_ret_t ret = {0, CAN_NO_DATA};
  // check for data
  ret.fifo = is_can_msg_pending();
  if (ret.fifo == CAN_NO_DATA){
    return ret;
  }

  // get the data
  CAN_RxHeaderTypeDef rx_header;
  uint8_t data[8];
  HAL_CAN_GetRxMessage(&hcan, ret.fifo , &rx_header, data);

  // fill a CanNode message
  rx_msg->id  = static_cast<uint16_t>(rx_header.StdId);
  rx_msg->len = static_cast<uint8_t>(rx_header.DLC);
  rx_msg->rtr = (rx_header.RTR == CAN_RTR_REMOTE);

  for (int i = 0; i < 8; i++) {
    rx_msg->data[i] = data[i];
  }

  ret.fmi = static_cast<uint8_t>(rx_header.FilterMatchIndex);
  return ret; 
}

void enable_notifications(bool en)
{
  if(en){
    HAL_CAN_ActivateNotification(&hcan, CAN_IER_FMPIE0 | CAN_IER_FMPIE1);
  }
  else
  {
    HAL_CAN_DeactivateNotification(&hcan, CAN_IER_FMPIE0 | CAN_IER_FMPIE1);
  }
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
  HAL_CAN_ResetError(hcan);
}

int is_can_msg_pending() {
  if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) > 0) return 0;
  if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO1) > 0) return 1; 
  return CAN_NO_DATA;
}
