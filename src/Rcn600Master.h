#ifndef RCN_600Master_h
#define RCN_600Master_h

#include "Arduino.h"
#include <stdint.h>
#include <stdbool.h>

#include <NmraDcc.h>

#ifdef __AVR__
#include <digitalPinFast.h>
#endif // __AVR__

#include "DataHeaders/SUSI_DATA_TYPE.h"
#include "Rcn600Master_Timer.h"


#define TIMER_PRESCALER			0x02		// Identifica il prescaler del Timer che genera' il Clock
#define TIMER_RETARD			0			// Identifica da che valore il Timer usato per il ISR deve partire

#define BUFFER_LENGTH			10			// Lunghezza buffer comandi da inviare agli Slave
#define MESSAGES_BEFORE_SYNC	1			// Numero di messaggi che e' possibile inviare prima di lasciare i 9ms per la sincronizzazione

typedef enum {
	ClockMode = 0,
	SyncMode,
} ISR_mode;

typedef struct {
	uint8_t Bytes[3];

	bool isCvManipulating;
	bool sent;
} Rcn600_Message;

class Rcn600Master {
	private:
		ISR_mode	modeISR;
#ifdef __AVR__
		digitalPinFast *_CLK_pin;		// Oggetto che contiene i dati del pin a cui e' collegata la linea Clock
		digitalPinFast *_DATA_pin;		// Oggetto che contiene i dati del pin a cui e' collegata la linea Data
#else
		uint8_t	_CLK_pin;				// pin a cui e' collegata la linea "Clock"
		uint8_t	_DATA_pin;				// pin a cui e' collegata la linea "Data"
#endif
		Rcn600_Message	_Buffer[BUFFER_LENGTH];	// Buffer contenenti i comandi SUSI da mandare agli slave
		bool			_SyncNeeded;

		/* Metodi Privati */
#ifdef __AVR__
		void writeLine(digitalPinFast* fastPin, uint8_t bit);
#else
		void writeLine(uint8_t Pin, uint8_t bit);
#endif
		bool writeBuffer(bool isCvManipulating, uint8_t firstByte, uint8_t secondByte, uint8_t thirdByte = 0);

	public:
		Rcn600Master(uint8_t CLK_pin_i, uint8_t DATA_pin_i);	// Creazione dell'oggetto Rcn600
		~Rcn600Master(void);									// Distruzionde dell'oggetto Rcn600

		void printBuffer(void);

		void init(void);										// Inizializza il bus e attiva la generazione del Clock

		void ISR_SUSI(void);									// Genera il Clock e scrive i dati sulla linea DATA
		void ISR_Clock(void);
		void ISR_Sync(void);

		bool sendSusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState);		// Invia agli Slave lo stato delle funzioni
		bool sendSusiRealSpeed(uint8_t realSpeed, SUSI_DIRECTION dir);				// Invia agli Slave la velocita' reale a cui viaggia il modello
		bool sendSusiRequestSpeed(uint8_t requestSpeed, SUSI_DIRECTION dir);		// Invia agli Slave la velocita' richiesta dalla centrale al modello
};

// Funzioni Esterne, implementabili a discrizione dell'utente
#if defined (__cplusplus)
extern "C" {
#endif
	/*
	*	notifySusiBufferFull() viene invocato quando: il buffer dei comandi da inviare e' pieno
	*	Input:
	*		- Nulla
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiBufferFull(void) __attribute__((weak));
#if defined (__cplusplus)
}
#endif

extern Rcn600Master* pointerToRcn600Master;

#endif
