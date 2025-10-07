#ifndef __INCLUDE_CORE_H
#define __INCLUDE_CORE_H

#include "main.h"
#include "stm32h5xx_hal.h"

void core_init(TIM_HandleTypeDef* htim_10us_i, TIM_HandleTypeDef* htim_100ns_tick_i, TIM_HandleTypeDef* htim_timing, FDCAN_HandleTypeDef* fdcan);

// TICK TIMER RELATED
uint64_t core_get_tick();
void core_reset_tick();

uint64_t core_get_us_tick();

void core_10us_callback();
void core_1ms_callback();
void core_100ms_callback();

void core_10us_loop();
void core_1ms_loop();
void core_100ms_loop();

#endif // __INCLUDE_CORE_H