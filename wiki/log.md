# Ovotronic Wiki — Log

Append-only, chronological record of wiki changes. Newest at the bottom.
Prefix format: `## [YYYY-MM-DD] action | Title`.

## [2026-06-26] create | Wiki bootstrapped
Created the wiki following the LLM-wiki structure (gist by karpathy). Added the
backbone files [[index]] and this log, plus the initial content pages:
[[overview]], [[hardware]], [[drivers]], [[state-machine]],
[[build-and-simulation]], and [[conventions]]. Content derived from the repo at
commit `c8c4f57`: the four implemented drivers (`i2c_bus`, `tmp102q1`,
`lcd1602`, `sg90`), the pseudocode skeleton in `main/main.c`, `CLAUDE.md`,
`wokwi.toml`, and `sdkconfig`. The state machine and the `limit_switch` /
`buzzer` drivers are documented as **PLANNED** — designed in discussion, not yet
in code.

## [2026-06-26] update | Buzzer implemented, limit_switch scaffolded
Synced the wiki to commit `214975d`. The `buzzer` driver is now **implemented**
(`buzzer_init` / `buzzer_ring`, LEDC `TIMER_1` ch 2, 10-bit, default 3 kHz, GPIO
48) and initialized in `app_main`; moved it from PLANNED to implemented across
[[drivers]], [[hardware]], [[overview]], [[index]]. The `limit_switch` component
is now **scaffolded** — registered in the build and listed in
`main/CMakeLists.txt` `REQUIRES`, with the consuming flags (`pan_at_top`,
`pan_at_bottom`, `breaker_at_begin`, `breaker_at_end`) declared in `main.c`, but
`limit_switch.c` / `.h` are still empty stubs. Updated the `main/CMakeLists.txt`
snippet in [[build-and-simulation]] and noted the diagram's four slide switches +
encoder in [[hardware]]. State machine remains **PLANNED**.

## [2026-06-27] update | State machine architecture decided (Option A)
Rewrote [[state-machine]] for the **single-orchestrator** design: one
`supervisor_task` owns the whole sequence and calls actuators as blocking
helpers. Recorded the key decision — **direction is local commanded state, never
inferred by toggling/counting interrupts** (the toggle approach desyncs on
contact bounce). Documented the implemented `limit_switch` flow: tiny
`IRAM_ATTR` ISR → `xTaskNotifyFromISR((1u << axis), eSetBits)` → supervisor
debounces with `vTaskDelay(30ms)` + `ulTaskNotifyValueClear` (a bare delay does
not debounce — bounce edges keep queueing notifications, the "fires twice"
symptom). Added the state table, the `pan_move_to` blocking-move pattern,
encoder-based user input, and the inline-vs-task thermostat options. Listed the
still-missing drivers with their real `diagram.json` pins: DRV8870 motors (pan
41/42, breaker 15/7, planetary 10/11) and the KY-040 encoder (CLK 18, DT 17,
SW 21). `limit_switch` moved from PLANNED toward implemented; the broader state
machine is now **IN PROGRESS**.

## [2026-06-27] update | Synced drivers + hardware to current design
Brought [[drivers]] and [[hardware]] in line with [[state-machine]]. In
[[drivers]]: moved `limit_switch` out of "Scaffolded" into a new "In progress"
section with the real public API (`limit_switch_init`,
`limit_switch_set_notify_task_handle`), the external pull-up / `NEGEDGE` wiring,
the ISR→task-notification approach, and the outstanding `POSEDGE`→`NEGEDGE` fix.
Replaced the vague "Motor / heater" PLANNED stub with three concrete planned
drivers — `dc_motor` (DRV8870 ×3 with the real pins and the LEDC-budget rationale
for PWM-only-on-the-mixer), `encoder` (KY-040), and `heater`. In [[hardware]]:
added `LEDC_TIMER_2` for the mixer PWM + a channel-budget note, and expanded the
peripheral table with the three DC motors, the rotary encoder, and the corrected
`limit_switch` row (GPIO 12/13, ISR → task notification). Confirmed the design
target: **one** FreeRTOS task (`supervisor_task`); a second (`thermostat_task`)
only if HEAT is later split out.

## [2026-06-27] update | Limit switch edge corrected to POSEDGE (NC in series)
Resolved a contradiction between the docs and `diagram.json`. The diagram wires
the two limit switches per axis as **normally-closed, in series** to one GPIO
(`j5→sw2→sw1→gnd`), which means: idle = both closed = line LOW; reaching a limit
**opens** the actuated switch → series breaks → line HIGH = **rising edge**. So
`GPIO_INTR_POSEDGE` in `limit_switch.c` is **correct** (it is the limit-reached
event), and the earlier "must switch to NEGEDGE" TODO was wrong. Corrected
[[hardware]], [[drivers]], and [[state-machine]] to describe NC / idle-LOW /
POSEDGE / fail-safe (broken wire reads as "limit reached"). Architecture for both
extremes confirmed: 2 GPIO + direction inference (`motion_direction[]` set by the
motor command before moving), not 4 independent GPIO; the event group stays
optional under the single-orchestrator design (Option A).
