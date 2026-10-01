/**
 * @file ina226_regs.h
 * @author Cafer Tuğra Çetin (cafercetin.tr@gmail.com)
 * @brief INA226 internal register definitions, masks, and bitfield positions.
 * @version 0.3
 * @date 2026-07-25
 * 
 * @copyright Copyright (c) 2026 Cafer Tuğra Çetin
 * SPDX-License-Identifier: MIT
 * 
 * Distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 * 
 */

#ifndef INA226_REGS_H_
#define INA226_REGS_H_

#include <stdint.h>

/* ========================================================================= */
/*                              REGISTER ADDRESSES                           */
/* ========================================================================= */

#define INA226_CONFIG_REG                   (0x00U) /**< Configuration Register (R/W). */
#define INA226_SH_VOLTAGE_REG               (0x01U) /**< Shunt Voltage Register (R). */
#define INA226_BUS_VOLTAGE_REG              (0x02U) /**< Bus Voltage Register (R). */
#define INA226_POWER_REG                    (0x03U) /**< Power Register (R). */
#define INA226_CURRENT_REG                  (0x04U) /**< Current Register (R). */
#define INA226_CALIBRATION_REG              (0x05U) /**< Calibration Register (R/W). */
#define INA226_MASK_EN_REG                  (0x06U) /**< Mask/Enable Register (R/W). */
#define INA226_ALERT_LIM_REG                (0x07U) /**< Alert Limit Register (R/W). */
#define INA226_MANUFACTURER_ID_REG          (0xFEU) /**< Manufacturer ID Register (R, 0x5449). */
#define INA226_DIE_ID_REG                   (0xFFU) /**< Die ID Register (R, 0x2260). */

/* Backward compatibility alias for previous spelling */
#define INA226_MANCUFACTURE_ID_REG          INA226_MANUFACTURER_ID_REG /**< Backward compatibility alias. */

/* ========================================================================= */
/*                              HARDWARE SCALING CONSTANTS                   */
/* ========================================================================= */

#define INA226_BUS_VOLTAGE_LSB_UV           (1250U) /**< Bus voltage LSB value in microvolts (1.25 mV). */

/* ========================================================================= */
/*                              REGISTER FIELD MASKS & POSITIONS             */
/* ========================================================================= */

#define INA226_CONFIG_RESET_MASK            (0x8000U) /**< Config register reset bitmask (bit 15). */

#define INA226_CONFIG_AVG_MASK              (0x0E00U) /**< Averaging mode bitmask (bits 11-9). */
#define INA226_CONFIG_AVG_POS               (9U)      /**< Averaging mode bit shift position. */

#define INA226_CONFIG_BUS_CT_MASK           (0x01C0U) /**< Bus conversion time bitmask (bits 8-6). */
#define INA226_CONFIG_BUS_CT_POS            (6U)      /**< Bus conversion time bit shift position. */

#define INA226_CONFIG_SHUNT_CT_MASK         (0x0038U) /**< Shunt conversion time bitmask (bits 5-3). */
#define INA226_CONFIG_SHUNT_CT_POS          (3U)      /**< Shunt conversion time bit shift position. */

#define INA226_CONFIG_MODE_MASK             (0x0007U) /**< Operating mode bitmask (bits 2-0). */
#define INA226_CONFIG_MODE_POS              (0U)      /**< Operating mode bit shift position. */

#define INA226_MASK_ENABLE_ALERT_FUNC_MASK  (0xFC00U) /**< Mask/Enable alert function bitmask (bits 15-10). */
#define INA226_MASK_ENABLE_ALERT_FUNC_POS   (10U)     /**< Mask/Enable alert function bit shift position. */

/* ========================================================================= */
/*                              ALERT FUNCTION CATEGORY MASKS                */
/* ========================================================================= */

#define INA226_ALERT_FUNC_MAIN_BITS_MASK      (0x3EU)  /**< Main alert function bits mask (bits 5-1). */
#define INA226_ALERT_FUNC_SHUNT_CATEGORY_MASK (0x30U)  /**< Shunt voltage alert category mask (bits 5-4). */
#define INA226_ALERT_FUNC_BUS_CATEGORY_MASK   (0x0CU)  /**< Bus voltage alert category mask (bits 3-2). */
#define INA226_ALERT_FUNC_POWER_CATEGORY_MASK (0x02U)  /**< Power alert category mask (bit 1). */

/* ========================================================================= */
/*                              MASK / ENABLE FLAG BITS                      */
/* ========================================================================= */

#define INA226_MASK_EN_AFF_BIT              (0x0010U) /**< Alert Function Flag (AFF, bit 4). */
#define INA226_MASK_EN_CVRF_BIT             (0x0008U) /**< Conversion Ready Flag (CVRF, bit 3). */
#define INA226_MASK_EN_LEN_BIT              (0x0001U) /**< Latch Enable bit (LEN, bit 0). */

#endif /* INA226_REGS_H_ */
