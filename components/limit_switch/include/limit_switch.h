#pragma once

#include "freertos/idf_additions.h"
#include "soc/gpio_num.h"
#include <stdbool.h>

#define PAN_AT_BOTTOM_BIT (1 << 0)
#define PAN_AT_TOP_BIT (1 << 1)
#define BREAKER_AT_BEGIN_BIT (1 << 2)
#define BREAKER_AT_END_BIT (1 << 3)
#define PAN_LIMIT_SWITCH_GPIO 13
#define EB_LIMIT_SWITCH_GPIO 12
#define LS_AXIS_COUNT 2

typedef enum { LS_AXIS_PAN = 0, LS_AXIS_EB = 1 } ls_axis_t;

void limit_switch_init(
    const gpio_num_t gpio_num);  // Inizializza il sensore di fine corsa

void limit_switch_set_notify_task_handle(TaskHandle_t task_handle);

// Ritorna true se l'asse e' stabilmente al finecorsa (linea HIGH = switch NC
// aperto). Da usare nel consumer dopo il debounce per validare la notifica.
bool limit_switch_is_at_limit(ls_axis_t axis);
