#include "test_ina226.h"
#include "unity.h"
#include "ina226.h"
#include "unity_internals.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define Q16_16_SHIFT          (16U)
#define Q16_16_FROM_INT(x)    ((uint32_t)(x) << Q16_16_SHIFT)

// Host-side mock register storage and I2C state for Unity tests.
static uint16_t mock_ina226_registers[256];
static bool mock_i2c_write_fail = false;
static bool mock_i2c_read_fail = false;
static uint8_t mock_last_dev_addr = 0U;

// Actual mock I2C write handler invoked via ina226_config_t function pointer.
static uint8_t Mock_I2C_Write_Callback(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len) {
    mock_last_dev_addr = dev_addr;

    if (mock_i2c_write_fail || data == NULL || len != 2U) {
        return INA226_ERR_I2C;
    }

    mock_ina226_registers[reg_addr] = (uint16_t)(((uint16_t)data[0] << 8U) | (uint16_t)data[1]);
    return INA226_OK;
}

// Actual mock I2C read handler invoked via ina226_config_t function pointer.
static uint8_t Mock_I2C_Read_Callback(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    mock_last_dev_addr = dev_addr;

    if (mock_i2c_read_fail || data == NULL || len != 2U) {
        return INA226_ERR_I2C;
    }

    uint16_t val = mock_ina226_registers[reg_addr];

    data[0] = (uint8_t)(val >> 8U);
    data[1] = (uint8_t)(val & 0xFFU);

    // Hardware behavior (SBOS547A Sec. 7.6.7): Reading Mask/Enable clears AFF and CVRF.
    if (reg_addr == INA226_MASK_EN_REG) {
        mock_ina226_registers[reg_addr] &= (uint16_t)~(INA226_MASK_EN_AFF_BIT | INA226_MASK_EN_CVRF_BIT);
    }

    return INA226_OK;
}

static const ina226_config_t mock_default_cfg = {
    .i2c_addr = 0x40U,
    .r_shunt_uOhm = 100000U,
    .platform_i2c_handle = NULL,
    .i2c_read = Mock_I2C_Read_Callback,
    .i2c_write = Mock_I2C_Write_Callback
};

static const ina226_config_t *mock_active_cfg = &mock_default_cfg;

static void Mock_Set_Active_Config(const ina226_config_t *cfg) {
    mock_active_cfg = cfg;
}

static ina226_config_t Make_Mock_Config(uint8_t addr, uint32_t r_shunt_uOhm) {
    ina226_config_t cfg = {
        .i2c_addr = addr,
        .r_shunt_uOhm = r_shunt_uOhm,
        .platform_i2c_handle = NULL,
        .i2c_read = Mock_I2C_Read_Callback,
        .i2c_write = Mock_I2C_Write_Callback
    };
    return cfg;
}

static ina226_handle_t Make_Mock_Handle(const ina226_config_t *cfg) {
    ina226_handle_t sensor = {
        .cfg = cfg,
        .current_lsb_q16_16_uA = 0U,
        .power_lsb_q16_16_uW = 0U
    };
    Mock_Set_Active_Config(cfg);
    return sensor;
}

// Directly seed a mock register value when a test needs a specific device state.
void Mock_I2C_Set_Register(uint8_t reg_addr, uint16_t value) {
    mock_ina226_registers[reg_addr] = value;
}

// Restore the mock device to its default power-on state before each test.
void Mock_I2C_Reset(void) {
    mock_i2c_write_fail = false;
    mock_i2c_read_fail = false;
    mock_last_dev_addr = 0U;
    mock_active_cfg = &mock_default_cfg;

    for (uint32_t i = 8U; i < 254U; i++) {
        mock_ina226_registers[i] = 0U;
    }

    // Set datasheet power-on reset (POR) default register values (SBOS547A Table 4).
    mock_ina226_registers[INA226_CONFIG_REG] = 0x4127U;
    mock_ina226_registers[INA226_SH_VOLTAGE_REG] = 0U;
    mock_ina226_registers[INA226_BUS_VOLTAGE_REG] = 0U;
    mock_ina226_registers[INA226_POWER_REG] = 0U;
    mock_ina226_registers[INA226_CURRENT_REG] = 0U;
    mock_ina226_registers[INA226_CALIBRATION_REG] = 0U;
    mock_ina226_registers[INA226_MASK_EN_REG] = 0U;
    mock_ina226_registers[INA226_ALERT_LIM_REG] = 0U;
    mock_ina226_registers[INA226_MANUFACTURER_ID_REG] = 0x5449U;
    mock_ina226_registers[INA226_DIE_ID_REG] = 0x2260U;
}

// Global platform I2C hooks wrap the ina226_config_t function pointers so that
// all mock I2C work executes through the config callbacks.
uint8_t INA226_Platform_I2C_Write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len) {
    if (mock_active_cfg == NULL || mock_active_cfg->i2c_write == NULL) {
        return INA226_ERR_I2C;
    }

    return mock_active_cfg->i2c_write(dev_addr, reg_addr, data, len);
}

uint8_t INA226_Platform_I2C_Read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    if (mock_active_cfg == NULL || mock_active_cfg->i2c_read == NULL) {
        return INA226_ERR_I2C;
    }

    return mock_active_cfg->i2c_read(dev_addr, reg_addr, data, len);
}

void setUp(void) {
    Mock_I2C_Reset();
}

void tearDown(void) {}

// =========================================================================
// Tests for INA226_Read_Current
// =========================================================================

void test_INA226_Read_Current_Should_Return_Error_On_Invalid_Params(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = Q16_16_FROM_INT(100U), .power_lsb_q16_16_uW = 0U };
    int32_t current_val = 0;

    // Sensor pointer is NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Current(NULL, &current_val));

    // Output pointer is NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Current(&sensor, NULL));

    // Both parameters are NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Current(NULL, NULL));

    // Config pointer inside handle is NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Current(&sensor_null_cfg, &current_val));
}

void test_INA226_Read_Current_Should_Return_Zero_When_Uncalibrated(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    int32_t current_val = 12345;

    mock_ina226_registers[INA226_CURRENT_REG] = 0U;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Current(&sensor, &current_val));
    TEST_ASSERT_EQUAL_INT32(0, current_val);

    // Even if the register holds a non-zero value, zero LSB scales to 0 uA.
    mock_ina226_registers[INA226_CURRENT_REG] = 500U;
    current_val = 12345;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Current(&sensor, &current_val));
    TEST_ASSERT_EQUAL_INT32(0, current_val);
}

void test_INA226_Read_Current_Should_Handle_Large_Q16_16_Values_Without_Overflow(void) {
    // With Q16.16 uint32_t current_lsb_q16_16_uA (< 65536 uA/LSB), full-scale signed 16-bit
    // register readings fit within int32_t without overflowing.
    ina226_config_t cfg = Make_Mock_Config(0x40U, 1000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    sensor.current_lsb_q16_16_uA = Q16_16_FROM_INT(65535U);
    int32_t current_val = 0;

    // Maximum positive register value: 0x7FFF (32767) * 65535 uA = 2,147,385,345 uA.
    mock_ina226_registers[INA226_CURRENT_REG] = 0x7FFFU;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Current(&sensor, &current_val));
    TEST_ASSERT_EQUAL_INT32(2147385345, current_val);

    // Minimum negative register value: 0x8000 (-32768) * 65535 uA = -2,147,450,880 uA.
    mock_ina226_registers[INA226_CURRENT_REG] = 0x8000U;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Current(&sensor, &current_val));
    TEST_ASSERT_EQUAL_INT32(-2147450880, current_val);
}

void test_INA226_Read_Current_Should_Calculate_Correct_Values(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 10000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    int32_t current_val = 0;

    // 10 mOhm shunt, 500 uA LSB -> CAL = 5.12e9 / (500 * 10000) = 1024 (exact).
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 500U));

    // 1. Positive value read.
    // Register = 100 (0x0064) -> Expected: 100 * 500 = 50,000 uA.
    mock_ina226_registers[INA226_CURRENT_REG] = 100U;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Current(&sensor, &current_val),
        "INA226_Read_Current not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32(50000, current_val);

    // 2. Negative value read (sign extension test).
    // Register = 0xFFFE (-2) -> Expected: -2 * 500 = -1000 uA.
    mock_ina226_registers[INA226_CURRENT_REG] = (uint16_t)((int16_t)-2);
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Current(&sensor, &current_val),
        "INA226_Read_Current not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32(-1000, current_val);

    // 3. Zero value read.
    // Register = 0 -> Expected: 0 uA.
    mock_ina226_registers[INA226_CURRENT_REG] = 0U;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Current(&sensor, &current_val),
        "INA226_Read_Current not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32(0, current_val);
}

