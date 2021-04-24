/*
Questo esempio permette di vedere, tramite seriale, il gruppo di Byte (con i rispettivi valori) ricveuti dal Master.
Per eseguire tale operazione E' INDISPENSABILE ABILITARE LA MODALITA' DI NOTIFY_RAW_MESSAGE NEL FILE Rcn600.h 
*/
#include <Rcn600.h>			// Includo la libreria per la gestione della SUSI
#include <EEPROM.h>

Rcn600 SUSI(2, 3);			// (CLK pin, DATA pin) il pin di Clock DEVE ESSERE di tipo interrupt, il pin Data puo' essere in pin qualsiasi: compresi gli analogici

void notifySusiRawMessage(uint8_t* rawMessage, uint8_t messageLength) {
    static uint8_t i;

    Serial.print("notifySusiRawMessage : ");

    for (i = 0; i < messageLength; ++i) {
        Serial.print(rawMessage[i], BIN);
        if (i < (messageLength - 1)) {
            Serial.print(" - ");
        }
    }

    Serial.print(" ( ");

    for (i = 0; i < messageLength; ++i) {
        Serial.print(rawMessage[i]);
        if (i < (messageLength - 1)) {
            Serial.print(" - ");
        }
    }

    Serial.println(" )");
}

uint8_t notifySusiCVRead(uint16_t CV) {
    return EEPROM.read(CV);
}

uint8_t notifySusiCVWrite(uint16_t CV, uint8_t Value) {
    EEPROM.update(CV, Value);

    return EEPROM.read(CV);
}

void setup() {
    Serial.begin(115200);   // Avvio la comunicazione Seriale

    while (!Serial) {}      // Attendo che la comunicazione seriale sia disponibile

    SUSI.init();			// Avvio la libreria
}

void loop() {
    SUSI.process();			// Elaboro piu' volte possibile i dati acquisiti
}