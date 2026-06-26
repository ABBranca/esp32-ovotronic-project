# Ovotronic

Embedded firmware for an automated egg-cooking machine. Target MCU: **ESP32-S3**
(`board-esp32-s3-devkitc-1`), built on **ESP-IDF v6.0.1** + **FreeRTOS**. Language: C.

## Status

Each peripheral lives in its own ESP-IDF component under `components/`. `app_main`
(`main/main.c`) currently performs hardware init only; the cooking state machine is
still pseudocode.

| Component | Role | Status |
|-----------|------|--------|
| `i2c_bus` | Shared I2C master bus (SDA 4, SCL 5) | implemented |
| `tmp102q1` | TMP102 temperature sensor (I2C `0x48`) | implemented |
| `lcd1602` | 16x2 LCD via PCF8574 backpack (I2C `0x27`) | implemented |
| `sg90` | SG90 servos via LEDC (supplements + egg breaker) | implemented |
| `buzzer` | Passive buzzer via LEDC (GPIO 48, 3 kHz) | implemented |
| `limit_switch` | End-of-travel sensors (pan + egg breaker) | scaffolded (driver empty) |
| Mixer motor / heater | DRV8870 H-bridge + thermostat | planned |

See the [`wiki/`](wiki/index.md) for the full design: [overview](wiki/overview.md),
[hardware map](wiki/hardware.md), [drivers](wiki/drivers.md), and the
[state machine](wiki/state-machine.md).

## Setup (per developer)

Toolchain paths and editor config are **per-machine** and are intentionally not committed, so
no one shares another machine's environment paths.

1. Clone the repo and open the folder in VS Code.
2. Install the **ESP-IDF** extension (Espressif) and install/point it at **ESP-IDF v6.0.1**.
3. Run Command Palette → **"ESP-IDF: Configure ESP-IDF Extension"** → *Use existing setup* and
   select your ESP-IDF install. This generates your own `.vscode/settings.json` and
   `.vscode/c_cpp_properties.json` (both gitignored) with the correct paths for *your* machine.
4. Copy `.vscode/settings.json.example` → `.vscode/settings.json` if you want the shared
   project defaults (OpenOCD board, IDF target), then re-run Configure to fill the paths.

The build target comes from the committed `sdkconfig.defaults` — on the first build, ESP-IDF
regenerates your local `sdkconfig` (gitignored) for **esp32s3** automatically.

## Build / Flash / Monitor

```bash
idf.py build                # outputs build/Ovotronic.bin + .elf
idf.py -p <PORT> flash
idf.py -p <PORT> monitor
idf.py flash monitor        # combined
```

## Simulation (Wokwi)

`wokwi.toml` + `diagram.json` enable in-editor simulation. Run `idf.py build` first, then launch
Wokwi. GDB server on port 3333.

## What is and isn't committed

- **Committed:** firmware sources (`main/`, `components/`), `CMakeLists.txt`, `sdkconfig.defaults`,
  simulation assets (`wokwi.toml`, `diagram.json`, `chips/`), portable editor config
  (`.vscode/launch.json`, `.vscode/tasks.json`, `.clangd`, `.clang-tidy`), `.devcontainer/`,
  and `.vscode/settings.json.example`.
- **Not committed (per-machine / generated):** `.vscode/settings.json`,
  `.vscode/c_cpp_properties.json`, `*.code-workspace`, `sdkconfig`, `build/`,
  `managed_components/`.
