# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

**Ovotronic** — embedded firmware for an automated egg-cooking machine. Target MCU: **ESP32-S3** (`board-esp32-s3-devkitc-1`). Built on **ESP-IDF** with **FreeRTOS**. Language: C.

The machine logic (from `main/main.c`) is a state sequence: reset pan to lowest position via limit switch → user selects recipe (Uova Strapazzate / Frittata) → user selects egg count → display shows supplement grams → on user start, break eggs + dispense supplements via SG90 servo → raise pan to limit → planetary mixer stirs → recipe-specific behavior (Frittata: stop mixer, lower pan; Strapazzate: keep pan, lower mixer RPM) → heat pan with thermostat-regulated resistor → on cook timeout, stop mixer/lower pan/cut heat → buzzer notifies completion.

`main/main.c` currently holds only the pseudocode skeleton (`[ ]` checklist comments in `app_main`); the state machine is unimplemented.

## Build / Flash / Monitor

ESP-IDF standard workflow (requires `IDF_PATH` env + sourced ESP-IDF export script):

```bash
idf.py set-target esp32s3   # already set in sdkconfig
idf.py build                # outputs build/Ovotronic.bin + .elf
idf.py -p <PORT> flash
idf.py -p <PORT> monitor    # serial console
idf.py flash monitor        # combined
```

Project name is `Ovotronic` (set in root `CMakeLists.txt`), so artifacts are `build/Ovotronic.{bin,elf}`.

## Simulation (Wokwi)

`wokwi.toml` + `diagram.json` enable in-editor simulation pointing at `build/Ovotronic.bin/.elf`. Run `idf.py build` first, then launch Wokwi. GDB server on port 3333. Circuit is a stub (breadboard + NMOS + motor); expand `diagram.json` as hardware (servo, limit switch, thermometer, display, buzzer) is added.

## Dev Environment

- `.devcontainer/` — ESP-IDF + QEMU container (`--privileged`), VS Code ESP-IDF extensions.
- Static analysis: `.clang-tidy` configured. `.clangd` strips `-f*`/`-m*` flags (GCC-only flags clangd can't parse) for IDE indexing.

## Conventions (from main.c README block)

- Declare functions in separate `.c` files, include via headers — keep `app_main` thin.
- Always use FreeRTOS primitives for task creation (no bare loops).
- Variable names explicit and descriptive.
- Comments/docs in Italian (project language).

## Notes

- `sdkconfig` is committed; `sdkconfig.old` is a prior snapshot.
- New source files must be added to `main/CMakeLists.txt` `SRCS` list.
