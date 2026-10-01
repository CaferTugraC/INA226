/**
 * @file ina226.c
 * @author Cafer Tura Çetin (cafercetin.tr@gmail.com)
 * @brief INA226 sensor driver source file.
 * @version 0.3
 * @date 2026-07-25
 * 
 * @copyright Copyright (c) 2026 Cafer Tura Çetin
 * SPDX-License-Identifier: MIT
 * 
 * Distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

/* ========================================================================= */
/*                                  INCLUDES                                 */
/* ========================================================================= */

#include "ina226.h"
#include "ina226_regs.h"
#include <stddef.h>

/* ========================================================================= */
/*                              PRIVATE FUNCTIONS                            */
/* ========================================================================= */

/**
 * @brief Read 16-bit register value from INA226.
 * 
 * @param dev_addr I2C 7-bit address of destination INA226.
 * @param reg_addr Address of register to be read. 
 * @param value    Pointer where the read 16-bit value will be stored.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Read.
 *         - 2 : INA226_ERR_INVALID_PARAM; value is NULL.
 * @details The INA226 transmits data in Big-Endian format. To avoid hardware
 *          architecture discrepancies, the data is fetched into a byte buffer and then
 *          safely shifted into the destination pointer in MSB-first order.
 */
static INA226_Status_t INA226_Read_Reg(uint8_t dev_addr, uint8_t reg_addr, uint16_t *value) {

    if (value == NULL) {
        return INA226_ERR_INVALID_PARAM;
    }

    uint8_t buffer[2] = {0U, 0U}; // buffer[0] = MSB, buffer[1] = LSB
    uint8_t i2c_status;

    i2c_status = INA226_Platform_I2C_Read(dev_addr, reg_addr, buffer, 2);

    if (i2c_status != 0) {
        return INA226_ERR_I2C; 
    }

    (*value) = (uint16_t)(((uint16_t)buffer[0] << 8U) | buffer[1]);

    return INA226_OK;
}

/**
 * @brief Write 16-bit value to INA226 register.
 * 
 * @param dev_addr I2C 7-bit address of destination INA226.
 * @param reg_addr Address of register to write. 
 * @param value    16-bit value to be written to the register.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; Error from INA226_Platform_I2C_Write.
 * @details The INA226 expects data in Big-Endian format. To avoid hardware
 *          architecture discrepancies, the 16-bit value is split and formatted 
 *          into a byte buffer (MSB first) before being sent over I2C.
 */
static INA226_Status_t INA226_Write_Reg(uint8_t dev_addr, uint8_t reg_addr, uint16_t value) {

    
    uint8_t buffer[2] = {0U, 0U};
    uint8_t i2c_status;

    buffer[0] = (uint8_t)(value >> 8U);
    buffer[1] = (uint8_t)(value & 0xFFU);

    i2c_status = INA226_Platform_I2C_Write(dev_addr, reg_addr, buffer, 2);

      if (i2c_status != 0) {
        return INA226_ERR_I2C; 
    }

    return INA226_OK;
}

/**
 * @brief Helper function to perform read-modify-write on the configuration register.
 * 
 * @param addr   I2C 7-bit address of destination INA226.
 * @param option Configuration field value to set.
 * @param mask   Bitmask corresponding to the configuration field.
 * @param pos    Bit shift position for the field.
 * @return INA226_Status_t
 *         - 0 : INA226_OK; Success.
 *         - 1 : INA226_ERR_I2C; Error during I2C read or write.
 */
static INA226_Status_t set_config_option(uint8_t addr, INA226_Config_Option_t option, uint16_t mask, uint8_t pos) {

    uint16_t reg_value = 0U;

    // Read the register value of destination device.
    INA226_Status_t op_status = INA226_Read_Reg(addr, INA226_CONFIG_REG, &reg_value);

    // check the I2C operation result.
    if (op_status != INA226_OK) {
        return op_status;
    }

    // Modify the interested bitfield of register: First clear the interested bitfield by using mask variable,
    // followed by setting the interested bitfield with the option.
    reg_value &= (uint16_t)(~mask);
    reg_value |= (uint16_t)((uint16_t)option << pos);

    // Write the interested register of destination device.
    return INA226_Write_Reg(addr, INA226_CONFIG_REG, reg_value);
}

