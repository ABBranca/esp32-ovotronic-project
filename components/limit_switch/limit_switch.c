#include "limit_switch.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "hal/gpio_types.h"
#include "soc/gpio_num.h"

static void IRAM_ATTR limit_switch_isr_handler(void *arg) {
  // Gestione dell'interrupt del sensore di fine corsa
  // Qui puoi implementare la logica per aggiornare le variabili globali
  // pan_at_top, pan_at_bottom, breaker_at_begin, breaker_at_end
  // in base al sensore che ha generato l'interrupt.
}

void limit_switch_init(const gpio_num_t gpio_num) {
  gpio_config_t gpio_conf = {.pin_bit_mask = (1ULL << gpio_num),
                             .mode = GPIO_MODE_INPUT,
                             .pull_up_en = GPIO_PULLUP_DISABLE,
                             .pull_down_en = GPIO_PULLDOWN_DISABLE,
                             .intr_type = GPIO_INTR_ANYEDGE};

  gpio_config(&gpio_conf);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(gpio_num, limit_switch_isr_handler, NULL);
}
