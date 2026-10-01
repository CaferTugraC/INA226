/**
 * @file ina226.h
 * @author Cafer Tura Çetin (cafercetin.tr@gmail.com)
 * @brief INA226 Bi-directional Current and Power Monitor driver header file.
 * @version 0.3
 * @date 2026-07-25
 * 
 * @copyright Copyright (c) 2026 Cafer Tura Çetin
 * SPDX-License-Identifier: MIT
 * 
 * Distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef INA226_H_
#define INA226_H_

#include <stdint.h>

/* ========================================================================= */
/*                                  VERSION                                  */
/* ========================================================================= */

#define INA226_VERSION_MAJOR        (0U) /**< Driver major version (incompatible API/ABI changes). */
#define INA226_VERSION_MINOR        (3U) /**< Driver minor version (backward compatible additions). */
#define INA226_VERSION_PATCH        (0U) /**< Driver patch version (backward compatible fixes). */

/**
 * @brief Driver version as a single comparable number: 0x00MMmmpp (major, minor, patch).
 *
 *        Example: "#if INA226_VERSION >= 0x000300UL" checks for version 0.3.0 or newer.
 *        Uses unsigned long arithmetic so the value is correct on 16-bit int targets (e.g. AVR)
 *        and inside preprocessor conditionals.
 */
#define INA226_VERSION              ((INA226_VERSION_MAJOR * 65536UL) + (INA226_VERSION_MINOR * 256UL) + INA226_VERSION_PATCH)

/**
 * @brief Driver version as a string literal ("MAJOR.MINOR.PATCH").
 */
#define INA226_VERSION_STRING       "0.3.0"

/* ========================================================================= */
/*                       PLATFORM I2C HOOK FUNCTIONS                         */
/* ========================================================================= */

/**
 * @brief Platform I2C write function to be implemented by the user.
 * 
 * @param dev_addr Destination INA226 7-bit device address.
 * @param reg_addr Destination register address of INA226.
 * @param data     Pointer to data bytes to be written.
 * @param len      Number of bytes to write.
 * @return uint8_t 0 = success, non-zero = communication error.
 */
extern uint8_t INA226_Platform_I2C_Write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);

/**
 * @brief Platform I2C read function to be implemented by the user.
 * 
 * @param dev_addr Destination INA226 7-bit device address.
 * @param reg_addr Destination register address of INA226.
 * @param data     Pointer to buffer where read data will be stored.
 * @param len      Number of bytes to read.
 * @return uint8_t 0 = success, non-zero = communication error.
 */
extern uint8_t INA226_Platform_I2C_Read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

/* ========================================================================= */
/*                              TYPES & DEFINES                              */
/* ========================================================================= */

/**
 * @brief INA226 configuration option type definition.
 * 
 */
typedef uint8_t INA226_Config_Option_t;

/**
 * @brief Return value status codes of the driver functions.
 * 
 */
typedef INA226_Config_Option_t INA226_Status_t;
#define INA226_OK                   ((INA226_Status_t)0U) /**< Operation completed successfully. */
#define INA226_ERR_I2C              ((INA226_Status_t)1U) /**< I2C communication failure. */
#define INA226_ERR_INVALID_PARAM    ((INA226_Status_t)2U) /**< NULL pointer or invalid parameter passed. */
#define INA226_ERR_MATH_OVERFLOW    ((INA226_Status_t)3U) /**< Calculated value exceeds representable range. */
#define INA226_ERR_INVALID_STATE    ((INA226_Status_t)4U) /**< Operation invalid in current configuration. */

/**
 * @brief INA226 Alert pin status macros.
 * 
 */
typedef INA226_Config_Option_t INA226_Alert_Status_t;
#define INA226_ALERT_STATUS_NONE                    ((INA226_Alert_Status_t)0U) /**< No alert asserted. */
#define INA226_ALERT_STATUS_LIMIT_EXCEEDED          ((INA226_Alert_Status_t)1U) /**< Configured alert limit exceeded (AFF). */
#define INA226_ALERT_STATUS_CONVERSION_READY        ((INA226_Alert_Status_t)2U) /**< Conversion ready flag asserted (CVRF). */
#define INA226_ALERT_STATUS_BOTH                    ((INA226_Alert_Status_t)3U) /**< Both limit exceeded and conversion ready asserted. */

