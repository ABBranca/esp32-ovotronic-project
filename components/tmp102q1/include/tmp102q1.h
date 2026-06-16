#pragma once
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"

#define TMP102_ADDRESS 0x48

extern i2c_device_config_t tmp102_cfg;
extern i2c_master_dev_handle_t tmp102_handle;

esp_err_t tmp102_init(i2c_master_bus_handle_t bus);