#ifndef SUSI_OVER_I2C_h
#define SUSI_OVER_I2C_h

#include <stdint.h>
#include <Wire.h>
#include <Rcn600.h>

/* Indirizzi Preimpostati Dispositivi su Bus I2C */
#define	MAIN_DECODER_ADDRESS			0x01
#define SLAVE_DECODER_ADDRESS     0x02

extern uint8_t nDevices;						// indica quanti dispositivi sono collegati al Bus I2c

// Funzioni per il Master del BUS 
  /*
  * initSusiOverI2C() inizializza il Bus I2C per spedire i messaggi - Usato dal Master
  * Input:
  *   - L'indirizzo del Modulo sul Bus i2c
  * Restituisce:
  *   - Nulla
  */
void initSusiOverI2C(uint8_t I2cAddr);
	/*
	*	endSusiOverI2C() disabilita il bus I2C
	*	Input:
	*		- Nulla
	*	Restituisce:
	*		- Nulla
	*/
void endSusiOverI2C(void);
	/*
	*	SearchExternalI2CDevices() controlla se sul Bus sono presenti dispositivi e ne memorizza l'indirizzo in una variabile privata
	*	Input:
	*		- Nulla
	*	Restituisce:
	*		- Il numero di dispositivi trovati
	*/
uint8_t SearchExternalI2CDevices(void);
  /*
  * sendByte() invia 1 Byte a tutti i dispositivi I2c collegati
  * Input:
  *   - il Byte da inviare
  * Restituisce:
  *   - Nulla
  */
void sendByte(uint8_t Byte);
	/*
	*	sendSusiMessage() invia, tramite Struct dedicata, tutti gli elementi di un messaggio Rcn600 Susi
	*	Input:
	*		- il primo Byte del messaggio
	*		- il Secondo Byte del messaggio
	*		- l'Argomento di manipolazione delle CVs
	*	Restituisce:
	*		- Nulla
	*/
void sendSusiMessage(uint8_t firstByte, uint8_t secondByte, uint8_t CvManipulating = 0);

#endif
