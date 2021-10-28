/*
*	Questo esempio permette di vedere la corretta decodifica dell'interfaccia SUSI:
*   -   Accendendo il led incorporato nella scheda quando la Funzione 0 e' attiva.
*   -   Permette la lettura/scrittura delle CVs salvate nella EEPROM del Microcontrollore
*/

#include <stdint.h>     // Libreria per i tipi "uintX_t"
#include <Rcn600.h>     // Includo la libreria per la gestione della SUSI
#include <EEPROM.h>     // Libreria per la gestione della EEPROM interna

Rcn600 SUSI(2, 3);      // (CLK pin, DATA pin) il pin di Clock DEVE ESSERE di tipo interrupt, il pin Data puo' essere in pin qualsiasi: compresi gli analogici

void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState) {                                           // Funzione CallBack che viene invocata quando e' decodificato un comando per le Funzioni
    switch (SUSI_FuncGrp) {                                                                                         // Scelgo a quale gruppo funzioni appartiene il comando
        case SUSI_FN_0_4: {                                                                                         // Funzioni da 0 a 4
            ((SUSI_FuncState & SUSI_FN_BIT_00) ? digitalWrite(LED_BUILTIN, HIGH) : digitalWrite(LED_BUILTIN, LOW)); // se la Funzione 0 e' attiva accendo il LED_BUILTIN
            break;
        }
        default: {}
    }
}

uint8_t notifySusiCVRead(uint16_t CV) {                                                                             // Funzione CallBack per leggere il valore di una CV memorizzata
    return EEPROM.read(CV);                                                                                         // Restituisce il valore memorizzato nella EEPROM
}

uint8_t notifySusiCVWrite(uint16_t CV, uint8_t Value) {                                                             // Funzione CallBack per scrivere il valore di una CV memorizzata
    EEPROM.update(CV, Value);                                                                                       // Se il valore richiesto e' diverso aggiorno la EEPROM, in caso contrario non modifico per non rovinarla

    return EEPROM.read(CV);                                                                                         // Restituisco il nuovo valore della EEPROM
}

void setup() {                                                                                                      // Setup del Codice
    pinMode(LED_BUILTIN, OUTPUT);                                                                                   // Imposto il pin a cui e' connesso il LED_BUILTIN come output
    SUSI.init();                                                                                                    // Avvio la libreria
}

void loop() {                                                                                                       // Loop del codice
    SUSI.process();                                                                                                 // Elaboro piu' volte possibile i dati acquisiti dalla libreria
}