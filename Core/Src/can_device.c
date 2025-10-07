#include "can_device.h"

can_device_t can_devtab[LOCAL_CAN_DEVTAB_SIZE];
size_t can_device_count;

canlib2_fdcan_t* can;

// initialize can devtab
// returns pointer to can devtab
can_device_t* can_dev_init_devtab(FDCAN_HandleTypeDef* fdcan) {
    can_device_count = 0;

    // create a canlib2_fdcan_t*
    can = canlib2_configure(fdcan);

    if (canlib2_enable_rx_interrupt(can, CANLIB2_RX_FIFO0_NEW_MESSAGE)) Error_Handler();
    if (canlib2_enable_rx_interrupt(can, CANLIB2_RX_FIFO1_NEW_MESSAGE)) Error_Handler();
    if (canlib2_change_global_filter_config(can, CANLIB2_NM_REJECT, CANLIB2_REJECT_REMOTE)) Error_Handler();
    if (canlib2_set_rx_callback(can, CANLIB2_FIFO0, can_dev_cmd_callback)) Error_Handler();
    if (canlib2_set_rx_callback(can, CANLIB2_FIFO1, can_dev_rcv_callback)) Error_Handler();

    // add required filters here

    // filter for commands
    if (canlib2_add_rx_filter(can, CANLIB2_FILTER_TO_FIFO0, 0x1, 0x0, 0xC0, 0x00)) Error_Handler();

    // filters for receives
    if (canlib2_add_rx_filter(can, CANLIB2_FILTER_TO_FIFO1, 0x1, 0x1, 0xFC, 0x40)) Error_Handler(); // o2
    if (canlib2_add_rx_filter(can, CANLIB2_FILTER_TO_FIFO1, 0x1, 0x1, 0xFC, 0x80)) Error_Handler(); // radio
    if (canlib2_add_rx_filter(can, CANLIB2_FILTER_TO_FIFO1, 0x1, 0x1, 0xE0, 0xC0)) Error_Handler(); // ddu
    if (canlib2_add_rx_filter(can, CANLIB2_FILTER_TO_FIFO1, 0x1, 0x1, 0xFC, 0xF0)) Error_Handler(); // debug

    return can_devtab;
}

// start the can peripheral for can devices
// must register can devices before starting!
void can_dev_start(FDCAN_HandleTypeDef* fdcan) {
    if (fdcan != can->fdcan) return;
    if (canlib2_start(can) == CANLIB2_ERROR) Error_Handler();
}

// register device in can devtab
// returns pointer to device placed in can devtab
can_device_t* can_dev_register(uint16_t id, uint8_t priority, uint8_t input_length) {
    if (can_device_count >= LOCAL_CAN_DEVTAB_SIZE) return NULL;
    can_device_t dev = {.id = id, .name="CAN device", 
                        .priority=priority, .input_length=input_length,
                        .prev_data={0, 0, 0, 0, 0, 0, 0, 0}, .callback=NULL};
    can_devtab[can_device_count] = dev;
    ++can_device_count;
    return &can_devtab[can_device_count-1];
}

// get device corresponding to id
// returns pointer to device, else NULL
can_device_t* can_dev_get_device(uint16_t id) {
    for (size_t i = 0; i < LOCAL_CAN_DEVTAB_SIZE; i++) {
        if (can_devtab[i].id == id) return &(can_devtab[i]);
    }
    return NULL;
}

void can_dev_set_callback(uint16_t id, can_dev_rcv_fn callback) {
    can_device_t* dev = can_dev_get_device(id);
    if (dev == NULL) return;
    dev->callback = callback;
}

// call ioctl corresponding to id
// returns ioctl result, else NULL
void can_dev_ioctl(uint16_t id, data_field_t* cmd) {
    can_device_t* dev = can_dev_get_device(id);
    if (dev == NULL) return;
    if (canlib2_send_data_p(can, (dev->priority << 1) | 0x0, dev->id & 0xFF, dev->input_length, cmd->data)) Error_Handler();
}

data_field_t can_dev_df;
data_field_t* can_dev_result_df;
void can_dev_cmd_callback(FDCAN_HandleTypeDef* fdcan, canlib2_rx_return_t ret) {
    if (fdcan != can->fdcan) return;
    if (ret.event == CANLIB2_RX_FIFO0_NEW_MESSAGE && ret.frame_type == CANLIB2_DATA_FRAME) {
        // if it is not a command, do not respond
        if (ret.identifier.priority | 0x0) return;

        // make call to local device
        uint16_t id = ret.identifier.address;
        device_t* dev = dev_get_device(id);
        if (dev == NULL) return;
        can_dev_df.length = ret.length;
        for (int i = 0; i < ret.length; i++) {
            can_dev_df.data[i] = ret.data[i];
        }
        can_dev_result_df = dev->ioctl(&can_dev_df);
        
        // return call over CAN
        uint16_t priority = ret.identifier.priority;
        if (can_dev_result_df == NULL) return;
        if (canlib2_send_data_p(
            can, priority | 0x1, dev->id & 0xFF, 
            can_dev_result_df->length, can_dev_result_df->data)
        ) Error_Handler();
    }
}

void can_dev_rcv_callback(FDCAN_HandleTypeDef* fdcan, canlib2_rx_return_t ret) {
    if (fdcan != can->fdcan) return;
    if (ret.event == CANLIB2_RX_FIFO0_NEW_MESSAGE && ret.frame_type == CANLIB2_DATA_FRAME) {
        uint16_t id = ret.identifier.address;
        can_device_t* dev = can_dev_get_device(id);
        if (dev == NULL) return;
        for (int i = 0; i < ret.length; i++) {
            dev->prev_data[i] = ret.data[i];
        }
        if (dev->callback != NULL) dev->callback(dev->prev_data);
    }
}

uint8_t* can_dev_rcv_test_fn (uint8_t* cmd) {
    UNUSED(cmd);
    return NULL;
}