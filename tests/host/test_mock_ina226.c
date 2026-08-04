#include "test_ina226.h"
#include "unity.h"
#include "ina226.h"
#include <stddef.h>
#include <stdbool.h>

// Host-side mock register storage for Unity tests.
static uint16_t mock_ina226_registers[256];

// Directly seed a mock register value when a test needs a specific device state.
void Mock_I2C_Set_Register(uint8_t reg_addr, uint16_t value) {
    mock_ina226_registers[reg_addr] = value;
}

// Restore the mock device to its default power-on state before each test.
void Mock_I2C_Reset(void) {

    for (uint32_t i = 8; i < 254; i++) mock_ina226_registers[i] = 0;

        // Set default register values.
    mock_ina226_registers[INA226_CONFIG_REG] = 0x4127;
    mock_ina226_registers[INA226_SH_VOLTAGE_REG] = 0;
    mock_ina226_registers[INA226_BUS_VOLTAGE_REG] = 0;
    mock_ina226_registers[INA226_POWER_REG] = 0;
    mock_ina226_registers[INA226_CURRENT_REG] = 0;
    mock_ina226_registers[INA226_CALIBRATION_REG] = 0;
    mock_ina226_registers[INA226_MASK_EN_REG] = 0;
    mock_ina226_registers[INA226_ALERT_LIM_REG] = 0;
    mock_ina226_registers[INA226_MANCUFACTURE_ID_REG] = 0x5449;
    mock_ina226_registers[INA226_DIE_ID_REG] = 0x2260;
}

uint8_t INA226_Platform_I2C_Write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len) {

    (void)dev_addr;

    if (len == 2) {

        mock_ina226_registers[reg_addr] = (uint16_t)((data[0] << 8U) | data[1]);

        return INA226_OK;
    }
        
    return INA226_ERR_I2C;
}

uint8_t INA226_Platform_I2C_Read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {

    (void)dev_addr;

    if (len == 2) {

        uint16_t val = mock_ina226_registers[reg_addr];

        data[0] = (uint8_t)(val >> 8);
        data[1] = (uint8_t)(val & 0xFF);

        return INA226_OK;
    }
    
    return INA226_ERR_I2C;
}

void setUp(void) {

    Mock_I2C_Reset();
}

void tearDown(void) {}

// Tests for INA226_Read_Current function
void test_INA226_Read_Current_Should_Return_Error_On_Invalid_Params(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40, .current_resolution_uA = 100 };
    int32_t current_val = 0;

    // Sensor pointer is NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Current(NULL, &current_val));

    // Output pointer is NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Current(&sensor, NULL));

    // Both parameters are NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Current(NULL, NULL));
}

void test_INA226_Read_Current_Should_Detect_Math_Overflow(void) {

    // INT32_MAX = 2,147,483,647 
    // (100,000 uA = 100mA)
    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40, .current_resolution_uA = 100000 };
    int32_t current_val = 0;

    // 1. Positive overflow case (> INT32_MAX).
    // Maximum positive register value: 0x7FFF (32767).
    // 32767 * 100000 = 3,276,700,000 (greater than INT32_MAX).
    mock_ina226_registers[INA226_CURRENT_REG] = 0x7FFF; 
    
    TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Read_Current(&sensor, &current_val));

    // 2. Negative overflow case (< INT32_MIN).
    // Minimum negative register value: 0x8000 (-32768).
    // -32768 * 100000 = -3,276,800,000 (less than INT32_MIN).
    mock_ina226_registers[INA226_CURRENT_REG] = 0x8000;
    
    TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Read_Current(&sensor, &current_val));
}

void test_INA226_Read_Current_Should_Calculate_Correct_Values(void) {
    
    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40, .current_resolution_uA = 500 }; // Current LSB is 500 uA.
    int32_t current_val = 0;

    // 1. Positive value read.
    // Register = 100 (0x0064) -> Expected: 100 * 500 = 50,000 uA.
    mock_ina226_registers[INA226_CURRENT_REG] = 100;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Current(&sensor, &current_val),
        "INA226_Read_Current not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32(50000, current_val);

    // 2. Negative value read (sign extension test).
    // Register = 0xFFFE (-2) -> Expected: -2 * 500 = -1000 uA.
    // This verifies the int16_t cast used for sign extension.
    mock_ina226_registers[INA226_CURRENT_REG] = (uint16_t)((int16_t)-2);
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Current(&sensor, &current_val),
        "INA226_Read_Current not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32(-1000, current_val);

    // 3. Zero value read.
    // Register = 0 -> Expected: 0 uA.
    mock_ina226_registers[INA226_CURRENT_REG] = 0;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Current(&sensor, &current_val),
        "INA226_Read_Current not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32(0, current_val);
}