/* ========================================================================= */
/*                              PUBLIC FUNCTIONS                             */
/* ========================================================================= */

INA226_Status_t INA226_Reset(const ina226_handle_t *sensor) {
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;
   
    uint16_t config_reg_val = INA226_CONFIG_RESET_MASK;
    
    // Setting the reset bit (15) is resets all register to default values. And this bit its self-clears.
    // There is no need to read current value of register, write reset mask directly to register. 
    return INA226_Write_Reg(sensor->ina226_i2c_addr, INA226_CONFIG_REG, config_reg_val);
}

INA226_Status_t INA226_Set_Shunt_Voltage_Conversion_Time(const ina226_handle_t *sensor, INA226_Conv_Time_t conv_time) {
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;

    // Validate the conv_time parameter
    if (conv_time > INA226_CT_8244_US) {
        return INA226_ERR_INVALID_PARAM;
    }

    // Set with Read Modify Write.
    return set_config_option(sensor->ina226_i2c_addr, conv_time, INA226_CONFIG_SHUNT_CT_MASK, INA226_CONFIG_SHUNT_CT_POS);
}

INA226_Status_t INA226_Set_Bus_Voltage_Conversion_Time(const ina226_handle_t *sensor, INA226_Conv_Time_t conv_time) {
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;

    // Validate the conv_time parameter
    if (conv_time > INA226_CT_8244_US) {
        return INA226_ERR_INVALID_PARAM;
    }

    // Set with Read Modify Write.
    return set_config_option(sensor->ina226_i2c_addr, conv_time, INA226_CONFIG_BUS_CT_MASK, INA226_CONFIG_BUS_CT_POS);
}

INA226_Status_t INA226_Set_Operating_Mode(const ina226_handle_t *sensor, INA226_Mode_t mode) {
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;
    
    // Validate the mode parameter.
    if (mode > INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE) { // Continuous bus and shunt voltage is highest value mode can be.
        return INA226_ERR_INVALID_PARAM;
    }

    // Set with Read Modify Write.
    return set_config_option(sensor->ina226_i2c_addr, mode, INA226_CONFIG_MODE_MASK, INA226_CONFIG_MODE_POS);
}

INA226_Status_t INA226_Set_Averaging_Mode(const ina226_handle_t *sensor, INA226_Avg_Time_t avg_time) {
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;

    // Validate the avg_time parameter.
    if (avg_time > INA226_AVG_1024) {
        return INA226_ERR_INVALID_PARAM;
    }
    
    // Set with Read Modify Write.
    return set_config_option(sensor->ina226_i2c_addr, avg_time, INA226_CONFIG_AVG_MASK, INA226_CONFIG_AVG_POS);
}

INA226_Status_t INA226_Set_Alert_Pin_Function(const ina226_handle_t *sensor, INA226_Alert_Func_t alert_func) {
    
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;

    // Validate the alert_func parameter using the bit-field structure of the alert function.
    // Extract the main alert source bits (bits [5:1]), excluding the Conversion Ready bit (bit 0).
    uint16_t main_bits = alert_func & INA226_ALERT_FUNC_MAIN_BITS_MASK;

    if (main_bits == 0U) {
        // No main alert source is selected. The only valid option in this case
        // is pure Conversion Ready (bit 0 only). Any other value is invalid.
        if (alert_func != INA226_ALERT_FUNC_CONVERSION_READY) {
            return INA226_ERR_INVALID_PARAM;
        }
    }
    else {
        // A main alert source is selected. Exactly one main bit must be set.
        // Power-of-two check: (x & (x - 1)) == 0 is true only when x has a single bit set.
        if ((main_bits & (main_bits - 1U)) != 0U) {
            return INA226_ERR_INVALID_PARAM;
        }
        // Remaining bits (after removing the main source and CVR bit) must be zero.
        // This rejects values with undefined or reserved bits set.
        if ((alert_func & ~(main_bits | INA226_ALERT_FUNC_CONVERSION_READY)) != 0U) {
            return INA226_ERR_INVALID_PARAM;
        }
    }

    uint16_t mask_en_reg = 0;

    INA226_Status_t op_status = INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_MASK_EN_REG, &mask_en_reg);

    if (op_status != INA226_OK) {
        return op_status;
    }

    mask_en_reg &= (uint16_t)(~INA226_MASK_ENABLE_ALERT_FUNC_MASK);
    mask_en_reg |= (uint16_t)((uint16_t)alert_func << (uint16_t)INA226_MASK_ENABLE_ALERT_FUNC_POS);

    return INA226_Write_Reg(sensor->ina226_i2c_addr, INA226_MASK_EN_REG, mask_en_reg);
}

