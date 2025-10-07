#include "timing_prediction.h"
#include "core.h"

// circular queue for data
timing_data_point_t tp_queue[TP_NUM_POINTS];
timing_data_point_t* tp_ptr = tp_queue;
uint8_t tp_data_count;
uint8_t tp_invalid_data_count;

void predict_log_new_data(uint32_t data) {
    // if data is too crazy, just skip it
    // but if theres too many, reset
    if (tp_data_count >= TP_NUM_POINTS && 
        (data > predict_get_data(1).period_us * 1.5 || data < predict_get_data(1).period_us / 1.5)) 
    {
        ++tp_invalid_data_count;
        if (tp_invalid_data_count >= TP_INVALID_RESET_THRESHOLD) predict_init();
        else return;
    } else { 
        if (tp_invalid_data_count > 0) --tp_invalid_data_count;
    }

    tp_ptr->timestamp = core_get_tick();
    tp_ptr->period_us = data;
    tp_ptr->time_us = 0;

    for (int i = 0; i < TP_NUM_POINTS; i++) {
        tp_queue[i].time_us -= data;
    }

    ++tp_ptr;
    if (tp_data_count < TP_NUM_POINTS) ++tp_data_count;
    if (tp_ptr > tp_queue + (TP_NUM_POINTS - 1)) tp_ptr = tp_queue;
}

timing_data_point_t predict_get_data(uint8_t i) {
    if (i <= 0 || i > TP_NUM_POINTS) {
        timing_data_point_t null_data = {.period_us = 0, .time_us = 0, .timestamp = core_get_tick()};
        return null_data;
    }
    int index = (tp_ptr - tp_queue) + 2 * TP_NUM_POINTS - i;
    index = index % TP_NUM_POINTS;
    return tp_queue[index];
}

void predict_init() {
    tp_data_count = 0;
    tp_invalid_data_count = 0;
    tp_ptr = tp_queue;
    tp_queue[TP_NUM_POINTS - 1].timestamp = core_get_tick();
}

// reset if period is too large
// called in core 100ms task
void predict_periodic_reset() {
    // if too much time has elapsed, clear the queue
    uint64_t now = core_get_tick();
    if (now - predict_get_data(1).timestamp > TP_ELAPSED_TICKS) {
        tp_data_count = 0;
        tp_invalid_data_count = 0;
        tp_ptr = tp_queue;
    }
}

uint32_t predict_next_period() {
    if (tp_data_count < TP_NUM_POINTS) {
        return 0x7FFF0000; // enormous period, should never fire
    } else {
        // average the last three derivatives
        timing_data_point_t p4 = predict_get_data(4);
        timing_data_point_t p3 = predict_get_data(3);
        timing_data_point_t p2 = predict_get_data(2);
        timing_data_point_t p1 = predict_get_data(1);

        float d3 = (p3.period_us - p4.period_us) / (float) (p3.time_us - p4.time_us);
        float d2 = (p2.period_us - p3.period_us) / (float) (p2.time_us - p3.time_us);
        float d1 = (p1.period_us - p2.period_us) / (float) (p1.time_us - p2.time_us);

        float d = (d3 + d2 + d1) / 3;

        float d2_2 = (d2 - d3) / (float) (p2.time_us - p3.time_us);
        float d2_1 = (d1 - d2) / (float) (p1.time_us - p2.time_us);

        float d2_ = (d2_2 + d2_1) / 2;

        // add it to last valid point 
        float period_0 = p1.period_us + d * (-p1.time_us) + 0.5 * d2_ * (p1.time_us * p1.time_us);
        /*
        // add it to average point from past three
        float pavg = (p3.period_us + p2.period_us + p1.period_us) / 3;
        float tavg = (p3.time_us + p2.time_us + p1.time_us) / 3;

        float period_0 = pavg + d * (-tavg);

        tp_d = d;
        tp_pavg = pavg;
        tp_tavg = tavg;
        */
        return (uint32_t) period_0;
    }
}