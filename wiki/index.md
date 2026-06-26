# Ovotronic Wiki — Index

Content-oriented catalog of the wiki. Each entry links a page and a one-line
summary. Structure follows the LLM-wiki convention: this `index.md` is the
catalog, `log.md` is the append-only history, and `CLAUDE.md` (repo root) is the
schema. Pages cross-reference with `[[wikilinks]]`.

## Pages

### Project
- [[overview]] — What Ovotronic is and the end-to-end cooking sequence.
- [[state-machine]] — The firmware control flow (states, transitions, tasks).

### Hardware
- [[hardware]] — Target MCU, peripherals, buses, and pin/channel assignments.

### Software
- [[drivers]] — The `components/` drivers and their public APIs.
- [[conventions]] — Coding/structure rules the firmware follows.

### Tooling
- [[build-and-simulation]] — Build/flash/monitor with ESP-IDF and Wokwi sim.

## Status legend
Pages describe both what exists (the five implemented drivers — `i2c_bus`,
`tmp102q1`, `lcd1602`, `sg90`, `buzzer`), what is scaffolded (the `limit_switch`
component — registered and with its flags in `main.c`, but an empty driver), and
what is planned (the state machine and the mixer-motor / heater control). Planned
and in-progress items are marked inline so the wiki stays honest about the gap
between design and code.
