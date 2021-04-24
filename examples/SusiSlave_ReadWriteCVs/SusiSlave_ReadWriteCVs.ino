/*
*	Questo esempio legge e salva CVs memorizzate nella EEPROM del microcontrollore
*/

#include <Rcn600.h>      // Includo la libreria per la gestione della SUSI
#include <EEPROM.h>

Rcn600 SUSI(2, 3);      // (CLK pin, DATA pin) il pin di Clock DEVE ESSERE di tipo interrupt, il pin Data puo' essere in pin qualsiasi: compresi gli analogici

uint8_t notifySusiCVRead(uint16_t CV){
  return EEPROM.read(CV);
}

uint8_t notifySusiCVWrite(uint16_t CV, uint8_t Value){
  EEPROM.update(CV, Value);

  return EEPROM.read(CV);
}

void setup() {   
	if((EEPROM.read(897) != 1) || (EEPROM.read(897) != 2) || (EEPROM.read(897) != 3) ) {	//controllo che la CV contenente l'indirizzo del Modulo sia nei valori consentiti
		EEPROM.update(897, 1);
	}

	SUSI.init();      // Avvio la libreria
}

void loop() {
  SUSI.process();     // Elaboro piu' volte possibile i dati acquisiti
}