/**
 * @brief INA226 Conversion time option macros.
 * 
 */
typedef INA226_Config_Option_t INA226_Conv_Time_t;
#define INA226_CT_140_US            ((INA226_Conv_Time_t)0x00U) /**< 140 µs conversion time. */
#define INA226_CT_204_US            ((INA226_Conv_Time_t)0x01U) /**< 204 µs conversion time. */
#define INA226_CT_332_US            ((INA226_Conv_Time_t)0x02U) /**< 332 µs conversion time. */
#define INA226_CT_588_US            ((INA226_Conv_Time_t)0x03U) /**< 588 µs conversion time. */
#define INA226_CT_1100_US           ((INA226_Conv_Time_t)0x04U) /**< 1.1 ms conversion time (default). */
#define INA226_CT_2116_US           ((INA226_Conv_Time_t)0x05U) /**< 2.116 ms conversion time. */
#define INA226_CT_4156_US           ((INA226_Conv_Time_t)0x06U) /**< 4.156 ms conversion time. */
#define INA226_CT_8244_US           ((INA226_Conv_Time_t)0x07U) /**< 8.244 ms conversion time. */

/**
 * @brief INA226 Averaging time option macros.
 * 
 */
typedef INA226_Config_Option_t INA226_Avg_Time_t;
#define INA226_AVG_1                ((INA226_Avg_Time_t)0x00U) /**< 1 sample (default, no averaging). */
#define INA226_AVG_4                ((INA226_Avg_Time_t)0x01U) /**< 4 samples averaged. */
#define INA226_AVG_16               ((INA226_Avg_Time_t)0x02U) /**< 16 samples averaged. */
#define INA226_AVG_64               ((INA226_Avg_Time_t)0x03U) /**< 64 samples averaged. */
#define INA226_AVG_128              ((INA226_Avg_Time_t)0x04U) /**< 128 samples averaged. */
#define INA226_AVG_256              ((INA226_Avg_Time_t)0x05U) /**< 256 samples averaged. */
#define INA226_AVG_512              ((INA226_Avg_Time_t)0x06U) /**< 512 samples averaged. */
#define INA226_AVG_1024             ((INA226_Avg_Time_t)0x07U) /**< 1024 samples averaged. */

/**
 * @brief INA226 Operation mode option macros.
 * 
 */
typedef INA226_Config_Option_t INA226_Mode_t;
#define INA226_SHUT_DOWN                        ((INA226_Mode_t)0x00U) /**< Power-down / Shutdown mode. */
#define INA226_TRIGGERED_SHUNT_VOLTAGE          ((INA226_Mode_t)0x01U) /**< Shunt voltage, single-shot (triggered). */
#define INA226_TRIGGERED_BUS_VOLTAGE            ((INA226_Mode_t)0x02U) /**< Bus voltage, single-shot (triggered). */
#define INA226_TRIGGERED_BUS_AND_SHUNT_VOLTAGE  ((INA226_Mode_t)0x03U) /**< Shunt and bus voltage, single-shot (triggered). */
#define INA226_SHUT_DOWN_ALT                    ((INA226_Mode_t)0x04U) /**< Alternative power-down mode (shutdown). */
#define INA226_CONTINUOUS_SHUNT_VOLTAGE         ((INA226_Mode_t)0x05U) /**< Shunt voltage, continuous measurement. */
#define INA226_CONTINUOUS_BUS_VOLTAGE           ((INA226_Mode_t)0x06U) /**< Bus voltage, continuous measurement. */
#define INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE ((INA226_Mode_t)0x07U) /**< Shunt and bus voltage, continuous (default). */

