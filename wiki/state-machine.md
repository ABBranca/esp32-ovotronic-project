# State Machine

The core application logic. **Status: PLANNED** — `main/main.c` currently does
hardware init only (I2C, TMP102, LCD, SG90 timer + two channels) and then leaves
the cooking sequence as `[ ]` pseudocode comments in `app_main`.

## Design

`app_main` should stay thin: init hardware, then create FreeRTOS tasks that run
the sequence (per [[conventions]] — no bare loops, always FreeRTOS primitives).
The pages in [[overview]] enumerate the 11 functional steps; as a control flow
they map to states:

```
RESET -> SELECT_RECIPE -> SELECT_COUNT -> SHOW_SUPPLEMENTS -> DISPENSE
      -> RAISE -> MIX -> {FRITTATA | STRAPAZZATE branch} -> HEAT
      -> COOK_TIMEOUT -> NOTIFY
```

### Branch behavior
- **Frittata:** mixer stops, pan lowers, then heat.
- **Uova Strapazzate:** pan stays raised, mixer drops RPM, then heat.

### Suggested task layout
- `recipe_task` — owns the main sequence above. Drives servos, motor, pan,
  and the LCD; reacts to the limit-switch flags during RESET and RAISE.
- `thermostat_task` — during HEAT, loops reading `tmp102_read_temperature()`
  and switching the heating resistor to hold target temperature; the one closed
  control loop in the system.

Task source files are planned to live under `main/tasks/` and must be listed
explicitly in `main/CMakeLists.txt` `SRCS` (no globbing) — see
[[build-and-simulation]].

### Limit switch integration
RESET and RAISE both wait on an end-of-travel limit switch. The planned
approach: a falling-edge **hardware interrupt** sets a `volatile bool` global
(`pan_at_bottom` / `pan_at_top`); the `recipe_task` starts the motor, then polls
the flag (with `vTaskDelay`) and stops the motor when set. See `limit_switch` in
[[drivers]].

## Dependencies still missing
To complete the state machine, the **PLANNED** drivers must land first:
`limit_switch`, `buzzer`, the DRV8870 mixer-motor control, and the heater
switch. See [[drivers]].

Related: [[overview]] · [[drivers]] · [[conventions]]
