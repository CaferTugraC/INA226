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
 * @param data     : Data to be read to register.
 * @param len      : Byte length for the data.
 * @return uint8_t 
 */
extern uint8_t INA226_Platform_I2C_Read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

/* ========================================================================= */
/*                              TYPES & DEFINES                              */
/* ========================================================================= */

/**
 * @brief Return value macros of the driver functions.
 * 
 */
typedef uint8_t INA226_Status_t;
#define INA226_OK                   ((INA226_Status_t)0U)
#define INA226_ERR_I2C              ((INA226_Status_t)1U)
#define INA226_ERR_INVALID_PARAM    ((INA226_Status_t)2U)

/**
 * @brief INA226 Alert pin status macros.
 * 
 */
typedef uint8_t INA226_Alert_Status_t;
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
typedef uint8_t INA226_Conv_Time_t;
#define INA226_CT_140_US            ((INA226_Conv_Time_t)0U)
#define INA226_CT_204_US            ((INA226_Conv_Time_t)1U)
#define INA226_CT_332_US            ((INA226_Conv_Time_t)2U)
#define INA226_CT_588_US            ((INA226_Conv_Time_t)3U)
#define INA226_CT_1100_US           ((INA226_Conv_Time_t)4U)
#define INA226_CT_2116_US           ((INA226_Conv_Time_t)5U)
#define INA226_CT_4156_US           ((INA226_Conv_Time_t)6U)
#define INA226_CT_8244_US           ((INA226_Conv_Time_t)7U)

/**
 * @brief INA226 Averaging time option macros.
 * 
 */
typedef uint8_t INA226_Avg_Time_t;
#define INA226_AVG_1                ((INA226_Avg_Time_t)0U)
#define INA226_AVG_4                ((INA226_Avg_Time_t)1U)
#define INA226_AVG_16               ((INA226_Avg_Time_t)2U)
#define INA226_AVG_64               ((INA226_Avg_Time_t)3U)
#define INA226_AVG_128              ((INA226_Avg_Time_t)4U)
#define INA226_AVG_256              ((INA226_Avg_Time_t)5U)
#define INA226_AVG_512              ((INA226_Avg_Time_t)6U)
#define INA226_AVG_1024             ((INA226_Avg_Time_t)7U)

/**
 * @brief INA226 Operation mode option macros.
 * 
 */
typedef uint8_t INA226_Mode_t;
#define INA226_SHUT_DOWN                        ((INA226_Mode_t)0U)
#define INA226_TRIGGERED_SHUNT_VOLTAGE          ((INA226_Mode_t)1U)
#define INA226_TRIGGERED_BUS_VOLTAGE            ((INA226_Mode_t)2U)
#define INA226_TRIGGERED_BUS_AND_SHUNT_VOLTAGE  ((INA226_Mode_t)3U)
#define INA226_SHUT_DOWN_ALT                     ((INA226_Mode_t)4U)
#define INA226_CONTINUOUS_SHUNT_VOLTAGE         ((INA226_Mode_t)5U)
#define INA226_CONTINUOUS_BUS_VOLTAGE           ((INA226_Mode_t)6U)
#define INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE ((INA226_Mode_t)7U)

/**
 * @brief INA226 Alert function option macros.
 * 
 */
typedef uint8_t INA226_Alert_Func_t;
#define INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT  ((INA226_Alert_Func_t)15U)
#define INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT ((INA226_Alert_Func_t)14U)
#define INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT    ((INA226_Alert_Func_t)13U)
#define INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT   ((INA226_Alert_Func_t)12U)
#define INA226_ALERT_FUNC_POWER_OVER_LIMIT          ((INA226_Alert_Func_t)11U)
#define INA226_ALERT_FUNC_CONVERSION_READY          ((INA226_Alert_Func_t)10U)


/* ========================================================================= */
/*                            FUNCTION PROTOTYPES                            */
/* ========================================================================= */

/**
 * @brief Reset the destination INA226 device.
 * 
 * @param addr : Destination INA226 device Address
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 */
INA226_Status_t INA226_Reset(uint8_t addr);

