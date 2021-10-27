#include <stdint.h>
#include <Rcn600.h>			// Includo la libreria per la gestione della SUSI
#include <EEPROM.h>

Rcn600 SUSI(EXTERNAL_CLOCK, 3); // Inizializzo la libreria usando una fonte esterna per il Clock: Clock Pin7, Data Pin3

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
    Serial.begin(500000);   // Avvio la comunicazione Seriale

    while (!Serial) {}      // Attendo che la comunicazione seriale sia disponibile
    Serial.println("PrintRawMessages with External Clock:");

    // Imposto il pin 7 come pin per il clock
    pinMode(7, INPUT);         	// 7 == PD7
    PCICR |= 0b00000100;      	// Abilito i "Port Change Interrupt" sulla porta D
    PCMSK2 |= 0b10000000;      	// Abilito, per la porta D, il pin 7 (PD7 == pin 7)

    SUSI.init();			// Avvio la libreria
}

void loop() {
	SUSI.process();			// Elaboro piu' volte possibile i dati acquisiti
}

ISR(PCINT2_vect) {              	// Port D, PCINT16 - PCIN23
    	/* 
    	* Da normativa i dati fanno letti sul "fronte di discesa" del Clock
	* Devo richiamre l'ISR della Libreria quando so che e' avvenuto il fronte di discesa
	*/ 
	if (!(PIND & (1 << PIND7))) {
        	SUSI.ISR_SUSI();
	}
}