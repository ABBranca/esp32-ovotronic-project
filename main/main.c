#include "buzzer.h"
#include "driver/gpio.h"
#include "hal/ledc_types.h"
#include "i2c_bus.h"
#include "lcd1602.h"
#include "limit_switch.h"
#include "sg90.h"
#include "soc/gpio_num.h"
#include "tmp102q1.h"
#include <stdint.h>

/*
README: Codice per l'implementazione di ESP32-S3 nel progetto Ovotronic.

XXX: Best Practices:

- Dichiarare sempre le funzioni in file ".c" separati e poi includere gli
header.

- Usare sempre funzioni di freeRTOS per la creazione dei Task.

- I nomi delle variabili devono essere il più chiari possibile.

*/

static const ledc_channel_t ds_sg90_channel = LEDC_CHANNEL_0;
static const ledc_channel_t eb_sg90_channel = LEDC_CHANNEL_1;
static const gpio_num_t ds_sg90_gpio = 2;
static const gpio_num_t eb_sg90_gpio = 47;
static const gpio_num_t buzzer_gpio = 48;
static const gpio_num_t pan_limit_switch_gpio = 13;
static const gpio_num_t eb_limit_switch_gpio = 12;

const uint32_t buzzer_default_frequency = BUZZER_DEFAULT_FREQUENCY;

void app_main(void) {
  /*
  Quando Ovotronic viene acceso l'ESP32-S3 carica sul bootloader questo
  programma e poi lo avvia. Di seguito sono riportati in pseudo-codice i
  passi del processo.
  */

  // Setup
  i2c_init();               // Inizializza bus per comunicazione I2C
  tmp102_init(bus_handle);  // Inizializza I2C sensore di temperatura
  lcd1602_init(bus_handle); // Inizializza schermo LCD1602
  buzzer_init(buzzer_gpio, buzzer_default_frequency); // Inizializza cicalino
  gpio_install_isr_service(0); // Inizializza ISR per sensori di fine corsa
  limit_switch_init(
      pan_limit_switch_gpio); // Inizializza sensore di fine corsa padella
  limit_switch_init(
      eb_limit_switch_gpio); // Inizializza sensore di fine corsa egg breaker

  lcd1602_set_cursor(0, 3);
  lcd1602_print("OVOTRONIC");

  sg90_timer_init(); // Inizializzazione Servo SG90

  sg90_channel_init(ds_sg90_channel,
                    ds_sg90_gpio); // Inizializzo canale sg90 per supplementi

  sg90_channel_init(eb_sg90_channel,
                    eb_sg90_gpio); // Inizializzo canale sg90 per egg breaker

  // [ ]: Ovotronic resetta la posizione verticale della padella,
  // assicurandosi che sia nella posizione più bassa tramite sensore fine
  // corsa.

  // [ ]: L'utente seleziona il tipo di preparazione tra Uova Strapazzate e
  // Frittata e prosegue

  // [ ]: L'utente seleziona il numero di uova da preparare e prosegue

  // [ ]: Ovotronic comunica all'utente tramite lo schermo i quantitativi in
  // grammi di supplementi da aggiungere all'imbuto superiore per il numero di
  // uova selezionate e attende il segnale di avvio da parte dell'utente.

  // [ ]: Dopo il via libera dell'utente, Ovotronic rompe le uova e convoglia
  // i supplementi nella padella tramite attuazione del servomotore SG90
  // dedicato.

  // [ ]: La padella si alza fino a fine corsa.

  // [ ]: La planetaria mescola gli ingredienti per un periodo di tempo
  // prestabilito.

  // [ ]: Se l'utente ha selezionato l'opzione "Frittata" allora la planetaria
  // smette di girare e la padella si abbassa.

  // [ ]: Se l'utente ha selezionato l'opzione "Uova Strapazzate" la padella
  // rimane in posizione ma la planetaria si abbassa di giri (se possiamo
  // farlo).

  // [ ]: La padella inizia a scaldarsi. Il termometro legge la temperatura e
  // regola lo spegnimento della resistenza di riscaldamento della padella

  // [ ]: Dopo il periodo di tempo previsto per la cottura delle uova, la
  // planetaria smette eventualmente di girare, la padella si abbassa e la
  // resistenza viene spenta.

  // [ ]: Il sistema notifica l'utente della fine del processo tramite un
  // cicalino.
};