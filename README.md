# Core Embedded System Firmware

## Overview

This project implements an embedded control and diagnostics framework for an STM32-based microcontroller, supporting **digital I/O**, **high-side driver (HSD)** control, **timing and prediction systems**, and **CAN-based communication** between multiple devices. It’s designed for modularity, so each functional unit—such as input, output, timing, or communication—is managed by an independent software module registered in a centralized device table.

The firmware leverages the STM32 HAL for hardware abstraction and is organized to enable predictable real-time performance with precise timing and device coordination.

---

## Project Goals

- **Unified Device Management:** Abstract all hardware peripherals (digital I/O, drivers, timing units, etc.) behind a standardized device interface.
- **CAN Network Communication:** Support distributed control and telemetry through CAN bus with device-level command handling.
- **Timing Control:** Provide event-based timing management, rotation period prediction, and state sequencing for time-critical applications (e.g., engine or motor control).
- **Predictive Control:** Integrate adaptive timing prediction based on historical data to improve response accuracy.
- **Scalability:** Allow new device types to be added easily with minimal integration effort.

---

## Project Structure

| File | Description |
|------|--------------|
| **main.c** | Initializes STM32 peripherals (GPIO, ADC, CAN, SPI, Timers, USB, etc.) and starts the core system. Calls `core_init()` to register all devices and begin periodic loops. |
| **core.c** | The central orchestrator of all system activities. It initializes devices, handles timing callbacks (10 µs, 1 ms, 100 ms), and manages periodic logic for digital inputs/outputs and HSD channels. |
| **device.c** | Implements the device registration and IOCTL dispatch system. Each hardware or logical module is registered as a `device_t` with a unique ID and an `ioctl` function pointer. |
| **din.c** | Manages **Digital Inputs**. Provides `din_get()` and `din_ioctl()` to read pin states from configured input channels. |
| **dout.c** | Manages **Digital Outputs**. Defines GPIO mappings and provides `dout_set()` and `dout_ioctl()` to control outputs safely. |
| **hsd.c** | Controls **High-Side Driver (HSD)** channels for both 12x and 5x devices. Supports diagnostics (current, temperature, and latch reads), enabling/disabling outputs, and state updates via `hsd_update_state()`. |
| **timing.c** | Handles timing sequences synchronized with physical events like top dead center (TDC). Uses timers to schedule state changes (HOLD, WAIT, SPARK, INVALID) and integrates predictive timing adjustments. |
| **timing_prediction.c** | Implements a lightweight time-series predictor for estimating the next timing cycle duration based on recent history. Uses a circular buffer and derivative-based extrapolation for adaptive control. |
| **can_device.c** | Manages CAN-level communication for registered devices. Defines RX filters, command callbacks, and remote IOCTL forwarding for distributed system control. |
| **canlib2.c** | Core CAN library abstraction layer. Wraps STM32 HAL FDCAN APIs to simplify configuration, message transmission, reception, and filter management. Supports both standard and remote frames. |
| **device.h, core.h, hsd.h, din.h, dout.h, timing.h, timing_prediction.h** | Header files defining structures, constants, and function prototypes for their corresponding modules. |

---

## Core Architecture

### Device Abstraction Layer
All hardware modules implement the `device_t` structure:
```c
typedef struct {
    uint16_t id;
    const char* name;
    data_field_t* (*ioctl)(data_field_t* cmd);
} device_t;
```
Each device defines an `ioctl` method to receive and respond to commands. For example:
- Digital outputs: `dout_ioctl()`
- Timing system: `timing_ioctl()`
- HSD diagnostics: `hsd_12x_dia_ioctl()`

These are registered via `dev_register()` during system initialization in `core_init()`.

### Timing and Prediction Engine
The **timing subsystem** handles recurring physical events (like rotations or pulses) by:
1. Measuring the time between top dead center events (`timing_tdc_callback()`).
2. Computing the RPM and using the predictor (`predict_next_period()`).
3. Scheduling new events (`SPARK`, `WAIT`, etc.) using timers with microsecond precision.

This makes the system capable of real-time closed-loop control for mechanical or motor-based systems.

### CAN Communication
The **CAN stack** (via `canlib2.c` and `can_device.c`) supports:
- Filtering and routing messages to appropriate devices.
- Executing remote IOCTL commands through CAN frames.
- Sending diagnostic or telemetry data back to a master node.

---

## Execution Flow

1. **Initialization**
   - `main.c` calls `core_init()`, which:
     - Configures timers, CAN, and device tables.
     - Registers digital, HSD, and timing devices.
     - Starts CAN communication and timing prediction.

2. **Runtime Loop**
   - `core_10us_callback()` executes every 10 microseconds:
     - Updates system ticks and invokes `core_10us_loop()`.
   - `core_1ms_callback()` and `core_100ms_callback()` handle slower updates, such as sampling inputs and updating outputs.

3. **CAN Communication**
   - Incoming CAN frames are decoded by `can_dev_cmd_callback()`.
   - The system invokes the corresponding device’s IOCTL and returns responses.

---

## Build and Deployment

### Requirements
- **STM32CubeIDE** or **Makefile-based build system**
- **STM32 HAL library**
- Hardware: STM32H5xx MCU or compatible board
- Optional: CAN transceiver and connected nodes for distributed testing

### Building
1. Import project into STM32CubeIDE or copy source into an existing HAL project.
2. Ensure all required GPIO, FDCAN, and timer peripherals are initialized in CubeMX.
3. Build and flash the firmware.

### Running
- Power up the MCU and ensure CAN lines are terminated properly.
- Observe periodic output toggling and diagnostics via CAN messages.
- Modify IOCTL calls to control devices dynamically through software or remote commands.

---

## Extending the System

To add a new device:
1. Create a new source file implementing:
   ```c
   const device_t my_device = {
       .id = MY_DEVICE_ID,
       .name = "My Device",
       .ioctl = my_device_ioctl
   };
   ```
2. Implement `my_device_ioctl()` to handle input/output operations.
3. Register it in `core_init()` using `dev_register(my_device);`.

