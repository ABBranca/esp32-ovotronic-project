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

## In progress

### `limit_switch` — end-of-travel sensors  **IN PROGRESS**
Registered in the build (`REQUIRES esp_driver_gpio`, listed in
`main/CMakeLists.txt`). GPIO input, **normally-closed switches in series** per
axis: the diagram wires them with an **external pull-up** (`r1`/`r2`, 10 kΩ to
3V3), so idle (both closed) holds the line **LOW**, and reaching a limit **opens**
the actuated switch → series breaks → line goes HIGH — interrupt on the **rising
edge** (`GPIO_INTR_POSEDGE`); internal pulls disabled. This is fail-safe: a broken
wire / disconnected switch reads as "limit reached". Four `wokwi-slide-switch`
parts sit on GPIO 12 (egg breaker) and GPIO 13 (pan).

Design follows the ISR-defers-to-task pattern (see [[state-machine]]): the ISR is
tiny and `IRAM_ATTR`, and instead of touching a `volatile bool` or the event
group from interrupt context, it sends a **task notification** to the supervisor
(`xEventGroupSetBitsFromISR` is not IRAM-safe; task notification is). The axis is
encoded as a one-hot bit (`1u << axis`) carried in the notification value, and the
per-pin axis is passed through `gpio_isr_handler_add`'s `void *arg`. Debounce is
the consumer's job (`vTaskDelay` settle + `ulTaskNotifyValueClear`), not the
ISR's. The GPIO ISR service is installed once app-wide in `app_main`. The
notify-target is registered by the supervisor at startup, keeping the component
decoupled from `main` (no back-dependency). Public API:
```c
void limit_switch_init(const gpio_num_t gpio_num);
void limit_switch_set_notify_task_handle(TaskHandle_t task_handle);
```
Remaining: direction must be driven by the motor command (see [[state-machine]]),
not by toggling a flag. (The configured edge `GPIO_INTR_POSEDGE` is correct for the
NC-in-series wiring — reaching a limit opens the switch and pulls the line HIGH.)

### `ec11` — rotary encoder + push button  **IN PROGRESS** (scaffolded)
User input for `SELECT_RECIPE` / `SELECT_COUNT` / START. KY-040 / EC11 quadrature
knob: CLK=GPIO 18, DT=GPIO 17, button SW=GPIO 21. Component scaffolded under
`components/ec11/` (`CMakeLists.txt` plus empty `ec11.c` / `include/ec11.h`); the
`REQUIRES` is still empty and `ec11` is not yet listed in `main/CMakeLists.txt`.

**Decided design (2026-06-29):**
- **Quadrature decode via the PCNT peripheral** — chosen over software GPIO-ISR
  decode. New IDF v6 driver `driver/pulse_cnt.h` (`pcnt_new_unit` /
  `pcnt_new_channel`, edge + level actions on CLK/DT), using its **hardware glitch
  filter** for robust debounce. `CONFIG_SOC_PCNT_SUPPORTED=y` is already set; do
  **not** use the legacy `driver/pcnt.h` (removed in v6.x).
- **Push button SW in the same component**, debounced like the limit switch: a
  tiny GPIO ISR (`GPIO_INTR_NEGEDGE`) defers to the supervisor via task
  notification, with an `esp_timer` one-shot (~10–20 ms) doing the edge→level
  debounce. This supersedes the earlier queue (`xQueueSendFromISR`) sketch.
- **Build deps** (`REQUIRES`): `esp_driver_pcnt esp_driver_gpio esp_timer`
  (`freertos` implicit). Then add `ec11` to `main/CMakeLists.txt` `REQUIRES`.

Indicative public API (notify pattern reused from `limit_switch` above):
```c
void ec11_init(const ec11_config_t *config);          // CLK / DT / SW pins
void ec11_set_notify_task_handle(TaskHandle_t task);  // SW press → supervisor
int  ec11_get_count(void);                            // PCNT accumulated position
bool ec11_button_pressed(void);
```

## Planned

### `dc_motor` — DRV8870 H-bridges  **PLANNED**
Three brushed-DC motors, each on a DRV8870. Direction = which input is asserted;
speed = PWM duty (`IN1`=PWM/`IN2`=0 → one way, swap for the other; 0/0 coast,
1/1 brake).

| Motor | IN1 | IN2 | Speed control |
|-------|-----|-----|---------------|
| Pan (raise/lower) | GPIO 41 | GPIO 42 | no → plain GPIO |
| Egg breaker | GPIO 15 | GPIO 7 | no → plain GPIO |
| Planetary mixer | GPIO 10 | GPIO 11 | **yes** → LEDC PWM |

LEDC budget (ESP32-S3: 8 channels / 4 timers; already used: servos ch0/ch1 on
`LEDC_TIMER_0`, buzzer ch2 on `LEDC_TIMER_1`): only the planetary mixer needs RPM
control (*Strapazzate*), so only it uses one PWM channel on its own timer
(`LEDC_TIMER_2`, ~20 kHz). Pan and breaker run full-speed via digital GPIO. The
driver exposes the blocking helpers the supervisor calls — `motor_pan_start(dir)`
/ `motor_pan_stop`, `motor_eb_*`, `motor_mixer_set_duty(...)` — and **sets
`motion_direction[]` before moving** (the single source of truth for direction).

### Heater  **PLANNED**
Heating resistor switched by GPIO/PWM, gated by `tmp102` reads in the `HEAT`
thermostat loop (inline in the supervisor, or a dedicated task). See
[[state-machine]].

Related: [[hardware]] · [[state-machine]] · [[conventions]]
