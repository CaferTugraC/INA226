# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
While the major version is `0`, a minor release may contain breaking changes; they are always listed under **Changed** and marked **BREAKING**.

## [0.3.0] - Unreleased

### Added
- Version macros `INA226_VERSION_MAJOR`, `INA226_VERSION_MINOR`, `INA226_VERSION_PATCH`, `INA226_VERSION` and `INA226_VERSION_STRING` in `ina226.h`.
- This changelog.
- GitHub Actions CI: host unit tests, `-Werror` builds with GCC and Clang, and cross builds for ARM Cortex-M0 and AVR ATmega328P with code size and stack usage reports.
- Unit tests for `INA226_Calibrate` ([#39]).
- `test` target, Unity submodule check and automatic `build/` directory creation in `tests/host/Makefile` ([#40]).
- On-target test skeleton `tests/test_target_ina226.c` ([#41]).
- Shorter alert function names ending in `_CVR` (for example `INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT_CVR`) ([#41]).
- Doxygen documentation for every public macro, the `ina226_handle_t` fields and the internal register definitions ([#43]).

### Changed
- **BREAKING (ABI):** `ina226_handle_t::shunt_resistor_uOhm` is now `uint32_t` instead of `uint16_t`, so the handle grows from 8 to 12 bytes. Rebuild all code that uses the handle ([#39]).
- **BREAKING (layout):** The public header moved from `inc/ina226.h` to `src/ina226.h`, and the driver now includes it as `"ina226.h"`. Update your include paths ([#38]).
- Internal register definitions moved to `src/ina226_regs.h`. This file must be added to your project together with `ina226.c` ([#41]).
- `INA226_Calibrate` now takes a `const ina226_handle_t *` ([#41]).
- Host tests are built with `-std=c99 -pedantic` ([#40]).
- README rewritten as a getting-started guide: installation with all three source files, porting examples, handle and address reference, measured code size and stack usage, usage notes and an API overview that links to the Doxygen site. Detailed API reference moved to the documentation site.

### Deprecated
- The `INA226_ALERT_FUNC_*_CON_READY_CVR` names. They remain as aliases of the new `_CVR` names and will be removed in a future release ([#41]).

### Fixed
- Shunt resistances above 65 535 µΩ, such as the common 0.1 Ω (R100) shunt, were silently truncated and gave a wrong calibration value ([#39]).
- The exhaustive alert function test now also covers `0xFFFF` ([#40]).
- `-Wconversion` warnings in `INA226_Set_Alert_Pin_Function` and `INA226_Set_Alert_Limit` ([#41]).
- Doxygen comments referring to the non-existent `cal_reg_value` parameter, and copy-paste errors in the ID function comments ([#43]).
- Author name spelling in file headers and the README ([#44]).
- `INA226_Read_Reg` could overflow a 16-bit `int` when shifting the register MSB (undefined behaviour on AVR and other 16-bit targets).

### Removed
- The `inc/` directory ([#38]).

## [0.2.0] - 2026-08-10

First versioned state of the driver.

### Added
- Bus voltage, shunt voltage, current and power readings in µV, µA and µW, using integer arithmetic only.
- Calibration register computation from shunt resistance and current LSB.
- Configuration of conversion times, averaging and operating mode.
- Alert system: alert function selection and readback, alert limit in physical units, alert status, latch mode ([#29]).
- `INA226_Read_Manufacturer_ID` and `INA226_Read_Die_ID` ([#32]).
- Host unit test suite based on Unity with a mocked register map.
- README ([#31]).

### Fixed
- Doxygen parameter names ([#36]).

[#29]: https://github.com/CaferTugraC/INA226/pull/29
[#31]: https://github.com/CaferTugraC/INA226/pull/31
[#32]: https://github.com/CaferTugraC/INA226/pull/32
[#36]: https://github.com/CaferTugraC/INA226/pull/36
[#38]: https://github.com/CaferTugraC/INA226/pull/38
[#39]: https://github.com/CaferTugraC/INA226/pull/39
[#40]: https://github.com/CaferTugraC/INA226/pull/40
[#41]: https://github.com/CaferTugraC/INA226/pull/41
[#43]: https://github.com/CaferTugraC/INA226/pull/43
[#44]: https://github.com/CaferTugraC/INA226/pull/44
