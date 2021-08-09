#include "SusiOverI2c.h"

/* Variabili Globali 'private' */
Rcn600* Susi;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void initSusiOverI2C(uint8_t I2cAddr, Rcn600 *SUSI) {
    Wire.onReceive(receiveEvent_SusiOverI2C);
    Wire.onRequest(requestEvent_SusiOverI2C);

    Wire.begin(I2cAddr);
    Wire.setClock(400000);

    Susi = SUSI;
}

void endSusiOverI2C(void) {
    Wire.onReceive(NULL);
    Wire.onRequest(NULL);

    Wire.end();

    Susi = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t Wire_readBytes(uint8_t *buffer, uint8_t size) {
    uint8_t i;

    for (i = 0; i < size; i++) {
        *buffer++ = Wire.read();
    }

    return i;
}

void receiveEvent_SusiOverI2C(int bytes) {
    uint8_t Buffer[32];

    Wire_readBytes((uint8_t*)&Buffer, bytes);

    if(bytes == sizeof(Rcn600Message)) {
        Rcn600Message *m = (Rcn600Message*)&Buffer;
    
        if(Susi != NULL) {
            Susi->addManualMessage(m->Byte[0], m->Byte[1],  m->Byte[2]);
        }
    }
    else {
        for(uint8_t i = 0; i < bytes; ++i) {
            Serial.println(Buffer[i]);
        }
    }
}

void requestEvent_SusiOverI2C(void) {

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