void test_INA226_Read_Current_Should_Apply_Fractional_Q16_16_LSB(void) {
    // Rshunt = 0.1 Ohm (100000 uOhm), requested current_lsb = 1000 uA.
    // CAL_exact = 51.2 -> CAL = 51.
    // Actual Current_LSB = 5.12e9 / (51 * 100000) = 1003.9215686... uA.
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    int32_t current_val = 0;

    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 1000U));
    TEST_ASSERT_EQUAL_HEX16(51U, mock_ina226_registers[INA226_CALIBRATION_REG]);

    // 1000 counts * 1003.9215686 uA = 1,003,921.57 uA (vs uncompensated 1,000,000 uA).
    mock_ina226_registers[INA226_CURRENT_REG] = 1000U;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Current(&sensor, &current_val));
    TEST_ASSERT_INT32_WITHIN(1, 1003922, current_val);

    // Negative direction: -1000 counts -> -1,003,921.57 uA.
    mock_ina226_registers[INA226_CURRENT_REG] = (uint16_t)((int16_t)-1000);
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Current(&sensor, &current_val));
    TEST_ASSERT_INT32_WITHIN(1, -1003922, current_val);

    // Exact half-LSB fraction: 100.5 uA in Q16.16.
    sensor.current_lsb_q16_16_uA = Q16_16_FROM_INT(100U) + (1UL << 15U);
    mock_ina226_registers[INA226_CURRENT_REG] = 10U;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Current(&sensor, &current_val));
    TEST_ASSERT_EQUAL_INT32(1005, current_val);

    mock_ina226_registers[INA226_CURRENT_REG] = (uint16_t)((int16_t)-10);
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Current(&sensor, &current_val));
    TEST_ASSERT_EQUAL_INT32(-1005, current_val);
}

// =========================================================================
// Tests for INA226_Read_Power
// =========================================================================

void test_INA226_Read_Power_Should_Return_Error_On_Invalid_Params(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = Q16_16_FROM_INT(100U), .power_lsb_q16_16_uW = 25U * Q16_16_FROM_INT(100U) };
    uint32_t power_val = 0U;

    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 100U));

    // Sensor pointer is NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Power(NULL, &power_val));

    // Output pointer is NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Power(&sensor, NULL));

    // Both parameters are NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Power(NULL, NULL));

    // Config pointer inside handle is NULL.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Power(&sensor_null_cfg, &power_val));

    // Valid input.
    mock_ina226_registers[INA226_POWER_REG] = 0x0000U;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Power(&sensor, &power_val));
}

void test_INA226_Read_Power_Should_Return_Zero_When_Uncalibrated(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    uint32_t power_val = 12345U;

    mock_ina226_registers[INA226_POWER_REG] = 0U;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Power(&sensor, &power_val));
    TEST_ASSERT_EQUAL_UINT32(0U, power_val);

    mock_ina226_registers[INA226_POWER_REG] = 500U;
    power_val = 12345U;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Power(&sensor, &power_val));
    TEST_ASSERT_EQUAL_UINT32(0U, power_val);
}

void test_INA226_Read_Power_Should_Handle_Large_Q16_16_Values_Without_Overflow(void) {
    // With Q16.16 uint32_t power_lsb (< 65536 uW/LSB), maximum register value 0xFFFF
    // fits within uint32_t without overflowing (65535 * 65535 = 4,294,836,225 < UINT32_MAX).
    ina226_config_t cfg = Make_Mock_Config(0x40U, 1000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    sensor.power_lsb_q16_16_uW = Q16_16_FROM_INT(65535U);
    uint32_t power_val = 0U;

    mock_ina226_registers[INA226_POWER_REG] = 0xFFFFU;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Power(&sensor, &power_val));
    TEST_ASSERT_EQUAL_UINT32(4294836225UL, power_val);
}

void test_INA226_Read_Power_Should_Calculate_Correct_Values(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 10000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    uint32_t power_val = 0U;

    // 10 mOhm shunt, 500 uA LSB -> CAL = 1024 (exact), power_lsb = 12500 uW (Q16.16).
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 500U));

    // 1. Positive value read.
    // Register = 100 (0x0064) -> Expected: 100 * (25 * 500) = 1,250,000 uW.
    mock_ina226_registers[INA226_POWER_REG] = 100U;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Power(&sensor, &power_val),
        "INA226_Read_Power not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_UINT32(1250000U, power_val);

    // 2. Zero value read.
    // Register = 0 -> Expected: 0 uW.
    mock_ina226_registers[INA226_POWER_REG] = 0U;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Power(&sensor, &power_val),
        "INA226_Read_Power not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_UINT32(0U, power_val);
}

void test_INA226_Read_Power_Should_Apply_Fractional_Q16_16_LSB(void) {
    // Rshunt = 0.1 Ohm (100000 uOhm), requested current_lsb = 1000 uA.
    // CAL = 51 -> Actual Current_LSB = 1003.9215686 uA -> Actual Power_LSB = 25098.0392157 uW.
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    uint32_t power_val = 0U;

    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 1000U));

    // 1000 counts * 25098.0392157 uW = 25,098,039.2 uW (vs uncompensated 25,000,000 uW).
    mock_ina226_registers[INA226_POWER_REG] = 1000U;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Power(&sensor, &power_val));
    TEST_ASSERT_UINT32_WITHIN(1U, 25098039U, power_val);
}

// =========================================================================
// Tests for INA226_Read_Shunt_Voltage
// =========================================================================

void test_INA226_Read_Shunt_Voltage_Should_Return_Error_On_Invalid_Params(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = 0U, .power_lsb_q16_16_uW = 0U };
    int32_t sh_voltage = 0;

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Shunt_Voltage(NULL, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Shunt_Voltage(NULL, &sh_voltage));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Shunt_Voltage(&sensor, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Shunt_Voltage(&sensor_null_cfg, &sh_voltage));
}

void test_INA226_Read_Shunt_Voltage_Should_Calculate_Correct_Values(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    int32_t sh_voltage = 0;

    mock_ina226_registers[INA226_SH_VOLTAGE_REG] = (uint16_t)((int16_t)-1);
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Shunt_Voltage(&sensor, &sh_voltage),
        "INA226_Read_Shunt_Voltage not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32(-3, sh_voltage);

    mock_ina226_registers[INA226_SH_VOLTAGE_REG] = 1U;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Shunt_Voltage(&sensor, &sh_voltage),
        "INA226_Read_Shunt_Voltage not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32(3, sh_voltage);

    mock_ina226_registers[INA226_SH_VOLTAGE_REG] = 2U;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Shunt_Voltage(&sensor, &sh_voltage),
        "INA226_Read_Shunt_Voltage not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_INT32(5, sh_voltage);

    // Datasheet Sec. 7.6.2 example: -80 mV -> 0x8300 (-32000 * 2.5 uV = -80000 uV).
    mock_ina226_registers[INA226_SH_VOLTAGE_REG] = 0x8300U;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Shunt_Voltage(&sensor, &sh_voltage));
    TEST_ASSERT_EQUAL_INT32(-80000, sh_voltage);
}

// =========================================================================
// Tests for INA226_Read_Bus_Voltage
// =========================================================================

void test_INA226_Read_Bus_Voltage_Should_Return_Error_On_Invalid_Params(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = 0U, .power_lsb_q16_16_uW = 0U };
    uint32_t bus_voltage = 0U;

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Bus_Voltage(NULL, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Bus_Voltage(NULL, &bus_voltage));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Bus_Voltage(&sensor, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Bus_Voltage(&sensor_null_cfg, &bus_voltage));
}

void test_INA226_Read_Bus_Voltage_Should_Calculate_Correct_Values(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    uint32_t bus_voltage = 0U;

    mock_ina226_registers[INA226_BUS_VOLTAGE_REG] = 100U;
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Read_Bus_Voltage(&sensor, &bus_voltage),
        "INA226_Read_Bus_Voltage not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_UINT32((100U * INA226_BUS_VOLTAGE_LSB_UV), bus_voltage);

    mock_ina226_registers[INA226_BUS_VOLTAGE_REG] = 0U;
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK,
        INA226_Read_Bus_Voltage(&sensor, &bus_voltage),
        "INA226_Read_Bus_Voltage not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_UINT32(0U, bus_voltage);

    // Datasheet Sec. 7.6.3 full-scale: 0x7FFF * 1.25 mV = 40,958,750 uV.
    mock_ina226_registers[INA226_BUS_VOLTAGE_REG] = 0x7FFFU;
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Bus_Voltage(&sensor, &bus_voltage));
    TEST_ASSERT_EQUAL_UINT32(40958750U, bus_voltage);
}

// =========================================================================
// Datasheet Table 1 End-to-End Verification (SBOS547A Sec. 7.5.1)
// =========================================================================

void test_INA226_Datasheet_Table1_Example_End_To_End(void) {
    // Load = 10 A, Rshunt = 2 mOhm (2000 uOhm), Current_LSB = 1 mA (1000 uA), Vbus = 11.98 V.
    ina226_config_t cfg = Make_Mock_Config(0x45U, 2000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 1000U));
    TEST_ASSERT_EQUAL_HEX8(0x45U, mock_last_dev_addr);
    TEST_ASSERT_EQUAL_HEX16(0x0A00U, mock_ina226_registers[INA226_CALIBRATION_REG]);
    TEST_ASSERT_EQUAL_UINT32(Q16_16_FROM_INT(1000U), sensor.current_lsb_q16_16_uA);
    TEST_ASSERT_EQUAL_UINT32(25U * Q16_16_FROM_INT(1000U), sensor.power_lsb_q16_16_uW);

    mock_ina226_registers[INA226_SH_VOLTAGE_REG] = 0x1F40U;  // 8000 -> 20 mV
    mock_ina226_registers[INA226_BUS_VOLTAGE_REG] = 0x2570U; // 9584 -> 11.98 V
    mock_ina226_registers[INA226_CURRENT_REG] = 0x2710U;     // 10000 -> 10 A
    mock_ina226_registers[INA226_POWER_REG] = 0x12B8U;       // 4792 -> 119.8 W

    int32_t shunt_uV = 0;
    uint32_t bus_uV = 0U;
    int32_t current_uA = 0;
    uint32_t power_uW = 0U;

    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Shunt_Voltage(&sensor, &shunt_uV));
    TEST_ASSERT_EQUAL_INT32(20000, shunt_uV);

    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Bus_Voltage(&sensor, &bus_uV));
    TEST_ASSERT_EQUAL_UINT32(11980000U, bus_uV);

    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Current(&sensor, &current_uA));
    TEST_ASSERT_EQUAL_INT32(10000000, current_uA);

    TEST_ASSERT_EQUAL(INA226_OK, INA226_Read_Power(&sensor, &power_uW));
    TEST_ASSERT_EQUAL_UINT32(119800000U, power_uW);
}

