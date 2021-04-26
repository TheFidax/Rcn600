/*
*	Questo esempio configura Arduino come Master del bus SUSI e invia ciclicamente il comando sul gruppo Funzioni per verificare la correttezza della comunicazione
*/

#include <Rcn600Master.h>      // Includo la libreria per la gestione della SUSI

Rcn600Master SusiMaster(2, 3);      // (CLK pin, DATA pin)

#define DELAY   500

void setup() {  
	pinMode(LED_BUILTIN, OUTPUT);
	SusiMaster.init();
}

void loop() {
	static uint8_t functState = 0;
	static uint32_t t = millis();

	if (millis() - t > DELAY) {
		static bool state = true;

		if (state) {
			functState += SUSI_FN_BIT_00;
			SusiMaster.sendSusiFunc(SUSI_FN_0_4, functState);
			functState += SUSI_FN_BIT_01;
			SusiMaster.sendSusiFunc(SUSI_FN_0_4, functState);
			functState += SUSI_FN_BIT_02;
			SusiMaster.sendSusiFunc(SUSI_FN_0_4, functState);
			functState += SUSI_FN_BIT_03;
			SusiMaster.sendSusiFunc(SUSI_FN_0_4, functState);
			functState += SUSI_FN_BIT_04;
			SusiMaster.sendSusiFunc(SUSI_FN_0_4, functState);
			digitalWrite(LED_BUILTIN, HIGH);
		}
		else {
			functState -= SUSI_FN_BIT_04;
			SusiMaster.sendSusiFunc(SUSI_FN_0_4, functState);
			functState -= SUSI_FN_BIT_03;
			SusiMaster.sendSusiFunc(SUSI_FN_0_4, functState);
			functState -= SUSI_FN_BIT_02;
			SusiMaster.sendSusiFunc(SUSI_FN_0_4, functState);
			functState -= SUSI_FN_BIT_01;
			SusiMaster.sendSusiFunc(SUSI_FN_0_4, functState);
			functState -= SUSI_FN_BIT_00;
			SusiMaster.sendSusiFunc(SUSI_FN_0_4, functState);
			digitalWrite(LED_BUILTIN, LOW);
		}

		state = !state;
		t = millis();
	}
}
