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
