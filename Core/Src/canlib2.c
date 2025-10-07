/**
 * @file    canlib2.c
 * @author  lutet88 / jyz4 / Jeff
 * @brief   canlib2 main source file.
 * @date    2023-10-05
*/

#include "canlib2.h"
#include "main.h"

// safe empty data pointer, allocated for safety on bad returns
uint8_t empty_data_ptr[8];

// allocated lots for fdcan peripherals
canlib2_fdcan_t canlib2_fdcan_lots[CANLIB2_MAX_FDCAN_DEVICES];

// safe empty rx_return_t, allocated for safety on bad returns
canlib2_rx_return_t empty_rx_return = {NULL,};

#ifdef CANLIB2_USE_PRECISION_TIMER
// timer descriptor for us timer, if used
TIM_HandleTypeDef* ustim;
#endif

// canlib2 default RX callback definition
void canlib2_default_rx_callback(FDCAN_HandleTypeDef* fdcan, canlib2_rx_return_t ret) {
    UNUSED(fdcan);
    UNUSED(ret);
    return;
}

// canlib2 default TX callback definition
void canlib2_default_tx_callback(FDCAN_HandleTypeDef* fdcan, canlib2_tx_return_t ret) {
    UNUSED(fdcan);
    UNUSED(ret);
    return;
}

canlib2_fdcan_t* __canlib2_allocate() {
    // iterate through allocated memory to find an unallocated peripheral
    for (int i = 0; i < CANLIB2_MAX_FDCAN_DEVICES; ++i) {
        if (canlib2_fdcan_lots[i].status == CANLIB2_STATUS_UNALLOCATED) {
            return &(canlib2_fdcan_lots[i]);
        }
    }

    // no available locations
    return NULL;
}

canlib2_fdcan_t* canlib2_configure(FDCAN_HandleTypeDef* fdcan) {
    // allocate a CAN peripheral
    canlib2_fdcan_t* can = __canlib2_allocate();
    if (can == NULL) return NULL;

    // configure the CAN instance
    can->fdcan = fdcan;
    can->status = CANLIB2_STATUS_DISABLED;
    //can->fdcan->Init.StdFiltersNbr = 0;
    can->rx_fifo0_callback = canlib2_default_rx_callback;
    can->rx_fifo1_callback = canlib2_default_rx_callback;
    can->tx_callback = canlib2_default_tx_callback;
    can->__filter_index = 0;

    // configure default global filter config
    if (canlib2_change_global_filter_config(can, CANLIB2_NM_REJECT, CANLIB2_ACCEPT_REMOTE) != CANLIB2_OK) Error_Handler();
    return can;
}

int canlib2_deconfigure(canlib2_fdcan_t* can) {
    // do nothing if it's not allocated
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_OK;

    // ensure CAN bus is stopped
    if (can->status == CANLIB2_STATUS_ENABLED || can->status == CANLIB2_STATUS_ERROR) canlib2_stop(can);

    // reset init config (disable filters & notifications)
    //can->fdcan->Init.StdFiltersNbr = 0;
    canlib2_disable_interrupts(can);

    // free the memory used to create CAN instance
    can->status = CANLIB2_STATUS_UNALLOCATED;

    return CANLIB2_OK;
}

int canlib2_start(canlib2_fdcan_t* can) {
    // do nothing if the instance is not allocated
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;

    // initialize CAN with any new Init parameters
    //if (can->status != CANLIB2_STATUS_ENABLED && HAL_FDCAN_Init(can->fdcan) != HAL_OK)
    //    return CANLIB2_ERROR;
    
    // start CAN if it's not started already
    if (can->status != CANLIB2_STATUS_ENABLED && HAL_FDCAN_Start(can->fdcan) != HAL_OK)
        return CANLIB2_ERROR;
    
    // mark the status as enabled
    can->status = CANLIB2_STATUS_ENABLED;
    return CANLIB2_OK;
}

int canlib2_stop(canlib2_fdcan_t* can) {
    // do nothing if the instance is not allocated
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;

    // stop CAN if it's not stopped already
    if (can->status != CANLIB2_STATUS_DISABLED && HAL_FDCAN_Stop(can->fdcan) != HAL_OK)
        return CANLIB2_ERROR;
    
    // mark the status as disabled
    can->status = CANLIB2_STATUS_DISABLED;
    return CANLIB2_OK;
}

