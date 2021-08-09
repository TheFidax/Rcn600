#include "SusiOverI2c.h"
#include <Rcn600.h>

void setup() {  
    Serial.begin(115200);

    //inizializzo il Bus I2C per eventuali Moduli esterni
    initSusiOverI2C(MAIN_DECODER_ADDRESS);

    while (SearchExternalI2CDevices() == 0) {}  // Attendo che venga trovato almeno un dispositivo I2c
}

void loop(void) {
    sendSusiMessage(0x060, 0b00010000, 0);	// Attivo la F0

    delay(1000);

    sendSusiMessage(0x60, 0, 0);		// Disattivo la F0

    delay(1000);
}


