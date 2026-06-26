# Build & Simulation

## Toolchain
ESP-IDF standard workflow. Requires `IDF_PATH` set and the ESP-IDF export script
sourced. The dev container under `.devcontainer/` provides ESP-IDF + QEMU.

## Build / Flash / Monitor
```bash
idf.py set-target esp32s3   # already set in sdkconfig
idf.py build                # outputs build/Ovotronic.bin + .elf
idf.py -p <PORT> flash
idf.py -p <PORT> monitor    # serial console
idf.py flash monitor        # combined
```
Project name is `Ovotronic` (root `CMakeLists.txt`), so artifacts are
`build/Ovotronic.{bin,elf}`.

## Adding code to the build
- **New driver:** create a folder under `components/` (auto-discovered), then add
  its name to `main/CMakeLists.txt` `REQUIRES`. See [[drivers]].
- **New task / source file under `main/`:** must be listed explicitly in
  `main/CMakeLists.txt` `SRCS` — there is no glob. Planned tasks go in
  `main/tasks/` with that dir added to `INCLUDE_DIRS`. A loose folder is *not*
  compiled just because a header is `#include`d. See [[state-machine]].

Current `main/CMakeLists.txt`:
```cmake
idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES i2c_bus tmp102q1 lcd1602 sg90 buzzer limit_switch)
```

## Wokwi simulation
`wokwi.toml` + `diagram.json` enable in-editor simulation against
`build/Ovotronic.bin/.elf`. Run `idf.py build` first, then launch Wokwi. GDB
server on port 3333.

Custom chips loaded by `wokwi.toml`:
- `drv8870` — motor driver (planetary mixer H-bridge).
- `tmp102` — temperature sensor.

The circuit in `diagram.json` is being expanded as hardware is added (servo,
limit switch, thermometer, display, buzzer). See [[hardware]].

## Static analysis
`.clang-tidy` configured. `.clangd` strips `-f*`/`-m*` GCC-only flags so clangd
can index the project in the IDE.

Related: [[drivers]] · [[hardware]] · [[conventions]]
