#ifndef __INCLUDE_DEVICE_H
#define __INCLUDE_DEVICE_H

#include "main.h"
#include "stm32h5xx_hal.h"

#define LOCAL_DEVTAB_SIZE 32

#define DEV_EMPTY_DEVICE_ID 0xF800
#define DEV_TEST_DEVICE_ID 0xF801

typedef struct device_data_field {
    uint8_t length;
    uint8_t padding[3];
    uint8_t data[8];
} data_field_t;

typedef data_field_t* (*device_ioctl) (data_field_t* cmd);

typedef struct device {
    uint16_t id; // 11-bit identifier corresponding to CAN (only top 8 should be used)
    const char* name;
    device_ioctl ioctl; // ioctl function: pass in a uint8_t* (up to 8 bytes) and returns a uint8_t* (up to 8 bytes)
} device_t;

// handle empty device cases
extern const char dev_empty_dev_name[];
data_field_t* dev_empty_dev_ioctl(__unused data_field_t* cmd);
extern const device_t dev_empty_dev;

// handle test device - life led
// input, output both 1 byte, write 0 or 1 to turn on or off led, returns new led state or 0xff if fail.
extern const char dev_test_dev_name[];
data_field_t* dev_test_dev_ioctl(data_field_t* cmd);
extern const device_t dev_test_dev;

// initialize devtab
// returns pointer to devtab
device_t* dev_init_devtab();

// register device in devtab
// returns pointer to device placed in devtab
device_t* dev_register(device_t dev);

// get device corresponding to id
// returns pointer to device, else NULL
device_t* dev_get_device(uint16_t id);

// call ioctl corresponding to id
// returns ioctl result, else NULL
data_field_t* dev_ioctl(uint16_t id, data_field_t* cmd);

#endif // __INCLUDE_DEVICE_H