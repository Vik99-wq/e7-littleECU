#ifndef __INCLUDE_TIMING_PREDICTION_H
#define __INCLUDE_TIMING_PREDICTION_H

#include <stdint.h>
#include "timing.h"

#define TP_NUM_POINTS 5 // Number of data points for quadratic fit
#define TP_ELAPSED_US TIMING_VALID_RANGE_MAX_US
#define TP_ELAPSED_TICKS 10 * TP_ELAPSED_US
#define TP_INVALID_RESET_THRESHOLD 10

typedef struct timing_data_point {
    uint64_t timestamp;
    int32_t time_us;    // x (negative, with latest point being closest to zero)
    int32_t period_us;  // y
} timing_data_point_t;

void predict_init();
void predict_log_new_data(uint32_t period);
timing_data_point_t predict_get_data(uint8_t i);
void predict_periodic_reset();

// Function to predict the next period 
uint32_t predict_next_period();


#endif // __INCLUDE_TIMING_PREDICTION_H