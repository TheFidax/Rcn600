#ifndef RCN_600_h
#define RCN_600_h

//#define	CHECK_FREE_RAM						//Se decommentata attiva i controlli sulla quantita' di ram disponibile prima di allocare risorse; per il corretto funzionamento e' necessaria una lireria esterna

#include <stdint.h>
#include <stdbool.h>
#include "Arduino.h"
#ifdef CHECK_FREE_RAM
#include "MemoryFree.h"						// disponibile qui: https://github.com/McNeight/MemoryFree
#endif

#include "DataHeaders/SUSI_FN_BIT.h"
#include "DataHeaders/SUSI_AUX_BIT.h"
#include "DataHeaders/SUSI_AN_BIT.h"

//#define	NOTIFY_RAW_MESSAGE	// Permette di esportare il messaggio grezzo per poterlo interpretare in maniera esterna alla libreria

#define	MANUFACTER_ID				13		//identifica il costrutte del modulo SUSI: 13 da normativa NMRA : https://www.nmra.org/sites/default/files/appendix_a2c_s-9.2.2.pdf
#define	SUSI_VER					10		//identifica la versione del protocollo SUSI: 1.0
#define DEFAULT_SLAVE_NUMBER		1		//identifica l'indirizzo dello Slave SUSI: default 1

#define	SYNC_TIME			9				//tempo necessario a sincronizzare Master e Slave: 9ms

typedef enum {
	SUSI_DIR_REV = 0,						// Il master sta procedendo in direzione 'Reverse'
	SUSI_DIR_FWD = 1,						// Il master sta procedendo in direzione 'ForWard'
} SUSI_DIRECTION;

typedef enum {
	SUSI_FN_0_4 = 1,
	SUSI_FN_5_12,
	SUSI_FN_13_20,
	SUSI_FN_21_28,
	SUSI_FN_29_36,
	SUSI_FN_37_44,
	SUSI_FN_45_52,
	SUSI_FN_53_60,
	SUSI_FN_61_68,
} SUSI_FN_GROUP;

typedef enum {
	SUSI_AUX_1_8 = 1,
	SUSI_AUX_9_16,
	SUSI_AUX_17_24,
	SUSI_AUX_25_32,
} SUSI_AUX_GROUP;

typedef enum {
	SUSI_AN_0_7 = 1,
	SUSI_AN_8_15,
	SUSI_AN_16_23,
	SUSI_AN_24_31,
	SUSI_AN_32_39,
	SUSI_AN_40_47,
	SUSI_AN_48_55,
	SUSI_AN_56_63,
} SUSI_AN_GROUP;

typedef struct {
	uint8_t		CLK_pin;			//pin a cui e' collegata la linea "Clock";		DEVE ESSERE DI TIPO INTERRUPT
	uint8_t		DATA_pin;			//pin a cui e' collegata la linea "Data";		puo' essere qualsiasi pin (compresi analogici)
	uint32_t	lastByte_time;		//tempo a cui e' stato letto l'ultimo Byte
	uint32_t	lastbit_time;		//tempo a cui e' stato letto l'ultimo bit
	uint8_t		MessageByte[4];		//Byte di cui e' composto un comando
	uint8_t		bitCounter;			//indica quale bit si deve leggere
	uint8_t		ByteCounter;		//indica quale Byte sta venendo letto
	bool		MessageComplete;	//indica se e' stato ricevuto un messaggio completo
} SUSI_t;

extern SUSI_t SusiData;

class Rcn600 {
	private:
		uint8_t	SlaveNumber;			//identifica il numero dello slave sul Bus SUSI (valori da 1 a 3)
		void Data_ACK(void);			//funzione per esguire l'ACK della linea DATA quando necessario
		bool isCVvalid(uint16_t CV);	//ritorna True se il numero della CV passato e' valido per questo modulo Slave

	public:
		Rcn600(uint8_t CLK_pin_i, uint8_t DATA_pin_i);	//Creazione dell'oggetto Rcn600
		void init(void);								//Inizializzazione della libreria: collegamento Interrupt, reset Contatori
		void process(void);								//funzione che decodifica i Byte ricevuti, DEVE ESSERE RICHIAMATA DAL CODICE PIU' VOLTE POSSIBILE
};

// Funzioni Esterne, implementabili a discrizione dell'utente
#if defined (__cplusplus)
extern "C" {
#endif
#ifdef NOTIFY_RAW_MESSAGE
	/*
	*	notifySusiRawMessage() viene invocato ogni volta che è presente un messaggio da decodificare
	*	Input:
	*		- i byte che compongono il messaggio acquisito dalla libreria
	*		- la lunghezza del messaggio
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiRawMessage(uint8_t *rawMessage, uint8_t messageLength) __attribute__((weak));
#endif
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
	/* RESET CVs, Utilizzato stesso metodo della Libreria NmraDcc
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
	extern void    notifyCVResetFactoryDefault(void) __attribute__((weak));

#if defined (__cplusplus)
}
#endif

#endif
