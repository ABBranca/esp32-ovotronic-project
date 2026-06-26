#pragma once
#include <stdint.h>

#include "soc/gpio_num.h"

#define BUZZER_DEFAULT_FREQUENCY 3000

void buzzer_init(const gpio_num_t gpio_num, const uint32_t frequency);
void buzzer_ring(const uint32_t frequency, const uint32_t duration);