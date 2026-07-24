#ifndef INA226_H_
#define INA226_H_

#include <stdint.h>

// --- Platform-specific I2C functions to be implemented by the user ---
extern uint8_t INA226_Platform_I2C_Write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);
extern uint8_t INA226_Platform_I2C_Read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

/* ========================================================================= */
/*                              TYPES & DEFINES                              */
/* ========================================================================= */

// 1. Library Function Return Statuses
typedef uint8_t INA226_Status_t;
#define INA226_OK                   ((INA226_Status_t)0U)
#define INA226_ERR_I2C              ((INA226_Status_t)1U)
#define INA226_ERR_INVALID_PARAM    ((INA226_Status_t)2U)

// 2. Alert Read Statuses
typedef uint8_t INA226_Alert_Status_t;
#define INA226_ALERT_NO_ALERT                   ((INA226_Alert_Status_t)0U)
#define INA226_ALERT_SHUNT_VOLTAGE_OVER_LIMIT   ((INA226_Alert_Status_t)1U)
#define INA226_ALERT_SHUNT_VOLTAGE_UNDER_LIMIT  ((INA226_Alert_Status_t)2U)
#define INA226_ALERT_BUS_VOLTAGE_OVER_LIMIT     ((INA226_Alert_Status_t)3U)
#define INA226_ALERT_BUS_VOLTAGE_UNDER_LIMIT    ((INA226_Alert_Status_t)4U)
#define INA226_ALERT_POWER_OVER_LIMIT           ((INA226_Alert_Status_t)5U)
#define INA226_ALERT_CONVERSION_READY           ((INA226_Alert_Status_t)6U)

// 3. Conversion Times (Matches register bits directly)
typedef uint8_t INA226_Conv_Time_t;
#define INA226_CT_140_US            ((INA226_Conv_Time_t)0U)
#define INA226_CT_204_US            ((INA226_Conv_Time_t)1U)
#define INA226_CT_332_US            ((INA226_Conv_Time_t)2U)
#define INA226_CT_588_US            ((INA226_Conv_Time_t)3U)
#define INA226_CT_1100_US           ((INA226_Conv_Time_t)4U)
#define INA226_CT_2116_US           ((INA226_Conv_Time_t)5U)
#define INA226_CT_4156_US           ((INA226_Conv_Time_t)6U)
#define INA226_CT_8244_US           ((INA226_Conv_Time_t)7U)

// 4. Averaging Modes (Matches register bits directly)
typedef uint8_t INA226_Avg_Time_t;
#define INA226_AVG_1                ((INA226_Avg_Time_t)0U)
#define INA226_AVG_4                ((INA226_Avg_Time_t)1U)
#define INA226_AVG_16               ((INA226_Avg_Time_t)2U)
#define INA226_AVG_64               ((INA226_Avg_Time_t)3U)
#define INA226_AVG_128              ((INA226_Avg_Time_t)4U)
#define INA226_AVG_256              ((INA226_Avg_Time_t)5U)
#define INA226_AVG_512              ((INA226_Avg_Time_t)6U)
#define INA226_AVG_1024             ((INA226_Avg_Time_t)7U)

// 5. Operating Modes (Matches register bits directly)
typedef uint8_t INA226_Mode_t;
#define INA226_SHUT_DOWN                        ((INA226_Mode_t)0U)
#define INA226_TRIGGERED_SHUNT_VOLTAGE          ((INA226_Mode_t)1U)
#define INA226_TRIGGERED_BUS_VOLTAGE            ((INA226_Mode_t)2U)
#define INA226_TRIGGERED_BUS_AND_SHUNT_VOLTAGE  ((INA226_Mode_t)3U)
#define INA226_SHUT_DOWN_ALT                     ((INA226_Mode_t)4U)
#define INA226_CONTINUOUS_SHUNT_VOLTAGE         ((INA226_Mode_t)5U)
#define INA226_CONTINUOUS_BUS_VOLTAGE           ((INA226_Mode_t)6U)
#define INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE ((INA226_Mode_t)7U)

// 6. Alert Pin Functions (Matches register bit positions directly)
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

// Configuration Register Operations
INA226_Status_t INA226_Reset(uint8_t addr);
INA226_Status_t INA226_Set_Shunt_Voltage_Conversion_Time(uint8_t addr, INA226_Conv_Time_t conv_time);
INA226_Status_t INA226_Set_Bus_Voltage_Conversion_Time(uint8_t addr, INA226_Conv_Time_t conv_time);
INA226_Status_t INA226_Set_Operating_Mode(uint8_t addr, INA226_Mode_t mode);
INA226_Status_t INA226_Set_Averaging_Mode(uint8_t addr, INA226_Avg_Time_t avg_time);

// Calibration
INA226_Status_t INA226_Set_Calibration_Reg(uint8_t addr, uint16_t cal_reg_value);

// Alert Operations
INA226_Status_t INA226_Set_Alert_Pin_Function(uint8_t addr, INA226_Alert_Func_t alert_func);
INA226_Status_t INA226_Set_Alert_Limit(uint8_t addr, uint16_t limit_value);
INA226_Status_t INA226_Get_Alert_Status(uint8_t addr, INA226_Alert_Status_t *alert_status);

// Read Measurements (Values are returned via pointer based on their type)
INA226_Status_t INA226_Read_Current(uint8_t addr, int16_t *current);
INA226_Status_t INA226_Read_Shunt_Voltage(uint8_t addr, int16_t *voltage);
INA226_Status_t INA226_Read_Bus_Voltage(uint8_t addr, uint16_t *voltage);
INA226_Status_t INA226_Read_Power(uint8_t addr, uint16_t *power);

#endif /* INA226_H_ */