#pragma once

#include "esp_attr.h"
#include "soc/gpio_num.h"

void limit_switch_init(const gpio_num_t gpio_num);
static void IRAM_ATTR limit_switch_isr_handler(void* arg);