# Conventions

Rules the firmware follows (from the README block in `main/main.c` and
`CLAUDE.md`).

## Code structure
- **Keep `app_main` thin.** Declare functions in separate `.c` files and include
  them via headers; `app_main` should init hardware and spawn tasks, not hold
  logic. See [[state-machine]].
- **Always use FreeRTOS primitives** for task creation — no bare `while` loops
  running free in `app_main`.
- **One peripheral = one component** under `components/`, with
  `CMakeLists.txt` + `<name>.c` + `include/<name>.h`. See [[drivers]].
- **Register new code:** new components go in `main/CMakeLists.txt` `REQUIRES`;
  new `.c` files under `main/` go in `SRCS` (no globbing). See
  [[build-and-simulation]].

## Naming & language
- Variable names as explicit and descriptive as possible.
- **Comments and docs in Italian** (project language). Identifiers/APIs are in
  English.

## Concurrency
- State shared between an ISR and a task must be `volatile` (compiler-visibility,
  not atomicity). Single-word flags are atomic on the 32-bit ESP32-S3; only
  bundle related fields under a critical section if they must stay mutually
  consistent. Prefer one independent `volatile bool` per event. See the
  `limit_switch` notes in [[drivers]].
- ISR handlers stay minimal and `IRAM_ATTR`; hand work off to a task (set a flag,
  or notify) rather than doing heavy/non-ISR-safe work in interrupt context.

## Repo notes
- `sdkconfig` is committed; `sdkconfig.old` is a prior snapshot.
- The schema document for this wiki is `CLAUDE.md` at the repo root.

Related: [[drivers]] · [[state-machine]] · [[build-and-simulation]]
