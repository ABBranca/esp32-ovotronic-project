#include "supervisor.h"
#include "dc_motor_driver.h"
#include "ec11.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_log_level.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/ledc_types.h"
#include "lcd1602.h"
#include "limit_switch.h"
#include "portmacro.h"
#include "soc/gpio_num.h"
#include <stdint.h>
#include <stdio.h>

#define HOMING_TIMEOUT_MS 10000
#define MH_MAX_RETRY 3

TaskHandle_t supervisor_task_handle = NULL;
static const char *TAG = "Supervisor";

static const struct dc_motor_config dc_motor_config_pan = {
    .gpio_num_in1 = GPIO_NUM_41,
    .gpio_num_in2 = GPIO_NUM_42,
    .channel = LEDC_CHANNEL_3};

static const struct dc_motor_config dc_motor_config_eb = {
    .gpio_num_in1 = GPIO_NUM_15,
    .gpio_num_in2 = GPIO_NUM_7,
    .channel = LEDC_CHANNEL_4};

static const struct dc_motor_config dc_motor_config_planetary = {
    .gpio_num_in1 = GPIO_NUM_10,
    .gpio_num_in2 = GPIO_NUM_11,
    .channel = LEDC_CHANNEL_5};

volatile bool motion_direction[LS_AXIS_COUNT] = {
    false, false}; // false = forward, true = backward

static bool mode_prep = false; // false = uova strapazzate, true = frittata.
static bool mode_changed = false;
char *mode_array[2] = {"Uova Strapazzate", "Frittata"};
static uint8_t egg_number = 0;

esp_err_t homing_sequence() {

  uint32_t pending;

  ESP_LOGI(TAG, "Avvio sequenza di homing");
  // lcd1602_clear();
  // lcd1602_set_cursor(0, 3);
  // lcd1602_print("HOMING...");

  const TickType_t start = xTaskGetTickCount();
  const TickType_t timeout = pdMS_TO_TICKS(HOMING_TIMEOUT_MS);

  dc_motor_driver_move_backward(&dc_motor_config_pan, 2048);
  motion_direction[LS_AXIS_PAN] = true;
  dc_motor_driver_move_backward(&dc_motor_config_eb, 2048);
  motion_direction[LS_AXIS_EB] = true;

  for (;;) {
    TickType_t elapsed = xTaskGetTickCount() - start;
    if (elapsed >= timeout) {
      dc_motor_driver_brake(&dc_motor_config_eb);
      dc_motor_driver_brake(&dc_motor_config_pan);
      ESP_LOG_LEVEL(ESP_LOG_ERROR, TAG, "Homing Timed Out");
      // lcd1602_clear();
      // lcd1602_set_cursor(0, 0);
      // lcd1602_print("ERROR: Homing");
      // lcd1602_set_cursor(1, 0);
      // lcd1602_print("timed out");
      return ESP_ERR_TIMEOUT;
    }

    xTaskNotifyWait(0, UINT32_MAX, &pending, timeout - elapsed);
    vTaskDelay(pdMS_TO_TICKS(30)); // Debounce
    // ulTaskNotifyValueClear(NULL, UINT32_MAX);

    if (limit_switch_is_at_limit(LS_AXIS_PAN)) {
      dc_motor_driver_brake(&dc_motor_config_pan);
    }
    if (limit_switch_is_at_limit(LS_AXIS_EB)) {
      dc_motor_driver_brake(&dc_motor_config_eb);
    }

    if (limit_switch_is_at_limit(LS_AXIS_PAN) &&
        limit_switch_is_at_limit(LS_AXIS_EB)) {
      ESP_LOGI(TAG, "Sequenza di homing completata");
      lcd1602_clear();
      lcd1602_set_cursor(0, 3);
      lcd1602_print("PRONTO!");
      return ESP_OK;
    }
  }

  // dc_motor_driver_brake(&dc_motor_config_pan);
  // dc_motor_driver_brake(&dc_motor_config_eb);
}

bool set_cancel_conferm(void) {
  lcd1602_clear();
  ec11_clear_count();

  uint32_t pending;
  bool cancel_conferm = true; // false = cancel, true = conferm.

  lcd1602_set_cursor(0, 2);
  lcd1602_print(">> Conferma");
  lcd1602_set_cursor(1, 2);
  lcd1602_print("   Annulla");

  int8_t last_detent = 0;

  for (;;) {
    int8_t detent = ec11_get_count() / 4;
    int8_t delta = detent - last_detent;
    if (delta) {
      cancel_conferm = !cancel_conferm;
      if (cancel_conferm) {
        lcd1602_set_cursor(0, 2);
        lcd1602_print(">> Conferma");
        lcd1602_set_cursor(1, 2);
        lcd1602_print("   Annulla");
      } else {
        lcd1602_set_cursor(0, 2);
        lcd1602_print("   Conferma");
        lcd1602_set_cursor(1, 2);
        lcd1602_print(">> Annulla");
      }
      last_detent = detent;
    }
    xTaskNotifyWait(0, UINT32_MAX, &pending, pdMS_TO_TICKS(20));
    if (pending & EC11_EVT_BUTTON) {
      return cancel_conferm;
    }
  }
}