// =========================================================================
// Tests for INA226_Set_Alert_Pin_Function
// =========================================================================

void test_INA226_Set_Alert_Pin_Function_Should_Return_Error_On_Invalid_Params(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = 0U, .power_lsb_q16_16_uW = 0U };
    INA226_Alert_Func_t alert_func = INA226_ALERT_FUNC_CONVERSION_READY;

    // NULL Test
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Alert_Pin_Function(NULL, alert_func));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Alert_Pin_Function(&sensor_null_cfg, alert_func));

    // Non-Valid alert pin function test
    for (uint32_t i = 0U; i <= 0xFFFFU; i++) {
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

        if (!is_valid) {
            TEST_ASSERT_EQUAL_MESSAGE(
                INA226_ERR_INVALID_PARAM,
                INA226_Set_Alert_Pin_Function(&sensor, (INA226_Alert_Func_t)i),
                "Set alert function return INA226_OK with invalid alert function option!"
            );
        }
    }
}

void test_INA226_Set_Alert_Pin_Function_Should_Do_Correct_Bitwise_Operation(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

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

    for (size_t i = 0U; i < num_functions; i++) {
        INA226_Alert_Func_t current_func = valid_functions[i];

        INA226_Status_t status = INA226_Set_Alert_Pin_Function(&sensor, current_func);
        TEST_ASSERT_EQUAL_MESSAGE(INA226_OK, status, "INA226_Set_Alert_Pin_Function not return INA226_OK.");

        uint16_t mask_en_reg = mock_ina226_registers[INA226_MASK_EN_REG];
        uint16_t extracted_func = (uint16_t)(mask_en_reg >> INA226_MASK_ENABLE_ALERT_FUNC_POS);

        TEST_ASSERT_EQUAL_HEX16(current_func, extracted_func);
    }
}

// =========================================================================
// Tests for INA226_Get_Alert_Pin_Function
// =========================================================================

void test_INA226_Get_Alert_Pin_Function_Should_Return_Error_On_Invalid_Params(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = 0U, .power_lsb_q16_16_uW = 0U };
    INA226_Alert_Func_t alert_func = 0U;

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Pin_Function(NULL, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Pin_Function(NULL, &alert_func));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Pin_Function(&sensor, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Pin_Function(&sensor_null_cfg, &alert_func));
}

void test_INA226_Get_Alert_Pin_Function_Should_Read_Correct_Alert_Function_Option(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    INA226_Alert_Func_t read_alert_func = 0U;

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

    for (size_t i = 0U; i < num_functions; i++) {
        INA226_Alert_Func_t actual_alert_func = valid_functions[i];

        uint16_t mask_en_reg = mock_ina226_registers[INA226_MASK_EN_REG];
        mask_en_reg &= (uint16_t)~(INA226_MASK_ENABLE_ALERT_FUNC_MASK);
        mask_en_reg |= (uint16_t)(actual_alert_func << INA226_MASK_ENABLE_ALERT_FUNC_POS);
        mock_ina226_registers[INA226_MASK_EN_REG] = mask_en_reg;

        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Get_Alert_Pin_Function(&sensor, &read_alert_func),
            "INA226_Get_Alert_Pin_Function not return INA226_OK."
        );

        TEST_ASSERT_EQUAL(actual_alert_func, read_alert_func);
    }
}

// =========================================================================
// Tests for INA226_Set_Alert_Limit
// =========================================================================

void test_INA226_Set_Alert_Limit_Should_Return_Error_On_Invalid_Params(void) {
    int32_t zero_limit_value = 0;
    int32_t valid_limit_value = 100;
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = Q16_16_FROM_INT(100U), .power_lsb_q16_16_uW = 25U * Q16_16_FROM_INT(100U) };

    INA226_Status_t status = INA226_Set_Alert_Pin_Function(&sensor, INA226_ALERT_FUNC_POWER_OVER_LIMIT);
    TEST_ASSERT_EQUAL_MESSAGE(INA226_OK, status, "INA226_Set_Alert_Pin_Function not return INA226_OK.");

    // Uncalibrated handle (current_lsb_q16_16_uA == 0, power_lsb == 0) for power alert.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Alert_Limit(&sensor, zero_limit_value));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Alert_Limit(&sensor, valid_limit_value));

    // NULL handle and NULL cfg.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Alert_Limit(NULL, valid_limit_value));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Alert_Limit(&sensor_null_cfg, valid_limit_value));
}

void test_INA226_Set_Alert_Limit_Should_Return_Invalid_State_When_No_Limit_Function_Configured(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    // Default Mask/Enable register (0x0000): no alert function enabled.
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_STATE, INA226_Set_Alert_Limit(&sensor, 1000));

    // Conversion Ready only: not a limit-based alert function.
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Set_Alert_Pin_Function(&sensor, INA226_ALERT_FUNC_CONVERSION_READY));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_STATE, INA226_Set_Alert_Limit(&sensor, 1000));
}

void test_INA226_Set_Alert_Limit_Should_Return_Error_Math_Overflow_For_Invalid_Shunt_Limit_Values(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    int32_t invalid_limit_value = 0;

    // Shunt category limit value tests
    INA226_Alert_Func_t shunt_category_alert_functions[] = {
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT,
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT,
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_OVER_LIMIT_CVR,
        INA226_ALERT_FUNC_SHUNT_VOLTAGE_UNDER_LIMIT_CON_READY_CVR
    };

    size_t num_category_alert_functions = sizeof(shunt_category_alert_functions) / sizeof(shunt_category_alert_functions[0]);

    for (size_t i = 0U; i < num_category_alert_functions; i++) {
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

        // Input type extremes: the conversion must not overflow before the range check.
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, INT32_MAX));
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, INT32_MIN));
    }
}

void test_INA226_Set_Alert_Limit_Should_Return_Error_Math_Overflow_For_Invalid_Bus_Limit_Values(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    int32_t invalid_limit_value = 0;

    // Bus category limit value tests
    INA226_Alert_Func_t bus_category_alert_functions[] = {
        INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT,
        INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT,
        INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT_CON_READY_CVR,
        INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT_CON_READY_CVR
    };

    size_t num_category_alert_functions = sizeof(bus_category_alert_functions) / sizeof(bus_category_alert_functions[0]);

    for (size_t i = 0U; i < num_category_alert_functions; i++) {
        INA226_Alert_Func_t alert_func = bus_category_alert_functions[i];

        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Set_Alert_Pin_Function(&sensor, alert_func),
            "INA226_Set_Alert_Pin_Function not return INA226_OK."
        );

        invalid_limit_value = (INT16_MAX + 1) * (int32_t)INA226_BUS_VOLTAGE_LSB_UV; // Maximum value of the bus voltage register + 1.
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, invalid_limit_value));

        // Smallest limit that rounds to INT16_MAX + 1 (issue #79).
        invalid_limit_value = INT16_MAX * (int32_t)INA226_BUS_VOLTAGE_LSB_UV + (int32_t)INA226_BUS_VOLTAGE_LSB_UV / 2;
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, invalid_limit_value));

        // Input type extreme: the rounding addition must not overflow int32 (issue #58).
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, INT32_MAX));
    }
}

void test_INA226_Set_Alert_Limit_Should_Return_Error_Math_Overflow_For_Invalid_Power_Limit_Values(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    int32_t invalid_limit_value = 0;

    // 100 mOhm shunt, 100 uA LSB -> CAL = 512 (exact), Power_LSB = 2500 uW.
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 100U));

    INA226_Alert_Func_t power_category_alert_functions[] = {
        INA226_ALERT_FUNC_POWER_OVER_LIMIT,
        INA226_ALERT_FUNC_POWER_OVER_LIMIT_CON_READY_CVR
    };

    size_t num_category_alert_functions = sizeof(power_category_alert_functions) / sizeof(power_category_alert_functions[0]);

    for (size_t i = 0U; i < num_category_alert_functions; i++) {
        INA226_Alert_Func_t alert_func = power_category_alert_functions[i];

        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Set_Alert_Pin_Function(&sensor, alert_func),
            "INA226_Set_Alert_Pin_Function not return INA226_OK."
        );

        int32_t power_lsb_uW = 2500;

        invalid_limit_value = ((int32_t)UINT16_MAX + 1) * power_lsb_uW; // Maximum value of the power register + 1.
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, invalid_limit_value));

        // Smallest limit that rounds to UINT16_MAX + 1.
        invalid_limit_value = (int32_t)UINT16_MAX * power_lsb_uW + power_lsb_uW / 2;
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, invalid_limit_value));

        // Input type extreme: must be rejected without overflowing the conversion.
        TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Set_Alert_Limit(&sensor, INT32_MAX));
    }
}

