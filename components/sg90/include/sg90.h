#pragma once
#include "esp_err.h"
#include "hal/ledc_types.h"
#include "soc/gpio_num.h"

#define SG90_PWM_FREQ 50
#define SG90_MIN_PULSE_US 500
#define SG90_MAX_PULSE_US 2500
#define SG90_MAX_ANGLE 180.0f
#define SG90_PERIOD_US 20000
#define SG90_DUTY_MAX (1 << 14)

esp_err_t sg90_channel_init(const ledc_channel_t channel,
                            const gpio_num_t gpio);
esp_err_t sg90_timer_init();

esp_err_t sg90_set_angle(const ledc_channel_t channel, float angle);
