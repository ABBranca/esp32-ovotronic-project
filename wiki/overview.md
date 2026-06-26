# Overview

**Ovotronic** is the embedded firmware for an automated egg-cooking machine. It
runs on an **ESP32-S3** (see [[hardware]]) on top of **ESP-IDF + FreeRTOS**, in
C. The machine prepares one of two recipes — *Uova Strapazzate* (scrambled) or
*Frittata* (omelette) — fully automatically once the user makes their
selections.

## What the machine does (end to end)

On power-up the ESP32-S3 boots this firmware and runs the cooking process. The
intended sequence (currently pseudocode in `main/main.c`, see
[[state-machine]]):

1. **Reset** — the pan is driven to its lowest position, confirmed by a
   limit switch (end-of-travel sensor).
2. **Select recipe** — user chooses *Uova Strapazzate* or *Frittata*.
3. **Select egg count** — user chooses how many eggs.
4. **Show supplements** — the LCD shows the grams of supplements to add to the
   upper funnel for that egg count; the machine waits for a start signal.
5. **Dispense** — on start, eggs are broken and supplements are conveyed into
   the pan via the dedicated SG90 servo.
6. **Raise pan** — the pan rises to its end-of-travel limit.
7. **Mix** — the planetary mixer stirs the ingredients for a set time.
8. **Recipe branch**:
   - *Frittata* — mixer stops, pan lowers.
   - *Uova Strapazzate* — pan stays up, mixer lowers its RPM.
9. **Heat** — the pan's heating resistor turns on; the thermometer reads
   temperature and regulates the resistor (thermostat loop).
10. **Cook timeout** — after the set cook time: mixer stops (if running), pan
    lowers, resistor off.
11. **Notify** — a buzzer signals completion.

## Implementation status

- **Implemented:** the four I2C/PWM drivers and hardware init in `app_main`
  (see [[drivers]]).
- **PLANNED:** the state machine that sequences the steps above, plus the
  `limit_switch` and `buzzer` drivers and the mixer-motor and heater control.

Related: [[hardware]] · [[drivers]] · [[state-machine]] · [[conventions]]
