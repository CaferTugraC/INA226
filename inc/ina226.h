/**
 * @file ina226.h
 * @author Cafer Tura Çetin
 * @brief INA226 sensor driver header file
 * @version 0.1
 * @date 2026-07-25
 * 
 * @copyright Copyright (c) 2026 Cafer Tura Çetin
 * SPDX-License-Identifier: MIT
 */

#ifndef INA226_H_
#define INA226_H_

#include <stdint.h>

// --- Platform-specific I2C functions to be implemented by the user ---
/**
 * @brief I2C write function for library use. 
 * 
 * @param dev_addr : Destination INA226 device address.
 * @param reg_addr : Destination register address of INA226.
 * @param data     : Data to be written to the register.
 * @param len      : Byte length for the data.
 * @return uint8_t
 */
extern uint8_t INA226_Platform_I2C_Write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);

/**
 * @brief I2C read function for library use.
 * 
 * @param dev_addr : Destination INA226 device address.
 * @param reg_addr : Destination register address of INA226.
 * @param data     : Data to be read from the register.
 * @param len      : Byte length for the data.
 * @return uint8_t 
 */
extern uint8_t INA226_Platform_I2C_Read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

/* ========================================================================= */
/*                              TYPES & DEFINES                              */
/* ========================================================================= */

typedef uint8_t INA226_Config_Option_t;

/**
 * @brief Return value macros of the driver functions.
 * 
 */
typedef INA226_Config_Option_t INA226_Status_t;
#define INA226_OK                   ((INA226_Status_t)0U)
#define INA226_ERR_I2C              ((INA226_Status_t)1U)
#define INA226_ERR_INVALID_PARAM    ((INA226_Status_t)2U)
#define INA226_ERR_MATH_OVERFLOW    ((INA226_Status_t)3U)

/**
 * @brief INA226 Alert pin status macros.
 * 
 */
typedef INA226_Config_Option_t INA226_Alert_Status_t;
#define INA226_ALERT_NO_ALERT                   ((INA226_Alert_Status_t)0U)
#define INA226_ALERT_SHUNT_VOLTAGE_OVER_LIMIT   ((INA226_Alert_Status_t)1U)
#define INA226_ALERT_SHUNT_VOLTAGE_UNDER_LIMIT  ((INA226_Alert_Status_t)2U)
#define INA226_ALERT_BUS_VOLTAGE_OVER_LIMIT     ((INA226_Alert_Status_t)3U)
#define INA226_ALERT_BUS_VOLTAGE_UNDER_LIMIT    ((INA226_Alert_Status_t)4U)
#define INA226_ALERT_POWER_OVER_LIMIT           ((INA226_Alert_Status_t)5U)
#define INA226_ALERT_CONVERSION_READY           ((INA226_Alert_Status_t)6U)

/**
 * @brief INA226 Conversion time option macros.
 * 
 */
typedef INA226_Config_Option_t INA226_Conv_Time_t;
#define INA226_CT_140_US            ((INA226_Conv_Time_t)0x00U)
#define INA226_CT_204_US            ((INA226_Conv_Time_t)0x01U)
#define INA226_CT_332_US            ((INA226_Conv_Time_t)0x02U)
#define INA226_CT_588_US            ((INA226_Conv_Time_t)0x03U)
#define INA226_CT_1100_US           ((INA226_Conv_Time_t)0x04U)
#define INA226_CT_2116_US           ((INA226_Conv_Time_t)0x05U)
#define INA226_CT_4156_US           ((INA226_Conv_Time_t)0x06U)
#define INA226_CT_8244_US           ((INA226_Conv_Time_t)0x07U)

/**
 * @brief INA226 Averaging time option macros.
 * 
 */
typedef INA226_Config_Option_t INA226_Avg_Time_t;
#define INA226_AVG_1                ((INA226_Avg_Time_t)0x00U)
#define INA226_AVG_4                ((INA226_Avg_Time_t)0x01U)
#define INA226_AVG_16               ((INA226_Avg_Time_t)0x02U)
#define INA226_AVG_64               ((INA226_Avg_Time_t)0x03U)
#define INA226_AVG_128              ((INA226_Avg_Time_t)0x04U)
#define INA226_AVG_256              ((INA226_Avg_Time_t)0x05U)
#define INA226_AVG_512              ((INA226_Avg_Time_t)0x06U)
#define INA226_AVG_1024             ((INA226_Avg_Time_t)0x07U)

