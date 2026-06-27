#include "limit_switch.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "portmacro.h"
#include "soc/gpio_num.h"

static EventGroupHandle_t limit_switch_event_group;
static volatile bool motion_direction[LS_AXIS_COUNT];

// TODO : Implementare la logica per aggiornare le variabili globali

static void IRAM_ATTR eb_limit_switch_isr_handler(void *arg) {
  // Gestione dell'interrupt del sensore di fine corsa
  // Qui puoi implementare la logica per aggiornare le variabili globali
  // pan_at_top, pan_at_bottom, breaker_at_begin, breaker_at_end
  // in base al sensore che ha generato l'interrupt.

  bool eb_towards_end =
      ((volatile bool *)arg)[LS_AXIS_EB]; // Variabile per tenere traccia della
                                          // direzione del movimento

  // Inverte la direzione del movimento dell'asse EB e la registra nell'event
  // group e variabile globale motion_direction

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (eb_towards_end) {
    xEventGroupSetBitsFromISR(limit_switch_event_group, BREAKER_AT_END_BIT,
                              &xHigherPriorityTaskWoken);
  } else {
    xEventGroupSetBitsFromISR(limit_switch_event_group, BREAKER_AT_BEGIN_BIT,
                              &xHigherPriorityTaskWoken);
  }

  portYIELD_FROM_ISR(
      xHigherPriorityTaskWoken); // Forza il contesto di esecuzione a

  ((volatile bool *)arg)[LS_AXIS_EB] = !eb_towards_end;
}

static void IRAM_ATTR pan_limit_switch_isr_handler(void *arg) {
  // Gestione dell'interrupt del sensore di fine corsa
  // Qui puoi implementare la logica per aggiornare le variabili globali
  // pan_at_top, pan_at_bottom, breaker_at_begin, breaker_at_end
  // in base al sensore che ha generato l'interrupt.

  bool pan_towards_top =
      ((volatile bool *)arg)[LS_AXIS_PAN]; // Variabile per tenere traccia della
                                           // direzione del movimento
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (pan_towards_top) {
    xEventGroupSetBitsFromISR(limit_switch_event_group, PAN_AT_TOP_BIT,
                              &xHigherPriorityTaskWoken);
  } else {
    xEventGroupSetBitsFromISR(limit_switch_event_group, PAN_AT_BOTTOM_BIT,
                              &xHigherPriorityTaskWoken);
  }

  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

  ((volatile bool *)arg)[LS_AXIS_PAN] = !pan_towards_top;
}

static gpio_isr_t select_limit_switch_isr_handler(const gpio_num_t gpio_num) {
#if defined(PAN_LIMIT_SWITCH_GPIO)
  if (gpio_num == (gpio_num_t)PAN_LIMIT_SWITCH_GPIO) {
    return pan_limit_switch_isr_handler;
  }
#endif

#if defined(EB_LIMIT_SWITCH_GPIO)
  if (gpio_num == (gpio_num_t)EB_LIMIT_SWITCH_GPIO) {
    return eb_limit_switch_isr_handler;
  }
#endif

  return eb_limit_switch_isr_handler;
}

void limit_switch_init(
    const gpio_num_t gpio_num) { // Inizializza il sensore di fine corsa

  if (limit_switch_event_group == NULL) {
    limit_switch_event_group = xEventGroupCreate();
  }

  gpio_config_t gpio_conf = {.pin_bit_mask = (1ULL << gpio_num),
                             .mode = GPIO_MODE_INPUT,
                             .pull_up_en = GPIO_PULLUP_DISABLE,
                             .pull_down_en = GPIO_PULLDOWN_DISABLE,
                             .intr_type = GPIO_INTR_POSEDGE};

  gpio_config(&gpio_conf);
  gpio_isr_handler_add(
      gpio_num, select_limit_switch_isr_handler(gpio_num),
      (void *)&motion_direction); // Passa un puntatore alla variabile
                                  // motion_direction come argomento
}

EventGroupHandle_t get_limit_switch_event_group(void) {
  return limit_switch_event_group;
}

bool get_axis_motion_direction(limit_switch_axis_t axis) {
  if (axis < LS_AXIS_COUNT) {
    return motion_direction[axis]; // Restituisce true se l'asse si sta muovendo
                                   // verso il limite superiore
  }
  return false; // Restituisce false se l'asse non è valido
}