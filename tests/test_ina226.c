#include "ina226.h"
#include "unity.h"

// Test-only register addresses and field masks.
// These mirror the driver definitions so the host mock can exercise the same behavior.
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


#ifdef EMBEDDED_TARGET
    // On-target test support

#else
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
        mock_ina226_registers[INA226_MANCUFACTURE_ID_REG] = 5449;
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

#endif

// Unity calls this before each test case so tests stay isolated from one another.
void setUp(void) {

#ifndef EMBEDDED_TARGET
    Mock_I2C_Reset();
#endif
}

// No teardown state is needed yet.
void tearDown(void) {}

// Test cases go below.
void test_INA226_Read_Current_Should_Return_Error_On_Invalid_Params(void) {

    ina226_handle_t sensor = { .ina226_i2c_addr = 0x40, .current_resolution_uA = 100 };
    int32_t current_val = 0;

    // Sensor pointer is NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Current(NULL, &current_val));

    // Output pointer is NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Current(&sensor, NULL));

    // Both parameters are NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Current(NULL, NULL));

    // Valid input.
    mock_ina226_registers[INA226_CURRENT_REG] = 0x0000;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Current(&sensor, &current_val));
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
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Current(&sensor, &current_val));
    TEST_ASSERT_EQUAL_INT32(50000, current_val);

    // 2. Negative value read (sign extension test).
    // Register = 0xFFFE (-2) -> Expected: -2 * 500 = -1000 uA.
    // This verifies the int16_t cast used for sign extension.
    mock_ina226_registers[INA226_CURRENT_REG] = (uint16_t)((int16_t)-2);
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Current(&sensor, &current_val));
    TEST_ASSERT_EQUAL_INT32(-1000, current_val);

    // 3. Zero value read.
    // Register = 0 -> Expected: 0 uA.
    mock_ina226_registers[INA226_CURRENT_REG] = 0;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Current(&sensor, &current_val));
    TEST_ASSERT_EQUAL_INT32(0, current_val);
}

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
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Power(&sensor, &power_val));
    TEST_ASSERT_EQUAL_UINT32(1250000U, power_val);

    // 2. Zero value read.
    // Register = 0 -> Expected: 0 uW.
    mock_ina226_registers[INA226_POWER_REG] = 0;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Power(&sensor, &power_val));
    TEST_ASSERT_EQUAL_UINT32(0U, power_val);
}