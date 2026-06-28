#include "dc_motor_driver.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "hal/gpio_types.h"
#include "hal/ledc_types.h"
#include "soc/clk_tree_defs.h"
#include "soc/gpio_num.h"
#include <stdint.h>

static inline uint32_t slow_decay_duty(uint32_t speed) {
  return DC_MOTOR_DUTY_MAX - speed;
}

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
  ledc_channel_config_t ledc_channel_gpio1 = {
      .channel = config->channel,
      .duty = 0,
      .gpio_num = config->gpio_num_in1, // GPIO pin for the motor control
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .hpoint = 0,
      .timer_sel = LEDC_TIMER_2};

  // Initialize the DC motor driver here
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_gpio1));
}

void dc_motor_driver_move_forward(const struct dc_motor_config *config,
                                  uint32_t speed) {
  gpio_reset_pin(config->gpio_num_in1);
  gpio_reset_pin(config->gpio_num_in2);

  ledc_set_pin(config->gpio_num_in2, LEDC_LOW_SPEED_MODE, config->channel);

  ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, config->channel,
                           slow_decay_duty(speed), 0);

  gpio_set_direction(config->gpio_num_in1, GPIO_MODE_OUTPUT);
  gpio_set_level(config->gpio_num_in1, 1);
}
void dc_motor_driver_move_backward(const struct dc_motor_config *config,
                                   uint32_t speed) {
  gpio_reset_pin(config->gpio_num_in1);
  gpio_reset_pin(config->gpio_num_in2);

  ledc_set_pin(config->gpio_num_in1, LEDC_LOW_SPEED_MODE, config->channel);

  ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, config->channel,
                           slow_decay_duty(speed), 0);

  gpio_set_direction(config->gpio_num_in2, GPIO_MODE_OUTPUT);
  gpio_set_level(config->gpio_num_in2, 1);
}

void dc_motor_driver_brake(const struct dc_motor_config *config) {
  ledc_stop(LEDC_LOW_SPEED_MODE, config->channel, 1);
  gpio_set_direction(config->gpio_num_in1, GPIO_MODE_OUTPUT);
  gpio_set_level(config->gpio_num_in1, 1);
  gpio_set_direction(config->gpio_num_in2, GPIO_MODE_OUTPUT);
  gpio_set_level(config->gpio_num_in2, 1);
}