INA226_Status_t INA226_Get_Alert_Pin_Function(const ina226_handle_t *sensor, INA226_Alert_Func_t *alert_func) {

    if (sensor == NULL || alert_func == NULL) return INA226_ERR_INVALID_PARAM;

    uint16_t mask_en_reg = 0;
    INA226_Status_t op_status = INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_MASK_EN_REG, &mask_en_reg);

    if (op_status != INA226_OK) {
        return op_status;
    }

    *alert_func = (INA226_Alert_Func_t)((mask_en_reg & INA226_MASK_ENABLE_ALERT_FUNC_MASK) >> INA226_MASK_ENABLE_ALERT_FUNC_POS);
    
    return INA226_OK;
}

INA226_Status_t INA226_Set_Alert_Limit(const ina226_handle_t *sensor, int32_t limit_value) {

    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;

    uint16_t alert_limit_reg_val = 0;
    INA226_Alert_Func_t alert_func;

    INA226_Status_t op_status = INA226_Get_Alert_Pin_Function(sensor, &alert_func);

    if (op_status != INA226_OK) {
        return op_status;
    }

    if (alert_func & INA226_ALERT_FUNC_SHUNT_CATEGORY_MASK) {

        // Shunt voltage alert: Convert limit from microvolts to register value.
        // Shunt Voltage LSB = 2.5 uV. Register = limit_value_uV / 2.5.
        // Integer round-to-nearest: (2 * x + bias) / 5, where bias corrects rounding direction.
        int64_t reg_val_64 = (2LL * (int64_t)limit_value + (limit_value >= 0 ? 2LL : -2LL)) / 5LL;

        if (reg_val_64 > INT16_MAX || reg_val_64 < INT16_MIN) {
            return INA226_ERR_MATH_OVERFLOW;
        }

        alert_limit_reg_val = (uint16_t)(int16_t)reg_val_64;

    }

    else if (alert_func & INA226_ALERT_FUNC_BUS_CATEGORY_MASK) {

        if (limit_value < 0) {
            return INA226_ERR_INVALID_PARAM;
        }

        // Bus voltage alert: Convert limit from microvolts to register value.
        // Bus Voltage LSB = 1.25 mV = 1250 uV. Register = limit_value_uV / 1250.
        // Integer round-to-nearest: (limit_value + half_lsb) / lsb.
        int32_t reg_val = (limit_value + ((int32_t)INA226_BUS_VOLTAGE_LSB_UV / 2)) / (int32_t)INA226_BUS_VOLTAGE_LSB_UV;

        if (reg_val > INT16_MAX) {
            return INA226_ERR_MATH_OVERFLOW;
        }

        alert_limit_reg_val = (uint16_t)reg_val;

    }

    else if (alert_func & INA226_ALERT_FUNC_POWER_CATEGORY_MASK) {

        if (limit_value < 0) {
            return INA226_ERR_INVALID_PARAM;
        }

        // Power alert: Convert limit from microwatts to register value.
        // Power LSB = 25 * Current_LSB (per datasheet). Register = limit_value_uW / power_lsb.
        int64_t power_lsb = 25U * (int64_t)sensor->current_resolution_uA;

        // Integer round-to-nearest: (limit_value + half_lsb) / lsb.
        int64_t reg_val_64 = ((int64_t)limit_value + (power_lsb / 2LL)) / power_lsb;

        if (reg_val_64 > (int64_t)UINT16_MAX) {
            return INA226_ERR_MATH_OVERFLOW;
        }

        alert_limit_reg_val = (uint16_t)reg_val_64;

    }

    else {

        // No recognized alert category bit is set. This means either:
        //  - The alert pin is configured as Conversion Ready (no limit needed), or
        //  - The Mask/Enable register contains a corrupted/unexpected value.
        // In both cases, writing to the alert limit register is not meaningful.
        return INA226_ERR_INVALID_STATE;

    }

    return INA226_Write_Reg(sensor->ina226_i2c_addr, INA226_ALERT_LIM_REG, alert_limit_reg_val);
}