int canlib2_change_mode(canlib2_fdcan_t* can, canlib2_hw_mode mode) {
    // do nothing if the instance is not allocated
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;

    // store target status: if the instance was running when we change mode, it should still be running after
    canlib2_status target_status = can->status;

    // stop the bus if it was running
    if (target_status != CANLIB2_STATUS_DISABLED && canlib2_stop(can) != CANLIB2_OK) return CANLIB2_ERROR;

    // set the new mode
    can->fdcan->Init.Mode = mode;

    // if it was supposed to be running, restart it
    if (target_status == CANLIB2_STATUS_ENABLED) return canlib2_start(can);
    return CANLIB2_OK;
}

int canlib2_update_init(canlib2_fdcan_t* can) {
    // do nothing if the instance is not allocated
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;

    // store target status: if the instance was enabled, start it again
    canlib2_status target_status = can->status;

    // if the instance is enabled, stop it
    if (target_status != CANLIB2_STATUS_DISABLED && canlib2_stop(can) != CANLIB2_OK) return CANLIB2_ERROR;

    // deinitialize fdcan
    if (HAL_FDCAN_DeInit(can->fdcan) != HAL_OK) return CANLIB2_ERROR;

    // delay for 4ms (see datasheet)
    HAL_Delay(4);

    // reinitialize fdcan
    if (HAL_FDCAN_Init(can->fdcan) != HAL_OK) return CANLIB2_ERROR;

    // if it was supposed to be running, restart it
    if (target_status == CANLIB2_STATUS_ENABLED) return canlib2_start(can);
    return CANLIB2_OK;
}

int canlib2_change_global_filter_config(canlib2_fdcan_t* can, canlib2_non_matching_action accept_non_matching, canlib2_remote_action accept_remote) {
    // do nothing if the instance is not allocated
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;

    // store target status: if the instance was running when we change mode, it should still be running after
    canlib2_status target_status = can->status;

    // stop the bus if it was running
    if ((target_status == CANLIB2_STATUS_ENABLED || target_status == CANLIB2_STATUS_ERROR) && canlib2_stop(can) != CANLIB2_OK) return CANLIB2_ERROR;
    
    // modify the global filter configuration
    if (
        HAL_FDCAN_ConfigGlobalFilter(   can->fdcan,
                                        (uint32_t) accept_non_matching, // non-matching standard ID
                                        FDCAN_REJECT,        // non-matching extended ID
                                        (uint32_t) accept_remote,       // remote standard ID
                                        FDCAN_REJECT_REMOTE  // remote extended ID
        ) != HAL_OK ) return CANLIB2_ERROR;

    // if it was supposed to be running, restart it
    if (target_status == CANLIB2_STATUS_ENABLED) return canlib2_start(can);
    return CANLIB2_OK;
}

#ifdef CANLIB2_USE_PRECISION_TIMER
void canlib2_configure_us_timer(TIM_HandleTypeDef* tim) {
    ustim = tim;
    if (HAL_TIM_Base_Start(ustim) != HAL_OK) Error_Handler();
}
#endif

void __canlib2_delay_packet() {
    #ifdef CANLIB2_USE_PRECISION_TIMER
        __HAL_TIM_SET_COUNTER(ustim, 0);  
	    while (__HAL_TIM_GET_COUNTER(ustim) < 232);  // wait for 112+4 bits to send
    #else
        HAL_Delay(1);
    #endif
}

// generic function for tx headers (data & remote frame)
FDCAN_TxHeaderTypeDef __canlib2_get_tx_header(canlib2_fdcan_t* can, uint16_t identifier, uint32_t frame_type) {
    // configure standard tx header settings
    UNUSED(can);
    FDCAN_TxHeaderTypeDef txh;
    txh.Identifier = identifier;
    txh.IdType = FDCAN_STANDARD_ID;
    txh.TxFrameType = frame_type;
    txh.DataLength = 0;
    txh.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txh.BitRateSwitch = FDCAN_BRS_OFF;
    txh.FDFormat = FDCAN_CLASSIC_CAN;
    txh.TxEventFifoControl = FDCAN_NO_TX_EVENTS; 
    txh.MessageMarker = 0;

    return txh;
}

FDCAN_TxHeaderTypeDef canlib2_get_tx_header(canlib2_fdcan_t* can, uint8_t addr) {
    return canlib2_get_tx_header_id(can, addr);
}

