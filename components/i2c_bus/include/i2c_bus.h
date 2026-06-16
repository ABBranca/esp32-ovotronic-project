#pragma once
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"

#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define I2C_PORT 0

extern i2c_master_bus_config_t i2c_mst_cfg;
extern i2c_master_bus_handle_t bus_handle;

esp_err_t i2c_init();