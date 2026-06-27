# State Machine

The core application logic. **Status: IN PROGRESS** — `app_main` does hardware
init (I2C, TMP102, LCD, buzzer, SG90 timer + two channels), prints `OVOTRONIC`
to the LCD, then creates `supervisor_task`. The `limit_switch` driver is
implemented (interrupt → task notification, see [[drivers]]); the cooking
sequence itself is still being filled in.

## Architecture decision — single orchestrator (Option A)

The sequence is **strictly sequential** (one state at a time, no concurrent
recipes). So a single task — `supervisor_task` — owns the whole flow and acts as
the only "brain". It calls the actuators (motors, servos, LCD, heater, buzzer)
as **blocking helper functions** and reacts to sensors (limit switches, encoder,
thermometer) as it advances.

Consequences of choosing A:

- **Direction is local state, not inferred.** When the supervisor commands a
  motor (e.g. "raise pan"), it *knows* the commanded direction. It must **not**
  be deduced by toggling/counting interrupts — that desyncs on any bounce or
  missed edge. The supervisor sets `motion_direction[axis]` *before* moving and
  reads it back when the matching limit fires to label the end reached.
- **`motion_direction[]` and `axis_motor_event_group` are optional in A.** They
  exist to decouple a *producer* task from a *consumer* task (Option B, separate
  motor tasks). With one task, the supervisor already holds that state locally;
  the limit ISR only needs to *unblock* it. They are kept for now as the
  signaling channel but can be retired once motor helpers carry direction.

## Limit switch integration (current implementation)

See [[drivers]] for the driver. The two limit switches per axis are **normally
closed and wired in series** to one GPIO, with an external pull-up: idle (both
closed) holds the line **LOW**, and reaching a limit **opens** that switch →
series breaks → line goes HIGH = **rising edge** → `GPIO_INTR_POSEDGE`. This is
fail-safe (a broken wire / disconnected switch reads as "limit reached"). The ISR
stays tiny and **defers** work to the supervisor:

```
GPIO rising edge
  → ISR (IRAM_ATTR): xTaskNotifyFromISR(supervisor, (1u << axis), eSetBits)
  → supervisor wakes, debounces, then acts
```

Why not set the FreeRTOS event group / a `volatile bool` directly in the ISR:
`xEventGroupSetBitsFromISR` defers to the timer daemon and is not IRAM-safe;
task notification is the lightest, IRAM-safe ISR→task signal. The axis is encoded
in the notification value as a one-hot bit (`1u << axis`), so multiple axes
firing before the supervisor runs accumulate without clobbering.

Debounce lives in the **consumer**, not the ISR:

```c
xTaskNotifyWait(0, UINT32_MAX, &pending, portMAX_DELAY);
vTaskDelay(pdMS_TO_TICKS(30));          // let contacts settle
ulTaskNotifyValueClear(NULL, UINT32_MAX); // drop the bounce burst
// (optional) gpio_get_level() to confirm a steady, valid level
```

A plain `vTaskDelay` is **not** enough on its own — bounce edges keep queueing
notifications that get serviced on the next loop (the "fires twice" symptom). The
`ulTaskNotifyValueClear` after the settle window collapses one physical press
into one logical event.

### Blocking move helper (the RESET / RAISE pattern)

```
pan_move_to(dir):                 // dir = UP | DOWN
    motion_direction[PAN] = dir   // local truth, set BEFORE moving
    motor_pan_start(dir)
    wait_limit(PAN)               // notify + debounce, as above
    motor_pan_stop()
```

`wait_limit` blocks the supervisor until the end-of-travel for that axis is
reported. RESET = `pan_move_to(DOWN)`; RAISE = `pan_move_to(UP)`.

## States

```
RESET -> SELECT_RECIPE -> SELECT_COUNT -> SHOW_SUPPLEMENTS -> (wait START)
      -> DISPENSE -> RAISE -> MIX -> { FRITTATA | STRAPAZZATE } -> HEAT
      -> COOK_TIMEOUT -> SHUTDOWN -> NOTIFY -> (idle / RESET)
```

