#include "supervisor.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "limit_switch.h"
#include "portmacro.h"
#include <stdint.h>
#include <stdio.h>

TaskHandle_t supervisor_task_handle = NULL;
EventGroupHandle_t axis_motor_event_group = NULL;

volatile bool motion_direction[LS_AXIS_COUNT] = {
    false, false}; // false = forward, true = backward

void supervisor_task(void *pvParameters) {

  if (axis_motor_event_group == NULL) {
    axis_motor_event_group = xEventGroupCreate();
  }

  if (supervisor_task_handle == NULL) {
    supervisor_task_handle = xTaskGetCurrentTaskHandle();
  }

  limit_switch_set_notify_task_handle(supervisor_task_handle);

  printf("Supervisor task started\n");

  while (true) {
    uint32_t pending;
    xTaskNotifyWait(0, UINT32_MAX, &pending, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(30));
    ulTaskNotifyValueClear(NULL, UINT32_MAX);

    if (pending & (1 << LS_AXIS_PAN)) {
      if (motion_direction[LS_AXIS_PAN]) {
        xEventGroupSetBits(axis_motor_event_group, PAN_AT_TOP_BIT);
        printf("PAN AT TOP\n");
      } else {
        xEventGroupSetBits(axis_motor_event_group, PAN_AT_BOTTOM_BIT);
        printf("PAN AT BOTTOM\n");
      }
      motion_direction[LS_AXIS_PAN] = !motion_direction[LS_AXIS_PAN];
    }
  }
}