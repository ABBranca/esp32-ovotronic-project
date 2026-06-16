#pragma once
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"

#define LCD1602_ADDRESS 0x27

extern i2c_device_config_t lcd1602_cfg;
extern i2c_master_dev_handle_t lcd1602_handle;

esp_err_t lcd1602_init(i2c_master_bus_handle_t bus);