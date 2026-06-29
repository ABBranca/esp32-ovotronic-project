#pragma once

#include "freertos/idf_additions.h"
#include "soc/gpio_num.h"

#define PCNT_MAX_COUNT 100
#define PCNT_MIN_COUNT -100
#define EC11_CLK_GPIO 18
#define EC11_DT_GPIO 17
#define EC11_SW_GPIO 21
#define EC11_EVT_BUTTON (1u << 3)

typedef enum { EC11_CCW = -1, EC11_NONE = 0, EC11_CW = 1 } ec11_direction_t;

typedef struct {
  gpio_num_t gpio_clk;
  gpio_num_t gpio_dt;
  gpio_num_t gpio_sw;
} ec11_config_t;

void ec11_set_notify_task_handle(const TaskHandle_t task_handle);

void ec11_init(const ec11_config_t *config);
int ec11_get_count(void);
void ec11_clear_count(void);
bool ec11_button_pressed(void);