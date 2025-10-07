/**
 * @file    canlib2.h
 * @author  lutet88 / jyz4 / Jeff
 * @brief   canlib2 header file.
 * @date    2023-10-05
*/

#ifndef __CANLIB2_H
#define __CANLIB2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include "stm32h5xx_hal.h"

#ifndef CANLIB2_MAX_FDCAN_DEVICES
/**
 * @brief Number of allocated canlib2_fdcan peripherals.
 * This is the maximum number of CAN peripherals that canlib2 will allocate memory for.
 * By default, 4 are allocated, but this can be configured depending on microcontroller series.
 * Each CAN peripheral takes up 20 bytes of memory on compile.
 * Can be overridden by a global C define.
*/
#define CANLIB2_MAX_FDCAN_DEVICES   4
#endif

/**
 * @brief Enable and use a precision timer when waiting to add packets to FIFO.
 * canlib2_configure_us_timer() must be called in the program to configure a 16-bit timer.
*/
// #define CANLIB2_USE_PRECISION_TIMER

/**
 * @enum canlib2_return_status
 * @brief Enum for general return status.
*/
typedef enum {
    // Return code for success (OK)
    CANLIB2_OK                      = 0,
    // Return code for an error (something went wrong)
    CANLIB2_ERROR                   = 1
} canlib2_return_status;

/**
 * @enum canlib2_status
 * @brief Enum for the status of a canlib2_fdcan peripheral.
*/
typedef enum {
    // Peripheral is not allocated (default state)
    CANLIB2_STATUS_UNALLOCATED      = 0x00U,
    // Peripheral is allocated and disabled (CAN is not running)
    CANLIB2_STATUS_DISABLED         = 0x01U,
    // Peripheral is allocated and enabled (CAN is likely running)
    CANLIB2_STATUS_ENABLED          = 0x02U,
    // Peripheral has encountered an error
    CANLIB2_STATUS_ERROR            = 0x03U
} canlib2_status;

/**
 * @enum canlib2_hw_mode
 * @brief Enum for hardware operational mode.
 * The STM32's hardware configuration has three modes:
 * 1. Normal Mode (CAN will read and write normally, and acknowledge all filtered packets
 * 2. Restricted Operation (CAN will read only, and acknowledge all filted packets)
 * 3. Bus Monitoring (CAN will monitor the bus, only reading, never acknowledging)
 * This enum refers to the HAL_FDCAN's hardware operational mode. 
 * See @ref canlib2_change_mode.
*/
typedef enum {
    // CAN will read and write normally, and acknowledge filtered packets
    CANLIB2_MODE_READ_WRITE         = FDCAN_MODE_NORMAL,
    // CAN will read only, and acknowledge filtered packets
    CANLIB2_MODE_READ_ACKNOWLEDGE   = FDCAN_MODE_RESTRICTED_OPERATION,
    // CAN will read only, and not acknowledge any packets, even if filtered
    CANLIB2_MODE_READ_ONLY          = FDCAN_MODE_BUS_MONITORING
} canlib2_hw_mode;

/**
 * @enum canlib2_frame_type
 * @brief Enum for CAN data frame type.
 * All frames are classic frames with 11-bit identifier.
*/
typedef enum {
    // CAN Data Frame (up to 8 bytes of data)
    CANLIB2_DATA_FRAME              = FDCAN_DATA_FRAME,
    // CAN Remote Frame (no data)
    CANLIB2_REMOTE_FRAME            = FDCAN_REMOTE_FRAME,
    // No CAN frame sent/received
    CANLIB2_NO_FRAME                    = 0x40000000U
} canlib2_frame_type;

