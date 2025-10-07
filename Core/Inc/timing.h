#ifndef __INCLUDE_TIMING_H
#define __INCLUDE_TIMING_H

#include "main.h"
#include "stm32h5xx_hal.h"
#include "device.h"
#include "core.h"
#include "hsd.h"

#define US_PER_S 1000000
#define S_PER_M 60

#define TIMING_VALID_RANGE_MIN_US 5000 // 12000rpm
#define TIMING_VALID_RANGE_MAX_US 120000 // 500rpm - cannot be more than 65536 * 2

#define NUM_TIMING_EVENTS 4

#define TIMING_DEV_ID 0x0016

typedef enum timing_state {
    TS_INVALID = 0,
    TS_HOLD = 1,
    TS_WAIT = 2,
    TS_SPARK = 3
} timing_state_t;

typedef struct timing_event {
    timing_state_t state;
    uint8_t halt_timer; // if 1, the timer will not start again once this event is transitioned into
    float time_fraction; // value less than 1 (where 1 is full rotation)
    uint64_t end_us; // autoupdated - at some point refactor so this is a pointer to a struct containing end times
    uint64_t real_us;
} timing_event_t;

// callback for top dead center
void timing_tdc_callback();

// init timing system
void timing_init(TIM_HandleTypeDef* tim);

// configure timer for n * 10us
void timing_timer_setup(uint32_t us);

// start timer
void timing_timer_begin();

// timer interrupt
void timing_timer_callback();

// configure state
void timing_set_state(timing_state_t state);

// ioctl commands
typedef enum timing_ioctl_cmd {
    TIC_GET_RPM = 0, // returns 4-byte RPM 
    TIC_GET_TICK = 1, // returns 8-byte tick counter
    TIC_GET_PERIOD = 2, // returns 4-byte period in us
    TIC_GET_STATE = 3 // returns 4-byte state enum (timing_state_t) 
} timing_ioctl_cmd_t;

// timing ioctl
// 1 byte input - timing_ioctl_cmd_t
// n bytes output depending on command
data_field_t* timing_ioctl(data_field_t* cmd);
extern const device_t timing_dev;

#endif // __INCLUDE_TIMING_H