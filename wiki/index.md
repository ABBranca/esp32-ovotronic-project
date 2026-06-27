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
Pages describe what exists, what is in progress, and what is planned, marked
inline so the wiki stays honest about the gap between design and code:
- **Implemented:** five drivers — `i2c_bus`, `tmp102q1`, `lcd1602`, `sg90`,
  `buzzer`.
- **In progress:** the `limit_switch` driver (interrupt → task notification,
  consumed by the supervisor) and the state machine (single-orchestrator design,
  Option A — see [[state-machine]]).
- **Planned:** the `dc_motor` (DRV8870 ×3), `encoder` (KY-040), and `heater`
  drivers.
