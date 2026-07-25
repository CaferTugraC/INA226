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
/*                                 FUNCTIONES                                */
/* ========================================================================= */


