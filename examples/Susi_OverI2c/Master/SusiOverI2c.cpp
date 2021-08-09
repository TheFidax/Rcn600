#include "SusiOverI2c.h"

/* Variabili Globali 'private' */
Rcn600* Susi;
uint8_t *i2cDeviceAddresses = NULL;
uint8_t nDevices = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void initSusiOverI2C(uint8_t I2cAddr) {
    Wire.onReceive(receiveEvent_SusiOverI2C);
    Wire.onRequest(requestEvent_SusiOverI2C);

    Wire.begin(I2cAddr);
    Wire.setClock(400000);

    Susi = NULL;
}

void endSusiOverI2C(void) {
    Wire.onReceive(NULL);
    Wire.onRequest(NULL);

    Wire.end();

    Susi = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t SearchExternalI2CDevices(void) {
    for (uint8_t address = 0; address < 127; ++address) {
        Wire.beginTransmission(address);

        if (Wire.endTransmission() == 0) {
            ++nDevices;
        }
    }

    if (nDevices > 0) {
        i2cDeviceAddresses = (uint8_t*)calloc(nDevices, sizeof(uint8_t)); //creo lo spazio per il primo indirizzo, se non ci saranno indirizzi lo liberero'
        uint8_t k = 0;

        for (uint8_t address = 0; address < 127; address++) {
            Wire.beginTransmission(address);

            if (Wire.endTransmission() == 0) {
                i2cDeviceAddresses[k] = address;
                ++k;
            }
        }
    }

    return nDevices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void sendByte(uint8_t Byte) {
    if (nDevices > 0) {
        for (uint8_t i = 0; i < nDevices; ++i) {
            Wire.beginTransmission(i2cDeviceAddresses[i]);
            Wire.write(&Byte, 1);
            Wire.endTransmission();
        }
    }
}

void sendSusiMessage(uint8_t firstByte, uint8_t secondByte, uint8_t CvManipulating) { 
    if (nDevices > 0) {
        Rcn600Message mess;

        mess.Byte[0] = firstByte;
        mess.Byte[1] = secondByte;
        mess.cvArgument = CvManipulating;

        for (uint8_t i = 0; i < nDevices; ++i) {
            Wire.beginTransmission(i2cDeviceAddresses[i]);
            Wire.write((uint8_t*)&mess, sizeof(Rcn600Message));
            Wire.endTransmission();
        }
    }
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
