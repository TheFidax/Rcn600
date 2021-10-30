/*
*	Questo esempio permette di vedere la corretta Acquisizione dei messaggi SUSI:
*   -   Stampa a video i Byte ricevuti (Esclusi quelli per la gestione CVs)
*   -   Permette la lettura/scrittura delle CVs salvate nella EEPROM del Microcontrollore
*/

#include <stdint.h>     // Libreria per i tipi "uintX_t"
#include <Rcn600.h>     // Includo la libreria per la gestione della SUSI
#include <EEPROM.h>     // Libreria per la gestione della EEPROM interna

Rcn600 SUSI(2, 3);      // (CLK pin, DATA pin) il pin di Clock DEVE ESSERE di tipo interrupt, il pin Data puo' essere in pin qualsiasi: compresi gli analogici

void notifySusiRawMessage(uint8_t firstByte, uint8_t secondByte) {                                                  // Funzione CallBack invocata quanto un messaggio e' in attesa di decodifica
    Serial.print("notifySusiRawMessage : ");

    Serial.print(firstByte, BIN);                                                                                   // Stampo i Byte ricevuti in formato Binario
    Serial.print(" - "); 
    Serial.print(secondByte, BIN); 

    Serial.print(" ( ");                                                                                            // Stampo il valore Decimale dei Byte ricevuti
    Serial.print(firstByte, DEC); 
    Serial.print(" - "); 
    Serial.print(secondByte, DEC); 
    Serial.println(" )");
}

uint8_t notifySusiCVRead(uint16_t CV) {                                                                             // Funzione CallBack per leggere il valore di una CV memorizzata
    return EEPROM.read(CV);                                                                                         // Restituisce il valore memorizzato nella EEPROM
}

uint8_t notifySusiCVWrite(uint16_t CV, uint8_t Value) {                                                             // Funzione CallBack per scrivere il valore di una CV memorizzata
    EEPROM.update(CV, Value);                                                                                       // Se il valore richiesto e' diverso aggiorno la EEPROM, in caso contrario non modifico per non rovinarla

    return EEPROM.read(CV);                                                                                         // Restituisco il nuovo valore della EEPROM
}

void setup() {                                                                                                      // Setup del Codice
    Serial.begin(500000);                                                                                           // Avvio la comunicazione Seriale
    while (!Serial) {}                                                                                              // Attendo che la comunicazione seriale sia disponibile

    Serial.println("SUSI Print Raw Messages:");                                                                     // Messaggio di Avvio

    SUSI.init();                                                                                                    // Avvio la libreria
}

void loop() {                                                                                                       // Loop del codice
    SUSI.process();                                                                                                 // Elaboro piu' volte possibile i dati acquisiti dalla libreria
}
