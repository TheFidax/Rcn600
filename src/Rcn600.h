#ifndef RCN_600_h
#define RCN_600_h

#include <stdint.h>
#include <stdbool.h>
#include "Arduino.h"

//#define	DEBUG_RCN_LIBRARY		//attiva la modalita' di debug

#define	MANUFACTER_ID		13		//identifica il costrutte del modulo SUSI: 13 da normativa NMRA : https://www.nmra.org/sites/default/files/appendix_a2c_s-9.2.2.pdf
#define	LIB_VER				1		//identifica la versione della libreria

#define	SYNC_TIME			9		//tempo necessario a sincronizzare Master e Slave: 9ms

typedef enum {
	SUSI_DIR_REV = 0,				// Il master sta procedendo in direzione 'Reverse'
	SUSI_DIR_FWD = 1,				// Il master sta procedendo in direzione 'ForWard'
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

#define SUSI_FN_BIT_00	0x10
#define SUSI_FN_BIT_01	0x01
#define SUSI_FN_BIT_02	0x02
#define SUSI_FN_BIT_03	0x04
#define SUSI_FN_BIT_04	0x08

#define SUSI_FN_BIT_05	0x01
#define SUSI_FN_BIT_06	0x02
#define SUSI_FN_BIT_07	0x04
#define SUSI_FN_BIT_08	0x08
#define SUSI_FN_BIT_09	0x10
#define SUSI_FN_BIT_10	0x20
#define SUSI_FN_BIT_11	0x40
#define SUSI_FN_BIT_12	0x80

#define SUSI_FN_BIT_13	0x01
#define SUSI_FN_BIT_14	0x02
#define SUSI_FN_BIT_15	0x04
#define SUSI_FN_BIT_16	0x08
#define SUSI_FN_BIT_17	0x10
#define SUSI_FN_BIT_18	0x20
#define SUSI_FN_BIT_19	0x40
#define SUSI_FN_BIT_20	0x80

#define SUSI_FN_BIT_21	0x01
#define SUSI_FN_BIT_22	0x02
#define SUSI_FN_BIT_23	0x04
#define SUSI_FN_BIT_24	0x08
#define SUSI_FN_BIT_25	0x10
#define SUSI_FN_BIT_26	0x20
#define SUSI_FN_BIT_27	0x40
#define SUSI_FN_BIT_28	0x80

#define SUSI_FN_BIT_29	0x01
#define SUSI_FN_BIT_30	0x02
#define SUSI_FN_BIT_31	0x04
#define SUSI_FN_BIT_32	0x08
#define SUSI_FN_BIT_33	0x10
#define SUSI_FN_BIT_34	0x20
#define SUSI_FN_BIT_35	0x40
#define SUSI_FN_BIT_36	0x80

#define SUSI_FN_BIT_37	0x01
#define SUSI_FN_BIT_38	0x02
#define SUSI_FN_BIT_39	0x04
#define SUSI_FN_BIT_40	0x08
#define SUSI_FN_BIT_41	0x10
#define SUSI_FN_BIT_42	0x20
#define SUSI_FN_BIT_43	0x40
#define SUSI_FN_BIT_44	0x80

#define SUSI_FN_BIT_45	0x01
#define SUSI_FN_BIT_46	0x02
#define SUSI_FN_BIT_47	0x04
#define SUSI_FN_BIT_48	0x08
#define SUSI_FN_BIT_49	0x10
#define SUSI_FN_BIT_50	0x20
#define SUSI_FN_BIT_51	0x40
#define SUSI_FN_BIT_52	0x80

#define SUSI_FN_BIT_53	0x01
#define SUSI_FN_BIT_54	0x02
#define SUSI_FN_BIT_55	0x04
#define SUSI_FN_BIT_56	0x08
#define SUSI_FN_BIT_57	0x10
#define SUSI_FN_BIT_58	0x20
#define SUSI_FN_BIT_59	0x40
#define SUSI_FN_BIT_60	0x80

#define SUSI_FN_BIT_61	0x01
#define SUSI_FN_BIT_62	0x02
#define SUSI_FN_BIT_63	0x04
#define SUSI_FN_BIT_64	0x08
#define SUSI_FN_BIT_65	0x10
#define SUSI_FN_BIT_66	0x20
#define SUSI_FN_BIT_67	0x40
#define SUSI_FN_BIT_68	0x80

typedef struct {
	uint8_t		CLK_pin;			//pin a cui è collegata la linea "Clock";		DEVE ESSERE DI TIPO INTERRUPT
	uint8_t		DATA_pin;			//pin a cui è collegata la linea "Data";		puo' essere qualsiasi pin (compresi analogici)
	uint32_t	lastByte_time;		//tempo a cui è stato letto l'ultimo Byte
	uint32_t	lastbit_time;		//tempo a cui è stato letto l'ultimo bit
	uint8_t		MessageByte[3];		//Byte di cui è composto un comando
	uint8_t		bitCounter;			//Indica quale bit si deve leggere
	uint8_t		ByteCounter;		//Indica quale Byte sta venendo letto
	bool		MessageComplete;	//indica se è stato ricevuto un messaggio completo
} SUSI_t;

extern SUSI_t SusiData;

class Rcn600 {
	private:
		uint8_t	SlaveNumber;			//Identifica il numero dello slave sul Bus SUSI (valori da 1 a 3)
		void Data_ACK(void);			//funzione per esguire l'ACK della linea DATA quando necessario
		bool isCVvalid(uint16_t CV);	//ritorna True se il numero della CV passato e' valido per questo modulo Slave

	public:
		Rcn600(uint8_t CLK_pin_i, uint8_t DATA_pin_i);	//Creazione dell'oggetto Rcn600
		void init(void);								//Inizializzazione della libreria: collegamento Interrupt, reset Contatori
		void process(void);								//funzione che decodifica i Byte ricevuti, DEVE ESSERE RICHIAMATA DAL CODICE PIU' VOLTE POSSIBILE
};

// Funzioni Esterne, implementabili a descrizione dell'utente
#if defined (__cplusplus)
extern "C" {
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
	*	notifySusiTriggerPulse() viene invocato quando: si riceve dal Master il comando di Trigger (o pulsazione) per eventuali sbuffi di vapore
	*	Input:
	*		- stato del comando Trigger/Pulse
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiTriggerPulse(uint8_t state) __attribute__((weak));
	/*
	*	notifySusiSpeedDeviation() viene invocato quando: si riceve dal Master i dati sulla differenza fra la velocita' reale e la velocita' richiesta dalla centrale
	*	Input:
	*		- Deviazione della velocita': da -128 a + 127 (gia' convertita dal Complemento a 2 originale)
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiSpeedDeviation(int deviation) __attribute__((weak));
	/*
	*	notifySusiCurrentAbsorption() viene invocato quando: si riceve dal Master i dati sull'assorbimento di Corrente da parte del Motore
	*	Input:
	*		- Assorbimento di Corrente: da -128 a + 127 (gia' convertita dal Complemento a 2 originale)
	*	Restituisce:
	*		- Nulla
	*/
	extern	void notifySusiCurrentAbsorption(int absorption) __attribute__((weak));
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
	
	
	/* Metodi per la manipolazione delle CVs */
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

#if defined (__cplusplus)
}
#endif

#endif
