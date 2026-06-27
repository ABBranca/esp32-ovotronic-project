#include "limit_switch.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "portmacro.h"
#include "soc/gpio_num.h"
#include <stdint.h>

static TaskHandle_t s_notify_target = NULL; // TaskHandle_t is a pointer.

void limit_switch_set_notify_task_handle(TaskHandle_t task_handle) {
  s_notify_target = task_handle;
}

bool limit_switch_is_at_limit(ls_axis_t axis) {
  gpio_num_t gpio =
      (axis == LS_AXIS_PAN) ? PAN_LIMIT_SWITCH_GPIO : EB_LIMIT_SWITCH_GPIO;
  return gpio_get_level(gpio) == 1; // at-limit = HIGH (switch NC aperto)
}

static void IRAM_ATTR limit_switch_isr_handler(void *arg) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  uint32_t axis =
      (uintptr_t)arg; // Cast the argument to uintptr_t and then to uint32_t
  xTaskNotifyFromISR(s_notify_target, (1u << axis), eSetBits,
                     &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void limit_switch_init(
    const gpio_num_t gpio_num) { // Inizializza il sensore di fine corsa

  gpio_config_t gpio_conf = {.pin_bit_mask = (1ULL << gpio_num),
                             .mode = GPIO_MODE_INPUT,
                             .pull_up_en = GPIO_PULLUP_DISABLE,
                             .pull_down_en = GPIO_PULLDOWN_DISABLE,
                             .intr_type = GPIO_INTR_POSEDGE};

  uintptr_t axis = 0; // Default value, will be set based on gpio_num

  if (gpio_num == PAN_LIMIT_SWITCH_GPIO) {
    axis = (uintptr_t)LS_AXIS_PAN;
  } else if (gpio_num == EB_LIMIT_SWITCH_GPIO) {
    axis = (uintptr_t)LS_AXIS_EB;
  }

  gpio_config(&gpio_conf);
  gpio_isr_handler_add(gpio_num, limit_switch_isr_handler, (void *)axis);
}
