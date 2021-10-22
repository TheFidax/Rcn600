/*
*	Questo esempio permette di vedere la corretta decodifica dell'interfaccia SUSI accendendo il led incorporato nella scheda quando la Funzione0 e' attiva
*/

#include <Rcn600.h>      // Includo la libreria per la gestione della SUSI
#include <EEPROM.h>

Rcn600 SUSI(2, 3);      // (CLK pin, DATA pin) il pin di Clock DEVE ESSERE di tipo interrupt, il pin Data puo' essere in pin qualsiasi: compresi gli analogici

void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState) {
    switch (SUSI_FuncGrp) {
    case SUSI_FN_0_4:
        ((SUSI_FuncState & SUSI_FN_BIT_00) ? digitalWrite(LED_BUILTIN, HIGH) : digitalWrite(LED_BUILTIN, LOW));
        break;

    default: {}
    }
}

uint8_t notifySusiCVRead(uint16_t CV) {
    return EEPROM.read(CV);
}

uint8_t notifySusiCVWrite(uint16_t CV, uint8_t Value) {
    EEPROM.update(CV, Value);

    return EEPROM.read(CV);
}

void setup() {
    if ((EEPROM.read(897) != 1) || (EEPROM.read(897) != 2) || (EEPROM.read(897) != 3)) {	//controllo che la CV contenente l'indirizzo del Modulo sia nei valori consentiti
        EEPROM.update(897, 1);
    }

    pinMode(LED_BUILTIN, OUTPUT);
    SUSI.init();      // Avvio la libreria
}

void loop() {
    SUSI.process();     // Elaboro piu' volte possibile i dati acquisiti
}