void test_INA226_Set_Alert_Limit_Should_Accept_Int32_Max_Power_Limit_For_Large_Power_LSB(void) {
    // 1 mOhm shunt, 2000 uA LSB -> CAL = 2560 (exact), power_lsb = 50000 uW (Q16.16 = 3,276,800,000).
    // INT32_MAX (2,147,483,647 uW) / 50000 uW = 42949.67 -> rounds to 42950 (<= 65535).
    ina226_config_t cfg = Make_Mock_Config(0x40U, 1000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 2000U));

    INA226_Alert_Func_t power_category_alert_functions[] = {
        INA226_ALERT_FUNC_POWER_OVER_LIMIT,
        INA226_ALERT_FUNC_POWER_OVER_LIMIT_CVR
    };

    size_t num_category_alert_functions = sizeof(power_category_alert_functions) / sizeof(power_category_alert_functions[0]);

    for (size_t i = 0U; i < num_category_alert_functions; i++) {
        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Set_Alert_Pin_Function(&sensor, power_category_alert_functions[i]),
            "INA226_Set_Alert_Pin_Function not return INA226_OK."
        );

        mock_ina226_registers[INA226_ALERT_LIM_REG] = 0x1234U;

        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Set_Alert_Limit(&sensor, INT32_MAX),
            "INA226_Set_Alert_Limit not return INA226_OK."
        );

        TEST_ASSERT_EQUAL_HEX16(42950U, mock_ina226_registers[INA226_ALERT_LIM_REG]);
    }
}

void test_INA226_Set_Alert_Limit_Should_Return_Invalid_Param_For_Negative_Bus_Limit_Values(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    INA226_Alert_Func_t bus_category_alert_functions[] = {
        INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT,
        INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT,
        INA226_ALERT_FUNC_BUS_VOLTAGE_OVER_LIMIT_CVR,
        INA226_ALERT_FUNC_BUS_VOLTAGE_UNDER_LIMIT_CVR
    };

    int32_t negative_limit_values[] = { -1, -(int32_t)INA226_BUS_VOLTAGE_LSB_UV, INT32_MIN };

    size_t num_category_alert_functions = sizeof(bus_category_alert_functions) / sizeof(bus_category_alert_functions[0]);
    size_t num_negative_limit_values = sizeof(negative_limit_values) / sizeof(negative_limit_values[0]);

    for (size_t i = 0U; i < num_category_alert_functions; i++) {
        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Set_Alert_Pin_Function(&sensor, bus_category_alert_functions[i]),
            "INA226_Set_Alert_Pin_Function not return INA226_OK."
        );

        for (size_t j = 0U; j < num_negative_limit_values; j++) {
            mock_ina226_registers[INA226_ALERT_LIM_REG] = 0x1234U;

            TEST_ASSERT_EQUAL_MESSAGE(
                INA226_ERR_INVALID_PARAM,
                INA226_Set_Alert_Limit(&sensor, negative_limit_values[j]),
                "INA226_Set_Alert_Limit not return INA226_ERR_INVALID_PARAM for a negative bus voltage limit."
            );

            TEST_ASSERT_EQUAL_HEX16_MESSAGE(
                0x1234U,
                mock_ina226_registers[INA226_ALERT_LIM_REG],
                "Alert limit register must not be written when the limit is rejected."
            );
        }
    }
}

void test_INA226_Set_Alert_Limit_Should_Return_Invalid_Param_For_Negative_Power_Limit_Values(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 100U));

    INA226_Alert_Func_t power_category_alert_functions[] = {
        INA226_ALERT_FUNC_POWER_OVER_LIMIT,
        INA226_ALERT_FUNC_POWER_OVER_LIMIT_CVR
    };

    int32_t power_lsb_uW = 2500;
    int32_t negative_limit_values[] = { -1, -power_lsb_uW, INT32_MIN };

    size_t num_category_alert_functions = sizeof(power_category_alert_functions) / sizeof(power_category_alert_functions[0]);
    size_t num_negative_limit_values = sizeof(negative_limit_values) / sizeof(negative_limit_values[0]);

    for (size_t i = 0U; i < num_category_alert_functions; i++) {
        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Set_Alert_Pin_Function(&sensor, power_category_alert_functions[i]),
            "INA226_Set_Alert_Pin_Function not return INA226_OK."
        );

        for (size_t j = 0U; j < num_negative_limit_values; j++) {
            mock_ina226_registers[INA226_ALERT_LIM_REG] = 0x1234U;

            TEST_ASSERT_EQUAL_MESSAGE(
                INA226_ERR_INVALID_PARAM,
                INA226_Set_Alert_Limit(&sensor, negative_limit_values[j]),
                "INA226_Set_Alert_Limit not return INA226_ERR_INVALID_PARAM for a negative power limit."
            );

            TEST_ASSERT_EQUAL_HEX16_MESSAGE(
                0x1234U,
                mock_ina226_registers[INA226_ALERT_LIM_REG],
                "Alert limit register must not be written when the limit is rejected."
            );
        }
    }
}

void test_INA226_Set_Alert_Limit_Should_Write_Correct_Limit_Value_To_Register_For_Shunt(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    // Shunt
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

    // Datasheet Table 22 example: 80 mV -> 0x7D00 (32000)
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Set_Alert_Limit(&sensor, 80000));
    TEST_ASSERT_EQUAL_HEX16(0x7D00U, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // middle
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 2500), // 2500 uV -> (2500 * 2) / 5 = 1000 to register
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(1000U, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // round to nearest
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 12), // (12 * 2 + 2) / 5 = 5.2 -> 5 to register.
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(5U, mock_ina226_registers[INA226_ALERT_LIM_REG]);
}

void test_INA226_Set_Alert_Limit_Should_Write_Correct_Limit_Value_To_Register_For_Bus(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

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

    // Limits above the maximum that still round down to INT16_MAX (issue #79).
    mock_ina226_registers[INA226_ALERT_LIM_REG] = 0U;
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 40958751), // 40,958,751 uV / 1250 uV = 32767.0008 -> 32767
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(INT16_MAX, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    mock_ina226_registers[INA226_ALERT_LIM_REG] = 0U;
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 40959374), // 40,959,374 uV / 1250 uV = 32767.4992 -> 32767
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(INT16_MAX, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // min
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 0), // 0 uV -> 0 * 1250 uV = 0
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(0U, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // middle
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 5000000), // 5000000 uV -> 5000000 / 1250 uV = 4000 to register
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(4000U, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // round to nearest
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 2000), // 2000 uV / 1250 = 1.6 -> 2 to register.
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(2U, mock_ina226_registers[INA226_ALERT_LIM_REG]);
}

void test_INA226_Set_Alert_Limit_Should_Write_Correct_Limit_Value_To_Register_For_Power(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    // 100 mOhm shunt, 100 uA LSB -> CAL = 512 (exact), Power_LSB = 2500 uW.
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 100U));

    // Power
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Pin_Function(&sensor, INA226_ALERT_FUNC_POWER_OVER_LIMIT),
        "INA226_Set_Alert_Pin_Function not return INA226_OK."
    );

    // max
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 163837500), // 65535 (UINT16_MAX) * 2500 uW = 163,837,500 uW
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // min
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 0),
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(0U, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // middle
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 2500000), // 1000 * 2500 uW = 2,500,000 uW
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(1000U, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // round to nearest
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Limit(&sensor, 4000), // 4000 / 2500 = 1.6 -> 2 to register.
        "INA226_Set_Alert_Limit not return INA226_OK."
    );
    TEST_ASSERT_EQUAL_HEX16(2U, mock_ina226_registers[INA226_ALERT_LIM_REG]);

    // Fractional Q16.16 power_lsb: Rshunt = 100000 uOhm, current_lsb = 1000 uA -> CAL = 51,
    // actual Power_LSB = 25098.0392 uW -> 25,098,039 uW maps to 1000 counts (vs 1004 uncompensated).
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 1000U));
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Set_Alert_Limit(&sensor, 25098039));
    TEST_ASSERT_EQUAL_HEX16(1000U, mock_ina226_registers[INA226_ALERT_LIM_REG]);
}

// =========================================================================
// Tests for INA226_Get_Alert_Status
// =========================================================================

void test_INA226_Get_Alert_Status_Should_Return_Error_On_Invalid_Params(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = 0U, .power_lsb_q16_16_uW = 0U };
    INA226_Alert_Status_t alert_status = INA226_ALERT_STATUS_NONE;

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Status(NULL, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Status(NULL, &alert_status));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Status(&sensor, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Get_Alert_Status(&sensor_null_cfg, &alert_status));
}

void test_INA226_Get_Alert_Status_Should_Read_Correct_Alert_Status(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    INA226_Alert_Status_t alert_status = INA226_ALERT_STATUS_NONE;

    mock_ina226_registers[INA226_MASK_EN_REG] = (uint16_t)(0xFFFFU & INA226_MASK_EN_AFF_BIT);
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Get_Alert_Status(&sensor, &alert_status),
        "INA226_Get_Alert_Status not return INA226_OK."
    );
    TEST_ASSERT_EQUAL(INA226_ALERT_STATUS_LIMIT_EXCEEDED, alert_status);

    mock_ina226_registers[INA226_MASK_EN_REG] = (uint16_t)(0xFFFFU & (INA226_MASK_EN_AFF_BIT | INA226_MASK_EN_CVRF_BIT));
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Get_Alert_Status(&sensor, &alert_status),
        "INA226_Get_Alert_Status not return INA226_OK."
    );
    TEST_ASSERT_EQUAL(INA226_ALERT_STATUS_BOTH, alert_status);

    mock_ina226_registers[INA226_MASK_EN_REG] = (uint16_t)(0xFFFFU & INA226_MASK_EN_CVRF_BIT);
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Get_Alert_Status(&sensor, &alert_status),
        "INA226_Get_Alert_Status not return INA226_OK."
    );
    TEST_ASSERT_EQUAL(INA226_ALERT_STATUS_CONVERSION_READY, alert_status);

    mock_ina226_registers[INA226_MASK_EN_REG] = 0x0000U;
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Get_Alert_Status(&sensor, &alert_status),
        "INA226_Get_Alert_Status not return INA226_OK."
    );
    TEST_ASSERT_EQUAL(INA226_ALERT_STATUS_NONE, alert_status);
}

