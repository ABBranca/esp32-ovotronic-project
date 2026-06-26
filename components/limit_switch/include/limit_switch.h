#pragma once

#include "freertos/idf_additions.h"
#include "soc/gpio_num.h"
#define PAN_AT_BOTTOM_BIT (1 << 0)
#define PAN_AT_TOP_BIT (1 << 1)
#define BREAKER_AT_BEGIN_BIT (1 << 2)
#define BREAKER_AT_END_BIT (1 << 3)
#define PAN_LIMIT_SWITCH_GPIO 13
#define EB_LIMIT_SWITCH_GPIO 12
#define LS_AXIS_COUNT 2

typedef enum { LS_AXIS_PAN, LS_AXIS_EB } limit_switch_axis_t;

void limit_switch_init(const gpio_num_t gpio_num);

EventGroupHandle_t get_limit_switch_event_group(void);