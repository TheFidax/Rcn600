/*
*	Questo esempio configura Arduino come Master del bus SUSI e invia ciclicamente il comando sul gruppo Funzioni per verificare la correttezza della comunicazione
*/

#include <Rcn600Master.h>      // Includo la libreria per la gestione della SUSI

Rcn600Master SusiMaster(2, 3);      // (CLK pin, DATA pin)

#define DELAY   500

void setup() {  
	SusiMaster.init();
}

void loop() {
	static uint32_t t = millis();

	if (millis() - t > DELAY) {
		static bool state = true;

		if (state) {
			SusiMaster.sendSusiRealSpeed(0, SUSI_DIR_FWD);
			SusiMaster.sendSusiRequestSpeed(127, SUSI_DIR_FWD);
		}
		else {
			SusiMaster.sendSusiRealSpeed(127, SUSI_DIR_FWD);
			SusiMaster.sendSusiRequestSpeed(0, SUSI_DIR_FWD);
		}

		state = !state;
		t = millis();
	}
}
