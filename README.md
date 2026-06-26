# Ovotronic

Embedded firmware for an automated egg-cooking machine. Target MCU: **ESP32-S3**
(`board-esp32-s3-devkitc-1`), built on **ESP-IDF v6.0.1** + **FreeRTOS**. Language: C.

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