// =========================================================================
// Tests for INA226_Calibrate
// =========================================================================

void test_INA226_Calibrate_Should_Return_Invalid_Param_Error_On_Null_Pointer(void) {
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = 0U, .power_lsb_q16_16_uW = 0U };

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Calibrate(NULL, 100U));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Calibrate(&sensor_null_cfg, 100U));
}

void test_INA226_Calibrate_Should_Return_Invalid_Param_Error_On_Zero_Values(void) {
    ina226_config_t cfg1 = Make_Mock_Config(0x40U, 0U);
    ina226_handle_t sensor1 = Make_Mock_Handle(&cfg1);
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Calibrate(&sensor1, 100U));
    TEST_ASSERT_EQUAL_UINT32(0U, sensor1.current_lsb_q16_16_uA);
    TEST_ASSERT_EQUAL_UINT32(0U, sensor1.power_lsb_q16_16_uW);

    ina226_config_t cfg2 = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor2 = Make_Mock_Handle(&cfg2);
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Calibrate(&sensor2, 0U));
    TEST_ASSERT_EQUAL_UINT32(0U, sensor2.current_lsb_q16_16_uA);
    TEST_ASSERT_EQUAL_UINT32(0U, sensor2.power_lsb_q16_16_uW);
}

void test_INA226_Calibrate_Should_Calculate_Correct_Values(void) {
    // Case 1: 100 uA LSB, 100,000 uOhm (100 mOhm) -> CAL = 512 (exact)
    ina226_config_t cfg1 = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor1 = Make_Mock_Handle(&cfg1);
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Calibrate(&sensor1, 100U),
        "INA226_Calibrate did not return INA226_OK for 100mOhm / 100uA."
    );
    TEST_ASSERT_EQUAL_HEX16(512U, mock_ina226_registers[INA226_CALIBRATION_REG]);
    TEST_ASSERT_EQUAL_UINT32(Q16_16_FROM_INT(100U), sensor1.current_lsb_q16_16_uA);
    TEST_ASSERT_EQUAL_UINT32(25U * Q16_16_FROM_INT(100U), sensor1.power_lsb_q16_16_uW);

    // Case 2: 1000 uA LSB, 2,000 uOhm (2 mOhm) -> CAL = 2560 (exact, Datasheet Table 1)
    ina226_config_t cfg2 = Make_Mock_Config(0x40U, 2000U);
    ina226_handle_t sensor2 = Make_Mock_Handle(&cfg2);
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Calibrate(&sensor2, 1000U),
        "INA226_Calibrate did not return INA226_OK for 2mOhm / 1000uA."
    );
    TEST_ASSERT_EQUAL_HEX16(2560U, mock_ina226_registers[INA226_CALIBRATION_REG]);
    TEST_ASSERT_EQUAL_UINT32(Q16_16_FROM_INT(1000U), sensor2.current_lsb_q16_16_uA);
    TEST_ASSERT_EQUAL_UINT32(25U * Q16_16_FROM_INT(1000U), sensor2.power_lsb_q16_16_uW);

    // Case 3: Boundary case - maximum valid CAL = 32767 (0x7FFF, bit 15 is 0)
    // 1 uA LSB, 156,255 uOhm -> 5.12e9 / 156255 = 32766.95 -> rounds to 32767
    // Actual Current_LSB = 5.12e9 * 65536 / (32767 * 156255) = 65535.9029... (Q16.16)
    ina226_config_t cfg3 = Make_Mock_Config(0x40U, 156255U);
    ina226_handle_t sensor3 = Make_Mock_Handle(&cfg3);
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Calibrate(&sensor3, 1U),
        "INA226_Calibrate did not return INA226_OK for maximum valid CAL (32767)."
    );
    TEST_ASSERT_EQUAL_HEX16(32767U, mock_ina226_registers[INA226_CALIBRATION_REG]);
    TEST_ASSERT_UINT32_WITHIN(1U, 65536U, sensor3.current_lsb_q16_16_uA);
    TEST_ASSERT_UINT32_WITHIN(25U, 25U * 65536U, sensor3.power_lsb_q16_16_uW);
}

void test_INA226_Calibrate_Should_BackCalculate_Actual_Q16_16_LSB_From_Rounded_Cal(void) {
    // Rshunt = 100,000 uOhm (0.1 Ohm), requested current_lsb = 1000 uA:
    // CAL_exact = 51.2 -> CAL = 51.
    // Actual Current_LSB = 5.12e9 / (51 * 100000) = 1003.9215686 uA -> Q16.16 = 65,793,003.92
    // Actual Power_LSB   = 25 * Actual Current_LSB = 25098.0392 uW -> Q16.16 = 1,644,825,098.04
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 1000U));
    TEST_ASSERT_EQUAL_HEX16(51U, mock_ina226_registers[INA226_CALIBRATION_REG]);
    TEST_ASSERT_UINT32_WITHIN(1U, 65793004U, sensor.current_lsb_q16_16_uA);
    TEST_ASSERT_UINT32_WITHIN(25U, 1644825098U, sensor.power_lsb_q16_16_uW);
}

void test_INA226_Calibrate_Should_Calculate_Correct_Values_For_All_Valid_Cal_Range(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    // Verify all 32,767 valid calibration register values (1 .. 0x7FFF)
    for (uint32_t expected_cal = 1U; expected_cal <= INA226_CALIBRATION_MAX_VAL; expected_cal++) {
        // For CAL = 1, product is 5.12e9 which exceeds UINT32_MAX for shunt when the LSB is 1 uA.
        // Using LSB = 2 uA yields shunt = 2.56e9 uOhm, fitting within uint32_t.
        uint32_t requested_lsb = (expected_cal == 1U) ? 2U : 1U;
        cfg.r_shunt_uOhm = (uint32_t)((5120000000ULL / expected_cal) / requested_lsb);

        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Calibrate(&sensor, requested_lsb),
            "INA226_Calibrate did not return INA226_OK in valid CAL range loop."
        );
        TEST_ASSERT_EQUAL_HEX16((uint16_t)expected_cal, mock_ina226_registers[INA226_CALIBRATION_REG]);
        TEST_ASSERT_GREATER_THAN_UINT32(0U, sensor.current_lsb_q16_16_uA);
        TEST_ASSERT_GREATER_THAN_UINT32(0U, sensor.power_lsb_q16_16_uW);
    }
}

void test_INA226_Calibrate_Should_Round_To_Nearest(void) {
    // 1000 uA LSB, 3000 uOhm -> 5.12e9 / 3e6 = 1706.666... -> rounds to 1707
    // Actual Current_LSB = 5.12e9 * 65536 / (1707 * 3000) = 65,523,202.50 (Q16.16)
    ina226_config_t cfg = Make_Mock_Config(0x40U, 3000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Calibrate(&sensor, 1000U),
        "INA226_Calibrate did not return INA226_OK for rounding test."
    );
    TEST_ASSERT_EQUAL_HEX16(1707U, mock_ina226_registers[INA226_CALIBRATION_REG]);
    TEST_ASSERT_UINT32_WITHIN(1U, 65523202U, sensor.current_lsb_q16_16_uA);
    TEST_ASSERT_UINT32_WITHIN(25U, 1638080062U, sensor.power_lsb_q16_16_uW);
}

void test_INA226_Calibrate_Should_Return_Invalid_Param_Error_On_Out_Of_Range_Cal(void) {
    // CAL = 32768 (0x8000): Bit 15 is set (reserved bit violation)
    // 1 uA LSB, 156,250 uOhm -> 5.12e9 / 156250 = 32768
    ina226_config_t cfg_bit15 = Make_Mock_Config(0x40U, 156250U);
    ina226_handle_t sensor_bit15 = Make_Mock_Handle(&cfg_bit15);
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Calibrate(&sensor_bit15, 1U));

    // CAL > 0xFFFF: 1 uA LSB, 1 uOhm -> CAL = 5120000000 > 32767
    ina226_config_t cfg_overflow = Make_Mock_Config(0x40U, 1U);
    ina226_handle_t sensor_overflow = Make_Mock_Handle(&cfg_overflow);
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Calibrate(&sensor_overflow, 1U));

    // CAL == 0: product too large
    ina226_config_t cfg_zero = Make_Mock_Config(0x40U, 4000000000UL);
    ina226_handle_t sensor_zero = Make_Mock_Handle(&cfg_zero);
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Calibrate(&sensor_zero, 4000000000UL));
}

