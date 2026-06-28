#pragma once

#include "hal/ledc_types.h" // ledc_channel_t, LEDC_CHANNEL_*
#include "soc/gpio_num.h"

struct dc_motor_config {
  gpio_num_t gpio_num_in1;
  gpio_num_t gpio_num_in2;
  const ledc_channel_t channel;
};

void pwm_dc_motor_driver_init(const struct dc_motor_config *config);
void pwm_dc_motor_driver_init_timer();
