#ifndef RCN_600_h
#define RCN_600_h

/* LIB_VERSION: 1.5.6 */

#include "Arduino.h"                                                                                                        // Libreria per le funzioni tipiche dell'Arduino IDE
#include <stdint.h>                                                                                                         // Libreria per i tipi 'uintX_t'

#ifdef __AVR__                                                                                                              // Se la piattaforma e' AVR puo' essere usata la libreria digitalPinFast; e' escludibile per risparmiare SRAM
    #include    <util/delay.h>                                                                                              // Libreria per la gestione del Delay tramite libreria AVR

    #define     DIGITAL_PIN_FAST                                                                                            // Libreria per la gestione 'fast' dei pin digitali, ESCLUDIBILE per risparmiare SRAM
#endif // __AVR__

#ifdef DIGITAL_PIN_FAST
    #include <digitalPinFast.h>                                                                                             // Libreria per utilizzare i metodi 'Fast' sui pin Digital
#endif

// Comandi Rapidi sul Pin Data
#ifdef DIGITAL_PIN_FAST
    #define READ_DATA_PIN   _DATA_pin->digitalReadFast()                                                                    // Legge il livello logico del Pin utilizzando la libreria Fast
    #define DATA_PIN_INPUT  _DATA_pin->pinModeFast(INPUT)                                                                   // Imposta ilPin come 'INPUT' utilizzando la libreria Fast
    #define DATA_PIN_OUTPUT _DATA_pin->pinModeFast(OUTPUT)
    #define DATA_PIN_HIGH   _DATA_pin->digitalWriteFast(HIGH)
    #define DATA_PIN_LOW    _DATA_pin->digitalWriteFast(LOW)
    #define DATA_PIN_DELETE delete _DATA_pin
    #define DATA_ACK        DATA_PIN_OUTPUT;    DATA_PIN_LOW;   _delay_us(1500);            DATA_PIN_HIGH;  DATA_PIN_INPUT  // Macro per esguire l'ACK della linea DATA quando necessario
#else
    #define READ_DATA_PIN   digitalRead(_DATA_pin)                                                                          // Legge il livello logico del Pin utilizzando le funzioni native di Arduino
    #define DATA_PIN_INPUT  pinMode(_DATA_pin, INPUT)                                                                       // Imposta ilPin come 'INPUT' utilizzando le funzioni native di Arduino
    #define DATA_PIN_OUTPUT pinMode(_DATA_pin, OUTPUT)
    #define DATA_PIN_HIGH   digitalWrite(_DATA_pin, HIGH)
    #define DATA_PIN_LOW    digitalWrite(_DATA_pin, LOW)
    #define DATA_PIN_DELETE DATA_PIN_INPUT
#ifdef __AVR__
    #define DATA_ACK        DATA_PIN_OUTPUT;    DATA_PIN_LOW;   _delay_us(1500);            DATA_PIN_HIGH;  DATA_PIN_INPUT  // Macro per esguire l'ACK della linea DATA quando necessario
#else
    #define DATA_ACK        DATA_PIN_OUTPUT;    DATA_PIN_LOW;   delayMicroseconds(1500);    DATA_PIN_HIGH;  DATA_PIN_INPUT  // Macro per esguire l'ACK della linea DATA quando necessario
#endif // __AVR__

#endif

#include "DataHeaders/SUSI_DATA_TYPE.h"                                                                                     // Tipi Simbolici per le Funzioni CallBack
#include "DataHeaders/SUSI_FN_BIT.h"                                                                                        // bit per il comando delle Funzioni Digitali
#include "DataHeaders/SUSI_AN_FN_BIT.h"                                                                                     // bit per il comando delle Funzioni Analogiche
#include "DataHeaders/SUSI_AUX_BIT.h"                                                                                       // bit per il comando delle AUX

#define	EXTERNAL_CLOCK              255                                                                                     // indica che il Clock e' acquisito tramite sistema Esterno alla libreria

