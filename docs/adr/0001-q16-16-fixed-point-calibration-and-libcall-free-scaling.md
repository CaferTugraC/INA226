# ADR-0001: Q16.16 Fixed-Point Calibration and Libcall-Free Scaling

## Status
Accepted

## Context
The root cause of Issue #62 is that `INA226_Calibrate()` computes the integer `CAL` register value from the user-requested `current_lsb` using round-to-nearest arithmetic. While the INA226 hardware internally scales measurements using the effective `Current_LSB` corresponding to that rounded integer `CAL` register value, the driver previously scaled readings and alert limits using the originally requested `current_lsb`. This mismatch introduced systematic arithmetic scaling error into current readings, power readings, and power alert limits.

The driver targets 8-bit and 32-bit microcontrollers without a hardware FPU and must remain freestanding, deterministic, free of floating-point and external library dependencies, and compact in Flash footprint. Frequently called measurement functions (hot path) must execute quickly without division, while infrequently called calibration and alert-limit configuration functions must achieve minimal scaling error without pulling in 64-bit software division libcalls.

## Decision
Current readings, power readings, and power alert limits are now scaled using the effective `actual_current_lsb` back-calculated from the rounded 15-bit `CAL` register value written to the device, rather than the user's requested `current_lsb`.

- **Q16.16 fixed-point representation (`uint32_t`, µA):** The back-calculated `actual_current_lsb` is stored in the handle as `current_lsb_q16_16_uA` in unsigned Q16.16 format (`µA × 2^16`). Q16.16 was chosen over floating-point (soft-float libcall overhead on FPU-less targets) and wider formats such as `Q32.32` (unnecessary RAM and 64-bit arithmetic cost) because its integer range (`0 .. 65535 µA`) maps directly to the `int32_t` current output range (`[-2147.48 A .. +2147.42 A]`), provides `15.26 pA` fractional resolution, and reduces hot-path scaling to a single multiply and bit shift (`>> 16`).
- **Dynamic `Power_LSB` derivation:** No separate `power_lsb` field is stored in the handle. Per the datasheet relation `Power_LSB = 25 × Current_LSB`, `Power_LSB` is derived dynamically in 64-bit integer arithmetic at runtime inside both `INA226_Read_Power()` and `INA226_Set_Alert_Limit()` (Power Over-Limit).
- **Elimination of 64-bit division libcalls:** To avoid compiler runtime library dependencies on targets lacking hardware 64-bit division, `INA226_Calibrate()` uses remainder-carrying 32-bit sequential division with a 17-step shift-and-subtract fractional loop, and `INA226_Set_Alert_Limit()` (Power Over-Limit) uses a 16-step restoring division loop.

## Consequences

### Scaling Error Reduction
- **Previous approach:** Because `CAL` is rounded to an integer ($|\text{CAL}_{\text{exact}} - \text{CAL}| \le 0.5$), the device's effective LSB deviated from the requested LSB by $\text{LSB}_{\text{actual}} = \text{LSB}_{\text{req}} \cdot \frac{\text{CAL}_{\text{exact}}}{\text{CAL}}$. The resulting relative reading error in reported current and power with respect to the true physical value was:

  $$\varepsilon_{\text{old}} = \frac{|I_{\text{old}} - I_{\text{true}}|}{I_{\text{true}}} = \frac{|\text{CAL}_{\text{exact}} - \text{CAL}|}{\text{CAL}_{\text{exact}}}$$

  *(Relative deviation with respect to the requested LSB was $\frac{|\text{CAL}_{\text{exact}} - \text{CAL}|}{\text{CAL}} \le \frac{0.5}{\text{CAL}}$.)*

  For small `CAL` values this error became significant: for example, with $R_{\text{shunt}} = 0.1\,\Omega$ and $\text{current\_lsb} = 1000\,\mu\text{A}$ ($\text{CAL}_{\text{exact}} = 51.2 \to \text{CAL} = 51$), the true current reading error was **0.3906% (`3906.25 ppm`)**. At the worst-case boundary ($\text{CAL} = 1$), the error approached **33.33%** as $\text{CAL}_{\text{exact}} \to 1.5^-$ (rounding down) and **100%** (nearly 2× the true current) as $\text{CAL}_{\text{exact}} \to 0.5^+$ (rounding up).

- **New approach (Q16.16):** The two-step back-calculation keeps absolute quantization error strictly below $1\,\text{LSB}_{\text{Q16.16}} = 2^{-16}\,\mu\text{A} \approx 15.26\,\text{pA}$ (and tighter still for `CAL > 1`). The relative scaling error becomes:

  $$\varepsilon_{\text{new}} = \frac{|I_{\text{new}} - I_{\text{true}}|}{I_{\text{true}}} < \frac{2^{-16}\,\mu\text{A}}{\text{LSB}_{\text{actual}}\;[\mu\text{A}]}$$

  For any $\text{LSB}_{\text{actual}} \ge 1\,\mu\text{A}$, relative current and power scaling error stays strictly below **0.00153% (`15.26 ppm`)**. At the same $0.1\,\Omega$ / $1000\,\mu\text{A}$ operating point, current reading error drops from **0.3906% to 0.000000119% (`0.00119 ppm`)** (a ~3.28 million× reduction; even under the worst-case `1 LSB` bound it is $\le 0.0152\,\text{ppm}$, >250,000× lower). In practice, the only remaining driver-side error is the final integer output rounding of $\pm 0.5\,\mu\text{A}$ / $\pm 0.5\,\mu\text{W}$.

### Computational Overhead, Libcall Dependencies, and Resource Impact
- **Read paths (hot path):** `INA226_Read_Current()` and `INA226_Read_Power()` perform no division—only a 64-bit multiply and a 16-bit shift (`>> 16`).
- **Calibration and power alert limit (cold path):** `INA226_Calibrate()` adds a bounded 17-iteration loop and `INA226_Set_Alert_Limit()` (Power Over-Limit) adds a bounded 16-iteration loop, keeping execution deterministic.
- **External library dependencies and Flash footprint:** Dependency on 64-bit software division libcalls (`__aeabi_uldivmod`, `__aeabi_ldivmod`, and underlying `libgcc` helpers) is completely eliminated; on targets with hardware 32-bit division, the driver links with zero external library calls. Although the driver object file itself grows slightly due to the bounded loops, removing the heavy 64-bit software division helpers produces a **net reduction in final linked Flash size**.
- **RAM and API impact:** The handle stores only 4 bytes of runtime calibration state (`current_lsb_q16_16_uA`, with no separate `power_lsb` field). Because `INA226_Calibrate()` and `INA226_Reset()` update this field, both now accept a mutable `ina226_handle_t *`.
- **Upper numeric bound:** Because `actual_current_lsb` must fit in unsigned Q16.16 `uint32_t`, configurations where $\text{CAL} \times R_{\text{shunt}} \le 78\,125$ ($\text{LSB}_{\text{actual}} \ge 65\,536\,\mu\text{A}$) are rejected with `INA226_ERR_MATH_OVERFLOW`. Conversely, deriving `Power_LSB` dynamically removes the previous ~2621 µA calibration ceiling.

This decision is implemented as part of the calibration API redesign PR targeting `dev`.
