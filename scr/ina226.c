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

#include "ina226.h"


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

    uint8_t buffer[2] = {0, 0};
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
    if (value == NULL) {
        return INA226_ERR_INVALID_PARAM;
    }

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

