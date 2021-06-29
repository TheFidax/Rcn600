#include <Rcn600.h>			// Includo la libreria per la gestione della SUSI
#include <EEPROM.h>

Rcn600 SUSI(2, 3);			// (CLK pin, DATA pin) il pin di Clock DEVE ESSERE di tipo interrupt, il pin Data puo' essere in pin qualsiasi: compresi gli analogici

void notifySusiRawMessage(uint8_t firstByte, uint8_t secondByte, uint8_t CvManipulating) {
    Serial.print("notifySusiRawMessage : ");

    Serial.print(firstByte, BIN); Serial.print(" - "); Serial.print(secondByte, BIN); Serial.print(" - "); Serial.print(CvManipulating, BIN);

    Serial.print(" ( ");

    Serial.print(firstByte, DEC); Serial.print(" - "); Serial.print(secondByte, DEC); Serial.print(" - "); Serial.print(CvManipulating, DEC);

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
    Serial.begin(115200); // Avvio la comunicazione Seriale

    while (!Serial) {}      // Attendo che la comunicazione seriale sia disponibile
    Serial.println("notifySusiRawMessage():");

    SUSI.init();			// Avvio la libreria
}

void loop() {
    SUSI.process();			// Elaboro piu' volte possibile i dati acquisiti
}