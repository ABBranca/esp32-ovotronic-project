# State Machine

The core application logic. **Status: PLANNED** — `main/main.c` currently does
hardware init only (I2C, TMP102, LCD, buzzer, SG90 timer + two channels), prints
`OVOTRONIC` to the LCD, declares the limit-switch flags, and then leaves the
cooking sequence as `[ ]` pseudocode comments in `app_main`.

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
RESET and RAISE both wait on end-of-travel limit switches. The flags are already
declared in `main.c` — `pan_at_bottom` / `pan_at_top` for the pan, and
`breaker_at_begin` / `breaker_at_end` for the egg breaker. The planned approach:
a falling-edge **hardware interrupt** sets the matching `volatile bool` global;
the `recipe_task` starts the motor, then polls the flag (with `vTaskDelay`) and
stops the motor when set. The `limit_switch` driver itself is still an empty stub
— see [[drivers]].

## Dependencies still missing
To complete the state machine, the remaining drivers must land first: the
`limit_switch` ISR implementation (the component is scaffolded but empty), the
DRV8870 mixer-motor control, and the heater switch. The `buzzer` for the final
NOTIFY step is already implemented. See [[drivers]].

Related: [[overview]] · [[drivers]] · [[conventions]]