void test_INA226_Calibrate_Should_Detect_Current_LSB_Q16_16_Overflow(void) {
    // current_lsb_q16_16_uA overflows uint32_t when actual Current_LSB >= 65536 uA.
    // Rshunt = 100 uOhm, current_lsb = 70000 uA -> CAL = 731 (valid),
    // actual_current_lsb_q16_16 ~= 4,590,209,576 > UINT32_MAX.
    ina226_config_t cfg_curr_ovf = Make_Mock_Config(0x40U, 100U);
    ina226_handle_t sensor_curr_ovf = Make_Mock_Handle(&cfg_curr_ovf);
    TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Calibrate(&sensor_curr_ovf, 70000U));
}

void test_INA226_Calibrate_Should_Detect_Power_LSB_Q16_16_Overflow(void) {
    // power_lsb (Q16.16 uW) overflows uint32_t when actual Current_LSB > 2621.43998 uA.
    // Rshunt = 1000 uOhm, current_lsb = 3000 uA -> CAL = 1707 (valid),
    // actual_current_lsb_q16_16 ~= 196,569,607 (fits in uint32_t),
    // power_lsb = 25 * 196,569,607 ~= 4,914,240,175 > UINT32_MAX.
    ina226_config_t cfg_power_ovf = Make_Mock_Config(0x40U, 1000U);
    ina226_handle_t sensor_power_ovf = Make_Mock_Handle(&cfg_power_ovf);
    TEST_ASSERT_EQUAL(INA226_ERR_MATH_OVERFLOW, INA226_Calibrate(&sensor_power_ovf, 3000U));
}

void test_INA226_Calibrate_Should_Return_I2C_Error_On_Write_Fail(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    mock_i2c_write_fail = true;
    TEST_ASSERT_EQUAL(INA226_ERR_I2C, INA226_Calibrate(&sensor, 100U));
    mock_i2c_write_fail = false;

    // Handle calibration fields must not be modified on I2C failure.
    TEST_ASSERT_EQUAL_UINT32(0U, sensor.current_lsb_q16_16_uA);
    TEST_ASSERT_EQUAL_UINT32(0U, sensor.power_lsb_q16_16_uW);
}

// =========================================================================
// Tests for INA226_Set_Averaging_Mode
// =========================================================================

void test_INA226_Set_Averaging_Mode_Should_Return_Invalid_Param_Error_On_Null_Pointer(void) {
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = 0U, .power_lsb_q16_16_uW = 0U };

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Averaging_Mode(NULL, INA226_AVG_1));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Averaging_Mode(&sensor_null_cfg, INA226_AVG_1));
}

void test_INA226_Set_Averaging_Mode_Should_Return_Invalid_Param_Error_On_Invalid_Avg_Time_Options(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    for (uint16_t i = 0U; i <= UINT8_MAX; i++) {
        bool is_valid = false;

        switch (i) {
            case INA226_AVG_1:
            case INA226_AVG_4:
            case INA226_AVG_16:
            case INA226_AVG_64:
            case INA226_AVG_128:
            case INA226_AVG_256:
            case INA226_AVG_512:
            case INA226_AVG_1024:
                is_valid = true;
                break;
            default:
                is_valid = false;
                break;
        }

        if (!is_valid) {
            TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Averaging_Mode(&sensor, (INA226_Avg_Time_t)i));
        }
    }
}

void test_INA226_Set_Averaging_Mode_Should_Write_Correct_Avg_Time_Option_To_Register(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    INA226_Avg_Time_t avg_time_options[] = {
        INA226_AVG_1,
        INA226_AVG_4,
        INA226_AVG_16,
        INA226_AVG_64,
        INA226_AVG_128,
        INA226_AVG_256,
        INA226_AVG_512,
        INA226_AVG_1024
    };

    size_t num_avg_time_options = sizeof(avg_time_options) / sizeof(avg_time_options[0]);

    for (size_t i = 0U; i < num_avg_time_options; i++) {
        INA226_Avg_Time_t avg_time = avg_time_options[i];

        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Set_Averaging_Mode(&sensor, avg_time),
            "INA226_Set_Averaging_Mode not return INA226_OK."
        );

        uint16_t actual_avg_time = (uint16_t)((mock_ina226_registers[INA226_CONFIG_REG] & INA226_CONFIG_AVG_MASK) >> INA226_CONFIG_AVG_POS);
        TEST_ASSERT_EQUAL(avg_time, actual_avg_time);
    }
}

// =========================================================================
// Tests for INA226_Set_Operating_Mode
// =========================================================================

void test_INA226_Set_Operating_Mode_Should_Return_Invalid_Param_Error_On_Null_Pointer(void) {
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = 0U, .power_lsb_q16_16_uW = 0U };

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Operating_Mode(NULL, INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Operating_Mode(&sensor_null_cfg, INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE));
}

void test_INA226_Set_Operating_Mode_Should_Return_Invalid_Param_Error_On_Invalid_Mode_Options(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    for (uint16_t i = 0U; i <= UINT8_MAX; i++) {
        bool is_valid = false;

        switch (i) {
            case INA226_SHUT_DOWN:
            case INA226_TRIGGERED_SHUNT_VOLTAGE:
            case INA226_TRIGGERED_BUS_VOLTAGE:
            case INA226_TRIGGERED_BUS_AND_SHUNT_VOLTAGE:
            case INA226_SHUT_DOWN_ALT:
            case INA226_CONTINUOUS_SHUNT_VOLTAGE:
            case INA226_CONTINUOUS_BUS_VOLTAGE:
            case INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE:
                is_valid = true;
                break;
            default:
                is_valid = false;
                break;
        }

        if (!is_valid) {
            TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Operating_Mode(&sensor, (INA226_Mode_t)i));
        }
    }
}

void test_INA226_Set_Operating_Mode_Should_Write_Correct_Mode_Option_To_Register(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    INA226_Mode_t mode_options[] = {
        INA226_SHUT_DOWN,
        INA226_TRIGGERED_SHUNT_VOLTAGE,
        INA226_TRIGGERED_BUS_VOLTAGE,
        INA226_TRIGGERED_BUS_AND_SHUNT_VOLTAGE,
        INA226_SHUT_DOWN_ALT,
        INA226_CONTINUOUS_SHUNT_VOLTAGE,
        INA226_CONTINUOUS_BUS_VOLTAGE,
        INA226_CONTINUOUS_BUS_AND_SHUNT_VOLTAGE
    };

    size_t num_mode_options = sizeof(mode_options) / sizeof(mode_options[0]);

    for (size_t i = 0U; i < num_mode_options; i++) {
        INA226_Mode_t mode = mode_options[i];

        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Set_Operating_Mode(&sensor, mode),
            "INA226_Set_Operating_Mode not return INA226_OK."
        );

        uint16_t actual_mode = (uint16_t)((mock_ina226_registers[INA226_CONFIG_REG] & INA226_CONFIG_MODE_MASK) >> INA226_CONFIG_MODE_POS);
        TEST_ASSERT_EQUAL(mode, actual_mode);
    }
}

// =========================================================================
// Tests for INA226_Set_Bus_Voltage_Conversion_Time
// =========================================================================

void test_INA226_Set_Bus_Voltage_Conversion_Time_Should_Return_Invalid_Param_Error_On_Null_Pointer(void) {
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = 0U, .power_lsb_q16_16_uW = 0U };

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Bus_Voltage_Conversion_Time(NULL, INA226_CT_140_US));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Bus_Voltage_Conversion_Time(&sensor_null_cfg, INA226_CT_140_US));
}

void test_INA226_Set_Bus_Voltage_Conversion_Time_Should_Return_Invalid_Param_Error_On_Invalid_Conv_Time_Options(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    for (uint16_t i = 0U; i <= UINT8_MAX; i++) {
        bool is_valid = false;

        switch (i) {
            case INA226_CT_140_US:
            case INA226_CT_204_US:
            case INA226_CT_332_US:
            case INA226_CT_588_US:
            case INA226_CT_1100_US:
            case INA226_CT_2116_US:
            case INA226_CT_4156_US:
            case INA226_CT_8244_US:
                is_valid = true;
                break;
            default:
                is_valid = false;
                break;
        }

        if (!is_valid) {
            TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Bus_Voltage_Conversion_Time(&sensor, (INA226_Conv_Time_t)i));
        }
    }
}

void test_INA226_Set_Bus_Voltage_Conversion_Time_Should_Write_Correct_Conv_Time_Option_To_Register(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    INA226_Conv_Time_t conv_time_options[] = {
        INA226_CT_140_US,
        INA226_CT_204_US,
        INA226_CT_332_US,
        INA226_CT_588_US,
        INA226_CT_1100_US,
        INA226_CT_2116_US,
        INA226_CT_4156_US,
        INA226_CT_8244_US
    };

    size_t num_options = sizeof(conv_time_options) / sizeof(conv_time_options[0]);

    for (size_t i = 0U; i < num_options; i++) {
        INA226_Conv_Time_t conv_time = conv_time_options[i];

        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Set_Bus_Voltage_Conversion_Time(&sensor, conv_time),
            "INA226_Set_Bus_Voltage_Conversion_Time not return INA226_OK."
        );

        uint16_t actual_conv_time = (uint16_t)((mock_ina226_registers[INA226_CONFIG_REG] & INA226_CONFIG_BUS_CT_MASK) >> INA226_CONFIG_BUS_CT_POS);
        TEST_ASSERT_EQUAL(conv_time, actual_conv_time);
    }
}

// =========================================================================
// Tests for INA226_Set_Shunt_Voltage_Conversion_Time
// =========================================================================

