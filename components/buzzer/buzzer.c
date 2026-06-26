#include "buzzer.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "hal/ledc_types.h"
#include "portmacro.h"
#include "soc/clk_tree_defs.h"
#include "soc/gpio_num.h"
#include <stdint.h>

void buzzer_init(const gpio_num_t gpio_num, const uint32_t frequency) {
  const ledc_timer_config_t buzzer_ledc_timer = {
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .timer_num = LEDC_TIMER_1,
      .freq_hz = frequency,
      .duty_resolution = LEDC_TIMER_10_BIT,
      .clk_cfg = LEDC_AUTO_CLK};
  ESP_ERROR_CHECK(ledc_timer_config(&buzzer_ledc_timer));

  const ledc_channel_config_t buzzer_ledc_channel = {.speed_mode =
                                                         LEDC_LOW_SPEED_MODE,
                                                     .channel = LEDC_CHANNEL_2,
                                                     .timer_sel = LEDC_TIMER_1,
                                                     .gpio_num = gpio_num,
                                                     .duty = 0,
                                                     .hpoint = 0};
  ESP_ERROR_CHECK(ledc_channel_config(&buzzer_ledc_channel));
}

void buzzer_ring(const uint32_t frequency, const uint32_t duration) {
  static const char *TAG = "buzzer";
  esp_err_t err;

  err = ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_1, frequency);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "ledc_set_freq failed: %s", esp_err_to_name(err));
  }

  err =
      ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 512); // 50% duty cycle
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "ledc_set_duty (on) failed: %s", esp_err_to_name(err));
  }

  err = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "ledc_update_duty (on) failed: %s", esp_err_to_name(err));
  }

  vTaskDelay(duration / portTICK_PERIOD_MS);

  err =
      ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 0); // Turn off buzzer
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "ledc_set_duty (off) failed: %s", esp_err_to_name(err));
  }

  err = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "ledc_update_duty (off) failed: %s", esp_err_to_name(err));
  }
}