/**
 * @enum canlib2_filter_action
 * @brief Enum for CAN filter action.
 * To receive any data from the CANFD peripheral, we have to use filters.
 * Typically, we send data to one of the CAN peripheral's two FIFO queues.
 * We can also send them to a queue with high priority, or mark a packet high priority for another filter.
 * When a filter other than REJECT is enabled, packets that fit the filter's criteria will be acknowledged.
*/
typedef enum {
    // Filter packets to FIFO 0
    CANLIB2_FILTER_TO_FIFO0         = FDCAN_FILTER_TO_RXFIFO0,
    // Filter packets to FIFO 1
    CANLIB2_FILTER_TO_FIFO1         = FDCAN_FILTER_TO_RXFIFO1,
    // Filter packets to FIFO 0 and mark them high priority
    CANLIB2_FILTER_TO_FIFO0_HP      = FDCAN_FILTER_TO_RXFIFO0_HP,
    // Filter packets to FIFO 1 and mark them high priority
    CANLIB2_FILTER_TO_FIFO1_HP      = FDCAN_FILTER_TO_RXFIFO1_HP,
    // Reject packets, and do not acknowledge
    CANLIB2_REJECT                  = FDCAN_FILTER_REJECT,
    // Mark packets as high priority, but do not send them to a FIFO.
    // If the packet must be received, it must be sent to a FIFO by another filter.
    CANLIB2_MARK_HP                 = FDCAN_FILTER_HP,
    // Disable this filter, and do not acknowledge.
    CANLIB2_DISABLE                 = FDCAN_FILTER_DISABLE
} canlib2_filter_action;

/**
 * @enum canlib2_fifo
 * @brief Enum to select a RX FIFO queue to receive from. 
*/
typedef enum {
    // FIFO 0
    CANLIB2_FIFO0                   = FDCAN_RX_FIFO0,
    // FIFO 1
    CANLIB2_FIFO1                   = FDCAN_RX_FIFO1
} canlib2_fifo;

/**
 * @enum canlib2_non_matching_action
 * @brief Global configuration enum for what to do with packets that do not match filters.
 * By default, we reject all non-matching packets, but it might make sense to filter them
 * into a FIFO if we want to monitor the system.
*/
typedef enum {
    // Send non-matching CAN packets to FIFO0
    CANLIB2_NM_TO_FIFO0             = FDCAN_ACCEPT_IN_RX_FIFO0,
    // Send non-matching CAN packets to FIFO1
    CANLIB2_NM_TO_FIFO1             = FDCAN_ACCEPT_IN_RX_FIFO1,
    // Reject non-matching CAN packets
    CANLIB2_NM_REJECT               = FDCAN_REJECT
} canlib2_non_matching_action;

/**
 * @enum canlib2_remote_action 
 * @brief Global configuration enum for what to do with remote frames.
 * CAN remote frames do not have a data field, and are usually used by a master device
 * to request data from devices along the bus.
 * By default, we filter remote frames and return them using @ref canlib2_rx_return_t,
 * which supports both data frames and remote frames.
*/
typedef enum {
    CANLIB2_ACCEPT_REMOTE           = FDCAN_FILTER_REMOTE,
    CANLIB2_REJECT_REMOTE           = FDCAN_REJECT_REMOTE
} canlib2_remote_action;

/**
 * @brief Enum for events related to sending data from this CAN peripheral.
*/
typedef enum {
    // TX transmission is complete
    CANLIB2_TX_COMPLETE             = FDCAN_IT_TX_COMPLETE,
    // TX transmission was aborted (the abortion is complete)
    CANLIB2_TX_ABORT_COMPLETE       = FDCAN_IT_TX_ABORT_COMPLETE,
    // TX FIFO is empty, meaning all packets that were queued are now done sending
    CANLIB2_TX_FIFO_EMPTY           = FDCAN_IT_TX_FIFO_EMPTY,
    // Bitmask for all TX events together (utility)
    CANLIB2_TX_ALL_EVENTS           = FDCAN_IT_TX_COMPLETE | FDCAN_IT_TX_ABORT_COMPLETE | FDCAN_IT_TX_FIFO_EMPTY
} canlib2_tx_event;

/**
 * @brief Enum for events related to receiving data with this CAN peripheral.
*/
typedef enum {
    // A message stored in RX FIFO 0 or destined for FIFO 0 was lost
    CANLIB2_RX_FIFO0_MESSAGE_LOST   = FDCAN_IT_RX_FIFO0_MESSAGE_LOST,
    // RX FIFO 0 is full
    CANLIB2_RX_FIFO0_FULL           = FDCAN_IT_RX_FIFO0_FULL,
    // A new message has been received in RX FIFO 0
    CANLIB2_RX_FIFO0_NEW_MESSAGE    = FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
    // A message store in RX FIFO 1 or destined for FIFO 1 was lost
    CANLIB2_RX_FIFO1_MESSAGE_LOST   = FDCAN_IT_RX_FIFO1_MESSAGE_LOST,
    // RX FIFO 1 is full
    CANLIB2_RX_FIFO1_FULL           = FDCAN_IT_RX_FIFO1_FULL,
    // A new message has been received in RX FIFO 1
    CANLIB2_RX_FIFO1_NEW_MESSAGE    = FDCAN_IT_RX_FIFO1_NEW_MESSAGE,
    // Bitmask for all RX events together (utility)
    CANLIB2_RX_ALL_EVENTS           = 0x003F  // least 6 bits high
} canlib2_rx_event;