// Tests for INA226_Read_Power function
void test_INA226_Read_Power_Should_Return_Error_On_Invalid_Params(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40, .current_resolution_uA = 100 };
    uint32_t power_val = 0;

    // Sensor pointer is NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Power(NULL, &power_val));

    // Output pointer is NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Power(&sensor, NULL));

    // Both parameters are NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Power(NULL, NULL));

    // Valid input.
    mock_ina226_registers[INA226_POWER_REG] = 0x0000;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Power(&sensor, &power_val));
}

void test_INA226_Read_Power_Should_Detect_Math_Overflow(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40, .current_resolution_uA = 100000 };
    uint32_t power_val = 0;

    // 1. Positive overflow case (> UINT32_MAX).
    // Power register = 0xFFFF, power LSB = 2,500,000 uW.
    // 65535 * 2,500,000 = 163,837,500,000, which exceeds UINT32_MAX.
    mock_ina226_registers[INA226_POWER_REG] = 0xFFFF;
    TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Read_Power(&sensor, &power_val));
}

void test_INA226_Read_Power_Should_Calculate_Correct_Values(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40, .current_resolution_uA = 500 };
    uint32_t power_val = 0;

    // 1. Positive value read.
    // Register = 100 (0x0064) -> Expected: 100 * (25 * 500) = 1,250,000 uW.
    mock_ina226_registers[INA226_POWER_REG] = 100;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Power(&sensor, &power_val),
        "INA226_Read_Power not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_UINT32(1250000U, power_val);

    // 2. Zero value read.
    // Register = 0 -> Expected: 0 uW.
    mock_ina226_registers[INA226_POWER_REG] = 0;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Power(&sensor, &power_val),
        "INA226_Read_Power not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_UINT32(0U, power_val);
}

// Tests for INA226_Read_Shunt_Voltage
void test_INA226_Read_Shunt_Voltage_Should_Return_Error_On_Invalid_Params(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };
    int32_t sh_voltage = 0;

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Shunt_Voltage(NULL, NULL));

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Shunt_Voltage(NULL, &sh_voltage));

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Shunt_Voltage(&sensor, NULL));

}

void test_INA226_Read_Shunt_Voltage_Should_Calculate_Correct_Values(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };
    int32_t sh_voltage = 0;

    mock_ina226_registers[INA226_SH_VOLTAGE_REG] = -1;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Shunt_Voltage(&sensor, &sh_voltage),
        "INA226_Read_Shunt_Voltage not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32(-3, sh_voltage);

    mock_ina226_registers[INA226_SH_VOLTAGE_REG] = 1;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Shunt_Voltage(&sensor, &sh_voltage),
        "INA226_Read_Shunt_Voltage not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32(3, sh_voltage);

    mock_ina226_registers[INA226_SH_VOLTAGE_REG] = 2;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Shunt_Voltage(&sensor, &sh_voltage),
        "INA226_Read_Shunt_Voltage not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32(5, sh_voltage);
}

// Tests for INA226_Read_Bus_Voltage
void test_INA226_Read_Bus_Voltage_Should_Return_Error_On_Invalid_Params(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };
    uint32_t bus_voltage = 0;

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Bus_Voltage(NULL, NULL));

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Bus_Voltage(NULL, &bus_voltage));

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Bus_Voltage(&sensor, NULL));

}

void test_INA226_Read_Bus_Voltage_Should_Calculate_Correct_Values(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };
    int32_t bus_voltage = 0;

    mock_ina226_registers[INA226_BUS_VOLTAGE_REG] = 100;
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Read_Shunt_Voltage(&sensor, &bus_voltage),
        "INA226_Read_Shunt_Voltage not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32((100 * 1250), bus_voltage);

    mock_ina226_registers[INA226_BUS_VOLTAGE_REG] = 0;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Shunt_Voltage(&sensor, &bus_voltage),
        "INA226_Read_Shunt_Voltage not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32(0, bus_voltage);
}

