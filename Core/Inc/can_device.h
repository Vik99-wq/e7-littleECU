#ifndef __INCLUDE_CAN_DEVICE_H
#define __INCLUDE_CAN_DEVICE_H

#include "canlib2.h"
#include "device.h"
#include "core.h"

#define LOCAL_CAN_DEVTAB_SIZE 256

typedef uint8_t* (*can_dev_rcv_fn) (uint8_t* cmd);

typedef struct can_device {
    uint16_t id; // 11-bit identifier corresponding to CAN (only top 8 should be used)
    const char* name;
    uint8_t priority;
    uint8_t input_length;
    uint8_t prev_data[8];
    can_dev_rcv_fn callback;
} can_device_t;

// initialize can devtab
// returns pointer to can devtab
can_device_t* can_dev_init_devtab(FDCAN_HandleTypeDef* fdcan);

// start can devtab
void can_dev_start(FDCAN_HandleTypeDef* fdcan);

void can_dev_set_callback(uint16_t id, can_dev_rcv_fn callback);

// register device in can devtab
// returns pointer to device placed in can devtab
can_device_t* can_dev_register(uint16_t id, uint8_t priority, uint8_t input_length);

// get device corresponding to id
// returns pointer to device, else NULL
can_device_t* can_dev_get_device(uint16_t id);

void can_dev_ioctl(uint16_t id, data_field_t* cmd);

void can_dev_cmd_callback(FDCAN_HandleTypeDef* fdcan, canlib2_rx_return_t ret);

void can_dev_rcv_callback(FDCAN_HandleTypeDef* fdcan, canlib2_rx_return_t ret);

uint8_t* can_dev_rcv_test_fn (uint8_t* cmd);


#endif // __INCLUDE_CAN_DEVICE_H