/**
 * @struct canlib2_identifier_t
 * @brief Definition for CAN identifier struct with priority and address.
 * canlib2 splits the 11-bit CAN identifier into 2 fields, a 3-bit priority and 
 * 8-bit address. This struct contains those two fields separately.
*/
typedef struct CANLib2_Identifier {
    /// @brief 3-bit priority (ID[10:8])
    uint8_t priority;

    /// @brief 8-bit address (ID[7:0])
    uint8_t address;
} canlib2_identifier_t;

/**
 * @struct canlib2_rx_return_t
 * @brief Definition for the return type sent to RX callbacks.
*/
typedef struct CANLib2_RX_Callback_ReturnType {
    /// @brief pointer to HAL FDCAN peripheral
    FDCAN_HandleTypeDef* fdcan;

    /// @brief event received
    /// @ref canlib2_rx_event
    canlib2_rx_event event;

    /// @brief identifier received
    /// @ref canlib2_identifier
    canlib2_identifier_t identifier;

    /// @brief type of received frame
    /// @ref canlib2_frame_type
    canlib2_frame_type frame_type;

    /// @brief data field length (0-8 bytes)
    uint8_t length;

    /// @brief pointer to data field
    uint8_t* data;
} canlib2_rx_return_t;

/**
 * @struct canlib2_tx_return_t
 * @brief Definition for the return type sent to TX callbacks.
*/
typedef struct CANLib2_TX_Callback_ReturnType {
    /// @brief pointer to HAL FDCAN peripheral
    FDCAN_HandleTypeDef* fdcan;
    
    /// @brief event received
    /// @ref canlib2_rx_event
    canlib2_tx_event event;
} canlib2_tx_return_t;

/**
 * @brief function pointer type for a valid canlib2 RX callback function.
*/
typedef void (*canlib2_rx_callback)(FDCAN_HandleTypeDef* fdcan, canlib2_rx_return_t ret);

/**
 * @brief function pointer type for a valid canlib2 TX callback function.
*/
typedef void (*canlib2_tx_callback)(FDCAN_HandleTypeDef* fdcan, canlib2_tx_return_t ret);

/**
 * @struct canlib2_fdcan_t
 * @name canlib2_fdcan
 * @brief Defintion for the struct representing the canlib2_fdcan peripheral.
 * This is the one generally used by the user.
*/
typedef struct CANLib2_FDCAN {
    /// @brief pointer to HAL FDCAN peripheral.
    FDCAN_HandleTypeDef* fdcan;

    /// @brief current status of the canlib2_fdcan peripheral.
    /// @ref canlib2_status
    canlib2_status status;

    /// @brief callback for events received by RX FIFO 0.
    /// @ref canlib2_rx_callback
    canlib2_rx_callback rx_fifo0_callback;

    /// @brief callback for events received by RX FIFO 1.
    /// @ref canlib2_rx_callback
    canlib2_rx_callback rx_fifo1_callback;

    /// @brief callback for packets received by RX FIFO 0.
    /// @ref canlib2_tx_callback
    canlib2_tx_callback tx_callback;

    /// @brief Internal variable to storing filter indices for filter configuration. 
    /// Incremented by 1 every time a filter is added.
    uint8_t __filter_index;
} canlib2_fdcan_t;

/**
 * @brief Internal function to allocate a canlib2_fdcan peripheral from available lots.
 * @returns canlib2_fdcan peripheral pointer
*/
canlib2_fdcan_t* __canlib2_allocate();

/**
 * @brief Create and configure a canlib2_fdcan peripheral from a HAL FDCAN device.
 * @note The peripheral starts off disabled. We can enable and start the CAN device with canlib2_start.
 * @param fdcan HAL FDCAN device
 * @returns canlib2_fdcan peripheral pointer
*/
canlib2_fdcan_t* canlib2_configure(FDCAN_HandleTypeDef* fdcan);

/**
 * @brief Removes and deallocates a canlib2_fdcan peripheral.
 * @param can canlib2_fdcan peripheral to remove
 * @returns 0 on success
*/
int canlib2_deconfigure(canlib2_fdcan_t* can);

