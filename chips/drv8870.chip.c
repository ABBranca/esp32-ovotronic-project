#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  pin_t pin_in1;
  pin_t pin_in2;
  pin_t pin_out1;
  pin_t pin_out2;
  pin_t pin_vref;
  pin_t pin_vm;
  pin_t pin_isen;
  pin_t pin_gnd;
} chip_state_t;

static void update_outputs(chip_state_t *chip) {
  uint32_t in1 = pin_read(chip->pin_in1);
  uint32_t in2 = pin_read(chip->pin_in2);

  if (in1 == LOW && in2 == LOW) {
    // Coast: le uscite entrano in Alta Impedenza (Hi-Z) impostandole come INPUT
    pin_mode(chip->pin_out1, INPUT);
    pin_mode(chip->pin_out2, INPUT);
  } else if (in1 == LOW && in2 == HIGH) {
    // Reverse
    pin_mode(chip->pin_out1, OUTPUT);
    pin_mode(chip->pin_out2, OUTPUT);
    pin_write(chip->pin_out1, LOW);
    pin_write(chip->pin_out2, HIGH);
  } else if (in1 == HIGH && in2 == LOW) {
    // Forward
    pin_mode(chip->pin_out1, OUTPUT);
    pin_mode(chip->pin_out2, OUTPUT);
    pin_write(chip->pin_out1, HIGH);
    pin_write(chip->pin_out2, LOW);
  } else if (in1 == HIGH && in2 == HIGH) {
    // Brake: entrambe le uscite a massa
    pin_mode(chip->pin_out1, OUTPUT);
    pin_mode(chip->pin_out2, OUTPUT);
    pin_write(chip->pin_out1, LOW);
    pin_write(chip->pin_out2, LOW);
  }
}

static void on_pin_change(void *user_data, pin_t pin, uint32_t value) {
  chip_state_t *chip = (chip_state_t *)user_data;
  update_outputs(chip);
}

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  // Inizializzazione input con pull-down interno integrato (comportamento
  // nativo DRV8870)
  chip->pin_in1 = pin_init("IN1", INPUT_PULLDOWN);
  chip->pin_in2 = pin_init("IN2", INPUT_PULLDOWN);

  // Inizializzazione output (partono in stato Coast / Hi-Z)
  chip->pin_out1 = pin_init("OUT1", INPUT);
  chip->pin_out2 = pin_init("OUT2", INPUT);

  // Pin ausiliari (utilizzati per completare la corrispondenza del pinout
  // fisico nel diagramma)
  chip->pin_vref = pin_init("VREF", INPUT);
  chip->pin_vm = pin_init("VM", INPUT);
  chip->pin_isen = pin_init("ISEN", INPUT);
  chip->pin_gnd = pin_init("GND", INPUT);

  const pin_watch_config_t watch_config = {
      .edge = BOTH, .pin_change = on_pin_change, .user_data = chip};

  // Monitoraggio attivo delle variazioni di stato sui pin di controllo
  pin_watch(chip->pin_in1, &watch_config);
  pin_watch(chip->pin_in2, &watch_config);

  // Sincronizzazione iniziale dello stato delle uscite al boot
  update_outputs(chip);

  printf("DRV8870 custom chip inizializzato correttamente.\n");
}