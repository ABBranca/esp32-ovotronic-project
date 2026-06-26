# Hardware

## MCU
- **ESP32-S3** (`board-esp32-s3-devkitc-1`). Target set in `sdkconfig`
  (`CONFIG_IDF_TARGET="esp32s3"`). 32-bit dual-core, FreeRTOS.

## Buses

### I2C (master, port 0)
Shared bus for the temperature sensor and the display. Configured by the
`i2c_bus` driver (see [[drivers]]).

| Signal | GPIO |
|--------|------|
| SDA    | 4    |
| SCL    | 5    |

### LEDC (PWM)
Used for the servos and the buzzer. Frequency/resolution are per **timer**, not
per channel — so peripherals needing different frequencies need different timers.

| Timer        | Freq    | Used by                         |
|--------------|---------|---------------------------------|
| `LEDC_TIMER_0` | 50 Hz | SG90 servos (channels 0 and 1)  |
| `LEDC_TIMER_1` | 3 kHz | buzzer (channel 2, 10-bit)      |

## Peripherals and pin/channel map

| Peripheral | Connection | Address / Channel / GPIO | Driver | Status |
|------------|-----------|--------------------------|--------|--------|
| TMP102 temperature sensor | I2C | addr `0x48` | `tmp102q1` | implemented |
| LCD1602 display (PCF8574 backpack) | I2C | addr `0x27` | `lcd1602` | implemented |
| SG90 servo — supplements (`ds`) | LEDC | ch 0, GPIO 2 | `sg90` | implemented |
| SG90 servo — egg breaker (`eb`) | LEDC | ch 1, GPIO 47 | `sg90` | implemented |
| Buzzer (completion notify) | LEDC, passive | ch 2 / `LEDC_TIMER_1`, GPIO 48, 3 kHz | `buzzer` | implemented |
| Limit switches — pan top/bottom, breaker begin/end | GPIO input | active-low, IRQ on falling edge | `limit_switch` | scaffolded (flags in `main.c`, driver empty) |
| Planetary mixer motor | DRV8870 H-bridge | — | **PLANNED** | designed |
| Heating resistor | GPIO/PWM, thermostat-gated | regulated by [[drivers]] TMP102 reads | **PLANNED** | designed |

## Simulation parts
`wokwi.toml` loads custom chips: `drv8870` (motor driver) and `tmp102`
(temperature sensor). See [[build-and-simulation]]. The simulated buzzer part
(`wokwi-buzzer`) is **passive** — it plays whatever PWM frequency it is driven
at, which is why the buzzer needs its own LEDC timer. `diagram.json` also wires
four `wokwi-slide-switch` parts (the limit switches, on GPIO 12/13) and a rotary
encoder for user input; firmware support for these is still in progress.

Related: [[drivers]] · [[overview]] · [[build-and-simulation]]