/* Gestione CVs */
#define	ADDRESS_CV                  897                                                                                     // identifica la CV in cui e' contentuto l'indirizzo del Modulo Slave
#define	FIRST_CV                    ADDRESS_CV                                                                              // identifica la prima CV dei moduli SUSI -> da 897 a 1023
#define	MANUFACTER_ID               13                                                                                      // identifica il costrutte del modulo SUSI: 13 da normativa NMRA : https://www.nmra.org/sites/default/files/appendix_a2c_s-9.2.2.pdf
#define	SUSI_VER                    10                                                                                      // identifica la versione del protocollo SUSI: 1.0

/* Indirizzi Moduli Slave */
#define DEFAULT_SLAVE_NUMBER        1                                                                                       // identifica l'indirizzo dello Slave SUSI: default 1
#define MAX_ADDRESS_VALUE           3                                                                                       // Numero massimo di Moduli SUSI collegabili al Decoder: 3

/* Timing Protocollo */
#define MAX_MESSAGES_DELAY          7                                                                                       // tempo Massimo che può trascorrere da due Byte di un messaggio
#define	SYNC_TIME                   9                                                                                       // tempo necessario a sincronizzare Master e Slave: 9ms
#define MIN_CLOCK_TIME              20                                                                                      // minima durata di un Clock: 10 + 10 uS
#define MAX_CLOCK_TIME              500                                                                                     // massima durata di un Clock : livello alto + livello basso

/* Buffer Acquisizione */
#define SUSI_BUFFER_LENGTH          5                                                                                       // lunghezza buffer dove sono contenuti i messaggi
#define FREE_MESSAGE_SLOT           (Rcn600Message*) this                                                                   // valore simbolico per contrassegnare gli slot del buffer come liberi

typedef struct messageRcn600 {
    uint8_t Byte[3];

    struct messageRcn600* nextMessage;
} Rcn600Message;

class Rcn600 {
    private:
        uint8_t	_slaveAddress;                                                                                              // identifica il numero dello slave sul Bus SUSI (valori da 1 a 3)
        uint8_t	_CLK_pin;                                                                                                   // pin a cui e' collegata la linea "Clock", se il clock e' gestito dalla libreria DEVE ESSERE DI TIPO INTERRUPT 

        Rcn600Message _Buffer[SUSI_BUFFER_LENGTH];                                                                          // Buffer contenenti i comandi SUSI ricevuti
        Rcn600Message* _BufferPointer;                                                                                      // Puntatore per scorrere il Buffer

#ifdef	DIGITAL_PIN_FAST
        digitalPinFast* _DATA_pin;                                                                                          // Oggetto che contiene i dati del pin a cui e' collegata la linea Data
#else
        uint8_t _DATA_pin;                                                                                                  // pin a cui e' collegata la linea "Data", puo' essere un pin qualsiasi (Compresi gli analogici)
#endif

    private:
        /*
        *   initClass() Inizializza la Classe e imposta a 'INPUT' i pin a cui e' connesso il Bus Rcn600
        *   Input:
        *       - Nulla
        *   Restituisce:
        *       - Nulla
        */
        void initClass(void);
        /*
        *   searchFreeMessage() Cerca nel Buffer uno slot dove salvare il messaggio in ricezione
        *   Input:
        *       - Nulla
        *   Restituisce:
        *       - Il Puntatore allo Slot libero nel Buffer (NULL se Nessuno Slot libero)
        */
        Rcn600Message* searchFreeMessage(void);
        /*
        *   setNextMessage() Inserisce nel buffer un messaggio ricevuto Completo dall'ISR
        *   Input:
        *       - Il puntatore al Messaggio salvato nel Buffer
        *   Restituisce:
        *       - Nulla
        */
        void setNextMessage(Rcn600Message* nextMessage);
        /*
        *   setNextMessage() Elabora Immediatamente un Messaggio che richiede un'interazione con le CVs
        *   Input:
        *       - Il puntatore al Messaggio da elaborare
        *   Restituisce:
        *       - Nulla
        */
        void processCVsMessage(Rcn600Message CvMessage);

    public:
        /*
        *   ~Rcn600() Creazione della Classe
        *   Input:
        *       - il Pin a cui e' connesso la linea 'Clock'
        *       - il Pin a cui e' connessa la linea 'Data'
        *   Restituisce:
        *       - Nulla
        */
        Rcn600(uint8_t CLK_pin, uint8_t DATA_pin);
        /*
        *   ~Rcn600() Distruttore Classe
        *   Input:
        *       - Nulla
        *   Restituisce:
        *       - Nulla
        */
        ~Rcn600(void);

