#pragma once
#include "freertos/idf_additions.h"
#include "hal/ledc_types.h"

void supervisor_task(void *pvParameters);

extern TaskHandle_t supervisor_task_handle;
extern const ledc_channel_t ds_sg90_channel;
extern const ledc_channel_t eb_sg90_channel;

extern EventGroupHandle_t axis_motor_event_group;