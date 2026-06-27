#pragma once

#include "driver/ledc.h"
#include "soc/gpio_num.h"

struct dc_motor_config {
  gpio_num_t gpio_num_in1;
  gpio_num_t gpio_num_in2;
  int channel1;
  int channel2;
};

void pwm_dc_motor_driver_init(const struct dc_motor_config *config);
void pwm_dc_motor_driver_init_timer();
void dc_motor_driver_init(const struct dc_motor_config *config);
