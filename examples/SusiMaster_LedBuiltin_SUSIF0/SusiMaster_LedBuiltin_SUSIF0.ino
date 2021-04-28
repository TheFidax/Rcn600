/*
*	Questo esempio configura Arduino come Master del bus SUSI e invia ciclicamente il comando sul gruppo Funzioni per verificare la correttezza della comunicazione
*/

#include <Rcn600Master.h>      // Includo la libreria per la gestione della SUSI

Rcn600Master SUSI(2, 3);      // (CLK pin, DATA pin)

#define DELAY   500

void setup() {  
	pinMode(LED_BUILTIN, OUTPUT);
	SUSI.init();
}

void loop() {
	static uint32_t t = millis();

	if (millis() - t > DELAY) {
		static bool state = true;

		if (state) {
			SUSI.sendSusiFunc(SUSI_FN_0_4, SUSI_FN_BIT_00);
			digitalWrite(LED_BUILTIN, HIGH);
		}
		else {
			SUSI.sendSusiFunc(SUSI_FN_0_4, 0);
			digitalWrite(LED_BUILTIN, LOW);
		}

		state = !state;
		t = millis();
	}
}