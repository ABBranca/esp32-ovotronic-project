#include "freertos/idf_additions.h"
#include <stdio.h>

/*
README: Codice per l'implementazione di ESP32-S3 nel progetto Ovotronic.

- Dichiarare sempre le funzioni in file ".c" separati e poi includere gli
header.

- Usare sempre funzioni di RTOS per la creazione dei Task.

- I nomi delle variabili devono essere il più chiari possibile.

*/

void app_main(void) {
  /*
  Quando Ovotronic viene acceso l'ESP32-S3 carica sul bootloader questo
  programma e poi lo avvia. Di seguito sono riportati in pseudo-codice i
  passi del processo.
  */

  // [ ]: Ovotronic resetta la posizione verticale della padella, assicurandosi
  // che sia nella posizione più bassa tramite sensore fine corsa.

  // [ ]: L'utente seleziona il tipo di preparazione tra Uova Strapazzate e
  // Frittata e prosegue

  // [ ]: L'utente seleziona il numero di uova da preparare e prosegue

  // [ ]: Ovotronic comunica all'utente tramite lo schermo i quantitativi in
  // grammi di supplementi da aggiungere all'imbuto superiore per il numero di
  // uova selezionate e attende il segnale di avvio da parte dell'utente.

  // [ ]: Dopo il via libera dell'utente, Ovotronic rompe le uova e convoglia i
  // supplementi nella padella tramite attuazione del servomotore SG90 dedicato.

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