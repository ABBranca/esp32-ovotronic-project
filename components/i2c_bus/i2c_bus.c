#include "i2c_bus.h"
#include "driver/i2c_types.h"

i2c_master_bus_config_t i2c_mst_cfg = {
    .i2c_port = I2C_PORT,
    .sda_io_num = I2C_SDA_PIN,
    .scl_io_num = I2C_SCL_PIN,
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 7,
};

i2c_master_bus_handle_t bus_handle;

esp_err_t i2c_init() {
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_cfg, &bus_handle));
  return ESP_OK;
}