void set_mode_prep(void) {

  uint32_t pending;
  lcd1602_clear();
  lcd1602_set_cursor(0, 2);
  lcd1602_print("Selez. Prep.");
  ec11_clear_count();

  while (true) {
    int count = ec11_get_count();
    ESP_LOGI(TAG, "EC11 Count: %d", count);
    if (count % 8) {
      if (mode_prep) {
        mode_prep = false;
        mode_changed = true;
      } else {
        mode_changed = false;
      }
    } else {
      if (!mode_prep) {
        mode_prep = true;
        mode_changed = true;
      } else {
        mode_changed = false;
      }
    }

    if (mode_changed) {
      lcd1602_clear();
      lcd1602_set_cursor(0, 2);
      lcd1602_print("Selez. Prep.");

      if (mode_prep) {
        lcd1602_set_cursor(1, 2);
        lcd1602_print("< FRITTATA >");
      } else {
        lcd1602_set_cursor(1, 0);
        lcd1602_print("< UOVA STRAP. >");
      }
    }

    xTaskNotifyWait(0, UINT32_MAX, &pending, pdMS_TO_TICKS(50));
    if (pending & EC11_EVT_BUTTON) {
      return;
    }
  }
}

void set_egg_number(void) {
  lcd1602_clear();
  lcd1602_set_cursor(0, 0);
  lcd1602_print("Selez. Num. Uova");
  lcd1602_set_cursor(1, 6);
  lcd1602_print("< 1 >");

  ec11_clear_count();
  egg_number = 1;
  uint32_t pending;
  int8_t last_detent = 0;

  for (;;) {
    int8_t detent = ec11_get_count() / 4;
    int8_t delta = detent - last_detent;
    if (delta) {
      uint8_t e = egg_number + delta;
      if (e < 1) {
        e = 1;
      }
      if (e > 6) {
        e = 6;
      }
      egg_number = e;
      last_detent = detent;

      lcd1602_clear();
      lcd1602_set_cursor(0, 0);
      lcd1602_print("Selez. Num. Uova");
      char buf[8];
      snprintf(buf, sizeof(buf), "< %d >", egg_number);
      lcd1602_set_cursor(1, 6);
      lcd1602_print(buf);
    }

    xTaskNotifyWait(0, UINT32_MAX, &pending, pdMS_TO_TICKS(20));
    if (pending & EC11_EVT_BUTTON) {
      break;
    }
  }
}

void supervisor_task(void *pvParameters) {

  // Configurazione motori DC
  // Inizializzazione canali PWM per motori DCs
  pwm_dc_motor_driver_init(&dc_motor_config_pan);
  pwm_dc_motor_driver_init(&dc_motor_config_eb);
  pwm_dc_motor_driver_init(&dc_motor_config_planetary);

  // Messaggio Intro.
  lcd1602_set_cursor(0, 3);
  lcd1602_print("OVOTRONIC");

  vTaskDelay(pdMS_TO_TICKS(3000));

  if (supervisor_task_handle == NULL) {
    supervisor_task_handle = xTaskGetCurrentTaskHandle();
  }

  limit_switch_set_notify_task_handle(supervisor_task_handle);
  ec11_set_notify_task_handle(supervisor_task_handle);

  ESP_LOGI(TAG, "Supervisor task started");

  while (true) {

    // [x]: Ovotronic resetta la posizione verticale della padella,
    // assicurandosi che sia nella posizione più bassa tramite sensore fine
    // corsa.

    int attempts = 0;
    while (homing_sequence() != ESP_OK) {
      if (++attempts >= MH_MAX_RETRY) {
        // FAULT LATCHED: motori fermi, LCD errore, halt
        lcd1602_clear();
        lcd1602_set_cursor(0, 0);
        lcd1602_print("PANIC: MOTOR");
        lcd1602_set_cursor(1, 0);
        lcd1602_print("FAULT!");
        ESP_LOGE(TAG, "Homing fault: %d tentativi falliti", attempts);
        for (;;)
          vTaskDelay(pdMS_TO_TICKS(1000)); // o stato fault dedicato
      }
      vTaskDelay(pdMS_TO_TICKS(5000));
    }

    // [x]: L'utente seleziona il tipo di preparazione tra Uova Strapazzate e
    // Frittata e prosegue

    set_mode_prep(); // Seleziona tra Uova Strapazzate o Frittata.
    ESP_LOGI(TAG, "Utente seleziona: %s", mode_array[mode_prep]);

    // [x]: L'utente seleziona il numero di uova da preparare e prosegue

    set_egg_number();
    if (!set_cancel_conferm()) {
      continue;
    } else {
      // break;
      ESP_LOGI(TAG, "Utente seleziona %d uova", egg_number);
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}