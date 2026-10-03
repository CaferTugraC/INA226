# INA226 Driver

Portable, bare-metal C99 driver for the Texas Instruments **INA226** 36 V, 16-bit, I²C current, voltage and power monitor.

<a href="https://github.com/CaferTugraC/INA226/actions/workflows/ci.yml"><img src="https://github.com/CaferTugraC/INA226/actions/workflows/ci.yml/badge.svg?branch=dev" alt="CI"></a>
<a href="https://cafertugrac.github.io/INA226/"><img src="https://img.shields.io/badge/docs-online-blue.svg" alt="Documentation"></a>
<a href="https://github.com/CaferTugraC/INA226/blob/main/LICENSE"><img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="License: MIT"></a>
<img src="https://img.shields.io/badge/C-99-informational.svg" alt="C99">

**API documentation:** <https://cafertugrac.github.io/INA226/>

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Resource Usage](#resource-usage)
- [Getting Started](#getting-started)
- [Porting Layer](#porting-layer)
- [Configuration](#configuration)
- [Usage Examples](#usage-examples)
- [Usage Notes](#usage-notes)
- [API Overview](#api-overview)
- [Error Codes](#error-codes)
- [Testing](#testing)
- [Repository Layout](#repository-layout)
- [Versioning](#versioning)
- [License](#license)
- [References](#references)

---

## Overview

The driver is one C source file and two headers. It talks to the INA226 only through two I²C functions that you provide, so it runs on any MCU, with or without an RTOS. All maths is done with fixed-point integers and results are returned in micro-units (µV, µA, µW).

```
Your application ──▶ ina226.h API ──▶ ina226.c ──▶ your 2 I²C hooks ──▶ INA226
```

## Features

| Category | Capability |
|---|---|
| Measurement | Bus voltage (µV), shunt voltage (µV), current (µA), power (µW) |
| Calibration | Calibration register computed from shunt resistance (µΩ) and current LSB (µA) |
| Configuration | Conversion time 140 µs – 8.244 ms (bus and shunt separately), averaging 1 – 1024 samples, operating mode (shutdown / triggered / continuous) |
| Alerts | Shunt and bus over/under limit, power over limit, conversion ready, combined limit + conversion ready, latched or transparent output, status readback |
| Identification | Manufacturer ID and die ID readout |
| Portability | Two user-supplied I²C hooks; byte-order independent; builds warning-free on GCC, Clang, ARM and AVR |
| Safety | NULL checks and configuration/argument range checks on public functions; overflow checks wherever a result can overflow |
| Footprint | No heap, no global or static variables, no floating point, no loops or recursion |

## Resource Usage

Measured with `-Os` by the [CI workflow](https://github.com/CaferTugraC/INA226/actions/workflows/ci.yml), which reports these values on every run.

| | ARM Cortex-M0 (`arm-none-eabi-gcc` 13.2) | AVR ATmega328P (`avr-gcc` 7.3) |
|---|---|---|
| Code size (`.text`) | 1212 B | 2444 B |
| Largest stack frame | 32 B | 28 B |
| Worst-case stack depth¹ | 72 B | — |
| RAM per device (`ina226_handle_t`) | 12 B | 9 B |
| Global / static data | 0 B | 0 B |

¹ `INA226_Set_Alert_Limit` → `INA226_Get_Alert_Pin_Function` → `INA226_Read_Reg`, computed from the GCC call graph. It does not include your I²C hook.

Dependencies: `<stdint.h>` and `<stddef.h>` only.

---

## Getting Started

### Requirements

- A C99 (or newer) compiler: GCC, Clang, IAR, Keil/ARMCC, XC8/16/32, …
- An I²C master peripheral and a working I²C driver on your target.

### Adding the driver to your project

1. Get the sources:
   ```bash
   git clone https://github.com/CaferTugraC/INA226.git
   ```
2. Copy these three files from `src/` into your project:

   | File | Purpose |
   |---|---|
   | `ina226.h` | Public API, the only header your code includes |
   | `ina226.c` | Driver implementation |
   | `ina226_regs.h` | Internal register definitions, needed to compile `ina226.c` |

3. Put the folder containing the headers on your include path and add `ina226.c` to your build.
4. Implement the two functions of the [porting layer](#porting-layer).

<details>
<summary>CMake</summary>

```cmake
add_library(ina226 STATIC path/to/INA226/src/ina226.c)
target_include_directories(ina226 PUBLIC path/to/INA226/src)
target_link_libraries(your_app PRIVATE ina226)
```
</details>

---

## Porting Layer

The driver declares two functions and does not define them. Your application must provide both:

```c
uint8_t INA226_Platform_I2C_Write(uint8_t dev_addr, uint8_t reg_addr,
                                  const uint8_t *data, uint16_t len);
uint8_t INA226_Platform_I2C_Read (uint8_t dev_addr, uint8_t reg_addr,
                                  uint8_t *data, uint16_t len);
```

| Parameter | Meaning |
|---|---|
| `dev_addr` | **7-bit** device address (`0x40`–`0x4F`), taken from the handle |
| `reg_addr` | 8-bit register address |
| `data` | Register value, MSB first (the driver handles byte order) |
| `len` | Always `2` |
| return | `0` on success, non-zero on any bus error |

A write is `START, addr+W, reg_addr, data…, STOP`. A read is `START, addr+W, reg_addr, RESTART, addr+R, data…, STOP`. Most HALs call these "memory write" and "memory read".

<details>
<summary>STM32 HAL</summary>

```c
extern I2C_HandleTypeDef hi2c1;

uint8_t INA226_Platform_I2C_Write(uint8_t dev_addr, uint8_t reg_addr,
                                  const uint8_t *data, uint16_t len)
{
    return (HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(dev_addr << 1), reg_addr,
                              I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, len, 10) == HAL_OK) ? 0U : 1U;
}

uint8_t INA226_Platform_I2C_Read(uint8_t dev_addr, uint8_t reg_addr,
                                 uint8_t *data, uint16_t len)
{
    return (HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(dev_addr << 1), reg_addr,
                             I2C_MEMADD_SIZE_8BIT, data, len, 10) == HAL_OK) ? 0U : 1U;
}
```
</details>

<details>
<summary>Arduino (Wire)</summary>

```cpp
#include <Wire.h>

extern "C" {
#include "ina226.h"

uint8_t INA226_Platform_I2C_Write(uint8_t dev_addr, uint8_t reg_addr,
                                  const uint8_t *data, uint16_t len)
{
    Wire.beginTransmission(dev_addr);
    Wire.write(reg_addr);
    Wire.write(data, len);
    return Wire.endTransmission();   /* 0 = success */
}

uint8_t INA226_Platform_I2C_Read(uint8_t dev_addr, uint8_t reg_addr,
                                 uint8_t *data, uint16_t len)
{
    Wire.beginTransmission(dev_addr);
    Wire.write(reg_addr);
    if (Wire.endTransmission(false) != 0) return 1;
    if (Wire.requestFrom(dev_addr, (uint8_t)len) != len) return 1;
    for (uint16_t i = 0; i < len; i++) data[i] = (uint8_t)Wire.read();
    return 0;
}
}
```
</details>

---

## Configuration

### Device handle

Each physical INA226 needs one `ina226_handle_t`. It is owned by your code, and the driver only reads it.

| Field | Unit | Valid range | Example |
|---|---|---|---|
| `ina226_i2c_addr` | — | `0x00` – `0xFF` (typically `0x40` – `0x4F`, 7-bit) | `0x40` |
| `shunt_resistor_uOhm` | µΩ | `1` – `4 294 967 295` | `100000` (0.1 Ω, "R100") |
| `current_resolution_uA` | µA | `1` – `65 536`, see [limits](#numeric-limits) | `100` |

> [!NOTE]
> The driver passes `ina226_i2c_addr` directly to the platform I²C hooks without validation. This allows hardware configurations with I²C address translators (e.g., LTC4316) or platforms whose HAL expects pre-shifted addresses.

### I²C address

Native INA226 address set by the A1 and A0 pins:

| A1 \ A0 | GND | VS | SDA | SCL |
|---|---|---|---|---|
| **GND** | `0x40` | `0x41` | `0x42` | `0x43` |
| **VS** | `0x44` | `0x45` | `0x46` | `0x47` |
| **SDA** | `0x48` | `0x49` | `0x4A` | `0x4B` |
| **SCL** | `0x4C` | `0x4D` | `0x4E` | `0x4F` |

### Choosing the current LSB

```
current_resolution_uA  ≥  I_max[µA] / 32768        (round up to a convenient value)
CAL                    =  5 120 000 000 / (current_resolution_uA × shunt_resistor_uOhm)
```

`INA226_Calibrate()` computes `CAL` with round-to-nearest and returns `INA226_ERR_INVALID_PARAM` if it does not fit in 1 … 32 767. The shunt input saturates at ±81.92 mV, so `I_max ≤ 81.92 mV / R_shunt`.

| Shunt | I_max | Current LSB | CAL |
|---|---|---|---|
| 100 mΩ | 0.8 A | 25 µA | 2048 |
| 100 mΩ | 0.8 A | 100 µA | 512 |
| 10 mΩ | 8 A | 250 µA | 2048 |
| 2 mΩ | 20 A | 1000 µA | 2560 |

---

## Usage Examples

### Basic measurement

```c
#include "ina226.h"

static const ina226_handle_t ina = {
    .ina226_i2c_addr       = 0x40,
    .shunt_resistor_uOhm   = 100000,   /* 0.1 Ω shunt */
    .current_resolution_uA = 100,      /* 100 µA per LSB */
};

int sensor_init(void)
{
    uint16_t id = 0;

    /* Check that an INA226 answers at this address. */
    if (INA226_Read_Manufacturer_ID(&ina, &id) != INA226_OK || id != 0x5449U) {
        return -1;
    }

    if (INA226_Reset(&ina)                                                       != INA226_OK ||
        INA226_Set_Averaging_Mode(&ina, INA226_AVG_64)                           != INA226_OK ||
        INA226_Set_Bus_Voltage_Conversion_Time(&ina, INA226_CT_1100_US)          != INA226_OK ||
        INA226_Set_Shunt_Voltage_Conversion_Time(&ina, INA226_CT_1100_US)        != INA226_OK ||
        INA226_Calibrate(&ina)                                                   != INA226_OK ||
        INA226_Set_Operating_Mode(&ina, INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE) != INA226_OK) {
        return -1;
    }
    return 0;
}

void sensor_poll(void)
{
    uint32_t bus_uV;
    int32_t  shunt_uV;
    int32_t  current_uA;
    uint32_t power_uW;

    if (INA226_Read_Bus_Voltage(&ina, &bus_uV)     == INA226_OK &&
        INA226_Read_Shunt_Voltage(&ina, &shunt_uV) == INA226_OK &&
        INA226_Read_Current(&ina, &current_uA)     == INA226_OK &&
        INA226_Read_Power(&ina, &power_uW)         == INA226_OK) {
        /* use the values */
    }
}
```

### Over-voltage alert (latched)

```c
/* Select the alert function first: the unit of the limit depends on it. */
INA226_Set_Alert_Pin_Function(&ina, INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT);
INA226_Set_Alert_Limit(&ina, 5000000);                  /* 5.000 V, in µV */
INA226_Set_Alert_Latch(&ina, INA226_ALERT_LATCH_ENABLE);

/* Later, e.g. after the ALERT pin interrupt: */
INA226_Alert_Status_t status;
if (INA226_Get_Alert_Status(&ina, &status) == INA226_OK &&
    (status == INA226_ALERT_STATUS_LIMIT_EXCEEDED || status == INA226_ALERT_STATUS_BOTH)) {
    /* handle over-voltage; this read has also cleared the latched alert */
}
```

### Single-shot (triggered) measurement

```c
INA226_Set_Alert_Pin_Function(&ina, INA226_ALERT_FUNC_CONVERSION_READY);
INA226_Set_Operating_Mode(&ina, INA226_TRIGGERED_BUS_AND_SHUNT_VOLTAGE);   /* starts one conversion */

INA226_Alert_Status_t status = INA226_ALERT_STATUS_NONE;
while (status != INA226_ALERT_STATUS_CONVERSION_READY && status != INA226_ALERT_STATUS_BOTH) {
    INA226_Get_Alert_Status(&ina, &status);   /* add a timeout in production code */
}
/* Read the results, then write the triggered mode again for the next sample. */
```

---

## Usage Notes

- **Order matters for alerts.** Call `INA226_Set_Alert_Pin_Function()` before `INA226_Set_Alert_Limit()`. The limit is converted using the active function (µV for shunt and bus, µW for power). If no limit-based function is active, the call returns `INA226_ERR_INVALID_STATE`.
- **One alert function at a time.** `INA226_Set_Alert_Pin_Function()` replaces the previous selection.
- **Reading the alert status has a side effect.** `INA226_Get_Alert_Status()` reads the Mask/Enable register. This clears the conversion-ready flag and, in latch mode, the latched alert.
- **Calibrate after reset.** `INA226_Reset()` clears the calibration register. Current and power read 0 until `INA226_Calibrate()` runs again.
- **Thread safety.** The driver keeps no internal state, so calls on *different* devices are reentrant. Calls on the *same* device are not atomic, because the configuration and alert setters use read-modify-write. If several threads or ISRs access one device, protect it with a mutex or a critical section.
- **Outputs on error.** If a function fails, it leaves its output argument unchanged.

### Numeric limits

| Quantity | Device LSB | Limit |
|---|---|---|
| Bus voltage | 1.25 mV | 0 – 40.96 V register range (device rated 0 – 36 V) |
| Shunt voltage | 2.5 µV | ±81.92 mV |
| Current | `current_resolution_uA` | `INA226_ERR_MATH_OVERFLOW` if the LSB is above 65 536 µA |
| Power | 25 × current LSB | `INA226_ERR_MATH_OVERFLOW` if the LSB is above 2621 µA |

---

## API Overview

Full parameter and return-value documentation: **<https://cafertugrac.github.io/INA226/>**

| Group | Function | Description |
|---|---|---|
| Device | `INA226_Reset` | Software reset to power-on defaults |
| | `INA226_Read_Manufacturer_ID` | Read the manufacturer ID (`0x5449`) |
| | `INA226_Read_Die_ID` | Read the die ID (`0x2260`) |
| Configuration | `INA226_Set_Averaging_Mode` | Number of averaged samples (`INA226_AVG_*`) |
| | `INA226_Set_Bus_Voltage_Conversion_Time` | Bus ADC conversion time (`INA226_CT_*`) |
| | `INA226_Set_Shunt_Voltage_Conversion_Time` | Shunt ADC conversion time (`INA226_CT_*`) |
| | `INA226_Set_Operating_Mode` | Shutdown / triggered / continuous |
| | `INA226_Calibrate` | Write the calibration register from the handle |
| Measurement | `INA226_Read_Bus_Voltage` | Bus voltage in µV |
| | `INA226_Read_Shunt_Voltage` | Shunt voltage in µV (signed) |
| | `INA226_Read_Current` | Current in µA (signed) |
| | `INA226_Read_Power` | Power in µW |
| Alerts | `INA226_Set_Alert_Pin_Function` | Select the alert source (`INA226_ALERT_FUNC_*`) |
| | `INA226_Get_Alert_Pin_Function` | Read back the selected source |
| | `INA226_Set_Alert_Limit` | Set the threshold in µV or µW |
| | `INA226_Set_Alert_Latch` | Latched or transparent ALERT output |
| | `INA226_Get_Alert_Status` | Read the alert and conversion-ready flags |

## Error Codes

Every function returns an `INA226_Status_t`:

| Code | Value | Returned when |
|---|---|---|
| `INA226_OK` | 0 | Success |
| `INA226_ERR_I2C` | 1 | An I²C hook returned non-zero |
| `INA226_ERR_INVALID_PARAM` | 2 | NULL pointer, option out of range, a zero shunt or LSB, a calibration value outside 1 … 32 767, or a negative bus voltage or power alert limit |
| `INA226_ERR_MATH_OVERFLOW` | 3 | A converted value does not fit its destination |
| `INA226_ERR_INVALID_STATE` | 4 | `INA226_Set_Alert_Limit` called while no limit-based alert function is selected |

---

## Testing

Host unit tests use [Unity](https://github.com/ThrowTheSwitch/Unity) and a mocked INA226 register map, so no hardware is needed:

```bash
git clone --recurse-submodules https://github.com/CaferTugraC/INA226.git
cd INA226
make -C tests/host
```

In an existing clone without submodules, run `git submodule update --init --recursive` first.

On every push and pull request, [CI](https://github.com/CaferTugraC/INA226/actions/workflows/ci.yml) runs the unit tests, builds the driver with `-Werror` on GCC and Clang, and cross-compiles it for ARM Cortex-M0 and AVR.

### Building the documentation locally

```bash
git submodule update --init docs/doxygen-awesome-css
doxygen Doxyfile          # requires Doxygen 1.11 or newer
# open docs/html/index.html
```

## Repository Layout

```
INA226/
├── src/
│   ├── ina226.h            # Public API
│   ├── ina226.c            # Driver implementation
│   └── ina226_regs.h       # Internal register definitions
├── tests/
│   ├── host/               # Host unit tests (Unity) and Makefile
│   ├── test_target_ina226.c  # On-target tests (planned)
│   └── unity/              # Unity test framework (submodule)
├── docs/
│   └── doxygen-awesome-css/  # Documentation theme (submodule)
├── .github/workflows/      # CI and documentation workflows
├── Doxyfile
├── CHANGELOG.md
├── LICENSE
└── README.md
```

## Versioning

The project follows [Semantic Versioning](https://semver.org). While the major version is 0, a minor release may contain breaking changes. Every change is listed in [CHANGELOG.md](CHANGELOG.md). The version is also available in code as `INA226_VERSION` and `INA226_VERSION_STRING`.

## License

Released under the [MIT License](https://github.com/CaferTugraC/INA226/blob/main/LICENSE).
Copyright © 2026 Cafer Tura Çetin

## References

- [INA226 datasheet (TI)](https://www.ti.com/lit/ds/symlink/ina226.pdf)
- [INA226 product page](https://www.ti.com/product/INA226)