FDCAN_TxHeaderTypeDef canlib2_get_tx_header_p(canlib2_fdcan_t* can, uint8_t priority, uint8_t addr) {
    return canlib2_get_tx_header_id(can, (priority << 8) | addr);
}

FDCAN_TxHeaderTypeDef canlib2_get_tx_header_id(canlib2_fdcan_t* can, uint16_t identifier) {
    return __canlib2_get_tx_header(can, identifier, FDCAN_DATA_FRAME);
}

FDCAN_TxHeaderTypeDef canlib2_get_tx_remote_header(canlib2_fdcan_t* can, uint8_t addr) {
    return canlib2_get_tx_remote_header_id(can, addr);
}

FDCAN_TxHeaderTypeDef canlib2_get_tx_remote_header_p(canlib2_fdcan_t* can, uint8_t priority, uint8_t addr) {
    return canlib2_get_tx_remote_header_id(can, (priority << 8) | addr);
}

FDCAN_TxHeaderTypeDef canlib2_get_tx_remote_header_id(canlib2_fdcan_t* can, uint16_t identifier) {
    return __canlib2_get_tx_header(can, identifier, FDCAN_REMOTE_FRAME);
}

int canlib2_send_data(canlib2_fdcan_t* can, uint8_t addr, uint8_t length, uint8_t* data) {
    return canlib2_send_data_id(can, addr, length, data);
}

int canlib2_send_data_p(canlib2_fdcan_t* can, uint8_t priority, uint8_t addr, uint8_t length, uint8_t* data) {
    return canlib2_send_data_id(can, (priority << 8) | addr, length, data);
}

// universal function for sending a data frame
int canlib2_send_data_id(canlib2_fdcan_t* can, uint16_t identifier, uint8_t length, uint8_t* data) {
    // do nothing if the instance is not allocated
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;

    // check if data length is valid
    if (length > 8) return CANLIB2_ERROR;
    
    // generate header
    FDCAN_TxHeaderTypeDef txh = canlib2_get_tx_header_id(can, identifier);
    
    // edit header to have correct data length code (see @group FDCAN_data_length_code)
    txh.DataLength = ((int) length); //<< 16;

    // if the fifo is too full, delay until we can add the message
    while (canlib2_tx_fifo_free(can) == 0) {
        __canlib2_delay_packet();
    }
    
    // add to queue
    return HAL_FDCAN_AddMessageToTxFifoQ(can->fdcan, &txh, data);
}

int canlib2_send_remote(canlib2_fdcan_t* can, uint8_t addr) {
    return canlib2_send_remote_id(can, addr);
}

int canlib2_send_remote_p(canlib2_fdcan_t* can, uint8_t priority, uint8_t addr) {
    return canlib2_send_remote_id(can, (priority << 8) | addr);
}

// universal function for sending a remote frame
int canlib2_send_remote_id(canlib2_fdcan_t* can, uint16_t identifier) {
    // do nothing if the instance is not allocated
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;    

    // generate header
    FDCAN_TxHeaderTypeDef txh = canlib2_get_tx_remote_header_id(can, identifier);

    // add to queue
    return HAL_FDCAN_AddMessageToTxFifoQ(can->fdcan, &txh, empty_data_ptr);
}

int canlib2_add_rx_filter_by_address(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t addr) {
    return canlib2_add_rx_filter(can, action, 0x0, 0x0, 0xFF, addr);
}

int canlib2_add_rx_filter_by_address_p(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority, uint8_t addr) {
    return canlib2_add_rx_filter(can, action, 0x7, priority, 0xFF, addr);
}

int canlib2_add_rx_filter_by_address_pm(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority_mask, uint8_t priority_match, uint8_t addr) {
    return canlib2_add_rx_filter(can, action, priority_mask, priority_match, 0xFF, addr);
}

int canlib2_add_rx_filter_by_partial_address(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t match_bit_count, uint8_t match) {
    return canlib2_add_rx_filter(can, action, 0x0, 0x0, (0xFF >> match_bit_count) << match_bit_count, match);
}

int canlib2_add_rx_filter_by_partial_address_p(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority, uint8_t match_bit_count, uint8_t match) {
    return canlib2_add_rx_filter(can, action, 0x7, priority, (0xFF >> match_bit_count) << match_bit_count, match);
}

