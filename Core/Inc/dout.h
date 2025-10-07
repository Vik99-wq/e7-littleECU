#ifndef __INCLUDE_DOUT_H
#define __INCLUDE_DOUT_H

#include "main.h"
#include "stm32h5xx_hal.h"
#include "device.h"

#define DOUT_COUNT 4
#define DOUT_DEVICE_ID 0x0022

typedef struct dout {
    GPIO_TypeDef* port;
    uint16_t pin;
} dout_t;

typedef struct dout_input {
    uint8_t i;
    uint8_t value;
} dout_input_t;

// add one to douttab size because we aren't using douttab[0]
// this is just for conventions on the littleECU v1
extern const dout_t douttab[DOUT_COUNT+1];

// initialize douttab 
void dout_init();

// set single output (ignores invalid)
void dout_set(uint8_t i, uint8_t v);

// 2 bytes input: dout number, dout value (dout_input_t)
// 1 byte output: 0 if success, -1 if invalid
data_field_t* dout_ioctl(data_field_t* cmd);

extern const device_t dout_dev;

#endif // __INCLUDE_DOUT_H