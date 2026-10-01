/**
 * @file ina226_regs.h
 * @author Cafer Tuğra Çetin (cafercetin.tr@gmail.com)
 * @brief INA226 internal register definitions, masks, and bitfield positions.
 * @version 0.3
 * 
 * @copyright Copyright (c) 2026 Cafer Tuğra Çetin
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

#define INA226_CONFIG_REG                   (0x00U)
#define INA226_SH_VOLTAGE_REG               (0x01U)
#define INA226_BUS_VOLTAGE_REG              (0x02U)
#define INA226_POWER_REG                    (0x03U)
#define INA226_CURRENT_REG                  (0x04U)
#define INA226_CALIBRATION_REG              (0x05U)
#define INA226_MASK_EN_REG                  (0x06U)
#define INA226_ALERT_LIM_REG                (0x07U)
#define INA226_MANUFACTURER_ID_REG          (0xFEU)
#define INA226_DIE_ID_REG                   (0xFFU)

/* Backward compatibility alias for previous spelling */
#define INA226_MANCUFACTURE_ID_REG          INA226_MANUFACTURER_ID_REG

/* ========================================================================= */
/*                              HARDWARE SCALING CONSTANTS                   */
/* ========================================================================= */

#define INA226_BUS_VOLTAGE_LSB_UV           (1250U)

/* ========================================================================= */
/*                              REGISTER FIELD MASKS & POSITIONS             */
/* ========================================================================= */

#define INA226_CONFIG_RESET_MASK            (0x8000U)

#define INA226_CONFIG_AVG_MASK              (0x0E00U) /* Bits 11-9 */
#define INA226_CONFIG_AVG_POS               (9U)

#define INA226_CONFIG_BUS_CT_MASK           (0x01C0U) /* Bits 8-6 */
#define INA226_CONFIG_BUS_CT_POS            (6U)

#define INA226_CONFIG_SHUNT_CT_MASK         (0x0038U) /* Bits 5-3 */
#define INA226_CONFIG_SHUNT_CT_POS          (3U)

#define INA226_CONFIG_MODE_MASK             (0x0007U) /* Bits 2-0 */
#define INA226_CONFIG_MODE_POS              (0U)

#define INA226_MASK_ENABLE_ALERT_FUNC_MASK  (0xFC00U) /* Bits 15-10 */
#define INA226_MASK_ENABLE_ALERT_FUNC_POS   (10U)

/* ========================================================================= */
/*                              ALERT FUNCTION CATEGORY MASKS                */
/* ========================================================================= */

#define INA226_ALERT_FUNC_MAIN_BITS_MASK      (0x3EU)  /* Bits 5-1: All main alert source bits */
#define INA226_ALERT_FUNC_SHUNT_CATEGORY_MASK (0x30U)  /* Bits 5-4: Shunt voltage alert sources */
#define INA226_ALERT_FUNC_BUS_CATEGORY_MASK   (0x0CU)  /* Bits 3-2: Bus voltage alert sources */
#define INA226_ALERT_FUNC_POWER_CATEGORY_MASK (0x02U)  /* Bit 1:    Power over-limit alert source */

/* ========================================================================= */
/*                              MASK / ENABLE FLAG BITS                      */
/* ========================================================================= */

#define INA226_MASK_EN_AFF_BIT              (0x0010U)
#define INA226_MASK_EN_CVRF_BIT             (0x0008U)
#define INA226_MASK_EN_LEN_BIT              (0x0001U)

#endif /* INA226_REGS_H_ */
