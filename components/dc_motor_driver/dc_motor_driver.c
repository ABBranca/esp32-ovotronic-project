#include "dc_motor_driver.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "hal/ledc_types.h"
#include "soc/clk_tree_defs.h"
#include "soc/gpio_num.h"

void pwm_dc_motor_driver_init_timer() {
  ledc_timer_config_t ledc_timer = {
      .duty_resolution = LEDC_TIMER_11_BIT, // Set duty resolution to 13 bits
      .freq_hz = 20000,                     // Set frequency to 20 kHz
      .speed_mode = LEDC_LOW_SPEED_MODE,    // Use low speed mode
      .timer_num = LEDC_TIMER_2,            // Use timer 2
      .clk_cfg = LEDC_AUTO_CLK};            // Auto select the clock source

  // Initialize the timer for the DC motor driver here
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
}

void pwm_dc_motor_driver_init(const struct dc_motor_config *config) {
  ledc_channel_config_t ledc_channel1 = {
      .channel = config->channel1,
      .duty = 0,
      .gpio_num = config->gpio_num_in1, // GPIO pin for the motor control
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .hpoint = 0,
      .timer_sel = LEDC_TIMER_2};

  // Initialize the DC motor driver here
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel1));

  ledc_channel_config_t ledc_channel2 = {
      .channel = config->channel2,
      .duty = 0,
      .gpio_num = config->gpio_num_in2, // GPIO pin for the motor control
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .hpoint = 0,
      .timer_sel = LEDC_TIMER_2};
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel2));
}
void dc_motor_driver_init(const struct dc_motor_config *config) {
  // Initialize the DC motor driver here
}