/**
 * @file ina226.c
 * @author Cafer Tura Çetin
 * @brief INA226 sensor driver source file
 * @version 0.1
 * @date 2026-07-25
 * 
 * @copyright Copyright (c) 2026 Cafer Tura Çetin
 * SPDX-License-Identifier: MIT
 * 
 */

/* ========================================================================= */
/*                                  INCLUDES                                 */
/* ========================================================================= */

#include "../inc/ina226.h"
#include <stddef.h>

/* ========================================================================= */
/*                              MACRO DEFINATIONS                            */
/* ========================================================================= */

/**
 * @brief Register address macros for INA226.
 * 
 */
#define INA226_CONFIG_REG                   (0x00)
#define INA226_SH_VOLTAGE_REG               (0x01)
#define INA226_BUS_VOLTAGE_REG              (0x02)
#define INA226_POWER_REG                    (0x03)
#define INA226_CURRENT_REG                  (0x04)
#define INA226_CALIBRATION_REG              (0x05)
#define INA226_MASK_EN_REG                  (0x06)
#define INA226_ALERT_LIM_REG                (0x07)
#define INA226_MANCUFACTURE_ID_REG          (0xFE)
#define INA226_DIE_ID_REG                   (0xFF)

#define INA226_SH_VOLTAGE_LSB_NV            (2500U)
#define INA226_BUS_VOLTAGE_LSB_UV           (1250U)


/* ========================================================================= */
/*                              PRIVATE FUNCTIONES                           */
/* ========================================================================= */

/**
 * @brief read 16-bit data from INA226.
 * 
 * @param dev_addr : I2C address of destination INA226
 * @param reg_addr : Address of register to be read. 
 * @param value    : An external pointer for the value read from the register.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Read_Reg.
 * @details The INA226 transmits data in Big-Endian format. To avoid hardware
 *           architecture discrepancies, the data is fetched into a byte buffer and then
 *           safely shifted into the destination pointer in the correct MSB-first order.
 */
static INA226_Status_t INA226_Read_Reg(uint8_t dev_addr, uint8_t reg_addr, uint16_t *value) {

    if (value == NULL) {
        return INA226_ERR_INVALID_PARAM;
    }

    uint8_t buffer[2] = {0, 0}; // buffer[0] = MSB, buffer[1] = LSB
    uint8_t i2c_status;

    i2c_status = INA226_Platform_I2C_Read(dev_addr, reg_addr, buffer, 2);

    if (i2c_status != 0) {
        return INA226_ERR_I2C; 
    }

    (*value) = (uint16_t)((buffer[0] << 8U) | buffer[1]);

    return INA226_OK;
}

/**
 * @brief write 16-bit data to INA226.
 * 
 * @param dev_addr : I2C address of destination IN226.
 * @param reg_addr : Address of register to be write. 
 * @param value    : value to be written to register.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Write.
 *         - 2 : INA226_ERR_INVALID_PARAM; Invalid param for INA226_Write_Reg.
 * @details The INA226 expects data in Big-Endian format. To avoid hardware
 *          architecture discrepancies, the 16-bit value is split and formatted 
 *          into a byte buffer (MSB first) before being sent over I2C.
 */
static INA226_Status_t INA226_Write_Reg(uint8_t dev_addr, uint8_t reg_addr, uint16_t value) {

    
    uint8_t buffer[2] = {0, 0};
    uint8_t i2c_status;

    buffer[0] = (uint8_t)(value >> 8U);
    buffer[1] = (uint8_t)(value & 0xFFU);

    i2c_status = INA226_Platform_I2C_Write(dev_addr, reg_addr, buffer, 2);

      if (i2c_status != 0) {
        return INA226_ERR_I2C; 
    }

    return INA226_OK;
}

static INA226_Status_t set_config_option(uint8_t addr, INA226_Config_Option_t option, uint16_t mask, uint8_t pos) {

    uint16_t reg_value = 0;

    // Read the register value of destination device.
    INA226_Status_t op_status = INA226_Read_Reg(addr, INA226_CONFIG_REG, &reg_value);

    // check the I2C operation result.
    if (op_status != INA226_OK) {
        return op_status;
    }

    // Modify the interested bitfield of register: First clear the interested bitfield by using mask variable,
    // followed by setting the interested bitfield with the option.
    reg_value &= ~(mask);
    reg_value |= (option << pos);

    // Write the interested register of destination device.
    return INA226_Write_Reg(addr, INA226_CONFIG_REG, reg_value);
}