/**
 * @brief INA226 Operation mode option macros.
 * 
 */
typedef INA226_Config_Option_t INA226_Mode_t;
#define INA226_SHUT_DOWN                        ((INA226_Mode_t)0x00U)
#define INA226_TRIGGERED_SHUNT_VOLTAGE          ((INA226_Mode_t)0x01U)
#define INA226_TRIGGERED_BUS_VOLTAGE            ((INA226_Mode_t)0x02U)
#define INA226_TRIGGERED_BUS_AND_SHUNT_VOLTAGE  ((INA226_Mode_t)0x03U)
#define INA226_SHUT_DOWN_ALT                    ((INA226_Mode_t)0x04U)
#define INA226_CONTINUOUS_SHUNT_VOLTAGE         ((INA226_Mode_t)0x05U)
#define INA226_CONTINUOUS_BUS_VOLTAGE           ((INA226_Mode_t)0x06U)
#define INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE ((INA226_Mode_t)0x07U)

/**
 * @brief INA226 Alert function option macros.
 * 
 */
typedef uint16_t INA226_Alert_Func_t;
#define INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT                  ((INA226_Alert_Func_t)0x8000U)
#define INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT                 ((INA226_Alert_Func_t)0x4000U)
#define INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT                    ((INA226_Alert_Func_t)0x2000U)
#define INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT                   ((INA226_Alert_Func_t)0x1000U)
#define INA226_ALERT_FUNC_POWER_OVER_LIMIT                          ((INA226_Alert_Func_t)0x0800U)
#define INA226_ALERT_FUNC_CONVERSION_READY                          ((INA226_Alert_Func_t)0x0400U)
#define INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT_CVR              ((INA226_Alert_Func_t)0x8400U)
#define INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT_CON_READY_CVR   ((INA226_Alert_Func_t)0x4400U)
#define INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT_CON_READY_CVR      ((INA226_Alert_Func_t)0x2400U)
#define INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT_CON_READY_CVR     ((INA226_Alert_Func_t)0x1400U)
#define INA226_ALERT_FUNC_POWER_OVER_LIMIT_CON_READY_CVR            ((INA226_Alert_Func_t)0xC000U)


/**
 * @brief INA226 sensor handle structure containing hardware details and calibration parameters.
 * 
 */
typedef struct {
    uint8_t ina226_i2c_addr;
    uint16_t shunt_resistor_uOhm;
    uint32_t current_resolution_uA;
    int32_t current_resolution_err_diff_uA;
} ina226_handle_t;

/* ========================================================================= */
/*                            FUNCTION PROTOTYPES                            */
/* ========================================================================= */

/**
 * @brief Reset the destination INA226 device.
 * 
 * @param addr : Destination INA226 device Address
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; I2C communication error during reset register access.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid internal parameter passed to register access helpers.
 */
INA226_Status_t INA226_Reset(const ina226_handle_t *sensor);

/**
 * @brief Set shunt voltage conversion time options to destination INA226 device.
 * 
 * @param addr      : Destination INA226 device Address
 * @param conv_time : Selected conversion time option for the device. This value must be a macro
 *                    or decimal value of bit combination from datasheet. 
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; I2C communication error while updating the configuration register.
 *         - 2 : INA226_ERR_INVALID_PARAM; conv_time is outside the supported 3-bit field range.
 */
INA226_Status_t INA226_Set_Shunt_Voltage_Conversion_Time(const ina226_handle_t *sensor, INA226_Conv_Time_t conv_time);

/**
 * @brief Set bus voltage conversion time option to destination INA226 device.
 * 
 * @param addr      : Destination INA226 device Address
 * @param conv_time : Selected conversion time option for the device. This value must be a macro
 *                    or decimal value of bit combination from datasheet.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; I2C communication error while updating the configuration register.
 *         - 2 : INA226_ERR_INVALID_PARAM; conv_time is outside the supported 3-bit field range.
 */
INA226_Status_t INA226_Set_Bus_Voltage_Conversion_Time(const ina226_handle_t *sensor, INA226_Conv_Time_t conv_time);

/**
 * @brief Set operation mode to destination INA226 device.
 * 
 * @param addr : Destination INA226 device Address.
 * @param mode : Selected operation mode for the device. This value must be a macro
 *                    or decimal value of bit combination from datasheet.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; I2C communication error while updating the configuration register.
 *         - 2 : INA226_ERR_INVALID_PARAM; mode is outside the supported 3-bit field range.
 */
