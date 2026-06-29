#include "ec11.h"
#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "hal/pcnt_types.h"
#include "portmacro.h"
#include "soc/gpio_num.h"
#include <stdint.h>
#include <string.h>

static pcnt_unit_config_t ec11_unit_config = {.high_limit = PCNT_MAX_COUNT,
                                              .low_limit = PCNT_MIN_COUNT};

static pcnt_unit_handle_t ec11_unit_handle = NULL;

static TaskHandle_t s_notify_target = NULL;
static gpio_num_t s_sw_gpio;  // per leggere/debounce SW
static int64_t s_last_us = 0; // anti-bounce

void ec11_set_notify_task_handle(const TaskHandle_t task_handle) {
  s_notify_target = task_handle;
}

void IRAM_ATTR isr_ec11_sw_evt(void *args) { // ISR handle definition
  if (s_notify_target == NULL) {
    return;
  }

  int64_t now = esp_timer_get_time();
  if (now - s_last_us < 15000)
    return;
  s_last_us = now;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xTaskNotifyFromISR(s_notify_target, EC11_EVT_BUTTON, eSetBits,
                     &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void ec11_init(const ec11_config_t *config) {
  s_sw_gpio = config->gpio_sw;
  const gpio_config_t ec11_sw_gpio = {.mode = GPIO_MODE_INPUT,
                                      .intr_type = GPIO_INTR_NEGEDGE,
                                      .pull_up_en = GPIO_PULLUP_DISABLE,
                                      .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                      .pin_bit_mask = 1ULL << s_sw_gpio};

  ESP_ERROR_CHECK(gpio_config(&ec11_sw_gpio));

  gpio_isr_handler_add(config->gpio_sw, isr_ec11_sw_evt, NULL);

  ESP_ERROR_CHECK(pcnt_new_unit(&ec11_unit_config, &ec11_unit_handle));
  pcnt_glitch_filter_config_t f = {.max_glitch_ns = 1000};
  ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(ec11_unit_handle, &f));

  pcnt_chan_config_t ec11_chan_config_a = {.edge_gpio_num = config->gpio_dt,
                                           .level_gpio_num = config->gpio_clk};

  pcnt_channel_handle_t ec11_chan_handle_a = NULL;
  ESP_ERROR_CHECK(pcnt_new_channel(ec11_unit_handle, &ec11_chan_config_a,
                                   &ec11_chan_handle_a));

  pcnt_chan_config_t ec11_chan_config_b = {.edge_gpio_num = config->gpio_clk,
                                           .level_gpio_num = config->gpio_dt};

  pcnt_channel_handle_t ec11_chan_handle_b = NULL;

  ESP_ERROR_CHECK(pcnt_new_channel(ec11_unit_handle, &ec11_chan_config_b,
                                   &ec11_chan_handle_b));
  ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
      ec11_chan_handle_b, PCNT_CHANNEL_EDGE_ACTION_DECREASE,
      PCNT_CHANNEL_EDGE_ACTION_INCREASE));
  ESP_ERROR_CHECK(pcnt_channel_set_level_action(
      ec11_chan_handle_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
      PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
  ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
      ec11_chan_handle_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
      PCNT_CHANNEL_EDGE_ACTION_DECREASE));
  ESP_ERROR_CHECK(pcnt_channel_set_level_action(
      ec11_chan_handle_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
      PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

  ESP_ERROR_CHECK(pcnt_unit_enable(ec11_unit_handle));
  ESP_ERROR_CHECK(pcnt_unit_clear_count(ec11_unit_handle));
  ESP_ERROR_CHECK(pcnt_unit_start(ec11_unit_handle));
}

void ec11_clear_count(void) {
  ESP_ERROR_CHECK(pcnt_unit_clear_count(ec11_unit_handle));
}

int ec11_get_count(void) {
  int count = 0;
  pcnt_unit_get_count(ec11_unit_handle, &count);
  return count;
}

bool ec11_button_pressed(void) { return gpio_get_level(s_sw_gpio) == 0; }