int canlib2_add_rx_filter_by_partial_address_pm(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority_mask, uint8_t priority_match, uint8_t match_bit_count, uint8_t match) {
    return canlib2_add_rx_filter(can, action, priority_mask, priority_match, (0xFF >> match_bit_count) << match_bit_count, match);
}

int canlib2_add_rx_filter_by_id(canlib2_fdcan_t* can, canlib2_filter_action action, uint16_t identifier) {
    return canlib2_add_rx_filter(can, action, 0x7, identifier >> 8, 0xFF, identifier && 0xFF);
}

int canlib2_add_rx_filter_by_priority(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority) {
    return canlib2_add_rx_filter(can, action, 0x7, priority, 0x00, 0x00);
}

int canlib2_add_rx_filter_by_priority_mask(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority_mask, uint8_t priority_match) {
    return canlib2_add_rx_filter(can, action, priority_mask, priority_match, 0x00, 0x00);
}

int canlib2_add_rx_filter(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority_mask, uint8_t priority_match, uint8_t addr_mask, uint8_t addr_match) {
    // do nothing if the instance is not allocated
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;
    if (can->__filter_index >= can->fdcan->Init.StdFiltersNbr) return CANLIB2_ERROR;

    // store target status: if the instance was running when we change mode, it should still be running after
    canlib2_status target_status = can->status;

    // stop the bus if it was running
    if ((target_status == CANLIB2_STATUS_ENABLED || target_status == CANLIB2_STATUS_ERROR) && canlib2_stop(can) != CANLIB2_OK) return CANLIB2_ERROR;
    
    FDCAN_FilterTypeDef filter;
    filter.IdType = FDCAN_STANDARD_ID;
    filter.FilterIndex = can->__filter_index;
    filter.FilterConfig = action;
    filter.FilterType = FDCAN_FILTER_MASK;
    filter.FilterID2 = (priority_mask << 8) | addr_mask;
    filter.FilterID1 = (priority_match << 8) | addr_match;
    
    ++can->__filter_index;
    
    //++can->fdcan->Init.StdFiltersNbr;
    // configure the filter
    if (HAL_FDCAN_ConfigFilter(can->fdcan, &filter) != HAL_OK) return CANLIB2_ERROR;

    // if it was supposed to be running, restart it
    if (target_status == CANLIB2_STATUS_ENABLED) return canlib2_start(can);

    return CANLIB2_OK;
}

int canlib2_tx_fifo_free(canlib2_fdcan_t* can) {
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return 0;

    return HAL_FDCAN_GetTxFifoFreeLevel(can->fdcan);
}

int canlib2_tx_fifo_full(canlib2_fdcan_t* can) {
    // tx fifo is full if it is not free
    return canlib2_tx_fifo_free(can) <= 0;
}

int canlib2_rx_fifo_fill(canlib2_fdcan_t* can, canlib2_fifo fifo) {
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return -1;

    // switch for fifo, or return -1 if fifo is invalid
    switch (fifo) {
        case CANLIB2_FIFO0:
            return HAL_FDCAN_GetRxFifoFillLevel(can->fdcan, FDCAN_RX_FIFO0);
        case CANLIB2_FIFO1:
            return HAL_FDCAN_GetRxFifoFillLevel(can->fdcan, FDCAN_RX_FIFO1);
        default:
            return -1;
    }
    return -1;
}

int canlib2_rx_fifo_full(canlib2_fdcan_t* can, canlib2_fifo fifo) {
    // AN5348 / Memory Management
    // Each RX FIFO can carry 64 FIFO elements
    return canlib2_rx_fifo_fill(can, fifo) >= 64;
}

canlib2_rx_return_t canlib2_receive_data(canlib2_fdcan_t* can, canlib2_fifo fifo) {    
    canlib2_rx_return_t ret;
    canlib2_identifier_t id;
    FDCAN_RxHeaderTypeDef rxh;

    if (can->status != CANLIB2_STATUS_ENABLED) return empty_rx_return; // nothing to read
    
    // switch fifo for location and event
    uint32_t rx_loc = 0;
    switch (fifo) {
        case CANLIB2_FIFO0:
            rx_loc = FDCAN_RX_FIFO0;
            ret.event = CANLIB2_RX_FIFO0_NEW_MESSAGE;
            break;
        case CANLIB2_FIFO1:
            rx_loc = FDCAN_RX_FIFO1;
            ret.event = CANLIB2_RX_FIFO1_NEW_MESSAGE;
            break;
        default:
            return empty_rx_return;
    }
    
    // get the message
    int status = HAL_FDCAN_GetRxMessage(can->fdcan, rx_loc, &rxh, (uint8_t*) (&ret.data));
    if (status != HAL_OK) return empty_rx_return;

    // complete unfilled parts of rx_return_t
    ret.fdcan = can->fdcan;
    ret.frame_type = rxh.RxFrameType;
    ret.length = rxh.DataLength >> 16;
    id.address = rxh.Identifier & 0xFF;
    id.priority = rxh.Identifier >> 8;
    ret.identifier = id;

    return ret;    
}

