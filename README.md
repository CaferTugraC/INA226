# INA226 — Bare-Metal C Driver

A lightweight, deterministic, and hardware-agnostic bare-metal C driver for the **Texas Instruments INA226** High/Low-Side Bi-Directional Current and Power Monitor. Designed for resource-constrained embedded systems where every byte and every cycle counts.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
![Language: C99](https://img.shields.io/badge/Language-C99-green.svg)
![No Dynamic Allocation](https://img.shields.io/badge/Heap-Zero%20Allocation-orange.svg)
![No FPU Required](https://img.shields.io/badge/FPU-Not%20Required-red.svg)

---

## Table of Contents

- [Key Design Principles](#key-design-principles)
- [Design Guarantees](#design-guarantees)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [API Reference](#api-reference)
- [Error Handling](#error-handling)
- [Testing](#testing)
- [Project Structure](#project-structure)
- [License](#license)

---

## Key Design Principles

This driver was engineered around three core pillars:

### ⚡ Speed — Zero Floating-Point, Pure Integer Arithmetic

All calculations — voltage, current, power, calibration, and alert limits — use fixed-point integer arithmetic (`int32_t` / `int64_t`). No `float`, no `double`, no soft-float emulation. Datasheet formulas like `CAL = 0.00512 / (Current_LSB × R_shunt)` are rescaled to pure integer equivalents with round-to-nearest biasing. This eliminates FPU dependency entirely and avoids the heavy cycle cost of software floating-point on MCUs without an FPU (e.g., Cortex-M0/M0+).

### 🔒 Determinism — Predictable, Bounded Execution

Every public function follows a fixed path: validate → read register → compute → write register. No loops, no recursion, no dynamic dispatch, no callbacks within the driver. I2C transactions are delegated to user-provided `extern` functions, keeping the driver's own timing fully predictable and WCET-analyzable.

### 📦 Minimal Memory — Zero Heap, Tiny Stack, Small Handle

No dynamic memory allocation (`malloc` / `free`) anywhere. All state lives in a caller-owned `ina226_handle_t` struct (~8 bytes). No internal global or static variables. Configuration options use `#define` constants instead of enums, ensuring zero additional ROM/RAM overhead.

---

## Design Guarantees

| Property | Guarantee |
|---|---|
| RAM per sensor | ~8 bytes (`ina226_handle_t`) |
| Global/static variables | 0 bytes |
| Heap allocations | None |
| Peak stack per call | < 32 bytes |
| Code size (ROM) | ~1.5–2.5 KB (ARM Cortex-M, -Os) |
| Floating-point operations | None |
| Loops / recursion in driver | None |
| Dynamic dispatch | None |
| Maximum call depth | 3 (API → helper → I2C read/write) |
| WCET analyzable | Yes — all paths are straight-line |
| Dependencies | `<stdint.h>`, `<stddef.h>` only |

> **Note:** The actual I2C transaction time depends on your platform's `INA226_Platform_I2C_Read/Write` implementation. The driver itself introduces zero non-determinism beyond those calls.

---

## Features

| Category | Capability |
|---|---|
| **Measurement** | Bus voltage (µV), shunt voltage (µV), current (µA), power (µW) |
| **Configuration** | Conversion time (140 µs – 8.244 ms), averaging (1 – 1024 samples), operating mode (continuous / triggered / shutdown) |
| **Alert System** | Shunt/Bus over/under-limit, power over-limit, conversion ready, latch mode, combined alert+CVR |
| **Calibration** | Automatic calibration register computation from shunt resistance (µΩ) and desired current LSB (µA) |
| **Portability** | Any MCU with I2C master — STM32, ESP32, AVR, PIC, nRF, MSP430, RISC-V, etc. |
| **Endianness** | Byte-level I2C buffer handling — correct on both Little-Endian and Big-Endian hosts |
| **Math Safety** | Overflow detection on every computed result (`INA226_ERR_MATH_OVERFLOW`) |
| **Validation** | Every function validates input parameters before any I2C transaction |

---

## Prerequisites

- **C99** compliant compiler (GCC, Clang, IAR, Keil, etc.)
- I2C Master peripheral on your target MCU
- Standard headers: `<stdint.h>`, `<stddef.h>`

---

## Installation

1. **Clone** the repository:
   ```bash
   git clone https://github.com/CaferTugraC/INA226.git
   ```

2. **Copy** `inc/ina226.h` and `src/ina226.c` into your project's source tree.

3. **Include** the header:
   ```c
   #include "ina226.h"
   ```

4. **Implement** the two platform I2C hook functions:
   ```c
   uint8_t INA226_Platform_I2C_Write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len) {
       // Your platform-specific I2C write implementation
       // Example (STM32 HAL):
       //   return HAL_I2C_Mem_Write(&hi2c1, dev_addr << 1, reg_addr, 1, (uint8_t*)data, len, 100);
       return 0; // 0 = success, non-zero = error
   }

   uint8_t INA226_Platform_I2C_Read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
       // Your platform-specific I2C read implementation
       // Example (STM32 HAL):
       //   return HAL_I2C_Mem_Read(&hi2c1, dev_addr << 1, reg_addr, 1, data, len, 100);
       return 0; // 0 = success, non-zero = error
   }
   ```

---

## Quick Start

### Basic Measurement

```c
#include "ina226.h"

int main(void) {
    // 1. Initialize your platform's I2C peripheral (platform-specific)
    // I2C_Init();

    // 2. Create and configure the sensor handle
    ina226_handle_t ina226;
    ina226.ina226_i2c_addr     = 0x40;       // I2C address (A0=GND, A1=GND)
    ina226.shunt_resistor_uOhm = 100000;     // 100 mΩ shunt resistor
    ina226.current_resolution_uA = 100;       // 100 µA per LSB

    // 3. Reset to defaults
    INA226_Reset(&ina226);

    // 4. Configure conversion and averaging
    INA226_Set_Averaging_Mode(&ina226, INA226_AVG_64);
    INA226_Set_Bus_Voltage_Conversion_Time(&ina226, INA226_CT_1100_US);
    INA226_Set_Shunt_Voltage_Conversion_Time(&ina226, INA226_CT_1100_US);

    // 5. Calibrate (computes and writes the calibration register)
    INA226_Calibrate(&ina226);

    // 6. Start continuous measurement
    INA226_Set_Operating_Mode(&ina226, INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE);

    // 7. Read measurements
    while (1) {
        int32_t  current_uA     = 0;
        int32_t  shunt_uV       = 0;
        uint32_t bus_voltage_uV = 0;
        uint32_t power_uW       = 0;

        INA226_Read_Current(&ina226, &current_uA);
        INA226_Read_Shunt_Voltage(&ina226, &shunt_uV);
        INA226_Read_Bus_Voltage(&ina226, &bus_voltage_uV);
        INA226_Read_Power(&ina226, &power_uW);

        // Use the values...
    }
}
```

### Alert Configuration Example

```c
// Configure an alert for bus voltage over-limit at 5V, with latch enabled
INA226_Set_Alert_Pin_Function(&ina226, INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT);
INA226_Set_Alert_Limit(&ina226, 5000000);  // 5,000,000 µV = 5.0 V
INA226_Set_Alert_Latch(&ina226, INA226_ALERT_LATCH_ENABLE);

// Later, check alert status
INA226_Alert_Status_t alert_status;
INA226_Get_Alert_Status(&ina226, &alert_status);

if (alert_status == INA226_ALERT_STATUS_LIMIT_EXCEEDED) {
    // Handle over-voltage condition
}
```

---

## API Reference

### Device Control

| Function | Description |
|---|---|
| `INA226_Reset` | Software reset — restores all registers to power-on defaults |

### Configuration

| Function | Description |
|---|---|
| `INA226_Set_Shunt_Voltage_Conversion_Time` | Set shunt voltage ADC conversion time (140 µs – 8.244 ms) |
| `INA226_Set_Bus_Voltage_Conversion_Time` | Set bus voltage ADC conversion time (140 µs – 8.244 ms) |
| `INA226_Set_Operating_Mode` | Set operating mode (shutdown / triggered / continuous) |
| `INA226_Set_Averaging_Mode` | Set hardware averaging sample count (1 – 1024) |
| `INA226_Calibrate` | Compute and write calibration register from shunt resistance and desired current LSB |

### Measurement

| Function | Returns |
|---|---|
| `INA226_Read_Current` | Signed current in **µA** (`int32_t`) |
| `INA226_Read_Shunt_Voltage` | Signed shunt voltage in **µV** (`int32_t`) |
| `INA226_Read_Bus_Voltage` | Unsigned bus voltage in **µV** (`uint32_t`) |
| `INA226_Read_Power` | Unsigned power in **µW** (`uint32_t`) |

### Alert System

| Function | Description |
|---|---|
| `INA226_Set_Alert_Pin_Function` | Configure alert source (shunt/bus/power limit, conversion ready, or combined) |
| `INA226_Get_Alert_Pin_Function` | Read back the currently configured alert function |
| `INA226_Set_Alert_Limit` | Set alert threshold in physical units (µV or µW, depending on configured alert function) |
| `INA226_Get_Alert_Status` | Read alert flags (limit exceeded / conversion ready / both / none) |
| `INA226_Set_Alert_Latch` | Enable or disable alert latch mode |

---

## Error Handling

Every function returns an `INA226_Status_t` status code:

| Status | Value | Meaning |
|---|---|---|
| `INA226_OK` | 0 | Operation completed successfully |
| `INA226_ERR_I2C` | 1 | I2C communication failure |
| `INA226_ERR_INVALID_PARAM` | 2 | NULL pointer or out-of-range parameter |
| `INA226_ERR_MATH_OVERFLOW` | 3 | Computed value exceeds representable integer range |
| `INA226_ERR_INVALID_STATE` | 4 | Operation not valid in current device state (e.g., setting alert limit without a limit-based alert configured) |

---

## Testing

The driver includes a comprehensive host-side unit test suite using the [Unity](https://github.com/ThrowTheSwitch/Unity) test framework. Tests run on the development machine (no target hardware needed) with a mock I2C backend that simulates the INA226 register map.

```bash
cd tests/host
make
```

**Test Coverage:**
- Parameter validation (NULL pointers, out-of-range values)
- Register-level bitfield correctness (read-modify-write operations)
- Measurement calculations (positive, negative, zero, boundary values)
- Math overflow detection (INT32 / UINT32 boundary conditions)
- Alert system (function set/get, limit conversion for all categories, status flags)
- Exhaustive invalid input sweeps (e.g., all 65535 possible alert function values tested)

---

## Project Structure

```
INA226/
├── inc/
│   └── ina226.h              # Public API header (types, macros, function prototypes)
├── src/
│   └── ina226.c              # Driver implementation
├── tests/
│   ├── host/
│   │   ├── test_mock_ina226.c # Host-side unit tests with mock I2C
│   │   └── Makefile           # Build and run tests
│   ├── test_ina226.h          # Shared test macros/constants
│   └── unity/                 # Unity test framework (git submodule)
├── LICENSE                    # MIT License
└── README.md
```

---

## License

Distributed under the **MIT License**. See [LICENSE](LICENSE) for details.

Copyright © 2026 Cafer Tuğra Çetin
