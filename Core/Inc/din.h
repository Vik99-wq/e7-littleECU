#ifndef __INCLUDE_DIN_H
#define __INCLUDE_DIN_H

#include "main.h"
#include "stm32h5xx_hal.h"
#include "device.h"

#define DIN_COUNT 4
#define DIN_DEVICE_ID 0x0021

typedef struct din {
    GPIO_TypeDef* port;
    uint16_t pin;
} din_t;

// add one to dintab size because we aren't using dintab[0]
// this is just for conventions on the littleECU v1
extern const din_t dintab[DIN_COUNT+1];

// initialize dintab 
void din_init();

// get single input (default 0, invalid -1)
uint8_t din_get(uint8_t i);

// 1 byte input: din number
// 1 byte output: din value
data_field_t* din_ioctl(data_field_t* cmd);

extern const device_t din_dev;

#endif // __INCLUDE_DIN_H