/*
*	Questo esempio configura Arduino come Master del bus SUSI e invia ciclicamente il comando sul gruppo Funzioni per verificare la correttezza della comunicazione
*/

#include <Rcn600Master.h>      // Includo la libreria per la gestione della SUSI

Rcn600Master SUSI(2, 3);      // (CLK pin, DATA pin)

#define DELAY   500

void setup() {  
	SUSI.init();
}

void loop() {
	static uint32_t t = millis();

	SUSI.process();

	if (millis() - t > DELAY) {
		static bool state = true;

		if (state) {
			SUSI.sendSusiRealSpeed(0, SUSI_DIR_FWD);
			SUSI.sendSusiRequestSpeed(127, SUSI_DIR_FWD);
		}
		else {
			SUSI.sendSusiRealSpeed(127, SUSI_DIR_FWD);
			SUSI.sendSusiRequestSpeed(0, SUSI_DIR_FWD);
		}

		state = !state;
		t = millis();
	}
}