/**
 * @brief Start the canlib2_fdcan peripheral.
 * @param can canlib2_fdcan peripheral to start
 * @returns 0 on success
*/
int canlib2_start(canlib2_fdcan_t* can);

/**
 * @brief Stop the canlib2_fdcan peripheral.
 * @param can canlib2_fdcan peripheral to stop 
 * @returns 0 on success
*/
int canlib2_stop(canlib2_fdcan_t* can);

/**
 * @brief Configure the canlib2_fdcan peripheral's hardware operational mode.
 * @ref canlib2_hw_mode
 * @param can canlib2_fdcan peripheral to change the operational mode of
 * @returns 0 on success
*/
int canlib2_change_mode(canlib2_fdcan_t* can, canlib2_hw_mode mode);

/**
 * @brief Deinitialize and reinitialize the fdcan peripheral to update any changes in hfdcan.Init.
 * Note that this clears most of the settings that have been configured.
 * @param can canlib2_fdcan peripheral to update the init status of.
 * @returns 0 on success
*/
int canlib2_update_init(canlib2_fdcan_t* can);

/**
 * @brief Configure the canlib2_fdcan's global filter configuration.
 * @param can canlib2_fdcan peripheral to change the global filter config of
 * @param accept_non_matching see @ref canlib2_non_matching_action
 * @param accept_remote see @ref canlib2_remote_action
 * @returns 0 on success
*/
int canlib2_change_global_filter_config(canlib2_fdcan_t* can, canlib2_non_matching_action accept_non_matching, canlib2_remote_action accept_remote);

#ifdef CANLIB2_USE_PRECISION_TIMER
/**
 * @defgroup canlib2_transmission
 * @{
*/
/**
 * @brief Set the internal us timer to use.
 * @note The timer should be a basic timer configured to count up at 1MHz.
 * @note The timer will be configured and started once this is called.
*/
void canlib2_configure_us_timer(TIM_HandleTypeDef* tim);
#endif

/**
 * @brief Internal function to delay roughly one CAN packet.
*/
void __canlib2_delay_packet();

/**
 * @brief Internal function for universally generating a tx header.
 * @param can canlib2_fdcan peripheral
 * @param identifier 11-bit CAN identifier
 * @param frame_type see @ref canlib2_frame_type\
 * @returns FDCAN TX Header
 * @note The data field length is not specified, and must be configured manually.
*/
FDCAN_TxHeaderTypeDef __canlib2_get_tx_header(canlib2_fdcan_t* can, uint16_t identifier, uint32_t frame_type);

/**
 * @brief Generate a FDCAN TxHeader for a Data Frame with an address
 * @param can canlib2_fdcan peripheral
 * @param addr 8-bit address
 * @returns FDCAN TX Header
 * @note The data field length is not specified, and must be configured manually.
*/
FDCAN_TxHeaderTypeDef canlib2_get_tx_header(canlib2_fdcan_t* can, uint8_t addr);

/**
 * @brief Generate a FDCAN TxHeader for a Data Frame with an address and priority
 * @param can canlib2_fdcan peripheral
 * @param priority 3-bit priority
 * @param addr 8-bit address
 * @returns FDCAN TX Header
 * @note The data field length is not specified, and must be configured manually.
*/
FDCAN_TxHeaderTypeDef canlib2_get_tx_header_p(canlib2_fdcan_t* can, uint8_t priority, uint8_t addr);

/**
 * @brief Generate a FDCAN TxHeader for a Data Frame with an 11-bit identifier
 * @param can canlib2_fdcan peripheral
 * @param identifier 11-bit identifier
 * @returns FDCAN TX Header
 * @note The data field length is not specified, and must be configured manually.
*/
FDCAN_TxHeaderTypeDef canlib2_get_tx_header_id(canlib2_fdcan_t* can, uint16_t identifier);

/**
 * @brief Send a packet of data using a Data Frame to an address
 * @param can canlib2_fdcan peripheral
 * @param addr 8-bit address
 * @param length length of data field
 * @param data pointer to data
 * @returns 0 on success
*/
int canlib2_send_data(canlib2_fdcan_t* can, uint8_t addr, uint8_t length, uint8_t* data);

/**
 * @brief Send a packet of data using a Data Frame to an address with a priority
 * @param can canlib2_fdcan peripheral
 * @param priority 3-bit priority
 * @param addr 8-bit address
 * @param length length of data field
 * @param data pointer to data
 * @returns 0 on success
*/
int canlib2_send_data_p(canlib2_fdcan_t* can, uint8_t priority, uint8_t addr, uint8_t length, uint8_t* data);

