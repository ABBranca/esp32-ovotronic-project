#include "supervisor.h"
#include "dc_motor_driver.h"
#include "driver/gpio.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/ledc_types.h"
#include "lcd1602.h"
#include "limit_switch.h"
#include "portmacro.h"
#include "soc/gpio_num.h"
#include <stdint.h>
#include <stdio.h>

TaskHandle_t supervisor_task_handle = NULL;

volatile bool motion_direction[LS_AXIS_COUNT] = {
    false, false}; // false = forward, true = backward

void supervisor_task(void *pvParameters) {

  // Configurazione motori DC
  struct dc_motor_config dc_motor_config_pan = {.gpio_num_in1 = GPIO_NUM_41,
                                                .gpio_num_in2 = GPIO_NUM_42,
                                                .channel = LEDC_CHANNEL_3};

  struct dc_motor_config dc_motor_config_eb = {.gpio_num_in1 = GPIO_NUM_15,
                                               .gpio_num_in2 = GPIO_NUM_7,
                                               .channel = LEDC_CHANNEL_4};

  struct dc_motor_config dc_motor_config_planetary = {
      .gpio_num_in1 = GPIO_NUM_10,
      .gpio_num_in2 = GPIO_NUM_11,
      .channel = LEDC_CHANNEL_5};

  // Messaggio Intro.
  lcd1602_set_cursor(0, 3);
  lcd1602_print("OVOTRONIC");

  vTaskDelay(pdMS_TO_TICKS(3000));

  if (supervisor_task_handle == NULL) {
    supervisor_task_handle = xTaskGetCurrentTaskHandle();
  }

  limit_switch_set_notify_task_handle(supervisor_task_handle);

  printf("Supervisor task started\n");

  printf("Idle PAN: %d, EB: %d\n", motion_direction[LS_AXIS_PAN],
         motion_direction[LS_AXIS_EB]);

  static uint32_t pending;

  printf("Avvio sequenza di homing\n");
  lcd1602_clear();
  lcd1602_set_cursor(0, 3);
  lcd1602_print("HOMING...");

  for (;;) {
    xTaskNotifyWait(0, UINT32_MAX, &pending, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(30)); // Debounce
    ulTaskNotifyValueClear(NULL, UINT32_MAX);

    if (limit_switch_is_at_limit(LS_AXIS_PAN) &&
        limit_switch_is_at_limit(LS_AXIS_EB))
      break;
  }

  printf("Homing completato\n");
  lcd1602_clear();
  lcd1602_set_cursor(0, 3);
  lcd1602_print("PRONTO!");

  while (true) {

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}