/**
 * @brief INA226 Alert function option macros.
 * 
 */
typedef uint16_t INA226_Alert_Func_t;
#define INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT                  ((INA226_Alert_Func_t)0x20U) /**< Shunt Voltage Over-Limit (SOL). */
#define INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT                 ((INA226_Alert_Func_t)0x10U) /**< Shunt Voltage Under-Limit (SUL). */
#define INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT                    ((INA226_Alert_Func_t)0x08U) /**< Bus Voltage Over-Limit (BOL). */
#define INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT                   ((INA226_Alert_Func_t)0x04U) /**< Bus Voltage Under-Limit (BUL). */
#define INA226_ALERT_FUNC_POWER_OVER_LIMIT                          ((INA226_Alert_Func_t)0x02U) /**< Power Over-Limit (POL). */
#define INA226_ALERT_FUNC_CONVERSION_READY                          ((INA226_Alert_Func_t)0x01U) /**< Conversion Ready (CVRF only). */
#define INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT_CVR              ((INA226_Alert_Func_t)0x21U) /**< Shunt Voltage Over-Limit with Conversion Ready. */
#define INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT_CVR             ((INA226_Alert_Func_t)0x11U) /**< Shunt Voltage Under-Limit with Conversion Ready. */
#define INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT_CVR                ((INA226_Alert_Func_t)0x09U) /**< Bus Voltage Over-Limit with Conversion Ready. */
#define INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT_CVR               ((INA226_Alert_Func_t)0x05U) /**< Bus Voltage Under-Limit with Conversion Ready. */
#define INA226_ALERT_FUNC_POWER_OVER_LIMIT_CVR                      ((INA226_Alert_Func_t)0x03U) /**< Power Over-Limit with Conversion Ready. */

/* Backward compatibility aliases */
#define INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT_CON_READY_CVR   INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT_CVR /**< Backward compatibility alias. */
#define INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT_CON_READY_CVR      INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT_CVR    /**< Backward compatibility alias. */
#define INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT_CON_READY_CVR     INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT_CVR   /**< Backward compatibility alias. */
#define INA226_ALERT_FUNC_POWER_OVER_LIMIT_CON_READY_CVR            INA226_ALERT_FUNC_POWER_OVER_LIMIT_CVR          /**< Backward compatibility alias. */

/**
 * @brief INA226 Alert latch mode option macros.
 * 
 */
typedef INA226_Config_Option_t INA226_Alert_Latch_t;
#define INA226_ALERT_LATCH_TRANSPARENT              ((INA226_Alert_Latch_t)0U) /**< Transparent mode (alert clears when condition clears). */
#define INA226_ALERT_LATCH_ENABLE                   ((INA226_Alert_Latch_t)1U) /**< Latch mode (alert latches until Mask/Enable is read). */

/**
 * @brief INA226 sensor handle structure containing hardware details and calibration parameters.
 * 
 */
typedef struct {
    uint8_t ina226_i2c_addr;          /**< I2C slave address (7-bit, e.g. 0x40). */
    uint32_t shunt_resistor_uOhm;    /**< Shunt resistor resistance in micro-ohms (1 µΩ .. 4.29 kΩ). */
    uint32_t current_resolution_uA;  /**< Current measurement resolution (LSB) in micro-amperes. */
} ina226_handle_t;

/* ========================================================================= */
/*                            FUNCTION PROTOTYPES                            */
/* ========================================================================= */

/**
 * @brief Reset the destination INA226 device to default register states.
 * 
 * @param sensor Destination INA226 device handle.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while writing to the configuration register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor is NULL.
 */
INA226_Status_t INA226_Reset(const ina226_handle_t *sensor);

/**
 * @brief Read the Manufacturer ID from destination INA226 device. 
 * 
 * @param sensor     Destination INA226 device handle.
 * @param out_mfg_id Pointer to store the Manufacturer ID (expected value: 0x5449, "TI").
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading the register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor or out_mfg_id is NULL.
 */
