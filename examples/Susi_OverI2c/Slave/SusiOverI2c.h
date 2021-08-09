#ifndef SUSI_OVER_I2C_h
#define SUSI_OVER_I2C_h

#include <stdint.h>
#include <Wire.h>
#include <Rcn600.h>

/* Indirizzi Preimpostati Dispositivi su Bus I2C */
#define	MAIN_DECODER_ADDRESS			0x01
#define SLAVE_DECODER_ADDRESS     0x02

/* Funzioni da Assegnare agli eventi I2C*/
void receiveEvent_SusiOverI2C(int bytes);
void requestEvent_SusiOverI2C(void);

	/*
	*	initSusiOverI2C() inizializza il Bus I2C per spedire i messaggi - Usato dagli Slave
	*	Input:
	*		- L'indirizzo del Modulo sul Bus i2c
	*		- L'Oggetto Rcn600 passato per riferimento
	*	Restituisce:
	*		- Nulla
	*/
void initSusiOverI2C(uint8_t I2cAddr, Rcn600* SUSI);
	/*
	*	endSusiOverI2C() disabilita il bus I2C
	*	Input:
	*		- Nulla
	*	Restituisce:
	*		- Nulla
	*/
void endSusiOverI2C(void);

#endif
