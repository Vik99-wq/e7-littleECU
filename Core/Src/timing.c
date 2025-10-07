#include "timing.h"
#include "timing_prediction.h"

TIM_HandleTypeDef* offset_timer;
uint64_t timing_prev_tick; // 100ns ticks
uint64_t timing_us_prev_rotation;
uint32_t timing_rpm;
timing_state_t timing_state;
uint8_t timing_set_up = 0;
uint32_t timing_pred_us;

timing_event_t* timing_current_event;
timing_event_t timing_events[NUM_TIMING_EVENTS] = 
{
    {
        .state = TS_HOLD,
        .halt_timer = 0,
        .time_fraction = 0
    }, {
        .state = TS_WAIT,
        .halt_timer = 0,
        .time_fraction = 12.0 / 360
    }, {
        .state = TS_SPARK,
        .halt_timer = 1,
        .time_fraction = 348.0 / 360
    }, {
        .state = TS_INVALID,
        .halt_timer = 1,
        .time_fraction = 1
    }
};


data_field_t hsd_df_one = {.length=1, .data={1}};
data_field_t hsd_df_zero = {.length=1, .data={0}};

// callback for top dead center
void timing_tdc_callback() {
    if (!timing_set_up) return;
    // start timing cycles

    uint64_t now = core_get_tick();
    timing_us_prev_rotation = (now - timing_prev_tick) / 10;
    timing_prev_tick = now;

    // calculate RPM for debug purposes
    timing_rpm = (uint32_t) (((float)(US_PER_S * S_PER_M)) / ((float) timing_us_prev_rotation));

    // add to predictor
    predict_log_new_data(timing_us_prev_rotation);
    // get prediction
    timing_pred_us = predict_next_period();

    // run timing system if we are within our range
    if (timing_pred_us < TIMING_VALID_RANGE_MAX_US && TIMING_VALID_RANGE_MIN_US < timing_us_prev_rotation
        && timing_pred_us < timing_us_prev_rotation * 1.2 && timing_pred_us > timing_us_prev_rotation * 0.8) {
        // calculate event end timings for timers
        for (int i = 0; i < NUM_TIMING_EVENTS-1; i++) {
            timing_events[i].end_us = timing_events[i+1].time_fraction * timing_us_prev_rotation;
        }
        timing_events[NUM_TIMING_EVENTS-1].end_us = timing_us_prev_rotation;

        // set current event to first event
        timing_current_event = &timing_events[0];
        timing_state = timing_current_event->state;
        timing_set_state(timing_state);
        timing_timer_setup(timing_current_event->end_us - (core_get_tick() - timing_prev_tick) / 10);
        timing_timer_begin();
    } else {
        timing_state = TS_INVALID;
        timing_set_state(TS_INVALID);
    }
}

// init timing system
void timing_init(TIM_HandleTypeDef* tim) {
    offset_timer = tim;
    timing_state = TS_INVALID;
    timing_prev_tick = 0x7fffffff; // arbitary large value
    timing_set_up = 1;
    // assume MX_Init correctly initialized timer to run at 10us (100kHz)
}

// configure timer for n * 10us
void timing_timer_setup(uint32_t us) {
    offset_timer->Instance->ARR = (us / 2) - 1;
    offset_timer->Instance->CNT = 0;
}

// start timer
void timing_timer_begin() {
    if (!timing_set_up) return;
    HAL_TIM_Base_Start_IT(offset_timer);
}

// timer interrupt
void timing_timer_callback() {
    if (!timing_set_up) return;
    
    // stop timer
    HAL_TIM_Base_Stop_IT(offset_timer);
    offset_timer->Instance->SR &= ~TIM_SR_UIF; // clear interrupt flag because the autoreset doesn't do anything after stop

    // write real us for debug
    timing_current_event->real_us = (core_get_tick() - timing_prev_tick) / 10;

    // increment the current event
    ++timing_current_event;

    // set the state for new event
    timing_set_state(timing_current_event->state);

    // if event calls for halt timer, return and do not proceed
    if (timing_current_event->halt_timer) return;

    // start timer for future - now
    timing_timer_setup(timing_current_event->end_us - (core_get_tick() - timing_prev_tick) / 10);
    timing_timer_begin();
}

// configure state
void timing_set_state(timing_state_t state) {
    if (!timing_set_up) return;
    timing_state = state;
    switch (state) {
        case TS_WAIT:
            dev_ioctl(HSD_120_ID, &hsd_df_zero);
            break;
        case TS_SPARK:
            dev_ioctl(HSD_120_ID, &hsd_df_one);
            break;
        case TS_INVALID:
            dev_ioctl(HSD_120_ID, &hsd_df_zero);
            break;
        case TS_HOLD:
            dev_ioctl(HSD_120_ID, &hsd_df_one);
            break;
    }
}

const device_t timing_dev = {
    .id = TIMING_DEV_ID,
    .ioctl = timing_ioctl,
    .name = "timing"
};

uint32_t timing_buf_4;
uint64_t timing_buf_8;

data_field_t timing_ioctl_data_field = {.length=0};
data_field_t* timing_ioctl(data_field_t* cmd) {
    if (!timing_set_up) return NULL;
    if (cmd == NULL) return NULL;
    if (cmd->length < 1) return NULL;
    switch (cmd->data[0]) {
        case TIC_GET_PERIOD:
            timing_buf_4 = timing_us_prev_rotation;
            *((uint32_t*) timing_ioctl_data_field.data) = timing_buf_4;
            timing_ioctl_data_field.length = 4;
            break;
        case TIC_GET_RPM:
            timing_buf_4 = timing_rpm;
            *((uint32_t*) timing_ioctl_data_field.data) = timing_buf_4;
            timing_ioctl_data_field.length = 4;
            break;
        case TIC_GET_STATE:
            timing_buf_4 = timing_state;
            *((uint32_t*) timing_ioctl_data_field.data) = timing_buf_4;
            timing_ioctl_data_field.length = 4;
            break;
        case TIC_GET_TICK:
            timing_buf_8 = timing_prev_tick;
            *((uint64_t*) timing_ioctl_data_field.data) = timing_buf_8;
            timing_ioctl_data_field.length = 8;
            break;
    }
    return &timing_ioctl_data_field;
}


