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

## Planned

### `limit_switch` — pan end-of-travel  **PLANNED**
GPIO input, **active-low** (switch to GND + internal pull-up), with a
**hardware interrupt on the falling edge** (`GPIO_INTR_NEGEDGE`). The ISR stays
tiny: it sets a `volatile bool` global per switch (e.g. `pan_at_bottom`,
`pan_at_top`); the consuming task polls that flag. The GPIO ISR service is
installed once app-wide (treat `ESP_ERR_INVALID_STATE` from a second install as
OK). Handler must be `IRAM_ATTR`. Contact bounce is the known gotcha — add a
time-based debounce only if false triggers appear. Proposed API:
```c
esp_err_t limit_switch_init(const gpio_num_t gpio, gpio_isr_t isr_handler, void *arg);
```

### `buzzer` — completion notify  **PLANNED**
If passive: LEDC on its **own** timer (`LEDC_TIMER_1`, ~2 kHz, channel 2) —
cannot share the servo's 50 Hz `LEDC_TIMER_0`. If active: plain GPIO on/off, no
LEDC needed. The Wokwi buzzer part is passive, so the LEDC path suits
simulation.

### Motor / heater  **PLANNED**
Planetary mixer driven by a DRV8870 H-bridge (PWM for RPM, since *Strapazzate*
lowers RPM). Heating resistor switched by GPIO/PWM, gated by `tmp102` reads in a
thermostat control task. See [[state-machine]].

Related: [[hardware]] · [[state-machine]] · [[conventions]]