        /*
        *   init() Inizializza la libreria, utilizzando i metodi notifySusiCVread
        *   Input:
        *       - Nulla
        *   Restituisce:
        *       - Nulla
        */
        void init(void);
        /*
        *   init() Inizializza la libreria passando l'indirizzo dello Slave
        *   Input:
        *       - indirizzo dello Slav : 1, 2, 3
        *   Restituisce:
        *       - Nulla
        */
        void init(uint8_t SlaveAddress);
        /*
        *   process() Va invocato il piu'possibile: decodifica i messaggi grezzi acquisiti
        *   Input:
        *       - Nulla
        *   Restituisce:
        *       - -1	MESSAGGIO NON VALIDO
        *       -  0	Nessun Messaggio in Coda di Decodifica
        *       -  1	MESSAGGIO VALIDO 
        */
        int8_t process(void);
        /*
        *   ISR_SUSI() Va invocato sul fronte di discesa del Clock, SOLO PER CLOCK ESTERNI
        *   Input:
        *       - Nulla
        *   Restituisce:
        *       - Nulla
        */
        void ISR_SUSI(void);
};

#if defined (__cplusplus)
extern "C" {                                                                                                                // Funzioni Esterne, implementabili a discrizione dell'utente
#endif
        /*
        *   notifySusiRawMessage() viene invocato ogni volta che è presente un messaggio (2 Byte) da decodificare. NON viene invocato per i Messaggi di Manipolazione CVs. Mostra il messaggio Grezzo: NON DECODIFICATO.
        *   Input:
        *       - il Primo Byte del Messaggio
        *       - il Secondo Byte del Messaggio
        *   Restituisce:
        *       - Nulla
        */
        extern	void notifySusiRawMessage(uint8_t firstByte, uint8_t secondByte) __attribute__((weak));
        /*
        *   notifySusiFunc() viene invocato quando: si ricevono i dati dal Master su un gruppo di funzioni digitali
        *   Input:
        *       - il gruppo Funzioni decodificato
        *       - lo stato del gruppo funzioni
        *   Restituisce:
        *       - Nulla
        */
        extern	void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState) __attribute__((weak));
        /*
        *   notifySusiBinaryState() viene invocato quando: si ricevono i dati dal Master sullo stato di UNA specifica funzione
        *   Input:
        *       - il numero della funzione (da 1 a 127)
        *       - lo stato della Funzione (attiva = 1, disattiva = 0)
        *   Restituisce:
        *       - Nulla
        */
        extern  void notifySusiBinaryState(uint16_t Command, uint8_t CommandState) __attribute__((weak));
        /*
        *   notifySusiAux() viene invocato quando: si ricevono i dati dal Master sullo stato di UNA specifica AUX
        *   Input:
        *       - il numero dell'AUX
        *       - lo stato dell'uscita (attiva = 1, disattiva = 0)
        *   Restituisce:
        *       - Nulla
        */
        extern  void notifySusiAux(SUSI_AUX_GROUP SUSI_auxGrp, uint8_t SUSI_AuxState) __attribute__((weak));
        /*
        *   notifySusiTriggerPulse() viene invocato quando: si riceve dal Master il comando di Trigger (o pulsazione) per eventuali sbuffi di vapore
        *   Input:
        *       - stato del comando Trigger/Pulse
        *   Restituisce:
        *       - Nulla
        */
        extern  void notifySusiTriggerPulse(uint8_t state) __attribute__((weak));
        /*
        *   notifySusiMotorCurrent() viene invocato quando: si riceve dal Master i dati sull'assorbimento di Corrente da parte del Motore
        *   Input:
        *       - Assorbimento di Corrente: da -128 a + 127 (gia' convertita dal Complemento a 2 originale)
        *   Restituisce:
        *       - Nulla
        */
        extern  void notifySusiMotorCurrent(int8_t current) __attribute__((weak));
        /*
        *   notifySusiRequestSpeed() viene invocato quando: si ricevono i dati sulla Velocita' e sulla Direzione richiesti dalla Centrale al Master
        *   Input:
        *       - la velocita' (128 step) richiesta
        *       - la direzione richiesta
        *   Restituisce:
        *       - Nulla
        */
        extern  void notifySusiRequestSpeed(uint8_t Speed, SUSI_DIRECTION Dir) __attribute__ ((weak));
        /*
        *   notifySusiRealSpeed() viene invocato quando: si ricevono i dati dal Master sulla Velocita' e sulla Direzione reali 
        *   Input:
        *       - la velocita' (128 step) reale
        *       - la direzione reale
        *   Restituisce:
        *       - Nulla
        */
        extern  void notifySusiRealSpeed(uint8_t Speed, SUSI_DIRECTION Dir) __attribute__ ((weak));
        /*
        *   notifySusiMotorLoad() viene invocato quando: si riceve dal Master i dati sul carico del Motore
        *   Input:
        *       - Carico del Motore: da -128 a + 127 (gia' convertita dal Complemento a 2 originale)
        *   Restituisce:
        *       - Nulla
        */
        extern	void notifySusiMotorLoad(int8_t load) __attribute__((weak));
        /*
        *   notifySusiAnalogFunction() viene invocato quando: si ricevono i dati dal Master su un gruppo di funzioni analogiche
        *   Input:
        *       - il gruppo Analogico decodificato
        *       - lo stato del gruppo
        *   Restituisce:
        *       - Nulla
        */
        extern  void notifySusiAnalogFunction(SUSI_AN_GROUP SUSI_AnalogGrp, uint8_t SUSI_AnalogState) __attribute__((weak));
        /*
        *   notifySusiAnalogDirectCommand() viene invocato quando: si ricevono i dati dal Master i comandi diretti per il funzionamento analogico
        *   Input:
        *       - il numero del comando: 1 o 2
        *       - i bit del comando
        *   Restituisce:
        *       - Nulla
        */
        extern  void notifySusiAnalogDirectCommand(uint8_t commandNumber, uint8_t Command) __attribute__((weak));
        /*
        *   notifySusiNoOperation() viene invocato quando: si riceve il comando "no operation", serve prevalentemente a scopo di test
        *   Input:
        *       - l'argomento del comando
        *   Restituisce:
        *       - Nulla
        */
        extern  void notifySusiNoOperation(uint8_t commandArgument) __attribute__((weak));
        /*
        *   notifySusiMasterAddress() viene invocato quando: si riceve l'indirizzo digitale del Master
        *   Input:
        *       - l'indirizzo Digitale del Master
        *   Restituisce:
        *       - Nulla
        */
        extern	void notifySusiMasterAddress(uint16_t MasterAddress) __attribute__((weak));
        /*
        *   notifySusiControlModule() viene invocato quando: si riceve il comando sul controllo del modulo
        *   Input:
        *       - byte contenete il controllo del modulo
        *   Restituisce:
        *       - Nulla
        */
        extern	void notifySusiControllModule(uint8_t ModuleControll) __attribute__((weak));

    

        /* METODI MANIPOLAZIONE CVs */

        /*
        *   notifySusiCVRead() viene invocato quando: e' richiesta la lettura di una CV
        *   Input:
        *       - il numero della CV da leggere
        *   Restituisce:
        *       - ritorna il valore della CV letta
        */
        extern uint8_t notifySusiCVRead(uint16_t CV) __attribute__((weak));
        /*
        *   notifySusiCVWrite() viene invocato quando: e' richiesta la Scrittura di una CV.
        *   Input:
        *       - il numero della CV richiesta
        *       - il Nuovo valore della CV
        *   Restituisce:
        *       - il valore letto (post scrittura) nella posizione richiesta
        */
        extern uint8_t notifySusiCVWrite(uint16_t CV, uint8_t Value) __attribute__((weak));
        /* RESET CVs, viene utilizzato lo stesso metodo della Libreria NmraDcc:
        * 
        *  notifyCVResetFactoryDefault() Called when CVs must be reset. This is called when CVs must be reset to their factory defaults.
        *   Inputs:
        *       - None                                                                                          
        *   Returns:
        *       - None
        */
        extern void notifyCVResetFactoryDefault(void) __attribute__((weak));
#if defined (__cplusplus)
}
#endif

#endif