/**
 * @brief Send a packet of data using a Data Frame with an identifier
 * @param can canlib2_fdcan peripheral
 * @param identifier 11-bit identifier
 * @param length length of data field
 * @param data pointer to data
 * @returns 0 on success
*/
int canlib2_send_data_id(canlib2_fdcan_t* can, uint16_t identifier, uint8_t length, uint8_t* data);

/**
 * @brief Generate a FDCAN TxHeader for a Remote Frame with an address
 * @param can canlib2_fdcan peripheral
 * @param addr 8-bit address
 * @returns FDCAN TX Header
*/
FDCAN_TxHeaderTypeDef canlib2_get_tx_remote_header(canlib2_fdcan_t* can, uint8_t addr);

/**
 * @brief Generate a FDCAN TxHeader for a Remote Frame with an address and priority
 * @param can canlib2_fdcan peripheral
 * @param priority 3-bit priority
 * @param addr 8-bit address
 * @returns FDCAN TX Header
*/
FDCAN_TxHeaderTypeDef canlib2_get_tx_remote_header_p(canlib2_fdcan_t* can, uint8_t priority, uint8_t addr);

/**
 * @brief Generate a FDCAN TxHeader for a Remote Frame with an identifier
 * @param can canlib2_fdcan peripheral
 * @param identifier 16-bit identifier
 * @returns FDCAN TX Header
*/
FDCAN_TxHeaderTypeDef canlib2_get_tx_remote_header_id(canlib2_fdcan_t* can, uint16_t identifier);

/**
 * @brief Send a request for data using a Remote Frame to an address
 * @param can canlib2_fdcan peripheral
 * @param addr 8-bit address
 * @returns 0 on success
*/
int canlib2_send_remote(canlib2_fdcan_t* can, uint8_t addr);

/**
 * @brief Send a request for data using a Remote Frame to an address with a priority
 * @param can canlib2_fdcan peripheral
 * @param priority 3-bit priority
 * @param addr 8-bit address
 * @returns 0 on success
*/
int canlib2_send_remote_p(canlib2_fdcan_t* can, uint8_t priority, uint8_t addr);

/**
 * @brief Send a request for data using a Remote Frame with an identifier
 * @param can canlib2_fdcan peripheral
 * @param identifier 11-bit identifier
 * @returns 0 on success
*/
int canlib2_send_remote_id(canlib2_fdcan_t* can, uint16_t identifier);
/**
 * @} // canlib2_tranmission
*/

/**
 * @defgroup canlib2_adding_filters
 * @{
*/

/**
 * @brief Add a filter to this canlib2_fdcan peripheral, filtering by address
 * @param can canlib2_fdcan peripheral
 * @param action see @ref canlib2_filter_action
 * @param addr 8-bit address to match exactly
 * @returns 0 on success
*/
int canlib2_add_rx_filter_by_address(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t addr);

/**
 * @brief Add a filter to this canlib2_fdcan peripheral, filtering by address and exact priority
 * @param can canlib2_fdcan peripheral
 * @param action see @ref canlib2_filter_action
 * @param priority 3-bit priority to match exactly
 * @param addr 8-bit address to match exactly
 * @returns 0 on success
*/
int canlib2_add_rx_filter_by_address_p(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority, uint8_t addr);

/**
 * @brief Add a filter to this canlib2_fdcan peripheral, filtering by address and a mask for priority
 * @param can canlib2_fdcan peripheral
 * @param action see @ref canlib2_filter_action
 * @param priority_mask 3-bit bitmask for priority (1 = match)
 * @param priority_match 3-bit match for the priority bitmask
 * @param addr 8-bit address to match exactly
 * @returns 0 on success
*/
int canlib2_add_rx_filter_by_address_pm(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority_mask, uint8_t priority_match, uint8_t addr);

/**
 * @brief Add a filter to this canlib2_fdcan peripheral, filtering by a partial address (bits [7:8-N])
 * @param can canlib2_fdcan peripheral
 * @param action see @ref canlib2_filter_action
 * @param match_bit_count number of bits, starting at MSB, to match (0-8)
 * @param match 8-bit address to match against
 * @returns 0 on success
*/
int canlib2_add_rx_filter_by_partial_address(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t match_bit_count, uint8_t match);

