#include "hsd.h"
#include "main.h"

hsd_t hsd_12x = {
    .config = {
        .dia_en_port = HSD120_DIA_EN_GPIO_Port,
        .dia_en_pin = HSD120_DIA_EN_Pin,
        .en1_port = HSD120_EN1_GPIO_Port,
        .en1_pin = HSD120_EN1_Pin,
        .en2_port = HSD120_EN2_GPIO_Port,
        .en2_pin = HSD120_EN2_Pin,
        .latch_port = HSD120_LATCH_GPIO_Port,
        .latch_pin = HSD120_LATCH_Pin,
        .sel1_port = HSD120_SEL1_GPIO_Port,
        .sel1_pin = HSD120_SEL1_Pin,
        .sel2_port = HSD120_SEL2_GPIO_Port,
        .sel2_pin = HSD120_SEL2_Pin
    }, 
    .dia_state = { // DIA_OFF state by default
        .dia_en = 0,
        .latch = 0,
        .sel1 = 0,
        .sel2 = 0
    },
    .en1 = 0,
    .en2 = 0
};
hsd_t hsd_5x = {
    .config = {
        .dia_en_port = HSD50_DIA_EN_GPIO_Port,
        .dia_en_pin = HSD50_DIA_EN_Pin,
        .en1_port = HSD50_EN1_GPIO_Port,
        .en1_pin = HSD50_EN1_Pin,
        .en2_port = HSD50_EN2_GPIO_Port,
        .en2_pin = HSD50_EN2_Pin,
        .latch_port = HSD50_LATCH_GPIO_Port,
        .latch_pin = HSD50_LATCH_Pin,
        .sel1_port = HSD50_SEL1_GPIO_Port,
        .sel1_pin = HSD50_SEL1_Pin,
        .sel2_port = HSD50_SEL2_GPIO_Port,
        .sel2_pin = HSD50_SEL2_Pin
    }, 
    .dia_state = { // DIA_OFF state by default
        .dia_en = 0,
        .latch = 0,
        .sel1 = 0,
        .sel2 = 0
    },
    .en1 = 0,
    .en2 = 0
};

const device_t hsd_120_dev = {
    .id = HSD_120_ID,
    .name = "HSD_120",
    .ioctl = hsd_120_ioctl
};
const device_t hsd_121_dev = {
    .id = HSD_121_ID,
    .name = "HSD_121",
    .ioctl = hsd_121_ioctl
};
const device_t hsd_12x_dia_dev = {
    .id = HSD_12X_DIA_ID,
    .name = "HSD_12X diagnostics",
    .ioctl = hsd_12x_dia_ioctl
};

const device_t hsd_50_dev = {
    .id = HSD_50_ID,
    .name = "HSD_50",
    .ioctl = hsd_50_ioctl
};
const device_t hsd_51_dev = {
    .id = HSD_51_ID,
    .name = "HSD_51",
    .ioctl = hsd_51_ioctl
};
const device_t hsd_5x_dia_dev = {
    .id = HSD_5X_DIA_ID,
    .name = "HSD_5X diagnostics",
    .ioctl = hsd_5x_dia_ioctl
};

// init
void hsd_init() {
    // assume MX_Init was successful and configures correctly
    hsd_update_state(&hsd_5x);
    hsd_update_state(&hsd_12x);
}