// Tests for INA226_Set_Alert_Pin_Function
void test_INA226_Set_Alert_Pin_Function_Should_Return_Error_On_Invalid_Params(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };
    INA226_Alert_Func_t alert_func = INA226_ALERT_FUNC_CONVERSION_READY;

    // NULL Test
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Alert_Pin_Function(NULL, alert_func));

    // Non-Valid alert pin function test
    for (uint32_t i = 0; i < 0xFFFF; i++) {

        bool is_valid = false;

        switch (i) {
            case INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT:
            case INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT:
            case INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT:
            case INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT:
            case INA226_ALERT_FUNC_POWER_OVER_LIMIT:
            case INA226_ALERT_FUNC_CONVERSION_READY:
            case INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT_CVR:
            case INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT_CON_READY_CVR:
            case INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT_CON_READY_CVR:
            case INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT_CON_READY_CVR:
            case INA226_ALERT_FUNC_POWER_OVER_LIMIT_CON_READY_CVR:
                is_valid = true;
                break;
            default:
                is_valid = false;
                break;
        }

        if(!is_valid) {
            TEST_ASSERT_EQUAL_MESSAGE(
                INA226_ERR_INVALID_PARAM,
                INA226_Set_Alert_Pin_Function(&sensor, (INA226_Alert_Func_t)i),
                "Set alert function return INA226_OK with invalid alert function option!"
            );
        }
    }
}

void test_INA226_Set_Alert_Pin_Function_Should_Do_Correct_Bitwise_Operation(void) {
    
    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };

    INA226_Alert_Func_t valid_functions[] = {
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT,
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT,
        INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT,
        INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT,
        INA226_ALERT_FUNC_POWER_OVER_LIMIT,
        INA226_ALERT_FUNC_CONVERSION_READY,
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT_CVR,
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT_CON_READY_CVR,
        INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT_CON_READY_CVR,
        INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT_CON_READY_CVR,
        INA226_ALERT_FUNC_POWER_OVER_LIMIT_CON_READY_CVR
    };

    size_t num_functions = sizeof(valid_functions) / sizeof(valid_functions[0]);

    for (size_t i = 0; i < num_functions; i++) {

        INA226_Alert_Func_t current_func = valid_functions[i];

        INA226_Status_t status = INA226_Set_Alert_Pin_Function(&sensor, current_func);
        TEST_ASSERT_EQUAL_MESSAGE(INA226_OK, status, "INA226_Set_Alert_Pin_Function not return INA226_OK.");

        uint16_t mask_en_reg = mock_ina226_registers[INA226_MASK_EN_REG];
        
        uint16_t extracted_func = (mask_en_reg) >> INA226_MASK_ENABLE_ALERT_FUNC_POS;

        TEST_ASSERT_EQUAL_HEX16(current_func, extracted_func);
    }
}

// Tests for INA226_Get_Alert_Pin_Function
void test_INA226_Get_Alert_Pin_Function_Should_Return_Error_On_Invalid_Params(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };
    INA226_Alert_Func_t alert_func;

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Pin_Function(NULL, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Pin_Function(NULL, &alert_func));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Pin_Function(&sensor, NULL));
}

void test_INA226_Get_Alert_Pin_Function_Should_Read_Correct_Alert_Function_Option(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };
    INA226_Alert_Func_t read_alert_func;

    INA226_Alert_Func_t valid_functions[] = {
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT,
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT,
        INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT,
        INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT,
        INA226_ALERT_FUNC_POWER_OVER_LIMIT,
        INA226_ALERT_FUNC_CONVERSION_READY,
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT_CVR,
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT_CON_READY_CVR,
        INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT_CON_READY_CVR,
        INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT_CON_READY_CVR,
        INA226_ALERT_FUNC_POWER_OVER_LIMIT_CON_READY_CVR
    };

    size_t num_functions = sizeof(valid_functions) / sizeof(valid_functions[0]);

    for (size_t i = 0; i < num_functions; i++) {

        INA226_Alert_Func_t actual_alert_func = valid_functions[i];

        uint16_t mask_en_reg = mock_ina226_registers[INA226_MASK_EN_REG];

        mask_en_reg &= ~(INA226_MASK_ENABLE_ALERT_FUNC_MASK);
        mask_en_reg |= (actual_alert_func << INA226_MASK_ENABLE_ALERT_FUNC_POS);
        mock_ina226_registers[INA226_MASK_EN_REG] = mask_en_reg;
    
        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Get_Alert_Pin_Function(&sensor, &read_alert_func),
            "INA226_Get_Alert_Pin_Function not return INA226_OK."
        );

        TEST_ASSERT_EQUAL(actual_alert_func, read_alert_func);
    }
    
}