/**
 * @brief Add a filter to this canlib2_fdcan peripheral, filtering by a partial address (bits [7:8-N]) and exact priority
 * @param can canlib2_fdcan peripheral
 * @param action see @ref canlib2_filter_action
 * @param priority 3-bit address to match exactly
 * @param match_bit_count number of bits, starting at MSB, to match (0-8)
 * @param match 8-bit address to match against
 * @returns 0 on success
*/
int canlib2_add_rx_filter_by_partial_address_p(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority, uint8_t match_bit_count, uint8_t match);

/**
 * @brief Add a filter to this canlib2_fdcan peripheral, filtering by a partial address (bits [7:8-N]) and priority mask
 * @param can canlib2_fdcan peripheral
 * @param action see @ref canlib2_filter_action
 * @param priority_mask 3-bit bitmask for priority (1 = match)
 * @param priority_match 3-bit match for the priority bitmask
 * @param match_bit_count number of bits, starting at MSB, to match (0-8)
 * @param match 8-bit address to match against
 * @returns 0 on success
*/
int canlib2_add_rx_filter_by_partial_address_pm(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority_mask, uint8_t priority_match, uint8_t match_bit_count, uint8_t match);

/**
 * @brief Add a filter to this canlib2_fdcan peripheral, filtering by its 11-bit identifier
 * @param can canlib2_fdcan peripheral
 * @param action see @ref canlib2_filter_action
 * @param identifier 11-bit identifier to match exactly
 * @returns 0 on success
*/
int canlib2_add_rx_filter_by_id(canlib2_fdcan_t* can, canlib2_filter_action action, uint16_t identifier);

/**
 * @brief Add a filter to this canlib2_fdcan peripheral, filtering by exact priority only
 * @param can canlib2_fdcan peripheral
 * @param action see @ref canlib2_filter_action
 * @param priority 3-bit address to match exactly
 * @returns 0 on success
*/
int canlib2_add_rx_filter_by_priority(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority);

/**
 * @brief Add a filter to this canlib2_fdcan peripheral, filtering by a priority mask only
 * @param can canlib2_fdcan peripheral
 * @param action see @ref canlib2_filter_action
 * @param priority_mask 3-bit bitmask for priority (1 = match)
 * @param priority_match 3-bit match for the priority bitmask
 * @returns 0 on success
*/
int canlib2_add_rx_filter_by_priority_mask(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority_mask, uint8_t priority_match);

/**
 * @brief Add a filter to this canlib2_fdcan peripheral, filtering by bitmasks for both priority and address
 * @param can canlib2_fdcan peripheral
 * @param action see @ref canlib2_filter_action
 * @param priority_mask 3-bit bitmask for priority (1 = match)
 * @param priority_match 3-bit match for the priority bitmask
 * @param addr_mask 8-bit bitmask for address (1 = match)
 * @param addr 8-bit match for the address bitmask
 * @returns 0 on success
*/
int canlib2_add_rx_filter(canlib2_fdcan_t* can, canlib2_filter_action action, uint8_t priority_mask, uint8_t priority_match, uint8_t addr_mask, uint8_t addr_match);

/**
 * @} // canlib2_adding_filters
*/

/**
 * @defgroup canlib2_receiving_data
 * @{
*/

/**
 * @brief Get the number of FIFO elements can be sent before the TX FIFO is full
 * See STM32 FDCAN application notes about FIFO elements sizes.
 * When the FIFO is full, free level will be 0 elements.
 * @param can canlib2_fdcan peripheral
 * @returns Number of free TX FIFO elements
*/
int canlib2_tx_fifo_free(canlib2_fdcan_t* can);

/**
 * @brief Returns whether the TX FIFO is full.
 * @param can canlib2_fdcan peripheral
 * @returns 1 if the TX FIFO is full, 0 otherwise.
*/
int canlib2_tx_fifo_full(canlib2_fdcan_t* can);

/**
 * @brief Returns the fill level (number of elements) of an RX FIFO.
 * See STM32 FDCAN application notes about FIFO elements sizes.
 * When the FIFO is full, fill level will be 64 elements.
 * @param can canlib2_fdcan peripheral
 * @param fifo CANLIB2_FIFO0 or CANLIB2_FIFO1
 * @returns Number of elements in the RX FIFO.
*/
int canlib2_rx_fifo_fill(canlib2_fdcan_t* can, canlib2_fifo fifo);

