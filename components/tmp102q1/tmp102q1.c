#include "tmp102q1.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include <stdint.h>

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

esp_err_t tmp102_read_temperature(float *out_temp) {

  uint8_t buffer[2];

  uint8_t reg = TMP102_TEMP_REG;
  esp_err_t err =
      i2c_master_transmit_receive(tmp102_handle, &reg, 1, buffer, 2, 100);
  if (err != ESP_OK) {
    return err; // Returns -1 if something goes south.
  }

  int16_t read = (int16_t)(buffer[0] << 8 | buffer[1]);
  *out_temp = (read >> 4) * 0.0625;

  return ESP_OK;
}