INA226_Status_t INA226_Get_Alert_Status(const ina226_handle_t *sensor, INA226_Alert_Status_t *alert_status) {

    if (sensor == NULL || alert_status == NULL) return INA226_ERR_INVALID_PARAM;

    uint16_t mask_en_reg = 0;

    INA226_Status_t op_status = INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_MASK_EN_REG, &mask_en_reg);

    if (op_status != INA226_OK) {
        return op_status;
    }

    // Check Bit 4 (AFF: Alert Function Flag, 0x0010) and Bit 3 (CVRF: Conversion Ready Flag, 0x0008)
    // 0x0018U = AFF | CVRF
    if ((mask_en_reg & (INA226_MASK_EN_AFF_BIT | INA226_MASK_EN_CVRF_BIT)) == 0U) {
        // Neither AFF nor CVRF flag is set; no alert condition has occurred.
        (*alert_status) = INA226_ALERT_STATUS_NONE;
    }
    else if ((mask_en_reg & (INA226_MASK_EN_AFF_BIT | INA226_MASK_EN_CVRF_BIT)) == (INA226_MASK_EN_AFF_BIT | INA226_MASK_EN_CVRF_BIT)) {
        // Bit 3 (CVRF) and Bit 4 (AFF) are both set; alert was triggered due to Conversion Ready and configured alert function (e.g. over/under limit) was triggered.
        (*alert_status) = INA226_ALERT_STATUS_BOTH;
    }
    else if ((mask_en_reg & INA226_MASK_EN_AFF_BIT) != 0U) {
        // Bit 4 (AFF) is set; configured alert function (e.g. over/under limit) was triggered.
        (*alert_status) = INA226_ALERT_STATUS_LIMIT_EXCEEDED;
    }
    else {
        // Bit 3 (CVRF) is set; alert was triggered due to Conversion Ready.
        (*alert_status) = INA226_ALERT_STATUS_CONVERSION_READY;
    }
    return INA226_OK;
}

INA226_Status_t INA226_Set_Alert_Latch(const ina226_handle_t *sensor, INA226_Alert_Latch_t latch) {
    if (sensor == NULL) return INA226_ERR_INVALID_PARAM;
    if (latch > INA226_ALERT_LATCH_ENABLE) return INA226_ERR_INVALID_PARAM;

    uint16_t mask_en_reg = 0;
    INA226_Status_t op_status = INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_MASK_EN_REG, &mask_en_reg);

    if (op_status != INA226_OK) {
        return op_status;
    }

    if (latch == INA226_ALERT_LATCH_ENABLE) {
        mask_en_reg |= INA226_MASK_EN_LEN_BIT;
    } else {
        mask_en_reg &= (uint16_t)(~INA226_MASK_EN_LEN_BIT);
    }

    return INA226_Write_Reg(sensor->ina226_i2c_addr, INA226_MASK_EN_REG, mask_en_reg);
}

INA226_Status_t INA226_Calibrate(const ina226_handle_t *sensor) {
    
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

    if (cal == 0ULL || cal > INA226_CALIBRATION_MAX_VAL) {
        return INA226_ERR_INVALID_PARAM;
    }

    uint16_t calibration_reg_val = (uint16_t)cal;
    
    return INA226_Write_Reg(sensor->ina226_i2c_addr, INA226_CALIBRATION_REG, calibration_reg_val);
}