| State | Actuators | Waits on / exit condition |
|-------|-----------|---------------------------|
| `RESET` | pan motor DOWN | pan bottom limit → stop |
| `SELECT_RECIPE` | LCD prompt | encoder rotate = choose, button = confirm |
| `SELECT_COUNT` | LCD prompt | encoder rotate = count, button = confirm |
| `SHOW_SUPPLEMENTS` | LCD shows grams | button = START |
| `DISPENSE` | SG90 egg breaker + supplements servo; EB DC motor | timed / breaker limits |
| `RAISE` | pan motor UP | pan top limit → stop |
| `MIX` | planetary motor ON | timer (`vTaskDelay`) |
| `FRITTATA` | mixer stop; pan motor DOWN | pan bottom limit |
| `STRAPAZZATE` | pan stays UP; mixer RPM down (PWM duty ↓) | — |
| `HEAT` | heater on, thermostat loop | cook timer elapsed |
| `COOK_TIMEOUT` | — | (entered when cook timer fires) |
| `SHUTDOWN` | mixer stop (if on), pan DOWN, heater off | pan bottom limit |
| `NOTIFY` | `buzzer_ring` | done |

### Recipe branch
- **Frittata:** mixer stops, pan lowers, then heat.
- **Uova Strapazzate:** pan stays raised, mixer lowers RPM (lower PWM duty),
  then heat.

## User input — rotary encoder (knob)

`SELECT_RECIPE` / `SELECT_COUNT` / START consume the KY-040 encoder (CLK=GPIO18,
DT=GPIO17, button SW=GPIO21). The encoder driver turns quadrature edges into
discrete *rotation events* (CW/CCW) and *button press* events, delivered to the
supervisor. Cleanest: encoder ISR/PCNT → `xQueueSendFromISR` of an input event;
the supervisor blocks on that queue while in a selection state. Reuses the same
ISR-defers-to-task discipline as the limit switch.

## Thermostat (HEAT)

The one closed control loop. Two options:
- **Inline sub-loop** in the `HEAT` state: supervisor loops
  `tmp102_read_temperature()` → switch heater → `vTaskDelay`, until the cook
  timer elapses. Simplest, fits Option A.
- **Dedicated `thermostat_task`** gated by the supervisor (notify start/stop).
  Use only if HEAT must overlap other work.

Start with the inline sub-loop; promote to a task only if needed.

## Concurrency model

- `supervisor_task` — the sequence (this page). Blocks on notifications/queues;
  never busy-spins (per [[conventions]]).
- ISRs (limit switch, encoder) — tiny, `IRAM_ATTR`, defer to the supervisor.
- Optional `thermostat_task` — only if HEAT is split out.

`app_main` stays thin: init hardware, create `supervisor_task`, return. Task
source files live under `main/tasks/` and must be listed explicitly in the
relevant `CMakeLists.txt` `SRCS` (no globbing) — see [[build-and-simulation]].

## Dependencies still missing

To complete the sequence the remaining drivers must land:

- **DC motor driver (DRV8870)** — three H-bridges: pan (IN1=GPIO41, IN2=GPIO42),
  egg breaker (IN1=GPIO15, IN2=GPIO7), planetary mixer (IN1=GPIO10, IN2=GPIO11).
  Direction = which input is asserted; speed = PWM duty (LEDC). Only the
  planetary needs RPM control (Strapazzate); pan and breaker can run full-speed
  (plain GPIO direction) to save LEDC channels. Provides `motor_pan_start/stop`
  and sets `motion_direction[]`. **PLANNED.**
- **Rotary encoder driver (KY-040)** — quadrature decode + button, emits input
  events to the supervisor. **PLANNED.**
- **Heater switch** — GPIO/PWM, gated by `tmp102` reads. **PLANNED.**

The `limit_switch` driver and `buzzer` are implemented. See [[drivers]].

Related: [[overview]] · [[drivers]] · [[hardware]] · [[conventions]]