INA226_Status_t INA226_Read_Manufacturer_ID(const ina226_handle_t *sensor, uint16_t *out_mfg_id);

/**
 * @brief Read the Die ID and revision from destination INA226 device.
 * 
 * @param sensor     Destination INA226 device handle.
 * @param out_die_id Pointer to store the Die ID (expected value: 0x2260).
 * @return INA226_Status_t 
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading the register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor or out_die_id is NULL.
 */
INA226_Status_t INA226_Read_Die_ID(const ina226_handle_t *sensor, uint16_t *out_die_id);

/**
 * @brief Set shunt voltage conversion time for destination INA226 device.
 * 
 * @param sensor    Destination INA226 device handle.
 * @param conv_time Selected conversion time option for the device (e.g. INA226_CT_1100_US).
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while updating the configuration register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor is NULL or conv_time is outside valid range.
 */
INA226_Status_t INA226_Set_Shunt_Voltage_Conversion_Time(const ina226_handle_t *sensor, INA226_Conv_Time_t conv_time);

/**
 * @brief Set bus voltage conversion time option for destination INA226 device.
 * 
 * @param sensor    Destination INA226 device handle.
 * @param conv_time Selected conversion time option for the device (e.g. INA226_CT_1100_US).
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while updating the configuration register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor is NULL or conv_time is outside valid range.
 */
INA226_Status_t INA226_Set_Bus_Voltage_Conversion_Time(const ina226_handle_t *sensor, INA226_Conv_Time_t conv_time);

/**
 * @brief Set operating mode for destination INA226 device.
 * 
 * @param sensor Destination INA226 device handle.
 * @param mode   Selected operating mode option (e.g. INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE).
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while updating the configuration register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor is NULL or mode is outside valid range.
 */
INA226_Status_t INA226_Set_Operating_Mode(const ina226_handle_t *sensor, INA226_Mode_t mode);

/**
 * @brief Set sample averaging mode for destination INA226 device.
 * 
 * @param sensor   Destination INA226 device handle.
 * @param avg_time Selected averaging mode option (e.g. INA226_AVG_64).
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while updating the configuration register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor is NULL or avg_time is outside valid range.
 */
INA226_Status_t INA226_Set_Averaging_Mode(const ina226_handle_t *sensor, INA226_Avg_Time_t avg_time);

/**
 * @brief Configure current and power measurement scaling by writing to the calibration register.
 * 
 * @details Calculates the 16-bit calibration register value using Equation 1 from the datasheet:
 *          CAL = 0.00512 / (Current_LSB [A] * Rshunt [Ω])
 *              = 5.12e9 / (current_resolution_uA * shunt_resistor_uOhm)
 *          with round-to-nearest integer arithmetic.
 * 
 * @note The physical calibration register is an integer. Setting it introduces a minor quantization
 *       (rounding) error. Thus, the hardware's actual Current LSB might slightly differ
 *       (sub-microampere) from the requested current_resolution_uA.
 * 
 * @param sensor Destination INA226 device handle containing valid shunt resistance and current resolution.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while writing the calibration register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor is NULL, shunt/resolution is 0, or calculated CAL is out of range (1..65535).
 */
INA226_Status_t INA226_Calibrate(const ina226_handle_t *sensor);

/**
 * @brief Configure alert pin function for destination INA226 device.
 * 
 * @param sensor     Destination INA226 device handle.
 * @param alert_func Selected alert function option macro (e.g. INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT).
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while updating the mask/enable register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor is NULL or alert_func combination is invalid.
 */
INA226_Status_t INA226_Set_Alert_Pin_Function(const ina226_handle_t *sensor, INA226_Alert_Func_t alert_func);

/**
 * @brief Get currently configured alert pin function from destination INA226 device.
 * 
 * @param sensor     Destination INA226 device handle.
 * @param alert_func Pointer to store the configured alert function.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading the mask/enable register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor or alert_func is NULL.
 */
