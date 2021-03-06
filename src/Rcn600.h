#ifndef RCN_600_h
#define RCN_600_h

//#define DEBUG_RCN600	// Permette di abilitare il debug tramite stream

#include "Arduino.h"
#include <stdint.h>
#include <stdbool.h>
#ifdef DEBUG_RCN600
	#include <Wire.h>
#endif

#ifdef __AVR__	// Se la piattaforma e' AVR posso usare la libreria digitalPinFast; e' escludibile per risparmiare RAM
	#define	DIGITAL_PIN_FAST
#endif // __AVR__

#ifdef DIGITAL_PIN_FAST
	#include <digitalPinFast.h>
#endif

#include "DataHeaders/SUSI_DATA_TYPE.h"

#define	MANUAL_MESSAGES				255		//indica che la libreria funzionera' solamente come Decoder di messaggi SUSI e non si occupera' di acquisirli

#define	MANUFACTER_ID				13		//identifica il costrutte del modulo SUSI: 13 da normativa NMRA : https://www.nmra.org/sites/default/files/appendix_a2c_s-9.2.2.pdf
#define	SUSI_VER					10		//identifica la versione del protocollo SUSI: 1.0
#define DEFAULT_SLAVE_NUMBER		1		//identifica l'indirizzo dello Slave SUSI: default 1
#define MAX_ADDRESS_VALUE			3

#define	SYNC_TIME					9		//tempo necessario a sincronizzare Master e Slave: 9ms
#define MIN_LEVEL_CLOCK_TIME		20		//minima durata di un livello di Clock
#define MAX_CLOCK_TIME				500		//massima durata di un Clock : livello alto + livello basso

#define SUSI_BUFFER_LENGTH			5

typedef struct message {
	uint8_t Byte[2];
	uint8_t cvArgument;

	struct message* nextMessage;
} Rcn600Message;

class Rcn600 {
	private:
		uint8_t	_slaveAddress;			// identifica il numero dello slave sul Bus SUSI (valori da 1 a 3)
		uint8_t	_CLK_pin;				// pin a cui e' collegata la linea "Clock";		DEVE ESSERE DI TIPO INTERRUPT

		Rcn600Message _Buffer[SUSI_BUFFER_LENGTH];	// Buffer contenenti i comandi SUSI ricevuti
		Rcn600Message* _BufferPointer;				// Puntatore per scorrere il Buffer

#ifdef	DIGITAL_PIN_FAST
		digitalPinFast *_DATA_pin;			// Oggetto che contiene i dati del pin a cui e' collegata la linea Data
#else
		uint8_t	_DATA_pin;				// pin a cui e' collegata la linea "Data";		Puo' essere un pin qualsiasi (Compresi gli analogici)
#endif

	private:				/* Metodi Privati */
		void initClass(void);							// Inizializza a Input i pin a cui e' connesso il bus
		uint8_t readData(void);							// Legge il bit dalla linea Data
		Rcn600Message* searchFreeMessage(void);			// Cerca nel Buffer uno slot dove salvare il messaggio in ricezione
		void setNextMessage(Rcn600Message* nextMessage);// Inserisce nel buffer un messaggio ricevuto Completo dall'ISR
		void Data_ACK(void);							// funzione per esguire l'ACK della linea DATA quando necessario
		bool isCVvalid(uint16_t CV);					// ritorna True se il numero della CV passato e' valido per questo modulo Slave

	public:					/* Metodi Pubblici */
		Rcn600(uint8_t CLK_pin_i, uint8_t DATA_pin_i);	// Creazione dell'oggetto Rcn600
		~Rcn600(void);									// Distruzionde dell'oggetto Rcn600
		void init(void);								// Inizializzazione della libreria: collegamento Interrupt, reset Contatori
		void init(uint8_t SlaveAddress);				// Inizializzazione della libreria: collegamento Interrupt, reset Contatori e permette di scegliere l'indirizzo del modulo da 1 a 3

		int8_t addManualMessage(uint8_t firstByte, uint8_t secondByte, uint8_t CvManipulating);	// Permette di aggiungere Manualmente un messaggio alla coda da processare

		void process(void);								// Metodo che decodifica i Byte ricevuti, DEVE ESSERE RICHIAMATA DAL CODICE PIU' VOLTE POSSIBILE
		void ISR_SUSI(void);							// Metodo che gestisce 'acquisizione dati tramite Interrupt
};