INA226_Status_t INA226_Set_Operating_Mode(const ina226_handle_t *sensor, INA226_Mode_t mode);

/**
 * @brief Set averaging mode to destination INA226 device.
 * 
 * @param addr     : Destination INA226 device Address.
 * @param avg_time : Selected averaging time option for the device. This value must be a macro
 *                    or decimal value of bit combination from datasheet.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; I2C communication error while updating the configuration register.
 *         - 2 : INA226_ERR_INVALID_PARAM; avg_time is outside the supported 3-bit field range.
 */
INA226_Status_t INA226_Set_Averaging_Mode(const ina226_handle_t *sensor, INA226_Avg_Time_t avg_time);

/**
 * @brief Set current and power measurement resolution by setting the calibration register.
 * 
 * @param addr          : Destination INA226 device Address.
 * @param cal_reg_value : Calibration register value for the device. This value must be calculated using Equation 1 from the datasheet.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; I2C communication error while writing the calibration register.
 *         - 2 : INA226_ERR_INVALID_PARAM; cal_reg_value is not valid for the selected measurement scaling.
 */
INA226_Status_t INA226_Calibrate(ina226_handle_t *sensor);

/**
 * @brief Set alert pin function to destination INA226 device.
 * 
 * @param addr       : Destination INA226 device Address.
 * @param alert_func : Selected alert pin function for the device. This value must be a macro
 *                    or decimal value of bit combination from datasheet.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; I2C communication error while updating the mask/enable register.
 *         - 2 : INA226_ERR_INVALID_PARAM; alert_func is outside the supported 5-bit alert selector values.
 */
INA226_Status_t INA226_Set_Alert_Pin_Function(const ina226_handle_t *sensor, INA226_Alert_Func_t alert_func);

/**
 * @brief Set alert limit value to destination INA226 device by setting the alert limit register.
 * 
 * @param addr        : Destination INA226 device Address.
 * @param limit_value : Alert limit value for the device.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; I2C communication error while writing the alert limit register.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid internal parameter passed to register access helpers.
 */
INA226_Status_t INA226_Set_Alert_Limit(const ina226_handle_t *sensor, uint16_t limit_value);

/**
 * @brief Get alert pin status for the destination INA226 device.
 * 
 * @param addr         : Destination INA226 device Address.
 * @param alert_status : External INA226_Alert_Status_t type pointer for the destination device alert pin status.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading the mask/enable register.
 *         - 2 : INA226_ERR_INVALID_PARAM; alert_status is NULL.
 */
INA226_Status_t INA226_Get_Alert_Status(const ina226_handle_t *sensor, INA226_Alert_Status_t *alert_status);

/**
 * @brief Read current value from destination INA226 device.
 * 
 * @param addr    : Destination INA226 device Address.
 * @param current : Pointer to store the measured current in microamperes.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading the current register.
 *         - 2 : INA226_ERR_INVALID_PARAM; current is NULL.
 */
INA226_Status_t INA226_Read_Current(const ina226_handle_t *sensor, int32_t *current);

/**
 * @brief Read shunt voltage from destination INA226 device.
 * 
 * @param addr    : Destination INA226 device Address.
 * @param voltage : Pointer to store the measured shunt voltage in microvolts.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading the shunt voltage register.
 *         - 2 : INA226_ERR_INVALID_PARAM; voltage is NULL.
 */
INA226_Status_t INA226_Read_Shunt_Voltage(const ina226_handle_t *sensor, int32_t *voltage);

/**
 * @brief  Read bus voltage from destination INA226 device.
 * 
 * @param addr    : Destination INA226 device Address.
 * @param voltage : Pointer to store the measured bus voltage in microvolts.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading the bus voltage register.
 *         - 2 : INA226_ERR_INVALID_PARAM; voltage is NULL.
 */
INA226_Status_t INA226_Read_Bus_Voltage(const ina226_handle_t *sensor, uint32_t *voltage);

/**
 * @brief Read power from destination INA226 device.
 * 
 * @param addr  : Destination INA226 device Address.
 * @param power : Pointer to store the measured power in microwatts.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; I2C communication error while reading the power register.
 *         - 2 : INA226_ERR_INVALID_PARAM; power is NULL.
 */
INA226_Status_t INA226_Read_Power(const ina226_handle_t *sensor, uint32_t *power);

#endif /* INA226_H_ */