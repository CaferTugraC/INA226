#ifndef INA226_H_
#define INA226_H_

#include <stdint.h>

// 1. Alert Okuma Durumları (8-bit)
typedef enum : uint8_t {
    INA226_ALERT_NO_ALERT                      = 0,
    INA226_ALERT_SHUNT_VOLTAGE_OVER_LIMIT      = 1,
    INA226_ALERT_SHUNT_VOLTAGE_UNDER_LIMIT     = 2,
    INA226_ALERT_BUS_VOLTAGE_OVER_LIMIT        = 3,
    INA226_ALERT_BUS_VOLTAGE_UNDER_LIMIT       = 4,
    INA226_ALERT_POWER_OVER_LIMIT              = 5,
    INA226_ALERT_CONVERSION_READY              = 6
} INA226_Alert_Status_t;

// 2. Dönüşüm Süreleri (Register bitiyle birebir eşleşir)
typedef enum : uint8_t {
    INA226_CT_140_US  = 0,
    INA226_CT_204_US  = 1,
    INA226_CT_332_US  = 2,
    INA226_CT_588_US  = 3,
    INA226_CT_1100_US = 4,
    INA226_CT_2116_US = 5,
    INA226_CT_4156_US = 6,
    INA226_CT_8244_US = 7 
} INA226_Conv_Time_t;

// 3. Örnekleme / Ortalamalar (Register bitiyle birebir eşleşir)
typedef enum : uint8_t {
    INA226_AVG_1    = 0,
    INA226_AVG_4    = 1,
    INA226_AVG_16   = 2,
    INA226_AVG_64   = 3,
    INA226_AVG_128  = 4,
    INA226_AVG_256  = 5,
    INA226_AVG_512  = 6,
    INA226_AVG_1024 = 7
} INA226_Avg_Time_t;

// 4. Çalışma Modları (Register bitiyle birebir eşleşir)
typedef enum : uint8_t {
    INA226_SHUT_DOWN                        = 0,
    INA226_TRIGGERED_SHUNT_VOLTAGE          = 1,
    INA226_TRIGGERED_BUS_VOLTAGE            = 2,
    INA226_TRIGGERED_BUS_AND_SHUNT_VOLTAGE  = 3,
    INA226_CONTINUOUS_SHUNT_VOLTAGE         = 5, 
    INA226_CONTINUOUS_BUS_VOLTAGE           = 6,
    INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE = 7  
} INA226_Mode_t;

// 5. Alert Fonksiyon Seçimi
typedef enum : uint8_t {
    INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT  = 0,
    INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT = 1,
    INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT    = 2,
    INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT   = 3,
    INA226_ALERT_FUNC_POWER_OVER_LIMIT          = 4,
    INA226_ALERT_FUNC_CONVERSION_READY          = 5
} INA226_Alert_Func_t;


#endif /* INA226_H_ */