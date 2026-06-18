#include "sg90.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "hal/ledc_types.h"
#include "soc/clk_tree_defs.h"
#include "soc/gpio_num.h"
#include <stdint.h>

esp_err_t sg90_timer_init(void) {

  const ledc_timer_config_t sg90_ledc_timer = {
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .timer_num = LEDC_TIMER_0,
      .freq_hz = SG90_PWM_FREQ,
      .duty_resolution = LEDC_TIMER_14_BIT,
      .clk_cfg = LEDC_AUTO_CLK};

  esp_err_t err = ledc_timer_config(&sg90_ledc_timer);
  if (err != ESP_OK) {
    return err;
  }

  return ESP_OK;
}

esp_err_t sg90_channel_init(const ledc_channel_t channel,
                            const gpio_num_t gpio) {

  ledc_channel_config_t sg90_ledc_channel = {.speed_mode = LEDC_LOW_SPEED_MODE,
                                             .channel = channel,
                                             .timer_sel = LEDC_TIMER_0,
                                             .gpio_num = gpio,
                                             .duty = 0,
                                             .hpoint = 0};

  esp_err_t err = ledc_channel_config(&sg90_ledc_channel);
  if (err != ESP_OK) {
    return err;
  }

  return ESP_OK;
}

esp_err_t sg90_set_angle(const ledc_channel_t channel, float angle) {

  if (angle < 0) {
    angle = 0;
  } else if (angle > 180) {
    angle = 180;
  }

  float pulse_us =
      SG90_MIN_PULSE_US +
      angle / SG90_MAX_ANGLE * (SG90_MAX_PULSE_US - SG90_MIN_PULSE_US);
  uint32_t duty = pulse_us / SG90_PERIOD_US * SG90_DUTY_MAX;

  esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty);
  if (err != ESP_OK) {
    return err;
  }
  err = ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
  if (err != ESP_OK) {
    return err;
  }

  return ESP_OK;
}