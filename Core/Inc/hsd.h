#ifndef __INCLUDE_HSD_H
#define __INCLUDE_HSD_H

#include "main.h"
#include "stm32h5xx_hal.h"
#include "device.h"

#define HSD_120_ID 0x0010
#define HSD_121_ID 0x0011
#define HSD_12X_DIA_ID 0x0012
#define HSD_50_ID 0x0013
#define HSD_51_ID 0x0014
#define HSD_5X_DIA_ID 0x0015

typedef struct hsd_dia_state {
    uint8_t latch : 1;
    uint8_t dia_en : 1;
    uint8_t sel1 : 1;
    uint8_t sel2 : 1;
} hsd_dia_state_t;

typedef struct hsd_config {
    GPIO_TypeDef* latch_port; 
    uint16_t latch_pin;
    GPIO_TypeDef* dia_en_port;
    uint16_t dia_en_pin;
    GPIO_TypeDef* sel1_port;
    uint16_t sel1_pin;
    GPIO_TypeDef* sel2_port;
    uint16_t sel2_pin;
    GPIO_TypeDef* en1_port;
    uint16_t en1_pin;
    GPIO_TypeDef* en2_port;
    uint16_t en2_pin;
    // TODO: add analog config for SNS (not implemented yet)
} hsd_config_t;

typedef enum hsd_dia_options {
    HSDD_DIA_OFF = 0,
    HSDD_I_CH1 = 1,
    HSDD_I_CH2 = 2,
    HSDD_TEMP = 3,
    HSDD_LATCH = 4,
    HSDD_READ = 0xFF
} hsd_dia_options_t;

typedef struct hsd {
    hsd_config_t config;
    hsd_dia_state_t dia_state;
    uint8_t en1;
    uint8_t en2;
} hsd_t;

extern hsd_t hsd_12x;
extern hsd_t hsd_5x;

extern const device_t hsd_120_dev;
extern const device_t hsd_121_dev;
extern const device_t hsd_12x_dia_dev;

extern const device_t hsd_50_dev;
extern const device_t hsd_51_dev;
extern const device_t hsd_5x_dia_dev;

// init
void hsd_init();

// update state for a given hsd, given current configuration
void hsd_update_state(hsd_t* hsd);

// input: 1 byte (1 or 0), output: 1 byte (0)
data_field_t* hsd_120_ioctl(data_field_t* cmd);
// input: 1 byte (1 or 0), output: 1 byte (0)
data_field_t* hsd_121_ioctl(data_field_t* cmd);
// input: 4 bytes (hsd_dia_options_t), output: 4 bytes (0 or 0xFF or ADC reading as a float)
data_field_t* hsd_12x_dia_ioctl(data_field_t* cmd);

// input: 1 byte (1 or 0), output: 1 byte (0)
data_field_t* hsd_50_ioctl(data_field_t* cmd);
// input: 1 byte (1 or 0), output: 1 byte (0)
data_field_t* hsd_51_ioctl(data_field_t* cmd);
// input: 4 bytes (hsd_dia_options_t), output: 4 bytes (0 or 0xFF or ADC reading as a float)
data_field_t* hsd_5x_dia_ioctl(data_field_t* cmd);

// three ioctls per hsd
// one for each enable output (en1, en2)
// one for diagnostics, either return or change configuration using hsd_sns_mux

#endif // __INCLUDE_HSD_H