INA226_Status_t INA226_Get_Alert_Pin_Function(const ina226_handle_t *sensor, INA226_Alert_Func_t *alert_func);

/**
 * @brief Set alert limit value for destination INA226 device.
 * 
 * @note The physical unit of limit_value depends on the currently configured alert function:
 *       - Shunt Voltage Over/Under Limit : limit_value in microvolts  [µV]
 *       - Bus Voltage Over/Under Limit   : limit_value in microvolts  [µV]
 *       - Power Over Limit               : limit_value in microwatts  [µW]
 * 
 * @param sensor      Destination INA226 device handle.
 * @param limit_value Alert limit value in physical micro units (µV or µW, see @note).
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading/writing alert registers.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor is NULL.
 *         - 3 : INA226_ERR_MATH_OVERFLOW; Converted register value exceeds representable range, or negative limit provided for bus/power alert.
 *         - 4 : INA226_ERR_INVALID_STATE; No limit-based alert function is configured (e.g. Conversion Ready).
 */
INA226_Status_t INA226_Set_Alert_Limit(const ina226_handle_t *sensor, int32_t limit_value);

/**
 * @brief Get alert pin assertion status from destination INA226 device.
 * 
 * @note Reading this status queries the Mask/Enable register, which hardware-clears
 *       the Alert Function Flag (AFF) in latch mode and the Conversion Ready Flag (CVRF).
 * 
 * @param sensor       Destination INA226 device handle.
 * @param alert_status Pointer to store the destination device alert pin status.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading the mask/enable register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor or alert_status is NULL.
 */
INA226_Status_t INA226_Get_Alert_Status(const ina226_handle_t *sensor, INA226_Alert_Status_t *alert_status);

/**
 * @brief Read measured current from destination INA226 device.
 * 
 * @param sensor  Destination INA226 device handle.
 * @param current Pointer to store the measured current in microamperes [µA].
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading the current register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor or current is NULL.
 *         - 3 : INA226_ERR_MATH_OVERFLOW; Scaled current exceeds 32-bit signed range.
 */
INA226_Status_t INA226_Read_Current(const ina226_handle_t *sensor, int32_t *current);

/**
 * @brief Read measured shunt voltage from destination INA226 device.
 * 
 * @param sensor  Destination INA226 device handle.
 * @param voltage Pointer to store the measured shunt voltage in microvolts [µV].
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading the shunt voltage register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor or voltage is NULL.
 */
INA226_Status_t INA226_Read_Shunt_Voltage(const ina226_handle_t *sensor, int32_t *voltage);

/**
 * @brief Read measured bus voltage from destination INA226 device.
 * 
 * @param sensor  Destination INA226 device handle.
 * @param voltage Pointer to store the measured bus voltage in microvolts [µV].
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading the bus voltage register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor or voltage is NULL.
 */
INA226_Status_t INA226_Read_Bus_Voltage(const ina226_handle_t *sensor, uint32_t *voltage);

/**
 * @brief Read measured power from destination INA226 device.
 * 
 * @param sensor Destination INA226 device handle.
 * @param power  Pointer to store the measured power in microwatts [µW].
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading the power register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor or power is NULL.
 *         - 3 : INA226_ERR_MATH_OVERFLOW; Scaled power exceeds 32-bit unsigned range.
 */
INA226_Status_t INA226_Read_Power(const ina226_handle_t *sensor, uint32_t *power);

/**
 * @brief Set alert latch mode for destination INA226 device.
 * 
 * @param sensor Destination INA226 device handle.
 * @param latch  Selected alert latch mode (INA226_ALERT_LATCH_TRANSPARENT or INA226_ALERT_LATCH_ENABLE).
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while updating the mask/enable register.
 *         - 2 : INA226_ERR_INVALID_PARAM; sensor is NULL or latch is not a valid latch option.
 */
INA226_Status_t INA226_Set_Alert_Latch(const ina226_handle_t *sensor, INA226_Alert_Latch_t latch);

#endif /* INA226_H_ */
