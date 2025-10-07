#include "dout.h"

const dout_t douttab[DOUT_COUNT+1] = {
    {.port=DOUT1_GPIO_Port, .pin=DOUT1_Pin}, // cloning 0 and 1 because of littleECU convention
    {.port=DOUT1_GPIO_Port, .pin=DOUT1_Pin},
    {.port=DOUT2_GPIO_Port, .pin=DOUT2_Pin},
    {.port=DOUT3_GPIO_Port, .pin=DOUT3_Pin},
    {.port=DOUT4_GPIO_Port, .pin=DOUT4_Pin}
};

// initialize dintab 
void dout_init() {
    // do nothing
    // assume MX_Init is configured correctly in main.c
}

// set single output (ignores invalid)
void dout_set(uint8_t i, uint8_t v) {
    if (i > DOUT_COUNT) return;
    HAL_GPIO_WritePin(douttab[i].port, douttab[i].pin, v ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// 2 bytes input: dout number, dout value (dout_input_t)
// 1 byte output: 0 if success, -1 if invalid
data_field_t dout_ioctl_result = {.length=1};
data_field_t* dout_ioctl(data_field_t* cmd) {
    if (cmd == NULL) return NULL;
    if (cmd->length < 1) return NULL;

    dout_ioctl_result.data[0] = 0;
    dout_input_t* dit = (dout_input_t*) cmd->data;
    if (dit->i > DOUT_COUNT || (dit->value != 0 && dit->value != 1)) {
        dout_ioctl_result.data[0] = -1;
        return &dout_ioctl_result;
    }

    dout_set(dit->i, dit->value);
    return &dout_ioctl_result;
}

const device_t dout_dev = {
    .id = DOUT_DEVICE_ID,
    .name = "DOUT",
    .ioctl = dout_ioctl
};
