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