// Tests for INA226_Set_Alert_Limit
void test_INA226_Set_Alert_Limit_Should_Return_Error_On_Invalid_Params(void) {

    int32_t limit_value = 0;

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Alert_Limit(NULL, limit_value));
}

void test_INA226_Set_Alert_Limit_Should_Return_Error_Math_Overflow_For_Invalid_Shunt_Limit_Values(void) {
    
    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };
    int32_t invalid_limit_value;

    // Shunt category limit value tests
    INA226_Alert_Func_t shunt_category_alert_functions[] = {
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT,
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT,
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT_CVR,
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT_CON_READY_CVR
    };

    size_t num_category_alert_functions = sizeof(shunt_category_alert_functions) / sizeof(shunt_category_alert_functions[0]);

    for (size_t i = 0; i < num_category_alert_functions; i++) {

        INA226_Alert_Func_t alert_func = shunt_category_alert_functions[i];

        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Set_Alert_Pin_Function(&sensor, alert_func),
            "INA226_Set_Alert_Pin_Function not return INA226_OK."
        );

        invalid_limit_value = (5 * (INT16_MAX + 1) + 1) / 2; // Maximum value of the shunt voltage register + 1.
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, invalid_limit_value));

        invalid_limit_value = (5 * (INT16_MIN - 1) - 1) / 2; // Minimum value of the shunt voltage register - 1.
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, invalid_limit_value));
    }
}

void test_INA226_Set_Alert_Limit_Should_Return_Error_Math_Overflow_For_Invalid_Bus_Limit_Values(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };
    int32_t invalid_limit_value;

    // Bus category limit value tests
    INA226_Alert_Func_t bus_category_alert_functions[] = {
        INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT,
        INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT,
        INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT_CON_READY_CVR,
        INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT_CON_READY_CVR
    };

    size_t num_category_alert_functions = sizeof(bus_category_alert_functions) / sizeof(bus_category_alert_functions[0]);

    for (size_t i = 0; i < num_category_alert_functions; i++){

        INA226_Alert_Func_t alert_func = bus_category_alert_functions[i];
        
        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Set_Alert_Pin_Function(&sensor, alert_func),
            "INA226_Set_Alert_Pin_Function not return INA226_OK."
        );

        invalid_limit_value = (INT16_MAX + 1) * INA226_BUS_VOLTAGE_LSB_UV; // Maximum value of the bus voltage register + 1.
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, invalid_limit_value));

        invalid_limit_value = -1 * INA226_BUS_VOLTAGE_LSB_UV; // Minimum value of the bus voltage register - 1.
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, invalid_limit_value));
    }
}

void test_INA226_Set_Alert_Limit_Should_Return_Error_Math_Overflow_For_Invalid_Power_Limit_Values(void) {
    
    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };
    int32_t invalid_limit_value;

    // Power category limit value tests
    sensor.current_resolution_uA = 100;

    INA226_Alert_Func_t power_category_alert_functions[] = {
        INA226_ALERT_FUNC_POWER_OVER_LIMIT,
        INA226_ALERT_FUNC_POWER_OVER_LIMIT_CON_READY_CVR
    };

    size_t num_category_alert_functions = sizeof(power_category_alert_functions) / sizeof(power_category_alert_functions[0]);

    for (size_t i = 0; i < num_category_alert_functions; i++) {

        INA226_Alert_Func_t alert_func = power_category_alert_functions[i];
        
        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Set_Alert_Pin_Function(&sensor, alert_func),
            "INA226_Set_Alert_Pin_Function not return INA226_OK."
        );

        int32_t power_lsb = 25 * sensor.current_resolution_uA;

        invalid_limit_value = (UINT16_MAX + 1) * power_lsb; // Maximum value of the power register + 1.
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, invalid_limit_value));

        invalid_limit_value = -1 * power_lsb; // Minimum value of the power register - 1.
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, invalid_limit_value));
    }
}

void test_INA226_Set_Alert_Limit_Should_Write_Correct_Limit_Value_To_Register_For_Shunt(void) {
    
    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };

    // shunt
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Pin_Function(&sensor, INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT),
        "INA226_Set_Alert_Pin_Function not return INA226_OK."
    );

    // max
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 81917), // (81917 * 2) / 5 = 32766.8 -> 32767 (INT16_MAX)
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(INT16_MAX, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // min
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, -81920), // -81920 uV -> (-81920 * 2) / 5 = -32768 (INT16_MIN)
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    // 0x8000 (two's complement)
    TEST_ASSERT_EQUAL_HEX16((uint16_t)INT16_MIN, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // midle
     TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 2500), // 2500 uV -> (2500 * 2) / 5 = 1000 to register
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(1000, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // round to nearest    
     TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 12), // (12 * 2 + 2) / 5 = 5.2 -> 5 to register.
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(5, mock_ina226_registers[INA226_ALERT_LIM_REG]);
}

