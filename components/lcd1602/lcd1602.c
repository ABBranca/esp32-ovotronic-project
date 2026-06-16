#include "lcd1602.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "hal/i2c_types.h"
#include "portmacro.h"
#include <stdint.h>

i2c_device_config_t lcd1602_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = LCD1602_ADDRESS,
    .scl_speed_hz = 100000,
};

i2c_master_dev_handle_t lcd1602_handle;

static uint8_t lcd_backlight = LCD_BL;

static esp_err_t lcd_pulse_enable(uint8_t data) {

  uint8_t b = data | LCD_EN;
  esp_err_t err = i2c_master_transmit(lcd1602_handle, &b, 1, -1);
  if (err != ESP_OK)
    return err;
  b = data & ~LCD_EN;
  err = i2c_master_transmit(lcd1602_handle, &b, 1, -1);
  if (err != ESP_OK)
    return err;

  return ESP_OK;
}

static esp_err_t lcd_send_nibble(uint8_t nibble, uint8_t mode) {
  uint8_t data = (nibble << 4) | mode | lcd_backlight;
  return lcd_pulse_enable(data);
}

static esp_err_t lcd_send_byte(const uint8_t byte, const uint8_t mode) {
  uint8_t alto = byte >> 4;
  esp_err_t err = lcd_send_nibble(alto, mode);
  if (err != ESP_OK)
    return err;

  uint8_t basso = byte & 0x0F;
  err = lcd_send_nibble(basso, mode);
  if (err != ESP_OK)
    return err;

  return ESP_OK;
}

// LCD1602 init Function API
esp_err_t lcd1602_init(i2c_master_bus_handle_t bus) {
  ESP_ERROR_CHECK(
      i2c_master_bus_add_device(bus, &lcd1602_cfg, &lcd1602_handle));

  vTaskDelay(40 / portTICK_PERIOD_MS);

  for (uint8_t i = 0; i < 3; i++) {
    lcd_send_nibble(0x3, 0);
    vTaskDelay(40 / portTICK_PERIOD_MS);
  }

  lcd_send_nibble(0x2, 0);

  lcd_send_byte(0x28, 0);
  lcd_send_byte(0x08, 0);
  lcd_send_byte(0x01, 0);
  vTaskDelay(20 / portTICK_PERIOD_MS);
  lcd_send_byte(0x06, 0);
  lcd_send_byte(0x0c, 0);

  return ESP_OK;
}