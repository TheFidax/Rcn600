/*
*	Questo esempio permette di vedere la corretta decodifica dell'interfaccia SUSI accendendo il led incorporato nella scheda quando la Funzione0 e' attiva
*/

#include <Rcn600.h>      // Includo la libreria per la gestione della SUSI

Rcn600 SusiSlave(2, 3);      // (CLK pin, DATA pin) il pin di Clock DEVE ESSERE di tipo interrupt, il pin Data puo' essere in pin qualsiasi: compresi gli analogici

void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState) {
  switch( SUSI_FuncGrp ) {
     case SUSI_FN_0_4:
       ((SUSI_FuncState & SUSI_FN_BIT_00) ? digitalWrite(LED_BUILTIN, HIGH) : digitalWrite(LED_BUILTIN, LOW));
       break;

     default: {}
   }
}

void setup() {   
  pinMode(LED_BUILTIN, OUTPUT);
  SusiSlave.init();      // Avvio la libreria
}

void loop() {
  SusiSlave.process();     // Elaboro piu' volte possibile i dati acquisiti
}
