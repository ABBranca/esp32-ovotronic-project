#include "supervisor.h"
#include "dc_motor_driver.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_log_level.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/ledc_types.h"
#include "lcd1602.h"
#include "limit_switch.h"
#include "portmacro.h"
#include "soc/gpio_num.h"
#include <stdint.h>

#define HOMING_TIMEOUT_MS 10000
#define MH_MAX_RETRY 3

TaskHandle_t supervisor_task_handle = NULL;
static const char *TAG = "Supervisor";

static const struct dc_motor_config dc_motor_config_pan = {
    .gpio_num_in1 = GPIO_NUM_41,
    .gpio_num_in2 = GPIO_NUM_42,
    .channel = LEDC_CHANNEL_3};

static const struct dc_motor_config dc_motor_config_eb = {
    .gpio_num_in1 = GPIO_NUM_15,
    .gpio_num_in2 = GPIO_NUM_7,
    .channel = LEDC_CHANNEL_4};

static const struct dc_motor_config dc_motor_config_planetary = {
    .gpio_num_in1 = GPIO_NUM_10,
    .gpio_num_in2 = GPIO_NUM_11,
    .channel = LEDC_CHANNEL_5};

volatile bool motion_direction[LS_AXIS_COUNT] = {
    false, false}; // false = forward, true = backward

esp_err_t homing_sequence() {

  uint32_t pending;

  ESP_LOGI(TAG, "Avvio sequenza di homing");
  lcd1602_clear();
  lcd1602_set_cursor(0, 3);
  lcd1602_print("HOMING...");

  const TickType_t start = xTaskGetTickCount();
  const TickType_t timeout = pdMS_TO_TICKS(HOMING_TIMEOUT_MS);

  dc_motor_driver_move_backward(&dc_motor_config_pan, 2048);
  motion_direction[LS_AXIS_PAN] = true;
  dc_motor_driver_move_backward(&dc_motor_config_eb, 2048);
  motion_direction[LS_AXIS_EB] = true;

  for (;;) {
    TickType_t elapsed = xTaskGetTickCount() - start;
    if (elapsed >= timeout) {
      dc_motor_driver_brake(&dc_motor_config_eb);
      dc_motor_driver_brake(&dc_motor_config_pan);
      ESP_LOG_LEVEL(ESP_LOG_ERROR, TAG, "Homing Timed Out");
      lcd1602_clear();
      lcd1602_set_cursor(0, 0);
      lcd1602_print("ERROR: Homing");
      lcd1602_set_cursor(1, 0);
      lcd1602_print("timed out");
      return ESP_ERR_TIMEOUT;
    }

    xTaskNotifyWait(0, UINT32_MAX, &pending, timeout - elapsed);
    vTaskDelay(pdMS_TO_TICKS(30)); // Debounce
    // ulTaskNotifyValueClear(NULL, UINT32_MAX);

    if (limit_switch_is_at_limit(LS_AXIS_PAN)) {
      dc_motor_driver_brake(&dc_motor_config_pan);
    }
    if (limit_switch_is_at_limit(LS_AXIS_EB)) {
      dc_motor_driver_brake(&dc_motor_config_eb);
    }

    if (limit_switch_is_at_limit(LS_AXIS_PAN) &&
        limit_switch_is_at_limit(LS_AXIS_EB)) {
      ESP_LOGI(TAG, "Sequenza di homing completata");
      lcd1602_clear();
      lcd1602_set_cursor(0, 3);
      lcd1602_print("PRONTO!");
      return ESP_OK;
    }
  }

  // dc_motor_driver_brake(&dc_motor_config_pan);
  // dc_motor_driver_brake(&dc_motor_config_eb);
}

void supervisor_task(void *pvParameters) {

  // Configurazione motori DC
  // Inizializzazione canali PWM per motori DCs
  pwm_dc_motor_driver_init(&dc_motor_config_pan);
  pwm_dc_motor_driver_init(&dc_motor_config_eb);
  pwm_dc_motor_driver_init(&dc_motor_config_planetary);

  // Messaggio Intro.
  lcd1602_set_cursor(0, 3);
  lcd1602_print("OVOTRONIC");

  vTaskDelay(pdMS_TO_TICKS(3000));

  if (supervisor_task_handle == NULL) {
    supervisor_task_handle = xTaskGetCurrentTaskHandle();
  }

  limit_switch_set_notify_task_handle(supervisor_task_handle);

  ESP_LOGI(TAG, "Supervisor task started");

  int attempts = 0;
  while (homing_sequence() != ESP_OK) {
    if (++attempts >= MH_MAX_RETRY) {
      // FAULT LATCHED: motori fermi, LCD errore, halt
      lcd1602_clear();
      lcd1602_set_cursor(0, 0);
      lcd1602_print("PANIC: MOTOR");
      lcd1602_set_cursor(1, 0);
      lcd1602_print("FAULT!");
      ESP_LOGE(TAG, "Homing fault: %d tentativi falliti", attempts);
      for (;;)
        vTaskDelay(pdMS_TO_TICKS(1000)); // o stato fault dedicato
    }
    vTaskDelay(pdMS_TO_TICKS(5000));
  }

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}