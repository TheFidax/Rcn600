/*
*   QUESTO ESEMPIO UTILIZZA, PER L'ACQUISIZIONE DEL SEGNALE DI CLOCK, IL SISTEMA "PORTCHANGEINTERRUPT"
*	Questo esempio permette di vedere la corretta Acquisizione dei messaggi SUSI:
*   -   Stampa a video i Byte ricevuti (Esclusi quelli per la gestione CVs)
*   -   Permette la lettura/scrittura delle CVs salvate nella EEPROM del Microcontrollore
*/

#include <stdint.h>     // Libreria per i tipi "uintX_t"
#include <Rcn600.h>     // Includo la libreria per la gestione della SUSI
#include <EEPROM.h>     // Libreria per la gestione della EEPROM interna

Rcn600 SUSI(EXTERNAL_CLOCK, 3);     // Inizializzo la libreria usando una fonte esterna per il Clock: Clock Pin7, Data Pin3

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

void setup() {
    Serial.begin(500000);                                                                                           // Avvio la comunicazione Seriale
    while (!Serial) {}                                                                                              // Attendo che la comunicazione seriale sia disponibile
    
    Serial.println("PrintRawMessages with External Clock:");                                                        // Messaggio di Avvio

    // Imposto il pin 7 come pin per il clock
    pinMode(7, INPUT);                                                                                              // 7 == PD7
    PCICR |= 0b00000100;      	                                                                                    // Abilito i "Port Change Interrupt" sulla porta D
    PCMSK2 |= 0b10000000;      	                                                                                    // Abilito, per la porta D, il pin 7 (PD7 == pin 7)

    SUSI.init();                                                                                                    // Avvio la libreria
}

void loop() {                                                                                                       // Loop del codice
    SUSI.process();                                                                                                 // Elaboro piu' volte possibile i dati acquisiti dalla libreria
}

ISR(PCINT2_vect) {                                                                                                  // Port D, PCINT16 - PCIN23
    if (!(PIND & (1 << PIND7))) {                                                                                   // Mi assicuro che il valore del pin sia LOW
        SUSI.ISR_SUSI();                                                                                            // Invoco il ISR della libreria
    }
}