// update state for a given hsd, given current configuration
void hsd_update_state(hsd_t* hsd) {
    HAL_GPIO_WritePin(hsd->config.dia_en_port, hsd->config.dia_en_pin, hsd->dia_state.dia_en ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hsd->config.latch_port, hsd->config.latch_pin, hsd->dia_state.latch ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hsd->config.sel1_port, hsd->config.sel1_pin, hsd->dia_state.sel1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hsd->config.sel2_port, hsd->config.sel2_pin, hsd->dia_state.sel2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hsd->config.en1_port, hsd->config.en1_pin, hsd->en1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hsd->config.en2_port, hsd->config.en2_pin, hsd->en2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

data_field_t success = {.length=1, .data={0,0,0,0,0,0,0,0}};
// input: 1 byte (1 or 0), output: 1 byte (0)
data_field_t* hsd_120_ioctl(data_field_t* cmd) {
    if (cmd == NULL) return NULL;
    if (cmd->length < 1) return NULL;
    if (cmd->data[0]) hsd_12x.en1 = 1; else hsd_12x.en1 = 0;
    hsd_update_state(&hsd_12x);
    return &success;
}
// input: 1 byte (1 or 0), output: 1 byte (0)
data_field_t* hsd_121_ioctl(data_field_t* cmd) {
    if (cmd == NULL) return NULL;
    if (cmd->length < 1) return NULL;
    if (cmd->data[0]) hsd_12x.en2 = 1; else hsd_12x.en2 = 0;
    hsd_update_state(&hsd_12x);
    return &success;
}

data_field_t hsd_12x_dia_data_field = {.length=1};
// input: 4 bytes (hsd_dia_options_t), output: 4 bytes (0 or 0xFF or ADC reading as a float)
data_field_t* hsd_12x_dia_ioctl(data_field_t* cmd) {
    if (cmd == NULL) return NULL;
    if (cmd->length < 1) return NULL;
hsd_dia_options_t hdo = *((hsd_dia_options_t*) &cmd->data);
    switch ((int) hdo) {
        case HSDD_DIA_OFF:
            hsd_12x.dia_state.dia_en = 0;
            hsd_12x.dia_state.latch = 0;
            hsd_12x.dia_state.sel1 = 0;
            hsd_12x.dia_state.sel2 = 0;
            break;
        case HSDD_I_CH1:
            hsd_12x.dia_state.dia_en = 1;
            hsd_12x.dia_state.latch = 0;
            hsd_12x.dia_state.sel1 = 0;
            hsd_12x.dia_state.sel2 = 0;
            break;
        case HSDD_I_CH2:
            hsd_12x.dia_state.dia_en = 1;
            hsd_12x.dia_state.latch = 0;
            hsd_12x.dia_state.sel1 = 0;
            hsd_12x.dia_state.sel2 = 1;
            break;
        case HSDD_TEMP:
            hsd_12x.dia_state.dia_en = 0;
            hsd_12x.dia_state.latch = 0;
            hsd_12x.dia_state.sel1 = 1;
            hsd_12x.dia_state.sel2 = 0;
            break;
        case HSDD_LATCH:
            hsd_12x.dia_state.dia_en = 0;
            hsd_12x.dia_state.latch = 1;
            hsd_12x.dia_state.sel1 = 0;
            hsd_12x.dia_state.sel2 = 0;
            break;
        case HSDD_READ:
            // TODO: add read functionality
            break;
        default:
            hsd_12x_dia_data_field.data[0] = 0xFF;
            return &hsd_12x_dia_data_field;
            break;
    }
    hsd_update_state(&hsd_12x);
    hsd_12x_dia_data_field.data[0] = 0x00;
    return &hsd_12x_dia_data_field;
}

// input: 1 byte (1 or 0), output: 1 byte (0)
data_field_t* hsd_50_ioctl(data_field_t* cmd) {
    if (cmd == NULL) return NULL;
    if (cmd->length < 1) return NULL;
    if (cmd->data[0]) hsd_5x.en1 = 1; else hsd_5x.en1 = 0;
    hsd_update_state(&hsd_5x);
    return &success;
}
// input: 1 byte (1 or 0), output: 1 byte (0)
data_field_t* hsd_51_ioctl(data_field_t* cmd) {
    if (cmd == NULL) return NULL;
    if (cmd->length < 1) return NULL;
    if (cmd->data[0]) hsd_5x.en2 = 1; else hsd_5x.en2 = 0;
    hsd_update_state(&hsd_5x);
    return &success;
}

data_field_t hsd_5x_dia_data_field = {.length=1};
// input: 4 bytes (hsd_dia_options_t), output: 4 bytes (0 or 0xFF or ADC reading as a float)
data_field_t* hsd_5x_dia_ioctl(data_field_t* cmd) {
    if (cmd == NULL) return NULL;
    if (cmd->length < 1) return NULL;
hsd_dia_options_t hdo = *((hsd_dia_options_t*) &cmd->data);
    switch ((int) hdo) {
        case HSDD_DIA_OFF:
            hsd_5x.dia_state.dia_en = 0;
            hsd_5x.dia_state.latch = 0;
            hsd_5x.dia_state.sel1 = 0;
            hsd_5x.dia_state.sel2 = 0;
            break;
        case HSDD_I_CH1:
            hsd_5x.dia_state.dia_en = 1;
            hsd_5x.dia_state.latch = 0;
            hsd_5x.dia_state.sel1 = 0;
            hsd_5x.dia_state.sel2 = 0;
            break;
        case HSDD_I_CH2:
            hsd_5x.dia_state.dia_en = 1;
            hsd_5x.dia_state.latch = 0;
            hsd_5x.dia_state.sel1 = 0;
            hsd_5x.dia_state.sel2 = 1;
            break;
        case HSDD_TEMP:
            hsd_5x.dia_state.dia_en = 0;
            hsd_5x.dia_state.latch = 0;
            hsd_5x.dia_state.sel1 = 1;
            hsd_5x.dia_state.sel2 = 0;
            break;
        case HSDD_LATCH:
            hsd_5x.dia_state.dia_en = 0;
            hsd_5x.dia_state.latch = 1;
            hsd_5x.dia_state.sel1 = 0;
            hsd_5x.dia_state.sel2 = 0;
            break;
        case HSDD_READ:
            // TODO: add read functionality
            break;
        default:
            hsd_5x_dia_data_field.data[0] = 0xFF;
            return &hsd_5x_dia_data_field;
            break;
    }
    hsd_update_state(&hsd_5x);
    hsd_5x_dia_data_field.data[0] = 0x00;
    return &hsd_5x_dia_data_field;
}