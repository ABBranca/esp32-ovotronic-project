#include "tmp102q1.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"

i2c_device_config_t tmp102_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = TMP102_ADDRESS,
    .scl_speed_hz = 100000,
};

i2c_master_dev_handle_t tmp102_handle;

esp_err_t tmp102_init(i2c_master_bus_handle_t bus) {
  ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &tmp102_cfg, &tmp102_handle));

  return ESP_OK;
}