void test_INA226_Set_Shunt_Voltage_Conversion_Time_Should_Return_Invalid_Param_Error_On_Null_Pointer(void) {
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = 0U, .power_lsb_q16_16_uW = 0U };

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Shunt_Voltage_Conversion_Time(NULL, INA226_CT_140_US));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Shunt_Voltage_Conversion_Time(&sensor_null_cfg, INA226_CT_140_US));
}

void test_INA226_Set_Shunt_Voltage_Conversion_Time_Should_Return_Invalid_Param_Error_On_Invalid_Conv_Time_Options(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    for (uint16_t i = 0U; i <= UINT8_MAX; i++) {
        bool is_valid = false;

        switch (i) {
            case INA226_CT_140_US:
            case INA226_CT_204_US:
            case INA226_CT_332_US:
            case INA226_CT_588_US:
            case INA226_CT_1100_US:
            case INA226_CT_2116_US:
            case INA226_CT_4156_US:
            case INA226_CT_8244_US:
                is_valid = true;
                break;
            default:
                is_valid = false;
                break;
        }

        if (!is_valid) {
            TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Shunt_Voltage_Conversion_Time(&sensor, (INA226_Conv_Time_t)i));
        }
    }
}

void test_INA226_Set_Shunt_Voltage_Conversion_Time_Should_Write_Correct_Conv_Time_Option_To_Register(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    INA226_Conv_Time_t conv_time_options[] = {
        INA226_CT_140_US,
        INA226_CT_204_US,
        INA226_CT_332_US,
        INA226_CT_588_US,
        INA226_CT_1100_US,
        INA226_CT_2116_US,
        INA226_CT_4156_US,
        INA226_CT_8244_US
    };

    size_t num_options = sizeof(conv_time_options) / sizeof(conv_time_options[0]);

    for (size_t i = 0U; i < num_options; i++) {
        INA226_Conv_Time_t conv_time = conv_time_options[i];

        TEST_ASSERT_EQUAL_MESSAGE(
            INA226_OK,
            INA226_Set_Shunt_Voltage_Conversion_Time(&sensor, conv_time),
            "INA226_Set_Shunt_Voltage_Conversion_Time not return INA226_OK."
        );

        uint16_t actual_conv_time = (uint16_t)((mock_ina226_registers[INA226_CONFIG_REG] & INA226_CONFIG_SHUNT_CT_MASK) >> INA226_CONFIG_SHUNT_CT_POS);
        TEST_ASSERT_EQUAL(conv_time, actual_conv_time);
    }
}

// =========================================================================
// Tests for INA226_Reset
// =========================================================================

void test_INA226_Reset_Should_Return_Invalid_Param_Error_On_Null_Pointer(void) {
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = Q16_16_FROM_INT(100U), .power_lsb_q16_16_uW = 25U * Q16_16_FROM_INT(100U) };

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Reset(NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Reset(&sensor_null_cfg));
}

void test_INA226_Reset_Should_Write_Reset_Command_And_Clear_Handle_Calibration(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    // Calibrate first so handle holds non-zero LSBs.
    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 100U));
    TEST_ASSERT_NOT_EQUAL_UINT32(0U, sensor.current_lsb_q16_16_uA);
    TEST_ASSERT_NOT_EQUAL_UINT32(0U, sensor.power_lsb_q16_16_uW);

    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Reset(&sensor),
        "INA226_Reset not return INA226_OK."
    );

    TEST_ASSERT_EQUAL_HEX16(INA226_CONFIG_RESET_MASK, mock_ina226_registers[INA226_CONFIG_REG]);

    // Reset must zero calibration fields while preserving cfg pointer.
    TEST_ASSERT_EQUAL_PTR(&cfg, sensor.cfg);
    TEST_ASSERT_EQUAL_UINT32(0U, sensor.current_lsb_q16_16_uA);
    TEST_ASSERT_EQUAL_UINT32(0U, sensor.power_lsb_q16_16_uW);
}

void test_INA226_Reset_Should_Preserve_Handle_State_On_I2C_Fail(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    TEST_ASSERT_EQUAL(INA226_OK, INA226_Calibrate(&sensor, 100U));
    uint32_t expected_current_lsb = sensor.current_lsb_q16_16_uA;
    uint32_t expected_power_lsb = sensor.power_lsb_q16_16_uW;

    mock_i2c_write_fail = true;
    TEST_ASSERT_EQUAL(INA226_ERR_I2C, INA226_Reset(&sensor));
    mock_i2c_write_fail = false;

    TEST_ASSERT_EQUAL_PTR(&cfg, sensor.cfg);
    TEST_ASSERT_EQUAL_UINT32(expected_current_lsb, sensor.current_lsb_q16_16_uA);
    TEST_ASSERT_EQUAL_UINT32(expected_power_lsb, sensor.power_lsb_q16_16_uW);
}

// =========================================================================
// Tests for INA226_Set_Alert_Latch
// =========================================================================

void test_INA226_Set_Alert_Latch_Should_Return_Invalid_Param_Error_On_Null_Pointer(void) {
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = 0U, .power_lsb_q16_16_uW = 0U };

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Alert_Latch(NULL, INA226_ALERT_LATCH_ENABLE));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Alert_Latch(&sensor_null_cfg, INA226_ALERT_LATCH_ENABLE));
}

void test_INA226_Set_Alert_Latch_Should_Return_Invalid_Param_Error_On_Invalid_Options(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Alert_Latch(&sensor, 2U));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Set_Alert_Latch(&sensor, 255U));
}

void test_INA226_Set_Alert_Latch_Should_Write_Correct_Bit_To_Register(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);

    // Test Enable
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Latch(&sensor, INA226_ALERT_LATCH_ENABLE),
        "INA226_Set_Alert_Latch not return INA226_OK for Enable."
    );
    TEST_ASSERT_EQUAL_HEX16(INA226_MASK_EN_LEN_BIT, mock_ina226_registers[INA226_MASK_EN_REG] & INA226_MASK_EN_LEN_BIT);

    // Test Transparent (Disable)
    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Set_Alert_Latch(&sensor, INA226_ALERT_LATCH_TRANSPARENT),
        "INA226_Set_Alert_Latch not return INA226_OK for Transparent."
    );
    TEST_ASSERT_EQUAL_HEX16(0U, mock_ina226_registers[INA226_MASK_EN_REG] & INA226_MASK_EN_LEN_BIT);
}

// =========================================================================
// Tests for INA226_Read_Manufacturer_ID
// =========================================================================

void test_INA226_Read_Manufacturer_ID_Should_Return_Invalid_Param_On_Null_Pointer(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = 0U, .power_lsb_q16_16_uW = 0U };
    uint16_t manufacturer_id = 0U;

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Manufacturer_ID(NULL, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Manufacturer_ID(&sensor, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Manufacturer_ID(NULL, &manufacturer_id));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Manufacturer_ID(&sensor_null_cfg, &manufacturer_id));
}

void test_INA226_Read_Manufacturer_ID_Should_Return_Correct_Reg_Val(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    uint16_t manufacturer_id = 0U;

    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Read_Manufacturer_ID(&sensor, &manufacturer_id),
        "INA226_Read_Manufacturer_ID not return INA226_OK."
    );

    TEST_ASSERT_EQUAL_HEX16(0x5449U, manufacturer_id);
    TEST_ASSERT_EQUAL(mock_ina226_registers[INA226_MANUFACTURER_ID_REG], manufacturer_id);
}

// =========================================================================
// Tests for INA226_Read_Die_ID
// =========================================================================

void test_INA226_Read_Die_ID_Should_Return_Invalid_Param_On_Null_Pointer(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    ina226_handle_t sensor_null_cfg = { .cfg = NULL, .current_lsb_q16_16_uA = 0U, .power_lsb_q16_16_uW = 0U };
    uint16_t die_id = 0U;

    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Die_ID(NULL, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Die_ID(&sensor, NULL));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Die_ID(NULL, &die_id));
    TEST_ASSERT_EQUAL(INA226_ERR_INVALID_PARAM, INA226_Read_Die_ID(&sensor_null_cfg, &die_id));
}

void test_INA226_Read_Die_ID_Should_Return_Correct_Reg_Val(void) {
    ina226_config_t cfg = Make_Mock_Config(0x40U, 100000U);
    ina226_handle_t sensor = Make_Mock_Handle(&cfg);
    uint16_t die_id = 0U;

    TEST_ASSERT_EQUAL_MESSAGE(
        INA226_OK,
        INA226_Read_Die_ID(&sensor, &die_id),
        "INA226_Read_Die_ID not return INA226_OK."
    );

    TEST_ASSERT_EQUAL_HEX16(0x2260U, die_id);
    TEST_ASSERT_EQUAL(mock_ina226_registers[INA226_DIE_ID_REG], die_id);
}

// =========================================================================
// Tests for INA226 version macros
// =========================================================================

void test_INA226_Version_Macros_Should_Be_Consistent(void) {
    char expected[16];

    snprintf(expected, sizeof(expected), "%u.%u.%u",
             INA226_VERSION_MAJOR, INA226_VERSION_MINOR, INA226_VERSION_PATCH);

    TEST_ASSERT_EQUAL_STRING_MESSAGE(
        expected,
        INA226_VERSION_STRING,
        "INA226_VERSION_STRING does not match MAJOR.MINOR.PATCH macros."
    );

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        INA226_VERSION_MAJOR,
        (uint32_t)((INA226_VERSION >> 16U) & 0xFFU),
        "INA226_VERSION major byte does not match INA226_VERSION_MAJOR."
    );

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        INA226_VERSION_MINOR,
        (uint32_t)((INA226_VERSION >> 8U) & 0xFFU),
        "INA226_VERSION minor byte does not match INA226_VERSION_MINOR."
    );

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(
        INA226_VERSION_PATCH,
        (uint32_t)(INA226_VERSION & 0xFFU),
        "INA226_VERSION patch byte does not match INA226_VERSION_PATCH."
    );
}