// Funzioni Esterne, implementabili a discrizione dell'utente
#if defined (__cplusplus)
extern "C" {
#endif
	/*
	*	notifySusiRawMessage() viene invocato ogni volta che è presente un messaggio da decodificare
	*	Input:
	*		- i byte che compongono il messaggio acquisito dalla libreria
	*		- la lunghezza del messaggio
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiRawMessage(uint8_t firstByte, uint8_t secondByte, uint8_t CvManipulating) __attribute__((weak));
	/*
	*	notifySusiFunc() viene invocato quando: si ricevono i dati dal Master su un gruppo di funzioni digitali
	*	Input:
	*		- il gruppo Funzioni decodificato
	*		- lo stato del gruppo funzioni
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState) __attribute__((weak));
	/*
	*	notifySusiBinaryState() viene invocato quando: si ricevono i dati dal Master sullo stato di UNA specifica funzione
	*	Input:
	*		- il numero della funzione (da 1 a 127)
	*		- lo stato della Funzione (attiva = 1, disattiva = 0)
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiBinaryState(uint16_t Command, uint8_t CommandState) __attribute__((weak));
	/*
	*	notifySusiAux() viene invocato quando: si ricevono i dati dal Master sullo stato di UNA specifica AUX
	*	Input:
	*		- il numero dell'AUX
	*		- lo stato dell'uscita (attiva = 1, disattiva = 0)
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiAux(SUSI_AUX_GROUP SUSI_auxGrp, uint8_t SUSI_AuxState) __attribute__((weak));
	/*
	*	notifySusiTriggerPulse() viene invocato quando: si riceve dal Master il comando di Trigger (o pulsazione) per eventuali sbuffi di vapore
	*	Input:
	*		- stato del comando Trigger/Pulse
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiTriggerPulse(uint8_t state) __attribute__((weak));
	/*
	*	notifySusiMotorCurrent() viene invocato quando: si riceve dal Master i dati sull'assorbimento di Corrente da parte del Motore
	*	Input:
	*		- Assorbimento di Corrente: da -128 a + 127 (gia' convertita dal Complemento a 2 originale)
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiMotorCurrent(int current) __attribute__((weak));
	/*
	*	notifySusiRequestSpeed() viene invocato quando: si ricevono i dati sulla Velocita' e sulla Direzione richiesti dalla Centrale al Master
	*	Input:
	*		- la velocita' (128 step) richiesta
	*		- la direzione richiesta
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiRequestSpeed(uint8_t Speed, SUSI_DIRECTION Dir) __attribute__ ((weak));
	/*
	*	notifySusiRealSpeed() viene invocato quando: si ricevono i dati dal Master sulla Velocita' e sulla Direzione reali 
	*	Input:
	*		- la velocita' (128 step) reale
	*		- la direzione reale
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiRealSpeed(uint8_t Speed, SUSI_DIRECTION Dir) __attribute__ ((weak));
	/*
	*	notifySusiMotorLoad() viene invocato quando: si riceve dal Master i dati sul carico del Motore
	*	Input:
	*		- Carico del Motore: da -128 a + 127 (gia' convertita dal Complemento a 2 originale)
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiMotorLoad(int load) __attribute__((weak));
	/*
	*	notifySusiAnalogFunction() viene invocato quando: si ricevono i dati dal Master su un gruppo di funzioni analogiche
	*	Input:
	*		- il gruppo Analogico decodificato
	*		- lo stato del gruppo
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiAnalogFunction(SUSI_AN_GROUP SUSI_AnalogGrp, uint8_t SUSI_AnalogState) __attribute__((weak));
	/*
	*	notifySusiAnalogDirectCommand() viene invocato quando: si ricevono i dati dal Master i comandi diretti per il funzionamento analogico
	*	Input:
	*		- il numero del comando: 1 o 2
	*		- i bit del comando
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiAnalogDirectCommand(uint8_t commandNumber, uint8_t Command) __attribute__((weak));
	/*
	*	notifySusiNoOperation() viene invocato quando: si riceve il comando "no operation", serve prevalentemente a scopo di test
	*	Input:
	*		- l'argomento del comando
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiNoOperation(uint8_t commandArgument) __attribute__((weak));
	/*
	*	notifySusiMasterAddress() viene invocato quando: si riceve l'indirizzo digitale del Master
	*	Input:
	*		- l'indirizzo Digitale del Master
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiMasterAddress(uint16_t MasterAddress) __attribute__((weak));
	/*
	*	notifySusiControlModule() viene invocato quando: si riceve il comando sul controllo del modulo
	*	Input:
	*		- byte contenete il controllo del modulo
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiControllModule(uint8_t ModuleControll) __attribute__((weak));

	

	/* METODI MANIPOLAZIONE CVs */
	/*
	*	notifySusiCVRead() viene invocato quando: e' richiesta la lettura di una CV
	*	Input:
	*		- il numero della CV da leggere
	*	Restituisce:
	*		- ritorna il valore della CV letta
	*/
	extern uint8_t notifySusiCVRead(uint16_t CV) __attribute__((weak));
	/*
	*	notifySusiCVWrite() viene invocato quando: e' richiesta la Scrittura di una CV.
	*	Input:
	*		- il numero della CV richiesta
	*		- il Nuovo valore della CV
	*	Restituisce:
	*		- il valore letto (post scrittura) nella posizione richiesta
	*/
	extern uint8_t notifySusiCVWrite(uint16_t CV, uint8_t Value) __attribute__((weak));
	/* RESET CVs, viene utilizzato lo stesso metodo della Libreria NmraDcc:
	* 
	*  notifyCVResetFactoryDefault() Called when CVs must be reset.
	*                                This is called when CVs must be reset
	*                                to their factory defaults.
	*
	*  Inputs:
	*    None
	*                                                                                                        
	*  Returns:
	*    None
	*/
	extern void notifyCVResetFactoryDefault(void) __attribute__((weak));

	/* ACK PER MESSAGGI ACQUISITI DA DIVERSO DISPOSITIVO */
	/*
	*	ackManualMessage() viene invocato quando: e' necessario un ACK post modifiche di una CVs.
	*	Quando la libreria Processa Solamente i messaggi senza acquisirli, e' lasciato all'utente la modalita' con cui comunicare al dispositivo master l'ACK.
	*	Input:
	*		- Nulla
	*	Restituisce:
	*		- Null
	*/
	extern void ackManualMessage(void) __attribute__((weak));
#if defined (__cplusplus)
}
#endif

#endif
