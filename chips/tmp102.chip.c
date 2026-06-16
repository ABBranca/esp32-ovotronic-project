#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Custom chip Wokwi per il sensore di temperatura TI TMP102-Q1.
 *
 * Modella un dispositivo slave sul bus I2C all'indirizzo 0x48. Il master
 * (ESP32-S3) scrive il pointer register e legge i 2 byte del registro di
 * temperatura (12 bit, allineati a sinistra, LSB = 0.0625 C). La temperatura
 * simulata e' impostata dall'utente tramite lo slider "temperature".
 *
 * Semplificazioni adottate per la simulazione:
 *  - Indirizzo I2C fisso 0x48: il pin ADD0 e' presente solo per fedelta' del
 *    pinout (SOT-563), non rimappa l'indirizzo.
 *  - Si modella il solo registro di temperatura (0x00); per qualunque valore
 *    del pointer register vengono comunque restituiti i byte di temperatura.
 *  - Il pin ALERT non e' pilotato (nessuna logica di soglia THIGH/TLOW).
 */

#define TMP102_I2C_ADDR 0x48
#define TMP102_REG_TEMP 0x00
#define TMP102_LSB_C    0.0625f /* risoluzione: 0.0625 C per LSB (12 bit) */

typedef struct {
  pin_t pin_vcc;
  pin_t pin_gnd;
  pin_t pin_sda;
  pin_t pin_scl;
  pin_t pin_add0;
  pin_t pin_alert;
  uint32_t temp_attr;    /* handle dell'attributo temperatura (slider) */
  uint8_t pointer_reg;   /* registro selezionato dal master */
  uint8_t read_byte_idx; /* byte corrente in lettura: 0 = MSB, 1 = LSB */
} chip_state_t;

/* Il chip viene indirizzato sul bus: inizio di una nuova transazione. */
static bool on_i2c_connect(void *user_data, uint32_t address, bool read) {
  chip_state_t *chip = (chip_state_t *)user_data;
  chip->read_byte_idx = 0; /* una lettura riparte sempre dal MSB */
  return true;             /* ACK */
}

/* Il master scrive un byte: e' il pointer register. */
static bool on_i2c_write(void *user_data, uint8_t data) {
  chip_state_t *chip = (chip_state_t *)user_data;
  chip->pointer_reg = data;
  return true; /* ACK */
}

/* Il master legge un byte dal registro di temperatura. */
static uint8_t on_i2c_read(void *user_data) {
  chip_state_t *chip = (chip_state_t *)user_data;

  /* Conversione: temperatura -> valore a 12 bit allineato a sinistra nei 16
     bit. Il cast a int16_t gestisce le temperature negative in complemento a
     due. Round-trip esatto con la formula di lettura firmware:
     raw = ((MSB << 8) | LSB) >> 4. */
  float t = attr_read_float(chip->temp_attr);
  int16_t raw12 = (int16_t)(t / TMP102_LSB_C); /* es. 25.0 C -> 400 (0x190) */
  int16_t reg = raw12 << 4;                    /* allinea: 0x1900 */

  uint8_t byte =
      (chip->read_byte_idx == 0) ? ((reg >> 8) & 0xFF) : (reg & 0xFF);
  if (chip->read_byte_idx < 1) chip->read_byte_idx++;
  return byte;
}

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  chip->pointer_reg = TMP102_REG_TEMP;
  chip->read_byte_idx = 0;
  chip->temp_attr = attr_init_float("temperature", 25.0f);

  /* Pin di alimentazione e ausiliari: presenti per fedelta' del pinout. */
  chip->pin_vcc = pin_init("VCC", INPUT);
  chip->pin_gnd = pin_init("GND", INPUT);
  chip->pin_add0 = pin_init("ADD0", INPUT);
  chip->pin_alert = pin_init("ALERT", INPUT);

  /* Linee I2C: open-drain con pull-up (riposo = HIGH). */
  chip->pin_sda = pin_init("SDA", INPUT_PULLUP);
  chip->pin_scl = pin_init("SCL", INPUT_PULLUP);

  const i2c_config_t i2c_config = {
      .user_data = chip,
      .address = TMP102_I2C_ADDR,
      .scl = chip->pin_scl,
      .sda = chip->pin_sda,
      .connect = on_i2c_connect,
      .read = on_i2c_read,
      .write = on_i2c_write,
      .disconnect = NULL,
  };
  i2c_init(&i2c_config);

  printf("TMP102-Q1 custom chip inizializzato (indirizzo I2C 0x%02X).\n",
         TMP102_I2C_ADDR);
}