INA226_Status_t INA226_Read_Current(const ina226_handle_t *sensor, int32_t *current) {

    if (sensor == NULL || current == NULL) return INA226_ERR_INVALID_PARAM;

    uint16_t current_reg = 0U;
    
    INA226_Status_t op_status = INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_CURRENT_REG, &current_reg);

    if (op_status != INA226_OK) {
        return op_status;
    }

    // current_reg * current_lsb = current;
    int64_t resolution_uA = (int64_t)sensor->current_resolution_uA;
    int64_t curr64_uA = (int64_t)((int16_t)current_reg) * resolution_uA;
    
    if (curr64_uA > (int64_t)INT32_MAX) {
        return INA226_ERR_MATH_OVERFLOW;
    }
    else if (curr64_uA < (int64_t)INT32_MIN) {
        return INA226_ERR_MATH_OVERFLOW;
    }
    else {
        (*current) = (int32_t)curr64_uA;
    }

    return INA226_OK;
}

INA226_Status_t INA226_Read_Shunt_Voltage(const ina226_handle_t *sensor, int32_t *voltage) {

    if (sensor == NULL || voltage == NULL) return INA226_ERR_INVALID_PARAM;

    uint16_t shunt_voltage_reg = 0U;

    INA226_Status_t op_status = INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_SH_VOLTAGE_REG, &shunt_voltage_reg);

    if (op_status != INA226_OK) {
        return op_status;
    }

    int32_t raw = (int32_t)(int16_t)shunt_voltage_reg;

    int64_t scaled = (int64_t)raw * 5LL;
    int32_t voltage_uV = (int32_t)((scaled + (scaled >= 0 ? 1LL : -1LL)) / 2LL); // round-to-nearest

    (*voltage) = voltage_uV;

    return INA226_OK;
}

INA226_Status_t INA226_Read_Bus_Voltage(const ina226_handle_t *sensor, uint32_t *voltage) {

    if (sensor == NULL || voltage == NULL) return INA226_ERR_INVALID_PARAM;

    uint16_t bus_voltage_reg = 0U;
    
    INA226_Status_t op_status = INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_BUS_VOLTAGE_REG, &bus_voltage_reg);

    if (op_status != INA226_OK) {
        return op_status;
    }

    // Bus Voltage [uV] = Bus Voltage Register Value * Bus Voltage LSB [uV]
    (*voltage) = (uint32_t)bus_voltage_reg * INA226_BUS_VOLTAGE_LSB_UV;

    return INA226_OK;
}

INA226_Status_t INA226_Read_Power(const ina226_handle_t *sensor, uint32_t *power) {

    if (sensor == NULL || power == NULL) return INA226_ERR_INVALID_PARAM;

    uint16_t power_reg = 0U;
    
    INA226_Status_t op_status = INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_POWER_REG, &power_reg);

    if (op_status != INA226_OK) {
        return op_status;
    }


    int64_t power_lsb = 25U * (int64_t)sensor->current_resolution_uA;

    // Power [uW] = Power Register Value * Power LSB [uW]
    int64_t pwr64_uW = ((int64_t)power_reg * power_lsb);

    if (pwr64_uW > (int64_t)UINT32_MAX) {
        return INA226_ERR_MATH_OVERFLOW;
    }
    else if (pwr64_uW < 0) {
        return INA226_ERR_MATH_OVERFLOW;
    }
    else {
        (*power) = (uint32_t)pwr64_uW;
    }

    return INA226_OK;
}

INA226_Status_t INA226_Read_Manufacturer_ID(const ina226_handle_t *sensor, uint16_t *out_mfg_id) {

    if (sensor == NULL || out_mfg_id == NULL) {
        return INA226_ERR_INVALID_PARAM;
    }

    return INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_MANUFACTURER_ID_REG, out_mfg_id);
}

INA226_Status_t INA226_Read_Die_ID(const ina226_handle_t *sensor, uint16_t *out_die_id) {
    
    if (sensor == NULL || out_die_id == NULL) {
        return INA226_ERR_INVALID_PARAM;
    } 

    return INA226_Read_Reg(sensor->ina226_i2c_addr, INA226_DIE_ID_REG, out_die_id);
}