/**
 * @brief Returns whether an RX FIFO is full.
 * @param can canlib2_fdcan peripheral
 * @param fifo CANLIB2_FIFO0 or CANLIB2_FIFO1
 * @returns 1 if the RX FIFO is full, 0 otherwise.
*/
int canlib2_rx_fifo_full(canlib2_fdcan_t* can, canlib2_fifo fifo);

/**
 * @brief Manually receive a packet of data from the canlib2_fdcan peripheral.
 * @param can canlib2_fdcan peripheral
 * @param fifo CANLIB2_FIFO0 or CANLIB2_FIFO1
 * @returns A @ref canlib2_rx_return_t containing the information in the packet.
 * @note This is slightly redundant, and may in the future be called by the FIFO0/FIFO1 callback for modularity.
*/
canlib2_rx_return_t canlib2_receive_data(canlib2_fdcan_t* can, canlib2_fifo fifo);
/**
 * @} // canlib2_receiving_data
*/

/**
 * @defgroup canlib2_events_and_callbacks
 * @{
*/

/**
 * @brief Enable (activate notifications) for an RX event, enabling that event to trigger an interrupt. 
 * @param can canlib2_fdcan peripheral
 * @param event see @ref canlib2_rx_event
*/
int canlib2_enable_rx_interrupt(canlib2_fdcan_t* can, canlib2_rx_event event);

/**
 * @brief Disable (disable notifications) for an RX event, stopping that event from trigger an interrupt. 
 * @param can canlib2_fdcan peripheral
 * @param event see @ref canlib2_rx_event
*/
int canlib2_disable_rx_interrupt(canlib2_fdcan_t* can, canlib2_rx_event event);

/**
 * @brief Enable (activate notifications) for a TX event, enabling that event to trigger an interrupt. 
 * @param can canlib2_fdcan peripheral
 * @param event see @ref canlib2_tx_event
*/
int canlib2_enable_tx_interrupt(canlib2_fdcan_t* can, canlib2_tx_event event);

/**
 * @brief Disable (disable notifications) for a TX event, stopping that event from trigger an interrupt. 
 * @param can canlib2_fdcan peripheral
 * @param event see @ref canlib2_tx_event
*/
int canlib2_disable_tx_interrupt(canlib2_fdcan_t* can, canlib2_tx_event event);

/**
 * @brief Disable all notifications for a canlib2_fdcan peripheral.
 * @param can canlib2_fdcan peripheral
*/
int canlib2_disable_interrupts(canlib2_fdcan_t* can);

/**
 * @brief Set the RX callback to a custom @ref canlib2_rx_callback.
 * @param can canlib2_fdcan peripheral
 * @param fifo CANLIB2_FIFO0 or CANLIB2_FIFO1
 * @param callback custom callback to register
*/
int canlib2_set_rx_callback(canlib2_fdcan_t* can, canlib2_fifo fifo, canlib2_rx_callback callback);

/**
 * @brief Set the TX callback to a custom @ref canlib2_tx_callback.
 * @param can canlib2_fdcan peripheral
 * @param fifo CANLIB2_FIFO0 or CANLIB2_FIFO1
 * @param callback custom callback to register
*/
int canlib2_set_tx_callback(canlib2_fdcan_t* can, canlib2_tx_callback callback);

/**
 * @brief Internal method to model a generic TX event.
 * Called by various TX interrupts, when enabled.
 * @param hfdcan HAL FDCAN device
 * @param TxEventFifoITs bitmask for @ref canlib2_tx_event
*/
void canlib2_generic_tx_event(FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs);

/**
 * @brief Default RX callback, fired when a custom callback is not set.
 * Currently does nothing.
*/
void canlib2_default_rx_callback(FDCAN_HandleTypeDef* fdcan, canlib2_rx_return_t ret);

/**
 * @brief Default TX callback, fired when a custom callback is not set.
 * Currently does nothing.
*/
void canlib2_default_tx_callback(FDCAN_HandleTypeDef* fdcan, canlib2_tx_return_t ret);
/**
 * @} // canlib2_events_and_callbacks
*/

void HAL_FDCAN_TxEventFifoCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs);
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);
void HAL_FDCAN_TxFifoEmptyCallback(FDCAN_HandleTypeDef *hfdcan);
void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes);
void HAL_FDCAN_TxBufferAbortCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes);

#ifdef __cplusplus
}
#endif

#endif