void test_INA226_Set_Alert_Limit_Should_Write_Correct_Limit_Value_To_Register_For_Bus(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };

    // Bus
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Pin_Function(&sensor, INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT),
        "INA226_Set_Alert_Pin_Function not return INA226_OK."
    );

    // max
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 40958750), // 32767 (INT16_MAX) * 1250 uV = 40,958,750 uV
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(INT16_MAX, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // min
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 0), // 0 uV -> 0 * 1250 uV = 0
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(0, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // midle
     TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 5000000), // 5000000 uV ->  5000000 / 1250 uV = 4000 to register
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(4000, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // round to nearest    
     TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 2000), // // 2000 uV / 1250 = 1.6 -> 2 to register.
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(2, mock_ina226_registers[INA226_ALERT_LIM_REG]);
}

void test_INA226_Set_Alert_Limit_Should_Write_Correct_Limit_Value_To_Register_For_Power(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40, .current_resolution_uA = 100 };

    // Power
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Pin_Function(&sensor, INA226_ALERT_FUNC_POWER_OVER_LIMIT),
        "INA226_Set_Alert_Pin_Function not return INA226_OK."
    );

    // max
    
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 163837500), // 65535 (UINT16_MAX) * (25 * 100 (current_resolution_uA)) = 163.837.500 uW
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // min
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 0),
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(0, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // midle
     TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 2500000), // 1000 * (25 * 100 (current_resolution_uA)) = 2.500.000 uW
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(1000, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // round to nearest    
     TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 4000), // 1.6 * (25 * 100 (current_resolution_uA)) = 4.000 uW -> 2 to register.
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(2, mock_ina226_registers[INA226_ALERT_LIM_REG]);
}


// Tests for INA226_Get_Alert_Status
void test_INA226_Get_Alert_Status_Should_Return_Error_On_Invalid_Params(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };
    INA226_Alert_Status_t alert_status;

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Status(NULL, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Status(NULL, &alert_status));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Status(&sensor, NULL));
}

void test_INA226_Get_Alert_Status_Should_Read_Correct_Alert_Status(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40 };
    INA226_Alert_Status_t alert_status;

    mock_ina226_registers[INA226_MASK_EN_REG] = (0xFFFF & INA226_MASK_EN_AFF_BIT);
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Get_Alert_Status(&sensor, &alert_status),
        "INA226_Get_Alert_Status not return INA226_OK."
    );
    TEST_ASSERT_EQUAL(INA226_ALERT_STATUS_LIMIT_EXCEEDED, alert_status);

    mock_ina226_registers[INA226_MASK_EN_REG] = (0xFFFF & (INA226_MASK_EN_AFF_BIT | INA226_MASK_EN_CVRF_BIT));
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Get_Alert_Status(&sensor, &alert_status),
        "INA226_Get_Alert_Status not return INA226_OK."
    );
    TEST_ASSERT_EQUAL(INA226_ALERT_STATUS_BOTH, alert_status);

    mock_ina226_registers[INA226_MASK_EN_REG] = (0xFFFF & INA226_MASK_EN_CVRF_BIT);
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Get_Alert_Status(&sensor, &alert_status),
        "INA226_Get_Alert_Status not return INA226_OK."
    );
    TEST_ASSERT_EQUAL(INA226_ALERT_STATUS_CONVERSION_READY, alert_status);

    mock_ina226_registers[INA226_MASK_EN_REG] = (0x0000);
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Get_Alert_Status(&sensor, &alert_status),
        "INA226_Get_Alert_Status not return INA226_OK."
    );
    TEST_ASSERT_EQUAL(INA226_ALERT_STATUS_NONE, alert_status);
}

// Tests for INA226_Calibrate

// Tests for INA226_Set_Averaging_Mode

// Tests for INA226_Set_Operating_Mode

// Test for INA226_Set_Bus_Voltage_Conversion_Time

// Tests for INA226_Set_Shunt_Voltage_Conversion_Time

// Tests for INA226_Reset