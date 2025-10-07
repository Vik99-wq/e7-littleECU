#include "din.h"

const din_t dintab[DIN_COUNT+1] = {
    {.port=DIN1_GPIO_Port, .pin=DIN1_Pin}, // cloning 0 and 1 because of littleECU convention
    {.port=DIN1_GPIO_Port, .pin=DIN1_Pin},
    {.port=DIN2_GPIO_Port, .pin=DIN2_Pin},
    {.port=DIN3_GPIO_Port, .pin=DIN3_Pin},
    {.port=DIN4_GPIO_Port, .pin=DIN4_Pin}
};

// initialize dintab 
void din_init() {
    // do nothing
    // assume MX_Init is configured correctly in main.c
}

// get single input (default 0, invalid -1)
uint8_t din_get(uint8_t i) {
    if (i > DIN_COUNT) return -1;
    return HAL_GPIO_ReadPin(dintab[i].port, dintab[i].pin);
}

// 1 byte input: din number
// 1 byte output: din value
data_field_t din_ioctl_result = {.length=1};
data_field_t* din_ioctl(data_field_t* cmd) {
    if (cmd == NULL) return NULL;
    if (cmd->length < 1) return NULL;
    uint8_t i = cmd->data[0];
    din_ioctl_result.data[0] = din_get(i);
    return &din_ioctl_result;
}

// DIN device
const device_t din_dev = {
    .id = DIN_DEVICE_ID,
    .name = "DIN",
    .ioctl = din_ioctl
};