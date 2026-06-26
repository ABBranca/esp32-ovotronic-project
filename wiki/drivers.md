# Drivers

Each peripheral lives in its own ESP-IDF component under `components/`, with the
layout: `CMakeLists.txt`, `<name>.c`, `include/<name>.h`. Components in
`components/` are auto-discovered by the build; a component is made available to
the app by adding its name to `main/CMakeLists.txt`'s `REQUIRES`. See
[[conventions]] and [[build-and-simulation]].

## Implemented

### `i2c_bus`
Initializes the shared I2C master bus (port 0, SDA 4, SCL 5). Exposes the bus
handle that the device drivers attach to.
```c
extern i2c_master_bus_handle_t bus_handle;
esp_err_t i2c_init();
```

### `tmp102q1` — temperature sensor
TMP102 over I2C (addr `0x48`). Attaches to `bus_handle`. Provides the reading
that the (PLANNED) thermostat loop uses to regulate the heater.
```c
esp_err_t tmp102_init(i2c_master_bus_handle_t bus);
esp_err_t tmp102_read_temperature(float *out_temp);
```

### `lcd1602` — display
16x2 character LCD via a PCF8574 I2C backpack (addr `0x27`). Used to prompt the
user (recipe, egg count, supplement grams).
```c
esp_err_t lcd1602_init(i2c_master_bus_handle_t bus);
esp_err_t lcd1602_print(const char *str);
esp_err_t lcd1602_set_cursor(uint8_t row, uint8_t col);
esp_err_t lcd1602_clear();
```

### `sg90` — servos
SG90 servo control via LEDC PWM. One `timer_init` (hardcoded `LEDC_TIMER_0` @
50 Hz, 14-bit), then one `channel_init` per servo. Two channels in use:
supplements (ch 0, GPIO 2) and egg breaker (ch 1, GPIO 47).
```c
esp_err_t sg90_timer_init(void);
esp_err_t sg90_channel_init(const ledc_channel_t channel, const gpio_num_t gpio);
esp_err_t sg90_set_angle(const ledc_channel_t channel, float angle);
```

### `buzzer` — completion notify
Passive piezo driven by LEDC on its **own** timer (`LEDC_TIMER_1`, channel 2,
10-bit resolution) — it cannot share the servo's 50 Hz `LEDC_TIMER_0`, since
frequency is per-timer. `buzzer_init` configures the timer + channel at duty 0
(silent); `buzzer_ring` retunes the timer, drives 50% duty for `duration` ms,
then returns to silence. Default tone is `BUZZER_DEFAULT_FREQUENCY` (3000 Hz);
wired to GPIO 48. Initialized in `app_main`.
```c
#define BUZZER_DEFAULT_FREQUENCY 3000
void buzzer_init(const gpio_num_t gpio_num, const uint32_t frequency);
void buzzer_ring(const uint32_t frequency, const uint32_t duration);
```

## Scaffolded

### `limit_switch` — end-of-travel sensors  **IN PROGRESS**
The component is registered in the build (`REQUIRES esp_driver_gpio`, listed in
`main/CMakeLists.txt`), but `limit_switch.c` / `limit_switch.h` are still empty
stubs — no driver code yet. The consuming side already exists: `main.c` declares
the `volatile bool` flags an ISR will set — `pan_at_top`, `pan_at_bottom` (pan
end-of-travel) and `breaker_at_begin`, `breaker_at_end` (egg-breaker travel).
The diagram wires four slide switches accordingly.

Planned approach: GPIO input, **active-low** (switch to GND + internal pull-up),
with a **hardware interrupt on the falling edge** (`GPIO_INTR_NEGEDGE`). The ISR
stays tiny — it sets the matching `volatile bool` flag; the consuming task polls
it. The GPIO ISR service is installed once app-wide (treat `ESP_ERR_INVALID_STATE`
from a second install as OK). Handler must be `IRAM_ATTR`. Contact bounce is the
known gotcha — add a time-based debounce only if false triggers appear. Proposed
API:
```c
esp_err_t limit_switch_init(const gpio_num_t gpio, gpio_isr_t isr_handler, void *arg);
```

## Planned

### Motor / heater  **PLANNED**
Planetary mixer driven by a DRV8870 H-bridge (PWM for RPM, since *Strapazzate*
lowers RPM). Heating resistor switched by GPIO/PWM, gated by `tmp102` reads in a
thermostat control task. See [[state-machine]].

Related: [[hardware]] · [[state-machine]] · [[conventions]]
