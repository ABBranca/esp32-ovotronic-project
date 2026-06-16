#include "lcd1602.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "hal/i2c_types.h"
#include "portmacro.h"

i2c_device_config_t lcd1602_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = LCD1602_ADDRESS,
    .scl_speed_hz = 100000,
};

i2c_master_dev_handle_t lcd1602_handle;

// LCD1602 init Function API
esp_err_t lcd1602_init(i2c_master_bus_handle_t bus) {
  ESP_ERROR_CHECK(
      i2c_master_bus_add_device(bus, &lcd1602_cfg, &lcd1602_handle));

  vTaskDelay(40 / portTICK_PERIOD_MS);

  return ESP_OK;
}