/* ========================================================================= */
/*                              PUBLIC FUNCTIONES                            */
/* ========================================================================= */

INA226_Status_t INA226_Reset(ina226_handle_t *sensor) {
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;
   
    uint16_t config_reg_val = INA226_CONFIG_RESET_MASK;
    
    // Setting the reset bit (15) is resets all register to default values. And this bit its self-clears.
    // There is ne need the read current value of register, write reset mask directly to register. 
    return INA226_Write_Reg(sensor->ina226_i2c_addr, INA226_CONFIG_REG, config_reg_val);
}

INA226_Status_t INA226_Set_Shunt_Voltage_Conversion_Time(ina226_handle_t *sensor, INA226_Conv_Time_t conv_time) {
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;

    // Validate the conv_time parameter
    if (conv_time > INA226_CT_8244_US) {
        return INA226_ERR_INVALID_PARAM;
    }

    // Set with Read Modify Write.
    return set_config_option(sensor->ina226_i2c_addr, conv_time, INA226_CONFIG_SHUNT_CT_MASK, INA226_CONFIG_SHUNT_CT_POS);
}

INA226_Status_t INA226_Set_Bus_Voltage_Conversion_Time(ina226_handle_t *sensor, INA226_Conv_Time_t conv_time) {
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;

    // Validate the conv_time parameter
    if (conv_time > INA226_CT_8244_US) {
        return INA226_ERR_INVALID_PARAM;
    }

    // Set with Read Modify Write.
    return set_config_option(sensor->ina226_i2c_addr, conv_time, INA226_CONFIG_BUS_CT_MASK, INA226_CONFIG_BUS_CT_POS);
}

INA226_Status_t INA226_Set_Operating_Mode(ina226_handle_t *sensor, INA226_Mode_t mode) {
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;
    
    // Validate the mode paramtere.
    if (mode > INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE) { // Continuous bus and shunt voltage is highest value mode can be.
        return INA226_ERR_INVALID_PARAM;
    }

    // Set with Read Modify Write.
    return set_config_option(sensor->ina226_i2c_addr, mode, INA226_CONFIG_MODE_MASK, INA226_CONFIG_MODE_POS);
}

INA226_Status_t INA226_Set_Averaging_Mode(ina226_handle_t *sensor, INA226_Avg_Time_t avg_time) {
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;

    // Validate the avg_time paramtere.
    if (avg_time > INA226_AVG_1024) {
        return INA226_ERR_INVALID_PARAM;
    }
    
    // Set with Read Modify Write.
    return set_config_option(sensor->ina226_i2c_addr, avg_time, INA226_CONFIG_AVG_MASK, INA226_CONFIG_AVG_POS);
}

INA226_Status_t INA226_Set_Alert_Pin_Function(ina226_handle_t *sensor, INA226_Alert_Func_t alert_func) {
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;

    // Validate the alert_function parameter.
    if (alert_func != INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT &&
        alert_func != INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT &&
        alert_func != INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT &&
        alert_func != INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT &&
        alert_func != INA226_ALERT_FUNC_POWER_OVER_LIMIT &&
        alert_func != INA226_ALERT_FUNC_CONVERSION_READY &&
        alert_func != INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT_CVR &&
        alert_func != INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT_CON_READY_CVR &&
        alert_func != INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT_CON_READY_CVR &&
        alert_func != INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT_CON_READY_CVR &&
        alert_func != INA226_ALERT_FUNC_POWER_OVER_LIMIT_CON_READY_CVR) {
            return INA226_ERR_INVALID_PARAM;
    }

    return INA226_Write_Reg(sensor->ina226_i2c_addr, INA226_MASK_EN_REG, alert_func);
}

INA226_Status_t INA226_Set_Alert_Limit(ina226_handle_t *sensor, uint16_t limit_value) {
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;
    return INA226_OK;
}

INA226_Status_t INA226_Get_Alert_Status(ina226_handle_t *sensor, INA226_Alert_Status_t *alert_status) {
    if (sensor == NULL || alert_status == NULL) return INA226_ERR_INVALID_PARAM;
    return INA226_OK;
}

