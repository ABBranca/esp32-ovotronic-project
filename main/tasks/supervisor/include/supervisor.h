#pragma once
#include "freertos/idf_additions.h"
#include "limit_switch.h"

extern volatile bool
    motion_direction[LS_AXIS_COUNT];  // false = forward, true = backward

void supervisor_task(void *pvParameters);

extern TaskHandle_t supervisor_task_handle;

extern EventGroupHandle_t axis_motor_event_group;