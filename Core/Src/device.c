#include "device.h"
#include "can_device.h"

device_t devtab[LOCAL_DEVTAB_SIZE];
size_t device_count;

// initialize devtab
// returns pointer to devtab
device_t* dev_init_devtab() {
    for (size_t i = 0; i < LOCAL_DEVTAB_SIZE; i++) {
        devtab[i] = dev_empty_dev;
    }
    device_count = 0;
    return devtab;
}

// register device in devtab
// returns pointer to device placed in devtab, or null if not enough devices
device_t* dev_register(device_t dev) {
    if (device_count >= LOCAL_DEVTAB_SIZE) return NULL;
    devtab[device_count] = dev;
    ++device_count;
    return &devtab[device_count-1];
}

// get device corresponding to id
// returns pointer to device, else NULL
device_t* dev_get_device(uint16_t id) {
    for (size_t i = 0; i < LOCAL_DEVTAB_SIZE; i++) {
        if (devtab[i].id == id) return &(devtab[i]);
    }
    return NULL;
}

// call ioctl corresponding to id
// returns ioctl result, else NULL
data_field_t dev_null_data = {.data = {0, 0, 0, 0, 0, 0, 0, 0}, .length=0};
data_field_t* dev_ioctl(uint16_t id, data_field_t* cmd) {
    device_t* dev = dev_get_device(id);
    if (dev == NULL) {
        // try can devices
        can_dev_ioctl(id, cmd);
        return &dev_null_data;
    }
    return dev->ioctl(cmd);
}

// ioctls for empty and test devices
const char dev_empty_dev_name[] = "DEVICE_EMPTY";
const device_t dev_empty_dev = {
    .id = DEV_EMPTY_DEVICE_ID,
    .name = dev_empty_dev_name,
    .ioctl = dev_empty_dev_ioctl
};
data_field_t* dev_empty_dev_ioctl(__unused data_field_t* cmd) {
    return &dev_null_data;
}

const char dev_test_dev_name[] = "DEVICE_TEST";
const device_t dev_test_dev = {
    .id = DEV_TEST_DEVICE_ID,
    .name = dev_test_dev_name,
    .ioctl = dev_test_dev_ioctl
};
data_field_t dev_test_data_field = {.length=1, .data={0,0,0,0,0,0,0,0}};
data_field_t* dev_test_dev_ioctl(data_field_t* cmd) {
    if (cmd == NULL) return NULL;
    if (cmd->length < 1) return NULL;
    if (cmd->data[0] == 1) HAL_GPIO_WritePin(LIFE_LED_GPIO_Port, LIFE_LED_Pin, GPIO_PIN_SET);
    else if (cmd->data[0] == 0) HAL_GPIO_WritePin(LIFE_LED_GPIO_Port, LIFE_LED_Pin, GPIO_PIN_RESET); 
    else dev_test_data_field.data[0] = 0xFF;
    return &dev_test_data_field;
}