INA226_Status_t INA226_Calibrate(ina226_handle_t *sensor) {
    
    if (sensor == NULL) {
        return INA226_ERR_INVALID_PARAM;
    }

    if (sensor->shunt_resistor_uOhm == 0 || sensor->current_resolution_uA == 0) {
        return INA226_ERR_INVALID_PARAM;
    }

    // CAL = 0.00512 / (Current_LSB[A] * Rshunt[Ohm])
    // CAL = 5.12e9 / (current_resolution_uA * shunt_resistor_uOhm)
    const uint64_t numerator = 5120000000ULL;
    const uint64_t denominator = (uint64_t)sensor->current_resolution_uA * 
                                 (uint64_t)sensor->shunt_resistor_uOhm;

    // round-to-nearest
    uint64_t cal = (numerator + (denominator / 2ULL)) / denominator;

    if (cal == 0ULL || cal > 0xFFFFULL) {
        return INA226_ERR_INVALID_PARAM;
    }

    const uint64_t actual_denominator = cal * (uint64_t)sensor->shunt_resistor_uOhm;
    const uint64_t actual_current_lsb_uA = numerator / actual_denominator;

    sensor->current_resolution_err_diff_uA =
        (int32_t)((int64_t)actual_current_lsb_uA - (int64_t)sensor->current_resolution_uA);

    uint16_t calibration_reg_val = (uint16_t)cal;
    
    return INA226_Write_Reg(sensor->ina226_i2c_addr, INA226_CALIBRATION_REG, calibration_reg_val);
}

INA226_Status_t INA226_Read_Current(ina226_handle_t *sensor, int32_t *current) {

    if (sensor == NULL || current == NULL) return INA226_ERR_INVALID_PARAM;

    uint16_t current_reg = 0;
    
    INA226_Status_t op_status = INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_CURRENT_REG, &current_reg);

    if (op_status != INA226_OK) {
        return op_status;
    }

    // current_reg * current_lsb = current;
    (*current) = (int32_t)((int16_t)current_reg) * ((int32_t)sensor->current_resolution_uA + (int32_t)sensor->current_resolution_err_diff_uA);

    return INA226_OK;
}

INA226_Status_t INA226_Read_Shunt_Voltage(ina226_handle_t *sensor, int32_t *voltage) {

    if (sensor == NULL || voltage == NULL) return INA226_ERR_INVALID_PARAM;

    uint16_t shunt_voltage_reg = 0;

    INA226_Status_t op_status = INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_SH_VOLTAGE_REG, &shunt_voltage_reg);

    if (op_status != INA226_OK) {
        return op_status;
    }

    int32_t raw = (int32_t)(int16_t)shunt_voltage_reg;

    int64_t scaled = (int64_t)raw * 5;
    int32_t voltage_uV = (int32_t)((scaled + (scaled >= 0 ? 1 : -1)) / 2); // round-to-nearest

    (*voltage) = voltage_uV;

    return INA226_OK;
}

INA226_Status_t INA226_Read_Bus_Voltage(ina226_handle_t *sensor, uint32_t *voltage) {

    if (sensor == NULL || voltage == NULL) return INA226_ERR_INVALID_PARAM;

    uint16_t bus_voltage_reg = 0;
    
    INA226_Status_t op_status = INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_BUS_VOLTAGE_REG, &bus_voltage_reg);

    if (op_status != INA226_OK) {
        return op_status;
    }

    // Bus Voltage [uV] = Bus Voltage Register Value * Bus Voltage LSB [uV]
    (*voltage) = (uint32_t)(bus_voltage_reg * INA226_BUS_VOLTAGE_LSB_UV);

    return INA226_OK;
}

INA226_Status_t INA226_Read_Power(ina226_handle_t *sensor, uint32_t *power) {

    if (sensor == NULL || power == NULL) return INA226_ERR_INVALID_PARAM;

    uint16_t power_reg = 0;
    
    INA226_Status_t op_status = INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_POWER_REG, &power_reg);

    if (op_status != INA226_OK) {
        return op_status;
    }

    uint32_t power_lsb = (uint32_t)((int32_t)sensor->current_resolution_uA + current_resolution_err_diff_uA);

    // Power [uW] = Power Register Value * Power LSB [uW]
    (*power) = ((uint32_t)power_reg * power_lsb);

    return INA226_OK;
}
