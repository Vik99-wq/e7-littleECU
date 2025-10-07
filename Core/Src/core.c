#include "core.h"
#include "main.h"
#include "device.h"
#include "din.h"
#include "dout.h"
#include "hsd.h"
#include "timing.h"
#include "timing_prediction.h"
#include "can_device.h"
#include "stm32h5xx_hal.h"

uint64_t core_100ns_tick;
uint64_t core_100ns_start;

uint32_t core_10us_tick;

TIM_HandleTypeDef* htim_10us;
TIM_HandleTypeDef* htim_100ns_tick;


void core_init(TIM_HandleTypeDef* htim_10us_i, TIM_HandleTypeDef* htim_100ns_tick_i, TIM_HandleTypeDef* htim_timing, FDCAN_HandleTypeDef* fdcan) {
    htim_10us = htim_10us_i;
    htim_100ns_tick = htim_100ns_tick_i;

    HAL_TIM_Base_Start_IT(htim_10us);
    HAL_TIM_Base_Start_IT(htim_100ns_tick);
    core_100ns_start = htim_100ns_tick->Instance->CNT;
    core_100ns_tick = 0;
    core_10us_tick = 0;

    dev_init_devtab();
    can_dev_init_devtab(fdcan);

    dev_register(dev_test_dev);

    din_init();
    dev_register(din_dev);

    dout_init();
    dev_register(dout_dev);

    hsd_init();
    dev_register(hsd_120_dev);
    dev_register(hsd_121_dev);
    dev_register(hsd_12x_dia_dev);
    dev_register(hsd_5x_dia_dev);
    dev_register(hsd_50_dev);
    dev_register(hsd_51_dev);

    timing_init(htim_timing);
    dev_register(timing_dev);

    can_dev_register(0xF0, 0x3, 0x1);

    can_dev_start(fdcan);
    predict_init();
}

uint64_t core_get_tick() {
    core_100ns_tick = htim_100ns_tick->Instance->CNT - core_100ns_start;
    return core_100ns_tick;
}

void core_reset_tick() {
    core_100ns_start = htim_100ns_tick->Instance->CNT;
    core_100ns_tick = 0;
    core_10us_tick = 0;
}

uint64_t core_get_us_tick() {
    return core_get_tick() / 10;
}

void core_10us_callback() {
    // update tick
    ++core_10us_tick;
    if (core_10us_tick % 100 == 0) core_1ms_callback();
    if (core_10us_tick % 10000 == 0) core_100ms_callback();
    core_10us_tick %= 100000;

    // update core_tick
    core_get_tick();
    // update the loop
    core_10us_loop();
}

void core_1ms_callback() {
    core_1ms_loop();
}

void core_100ms_callback() {
    core_100ms_loop();
}

uint32_t counter = 0;
const uint32_t compare = 5000;
uint8_t greater = 0;

void core_10us_loop() {
    greater = counter <= compare ? 1 : 0;
    ++counter;
    counter %= 100000;

    dev_ioctl(HSD_121_ID, (void*) &(greater));
}

void core_1ms_loop() {
    for (uint8_t i = 1; i < 5; i++) {
        data_field_t df = {.length=1, .data={i}};
        uint8_t value = dev_ioctl(DIN_DEVICE_ID, &df)->data[0];
        dout_input_t dit = {.i = i, .value = value};
        //dev_ioctl(DOUT_DEVICE_ID, (void*) &dit);
        switch (i) {
            case 1:
                //dev_ioctl(HSD_120_ID, (void*) &(dit.value));
                break;
            case 2:
                //dev_ioctl(HSD_121_ID, (void*) &(dit.value));
                break;
            case 3:
                dev_ioctl(HSD_50_ID, (void*) &(dit.value));
                break;
            case 4:
                dev_ioctl(HSD_51_ID, (void*) &(dit.value));
                break;
        }
    }
}

uint8_t i;
void core_100ms_loop() {
    ++i;
    i %= 2;

    data_field_t df = {.length=1, .data={i}};
    dev_ioctl(DEV_TEST_DEVICE_ID, &df);
    dev_ioctl(0xF0, &df);

    predict_periodic_reset();
}
