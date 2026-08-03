#ifndef TEST_INA226_H_
#define TEST_INA226_H_

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

#define INA226_CONFIG_RESET_MASK          (0x8000U)
#define INA226_CONFIG_RESET_POS           (15U)

#define INA226_CONFIG_AVG_MASK            (0x0E00U) // Bits 11-9 (0000 1110 0000 0000)
#define INA226_CONFIG_AVG_POS             (9U)

#define INA226_CONFIG_BUS_CT_MASK         (0x01C0U) // Bits 8-6 (0000 0001 1100 0000)
#define INA226_CONFIG_BUS_CT_POS          (6U)

#define INA226_CONFIG_SHUNT_CT_MASK       (0x0038U) // Bits 5-3 (0000 0000 0011 1000)
#define INA226_CONFIG_SHUNT_CT_POS        (3U)

#define INA226_CONFIG_MODE_MASK           (0x0007U) // Bits 2-0 (0000 0000 0000 0111)
#define INA226_CONFIG_MODE_POS            (0U)

#define INA226_MASK_ENABLE_ALERT_FUNC_MASK   (0xFC00U) // Bits 15-10
#define INA226_MASK_ENABLE_ALERT_FUNC_POS    (10U)


#define INA226_ALERT_FUNC_MAIN_BITS_MASK      (0x3EU)  // Bits 5-1: All main alert source bits
#define INA226_ALERT_FUNC_SHUNT_CATEGORY_MASK (0x30U)  // Bits 5-4: Shunt voltage alert sources
#define INA226_ALERT_FUNC_BUS_CATEGORY_MASK   (0x0CU)  // Bits 3-2: Bus voltage alert sources
#define INA226_ALERT_FUNC_POWER_CATEGORY_MASK (0x02U)  // Bit 1:    Power over-limit alert source

#define INA226_MASK_EN_AFF_BIT                  (0x0010U)
#define INA226_MASK_EN_CVRF_BIT                 (0x0008U)


#endif /* TEST_INA226_H_ */