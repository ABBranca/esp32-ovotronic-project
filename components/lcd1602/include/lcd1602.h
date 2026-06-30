#pragma once
#include <stddef.h>
#include <stdint.h>

#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"

#define LCD1602_ADDRESS 0x27
#define LCD_RS 0x01  // P0
#define LCD_RW 0x02  // P1 (sempre 0)
#define LCD_EN 0x04  // P2
#define LCD_BL 0x08  // P3 Backlight
#define LCD_CHAR_WIDTH 16

extern i2c_device_config_t lcd1602_cfg;
extern i2c_master_dev_handle_t lcd1602_handle;

esp_err_t lcd1602_init(i2c_master_bus_handle_t bus);
esp_err_t lcd1602_print(const char* str);
esp_err_t lcd1602_set_cursor(uint8_t row, uint8_t col);
esp_err_t lcd1602_clear();

void lcd1602_step_marquee(const char* text, size_t lenght, size_t offset);