int canlib2_enable_rx_interrupt(canlib2_fdcan_t* can, canlib2_rx_event event) {
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;
    return HAL_FDCAN_ActivateNotification(can->fdcan, (uint32_t) event, 0);
}

int canlib2_disable_rx_interrupt(canlib2_fdcan_t* can, canlib2_rx_event event) {
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;
    return HAL_FDCAN_DeactivateNotification(can->fdcan, (uint32_t) event);
}

int canlib2_enable_tx_interrupt(canlib2_fdcan_t* can, canlib2_tx_event event) {
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;
    return HAL_FDCAN_ActivateNotification(can->fdcan, (uint32_t) event, 0);
}

int canlib2_disable_tx_interrupt(canlib2_fdcan_t* can, canlib2_tx_event event) {
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;
    return HAL_FDCAN_DeactivateNotification(can->fdcan, (uint32_t) event);
}

int canlib2_disable_interrupts(canlib2_fdcan_t* can) {
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;
    return HAL_FDCAN_DeactivateNotification(can->fdcan, CANLIB2_RX_ALL_EVENTS | CANLIB2_TX_ALL_EVENTS);
}

int canlib2_set_rx_callback(canlib2_fdcan_t* can, canlib2_fifo fifo, canlib2_rx_callback callback) {
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;
    
    // switch for fifo
    switch (fifo) {
        case CANLIB2_FIFO0:
            can->rx_fifo0_callback = callback;
            break;
        case CANLIB2_FIFO1:
            can->rx_fifo1_callback = callback;
            break;
        default:
            break;
    }
    return CANLIB2_OK;
}

int canlib2_set_tx_callback(canlib2_fdcan_t* can, canlib2_tx_callback callback) {
    if (can->status == CANLIB2_STATUS_UNALLOCATED) return CANLIB2_ERROR;

    can->tx_callback = callback;
    return CANLIB2_OK;
}

void canlib2_generic_tx_event(FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs) {
    canlib2_fdcan_t can;
    uint8_t i;
    // find the right canlib2 can object
    for (i = 0; i < CANLIB2_MAX_FDCAN_DEVICES; ++i) {
        if (canlib2_fdcan_lots[i].fdcan == hfdcan) can = canlib2_fdcan_lots[i];
    }
    // if not found, do nothing
    if (i == CANLIB2_MAX_FDCAN_DEVICES) return;

    // universal returns
    canlib2_tx_return_t ret;
    ret.event = TxEventFifoITs;
    ret.fdcan = hfdcan;
    can.tx_callback(hfdcan, ret);
}