int main(void) {
    UNITY_BEGIN();

    // INA226_Read_Current
    RUN_TEST(test_INA226_Read_Current_Should_Return_Error_On_Invalid_Params);
    RUN_TEST(test_INA226_Read_Current_Should_Return_Zero_When_Uncalibrated);
    RUN_TEST(test_INA226_Read_Current_Should_Handle_Large_Q16_16_Values_Without_Overflow);
    RUN_TEST(test_INA226_Read_Current_Should_Calculate_Correct_Values);
    RUN_TEST(test_INA226_Read_Current_Should_Apply_Fractional_Q16_16_LSB);

    // INA226_Read_Power
    RUN_TEST(test_INA226_Read_Power_Should_Return_Error_On_Invalid_Params);
    RUN_TEST(test_INA226_Read_Power_Should_Return_Zero_When_Uncalibrated);
    RUN_TEST(test_INA226_Read_Power_Should_Handle_Large_Q16_16_Values_Without_Overflow);
    RUN_TEST(test_INA226_Read_Power_Should_Calculate_Correct_Values);
    RUN_TEST(test_INA226_Read_Power_Should_Apply_Fractional_Q16_16_LSB);

    // INA226_Read_Shunt_Voltage
    RUN_TEST(test_INA226_Read_Shunt_Voltage_Should_Return_Error_On_Invalid_Params);
    RUN_TEST(test_INA226_Read_Shunt_Voltage_Should_Calculate_Correct_Values);

    // INA226_Read_Bus_Voltage
    RUN_TEST(test_INA226_Read_Bus_Voltage_Should_Return_Error_On_Invalid_Params);
    RUN_TEST(test_INA226_Read_Bus_Voltage_Should_Calculate_Correct_Values);

    // Datasheet Table 1 End-to-End
    RUN_TEST(test_INA226_Datasheet_Table1_Example_End_To_End);

    // INA226_Set_Alert_Pin_Function
    RUN_TEST(test_INA226_Set_Alert_Pin_Function_Should_Return_Error_On_Invalid_Params);
    RUN_TEST(test_INA226_Set_Alert_Pin_Function_Should_Do_Correct_Bitwise_Operation);

    // INA226_Get_Alert_Pin_Function
    RUN_TEST(test_INA226_Get_Alert_Pin_Function_Should_Return_Error_On_Invalid_Params);
    RUN_TEST(test_INA226_Get_Alert_Pin_Function_Should_Read_Correct_Alert_Function_Option);

    // INA226_Set_Alert_Limit
    RUN_TEST(test_INA226_Set_Alert_Limit_Should_Return_Error_On_Invalid_Params);
    RUN_TEST(test_INA226_Set_Alert_Limit_Should_Return_Invalid_State_When_No_Limit_Function_Configured);
    RUN_TEST(test_INA226_Set_Alert_Limit_Should_Return_Error_Math_Overflow_For_Invalid_Shunt_Limit_Values);
    RUN_TEST(test_INA226_Set_Alert_Limit_Should_Return_Error_Math_Overflow_For_Invalid_Bus_Limit_Values);
    RUN_TEST(test_INA226_Set_Alert_Limit_Should_Return_Error_Math_Overflow_For_Invalid_Power_Limit_Values);
    RUN_TEST(test_INA226_Set_Alert_Limit_Should_Accept_Int32_Max_Power_Limit_For_Large_Power_LSB);
    RUN_TEST(test_INA226_Set_Alert_Limit_Should_Return_Invalid_Param_For_Negative_Bus_Limit_Values);
    RUN_TEST(test_INA226_Set_Alert_Limit_Should_Return_Invalid_Param_For_Negative_Power_Limit_Values);
    RUN_TEST(test_INA226_Set_Alert_Limit_Should_Write_Correct_Limit_Value_To_Register_For_Shunt);
    RUN_TEST(test_INA226_Set_Alert_Limit_Should_Write_Correct_Limit_Value_To_Register_For_Bus);
    RUN_TEST(test_INA226_Set_Alert_Limit_Should_Write_Correct_Limit_Value_To_Register_For_Power);

    // INA226_Get_Alert_Status
    RUN_TEST(test_INA226_Get_Alert_Status_Should_Return_Error_On_Invalid_Params);
    RUN_TEST(test_INA226_Get_Alert_Status_Should_Read_Correct_Alert_Status);

    // INA226_Calibrate
    RUN_TEST(test_INA226_Calibrate_Should_Return_Invalid_Param_Error_On_Null_Pointer);
    RUN_TEST(test_INA226_Calibrate_Should_Return_Invalid_Param_Error_On_Zero_Values);
    RUN_TEST(test_INA226_Calibrate_Should_Calculate_Correct_Values);
    RUN_TEST(test_INA226_Calibrate_Should_BackCalculate_Actual_Q16_16_LSB_From_Rounded_Cal);
    RUN_TEST(test_INA226_Calibrate_Should_Calculate_Correct_Values_For_All_Valid_Cal_Range);
    RUN_TEST(test_INA226_Calibrate_Should_Round_To_Nearest);
    RUN_TEST(test_INA226_Calibrate_Should_Return_Invalid_Param_Error_On_Out_Of_Range_Cal);
    RUN_TEST(test_INA226_Calibrate_Should_Detect_Current_LSB_Q16_16_Overflow);
    RUN_TEST(test_INA226_Calibrate_Should_Detect_Power_LSB_Q16_16_Overflow);
    RUN_TEST(test_INA226_Calibrate_Should_Return_I2C_Error_On_Write_Fail);

    // INA226_Set_Averaging_Mode
    RUN_TEST(test_INA226_Set_Averaging_Mode_Should_Return_Invalid_Param_Error_On_Null_Pointer);
    RUN_TEST(test_INA226_Set_Averaging_Mode_Should_Return_Invalid_Param_Error_On_Invalid_Avg_Time_Options);
    RUN_TEST(test_INA226_Set_Averaging_Mode_Should_Write_Correct_Avg_Time_Option_To_Register);

    // INA226_Set_Operating_Mode
    RUN_TEST(test_INA226_Set_Operating_Mode_Should_Return_Invalid_Param_Error_On_Null_Pointer);
    RUN_TEST(test_INA226_Set_Operating_Mode_Should_Return_Invalid_Param_Error_On_Invalid_Mode_Options);
    RUN_TEST(test_INA226_Set_Operating_Mode_Should_Write_Correct_Mode_Option_To_Register);

    // INA226_Set_Bus_Voltage_Conversion_Time
    RUN_TEST(test_INA226_Set_Bus_Voltage_Conversion_Time_Should_Return_Invalid_Param_Error_On_Null_Pointer);
    RUN_TEST(test_INA226_Set_Bus_Voltage_Conversion_Time_Should_Return_Invalid_Param_Error_On_Invalid_Conv_Time_Options);
    RUN_TEST(test_INA226_Set_Bus_Voltage_Conversion_Time_Should_Write_Correct_Conv_Time_Option_To_Register);

    // INA226_Set_Shunt_Voltage_Conversion_Time
    RUN_TEST(test_INA226_Set_Shunt_Voltage_Conversion_Time_Should_Return_Invalid_Param_Error_On_Null_Pointer);
    RUN_TEST(test_INA226_Set_Shunt_Voltage_Conversion_Time_Should_Return_Invalid_Param_Error_On_Invalid_Conv_Time_Options);
    RUN_TEST(test_INA226_Set_Shunt_Voltage_Conversion_Time_Should_Write_Correct_Conv_Time_Option_To_Register);

    // INA226_Reset
    RUN_TEST(test_INA226_Reset_Should_Return_Invalid_Param_Error_On_Null_Pointer);
    RUN_TEST(test_INA226_Reset_Should_Write_Reset_Command_And_Clear_Handle_Calibration);
    RUN_TEST(test_INA226_Reset_Should_Preserve_Handle_State_On_I2C_Fail);

    // INA226_Set_Alert_Latch
    RUN_TEST(test_INA226_Set_Alert_Latch_Should_Return_Invalid_Param_Error_On_Null_Pointer);
    RUN_TEST(test_INA226_Set_Alert_Latch_Should_Return_Invalid_Param_Error_On_Invalid_Options);
    RUN_TEST(test_INA226_Set_Alert_Latch_Should_Write_Correct_Bit_To_Register);

    // INA226_Read_Manufacturer_ID
    RUN_TEST(test_INA226_Read_Manufacturer_ID_Should_Return_Invalid_Param_On_Null_Pointer);
    RUN_TEST(test_INA226_Read_Manufacturer_ID_Should_Return_Correct_Reg_Val);

    // INA226_Read_Die_ID
    RUN_TEST(test_INA226_Read_Die_ID_Should_Return_Invalid_Param_On_Null_Pointer);
    RUN_TEST(test_INA226_Read_Die_ID_Should_Return_Correct_Reg_Val);

    // INA226 version macros
    RUN_TEST(test_INA226_Version_Macros_Should_Be_Consistent);

    return UNITY_END();
}