/**
 * @brief Set shunt voltage conversion time options to destination INA226 device.
 * 
 * @param addr      : Destination INA226 device Address
 * @param conv_time : Selected conversion time option for the device. This value must be from macro
 *                    or decimal value of bit combination from datasheet. 
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 */
INA226_Status_t INA226_Set_Shunt_Voltage_Conversion_Time(uint8_t addr, INA226_Conv_Time_t conv_time);

/**
 * @brief Set bus voltage conversion time option to destination INA226 device.
 * 
 * @param addr      : Destination INA226 device Address
 * @param conv_time : Selected conversion time option for the device. This value must be from macro
 *                    or decimal value of bit combination from datasheet.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 */
INA226_Status_t INA226_Set_Bus_Voltage_Conversion_Time(uint8_t addr, INA226_Conv_Time_t conv_time);

/**
 * @brief Set operation mode to destination INA226 device.
 * 
 * @param addr : Destination INA226 device Address.
 * @param mode : Selected operation mode for the device. This value must be from macro
 *                    or decimal value of bit combination from datasheet.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 */
INA226_Status_t INA226_Set_Operating_Mode(uint8_t addr, INA226_Mode_t mode);

/**
 * @brief Set averaging mode to destination INA226 device.
 * 
 * @param addr     : Destination INA226 device Address.
 * @param avg_time : Selected averaging time option for the device. This value must be from macro
 *                    or decimal value of bit combination from datasheet.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 */
INA226_Status_t INA226_Set_Averaging_Mode(uint8_t addr, INA226_Avg_Time_t avg_time);

/**
 * @brief Set current and power measurement resolution by setting the calibration register.
 * 
 * @param addr          : Destination INA226 device Address.
 * @param cal_reg_value : Calibration register value for the device. This value must calculate with equation 1 from datasheet.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 */
INA226_Status_t INA226_Set_Calibration_Reg(uint8_t addr, uint16_t cal_reg_value);

/**
 * @brief Set alert pin function to destination INA226 device.
 * 
 * @param addr       : Destination INA226 device Address.
 * @param alert_func : Selected alert pin function for the device. This value must be from macro
 *                    or decimal value of bit combination from datasheet.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 */
INA226_Status_t INA226_Set_Alert_Pin_Function(uint8_t addr, INA226_Alert_Func_t alert_func);

/**
 * @brief Set alert limit value to destination INA226 device by setting the alert limit register.
 * 
 * @param addr        : Destination INA226 device Address.
 * @param limit_value : Alert limit value for the device.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 */
INA226_Status_t INA226_Set_Alert_Limit(uint8_t addr, uint16_t limit_value);

/**
 * @brief Get alert pin status to destination INA226 device.
 * 
 * @param addr         : Destination INA226 device Address.
 * @param alert_status : External INA226_Alert_Status_t type pointer for the destination device alert pin status.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 */
INA226_Status_t INA226_Get_Alert_Status(uint8_t addr, INA226_Alert_Status_t *alert_status);

/**
 * @brief Read current value from destination INA226 device.
 * 
 * @param addr    : Destination INA226 device Address.
 * @param current : Pointer to store the measured current in milliamperes.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 */
INA226_Status_t INA226_Read_Current(uint8_t addr, int16_t *current);

/**
 * @brief Read shunt voltage from destination INA226 device.
 * 
 * @param addr    : Destination INA226 device Address.
 * @param voltage : Pointer to store the measured shunt voltage in millivolts.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 */
INA226_Status_t INA226_Read_Shunt_Voltage(uint8_t addr, int16_t *voltage);

/**
 * @brief  Read bus voltage from destination INA226 device.
 * 
 * @param addr    : Destination INA226 device Address.
 * @param voltage : Pointer to store the measured bus voltage in millivolts.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 */
INA226_Status_t INA226_Read_Bus_Voltage(uint8_t addr, uint16_t *voltage);

/**
 * @brief Read power from destination INA226 device.
 * 
 * @param addr  : Destination INA226 device Address.
 * @param power : Pointer to store the measured power in milliwatts.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 */
INA226_Status_t INA226_Read_Power(uint8_t addr, uint16_t *power);

#endif /* INA226_H_ */