void HAL_FDCAN_TxEventFifoCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs) {
    canlib2_generic_tx_event(hfdcan, TxEventFifoITs);
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    canlib2_fdcan_t can;
    uint8_t i;
    // find the right canlib2 can object
    for (i = 0; i < CANLIB2_MAX_FDCAN_DEVICES; ++i) {
        if (canlib2_fdcan_lots[i].fdcan == hfdcan) {
            can = canlib2_fdcan_lots[i];
            break;
        }
    }
    // if not found, do nothing
    if (i == CANLIB2_MAX_FDCAN_DEVICES) return;

    // universal returns
    canlib2_rx_return_t ret;
    canlib2_identifier_t id;
    ret.fdcan = hfdcan;

    // depending on the event type, fire the callback differently
    if (RxFifo0ITs & CANLIB2_RX_FIFO0_FULL) {
        ret.event = CANLIB2_RX_FIFO0_FULL;
        ret.length = 0;
        ret.data = empty_data_ptr;
        ret.frame_type = CANLIB2_NO_FRAME;
        id.address = 0x00;
        id.priority = 0x0;
        ret.identifier = id;
        can.rx_fifo0_callback(hfdcan, ret);
    }

    if (RxFifo0ITs & CANLIB2_RX_FIFO0_MESSAGE_LOST) {
        ret.event = CANLIB2_RX_FIFO0_MESSAGE_LOST;
        ret.length = 0;
        ret.data = empty_data_ptr;
        ret.frame_type = CANLIB2_NO_FRAME;
        id.address = 0x00;
        id.priority = 0x0;
        ret.identifier = id;
        can.rx_fifo0_callback(hfdcan, ret);
    }

    if (RxFifo0ITs & CANLIB2_RX_FIFO0_NEW_MESSAGE) {
        FDCAN_RxHeaderTypeDef rxh;
        uint8_t data[8];
        HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxh, data);
        ret.event = CANLIB2_RX_FIFO0_NEW_MESSAGE;
        ret.length = rxh.DataLength;
        ret.data = data;
        ret.frame_type = rxh.RxFrameType;
        id.address = rxh.Identifier & 0xFF;
        id.priority = rxh.Identifier >> 8;
        ret.identifier = id;
        can.rx_fifo0_callback(hfdcan, ret);
    }
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
    canlib2_fdcan_t can;
    uint8_t i;
    // find the right canlib2 can object
    for (i = 0; i < CANLIB2_MAX_FDCAN_DEVICES; ++i) {
        if (canlib2_fdcan_lots[i].fdcan == hfdcan) can = canlib2_fdcan_lots[i];
    }
    // if not found, do nothing
    if (i == CANLIB2_MAX_FDCAN_DEVICES) return;

    // universal returns
    canlib2_rx_return_t ret;
    canlib2_identifier_t id;
    ret.fdcan = hfdcan;

    // depending on the event type, fire the callback differently
    if (RxFifo1ITs & CANLIB2_RX_FIFO1_FULL) {
        ret.event = CANLIB2_RX_FIFO1_FULL;
        ret.length = 0;
        ret.data = empty_data_ptr;
        ret.frame_type = CANLIB2_NO_FRAME;
        id.address = 0x00;
        id.priority = 0x0;
        ret.identifier = id;
        can.rx_fifo0_callback(hfdcan, ret);
    }

    if (RxFifo1ITs & CANLIB2_RX_FIFO1_MESSAGE_LOST) {
        ret.event = CANLIB2_RX_FIFO1_MESSAGE_LOST;
        ret.length = 0;
        ret.data = empty_data_ptr;
        ret.frame_type = CANLIB2_NO_FRAME;
        id.address = 0x00;
        id.priority = 0x0;
        ret.identifier = id;
        can.rx_fifo1_callback(hfdcan, ret);
    }

    if (RxFifo1ITs & CANLIB2_RX_FIFO1_NEW_MESSAGE) {
        FDCAN_RxHeaderTypeDef rxh;
        uint8_t data[8];
        HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &rxh, data);
        ret.event = CANLIB2_RX_FIFO1_NEW_MESSAGE;
        ret.length = rxh.DataLength;
        ret.data = data;
        ret.frame_type = rxh.RxFrameType;
        id.address = rxh.Identifier & 0xFF;
        id.priority = rxh.Identifier >> 8;
        ret.identifier = id;
        can.rx_fifo1_callback(hfdcan, ret);
    }
}

void HAL_FDCAN_TxFifoEmptyCallback(FDCAN_HandleTypeDef *hfdcan) {
    canlib2_generic_tx_event(hfdcan, CANLIB2_TX_FIFO_EMPTY);
}
void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes) {
    UNUSED(BufferIndexes);
    canlib2_generic_tx_event(hfdcan, CANLIB2_TX_COMPLETE);
}
void HAL_FDCAN_TxBufferAbortCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes) {
    UNUSED(BufferIndexes);
    canlib2_generic_tx_event(hfdcan, CANLIB2_TX_ABORT_COMPLETE);
}

// not implemented
void HAL_FDCAN_HighPriorityMessageCallback(FDCAN_HandleTypeDef *hfdcan);
void HAL_FDCAN_TimestampWraparoundCallback(FDCAN_HandleTypeDef *hfdcan);
void HAL_FDCAN_TimeoutOccurredCallback(FDCAN_HandleTypeDef *hfdcan);
void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan);
void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs);
