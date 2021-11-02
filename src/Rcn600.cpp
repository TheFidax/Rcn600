/* LIB_VERSION: 1.5.2 */

#include "Rcn600.h"

Rcn600* pointerToRcn600;																			// Puntatore alle Classe Rcn600

static void Rcn600InterruptHandler(void) {															// Handle per l'ISR del Clock
	pointerToRcn600->ISR_SUSI();																	// Chiamata all'ISR della Classe
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Rcn600::Rcn600(uint8_t CLK_pin, uint8_t DATA_pin) {
	_CLK_pin = CLK_pin;																				// Memorizzo il numero del pin di Clock sia per gestire l'Interrupt oppure per verificare se il clock e' esterno

#ifdef DIGITAL_PIN_FAST
	_DATA_pin = new digitalPinFast(DATA_pin);														// Se e' attiva la libreria 'Fast' ne creo la Classe
#else
	_DATA_pin = DATA_pin;																			// In caso contrario memorizzo il numero del pin per la gestione 'standard' dei pin digitali
#endif // DIGITAL_PIN_FAST	
}

Rcn600::~Rcn600(void) {
	if (_CLK_pin != EXTERNAL_CLOCK) {																// Se il Pin Clock solo e' Gestito dalla Libreria
		detachInterrupt(digitalPinToInterrupt(_CLK_pin));											// Disattivo la gestione dell'Interrupt
		pinMode(_CLK_pin, INPUT);																	// Imposto lo stato del pin come INPUT
	}

	DATA_PIN_DELETE;																				//	Metto il pin Data ad INPUT (e se occore elimino la Classe che lo gestiva)
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Rcn600::initClass(void) {
	pointerToRcn600 = this;																			// Assegno al puntatore l'indirizzo della Seguente Classe

	if (_CLK_pin != EXTERNAL_CLOCK) {																// Controllo se e' presente il numero di un pin
		pinMode(_CLK_pin, INPUT);																	// il pin di Clock deve essere ad alta impedenza: INTERRUPT
		attachInterrupt(digitalPinToInterrupt(_CLK_pin), Rcn600InterruptHandler, FALLING);			//da normativa i dati fanno letti sul "fronte di discesa" del Clock
	}

	DATA_PIN_INPUT;																					// Pin Data ad alta impedenza: Interrupr

	for (uint8_t i = 0; i < SUSI_BUFFER_LENGTH; ++i) {												// Imposto gli slot del Buffer come liberi
		_Buffer[i].nextMessage = FREE_MESSAGE_SLOT;													// Assegno al 'messaggio successivo' un indicatore simbolico
	}

	_BufferPointer = NULL;																			// Imposto il puntatore per indicare l'assenza di messaggi da decodificare
}

void Rcn600::init(void) {
	if (notifySusiCVRead) {																			// Se e' presente il sistema di memorizzazione CV
		_slaveAddress = notifySusiCVRead(ADDRESS_CV);												// Leggo il valore memorizzato nella CV dell'indirizzo

		if (_slaveAddress > MAX_ADDRESS_VALUE) {													// Se l'indirizzo e' maggiore di quelli consentiti
			if (notifySusiCVWrite) {																// Controllo se e' possibile aggiornare il valore con uno corretto
				notifySusiCVWrite(ADDRESS_CV, DEFAULT_SLAVE_NUMBER);								// Scrivo l'indirizzo di Default
			}
			_slaveAddress = DEFAULT_SLAVE_NUMBER;													// Utilizzo l'indirizzo di Default: 1
		}
	}	
	else {																							// Se NON E' presente un sistema di memorizzazione CVs
		_slaveAddress = DEFAULT_SLAVE_NUMBER;														// Utilizzo l'indirizzo di Default: 1
	}
	
	initClass();																					// Inizializzo la Classe ed i suoi componenti
}

void Rcn600::init(uint8_t SlaveAddress) {															// Inizializzazione con indirizzo scelto dall'utente nel codice
	_slaveAddress = SlaveAddress;																	// Salvo l'indirizzo 

	if (_slaveAddress > MAX_ADDRESS_VALUE) {														// Se l'indirizzo e' maggiore di quelli consentiti
		_slaveAddress = DEFAULT_SLAVE_NUMBER;														// Utilizzo l'indirizzo di Default: 1
	}

	initClass();																					// Inizializzo la Classe ed i suoi componenti
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Rcn600Message* Rcn600::searchFreeMessage(void) {
	for (uint8_t i = 0; i < SUSI_BUFFER_LENGTH; ++i) {
		if (_Buffer[i].nextMessage == FREE_MESSAGE_SLOT) {
			return &_Buffer[i];
		}
	}
	
	/* Se non trovo uno slot libero ritorno NULL */
	return NULL;
}

void Rcn600::setNextMessage(Rcn600Message* nextMessage) {
	Rcn600Message* p = _BufferPointer;

	if (p != NULL) {	// Ce'e' Almeno un messaggio in coda 
		while (p->nextMessage != NULL) {
			p = p->nextMessage;
		}

		p->nextMessage = nextMessage;
	}
	else {				// Ancora nessun messaggio pronto per la decodifica
		_BufferPointer = nextMessage;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Rcn600::ISR_SUSI(void) {
	// Variabili 'statiche' per i confronti e/o immagazzinare informazioni
	static uint32_t lastByte_time = millis();														// tempo a cui e' stato letto l'ultimo Byte
	static uint32_t lastbit_time = (micros() - MIN_CLOCK_TIME);										// tempo a cui e' stato letto l'ultimo bit
	static uint8_t	bitCounter = 0;																	// indica quale bit si deve leggere
	static Rcn600Message* messageSlot;																// indica in quale slot sta venendo salvato il messaggio in ricezione

	// Variaibli 'dinamiche' allocate ad ogni iterazione
	uint32_t microsActualISR = micros();															// indica a che 'microsecondi' sta avvenendo l'attuale ISR
	uint32_t millisActualISR = millis();															// indica a che 'millisecondi' sta avvenendo l'attuale ISR
	uint8_t	millisDelayFromLastByte = (millisActualISR - lastByte_time);
	uint16_t microsDelayFromLastBit = (microsActualISR - lastbit_time);

	if (bitCounter == 0) {																			// Se NON E' disponibile uno slot dove salvare i dati acquisiti
		messageSlot = searchFreeMessage();															// ne cerco uno libero
		
		if (messageSlot == NULL) {	return;	}														// Nessuno slot disponibile -> Non acquisisco nulla
		else {																						// Slot libero trovato -> Acquisisco il primo bit
			messageSlot->nextMessage = NULL;														// Slot libero trovato -> Imposto lo Slot come 'in utilizzo'
			if (microsDelayFromLastBit < MIN_CLOCK_TIME) {}											// Passati MENO di 20uS -> Errore
			else if (microsDelayFromLastBit > MAX_CLOCK_TIME) {}									// Passati PIU' di 500uS -> Errore
			else {																					// Timing Corretto
				messageSlot->Byte[0] = READ_DATA_PIN;												// Salvo il bit nella posizione 0
				++bitCounter;																		// Incremento il contatore dei bit letti
				lastbit_time = microsActualISR;														// Memorizzo in che 'microsecondo' e' stato letto l'ultimo bit
			}
		}																			
	}
	if (millisDelayFromLastByte < MAX_MESSAGES_DELAY) {												// Dall'ultimo Byte sono passato meno di 7ms -> Timing 'millis' valido
		if (microsDelayFromLastBit < MIN_CLOCK_TIME) {}												// Passati MENO di 20uS -> Errore
		else if(microsDelayFromLastBit > MAX_CLOCK_TIME) {}											// Passati PIU' di 500uS -> Errore
		else {																						// Timing Corretto
			bitWrite(messageSlot->Byte[bitCounter / 8], (bitCounter % 8), READ_DATA_PIN);			// Salvo il bit letto nello slot libero
			++bitCounter;																			// Incremento il contatore dei bit letti
			lastbit_time = microsActualISR;															// Memorizzo in che 'microsecondo' e' stato letto l'ultimo bit

			if ((bitCounter % 8) == 0) {															// Controllo se sono stati letti 8 o multipli di 8 bit
				if (bitCounter == 16) {																// Ho letto 2 Byte completi, lunghezza dei comandi 'normali' (NO CV)
					if (messageSlot->Byte[0] < 118) {												// Controllo che il comando NON sia per le CV -> i comandi CV sono maggiori di 118 (119, 123, 127)
						setNextMessage(messageSlot);												// Se e' un messaggio normale lo inserisco nella coda di quelli da decodificare

						bitCounter = 0;																// Azzero il contatore dei bit per leggere un nuovo messaggio
					}
					//else {}																		// Comando per manipolare le CVs, NON faccio niente
				}
				else if (bitCounter == 24) {														// Ho letto 3 Byte -> Comando manipolazione CVs
					processCVsMessage(*messageSlot);												// Processo IMMEDIATAMENTE il messaggio ricevuto

					messageSlot->nextMessage = FREE_MESSAGE_SLOT;									// Libero lo Slot per poterlo usare in futuro

					bitCounter = 0;																	// Azzero il contatore dei bit per leggere un nuovo messaggio
				}

				lastByte_time = millisActualISR;													// Memorizzo in che 'millis' e' stata completata la lettura del Byte
			}
		}
	}
	else if (millisDelayFromLastByte > SYNC_TIME) {													// Sono passati piu' di 7ms -> Controllo se e' avvenuta la 'sincronizzazione': eseguo un reset dei contatori per acquisisire un messaggio da 0
		bitCounter = 0;																				// dopo il SYNC leggero' il primo bit
		lastByte_time = millisActualISR;															// imposto questo istante come ultimo Byte letto
 
		messageSlot->Byte[0] = READ_DATA_PIN;														// Sto leggendo il primo bit del messaggio

		++bitCounter;																				// Ho letto il bit 0, il prossimo da leggere e' il bit 1
		lastbit_time = microsActualISR;																// memorizzo l'istante in cui e' stato letto il bit
	}
	// else {}																						// trascorsi PIU' di 7ms ma MENO di 9ms -> Errore
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Rcn600::processCVsMessage(Rcn600Message CvMessage) {
	uint8_t CV_Offset = CvMessage.Byte[1] & 0b01111111;												// Elimino il bit piu' significativo (bit7) dal Byte contenente l'Offset delle CVs	
	uint8_t	CV_Value;																				// Tiene traccia del valore della CV

	// Controllo se la CV richiesta e' per questo Slave, in caso negativo non faccio nulla (e ritorno)
	//	Slave 1:	897 + 900 / 939		+ 1020 / 1024
	//	Slave 2:	897 + 940 / 979		+ 1020 / 1024
	//	Slave 3:	897 + 980 / 1019	+ 1020 / 1024
	if ((CV_Offset > 2) && (CV_Offset < 43)) {														// Range 900(=897+3) / 939(=897 + 42) -> Solo Slave 1
		if (_slaveAddress != 1) {																	// Controllo se l'indirizzo dello Slave e' diverso da 1
			return;																					// In caso affermativo esco, questa CV non e' per questo slave
		}
	}
	else if ((CV_Offset > 42) && (CV_Offset < 83)) {												// Range 940(=897+43) / 979(=897 + 82) -> Solo Slave 1
		if (_slaveAddress != 2) {																	// Controllo se l'indirizzo dello Slave e' diverso da 2
			return;																					// In caso affermativo esco, questa CV non e' per questo slave
		}
	}
	else if ((CV_Offset > 82) && (CV_Offset < 123)) {												// Range 980(=897+83) / 1019(=897 + 122) -> Solo Slave 1
		if (_slaveAddress != 3) {																	// Controllo se l'indirizzo dello Slave e' diverso da 3
			return;																					// In caso affermativo esco, questa CV non e' per questo slave
		}
	}
	else if ((CV_Offset == 2) || (CV_Offset == 3)) {												// CV riservate!
		return;																						// Queste CV vanno ignorate
	}
	//else { /*		CV_Offset>= 124(= >= 1020)	||	CV_Offset == 0(=897)	*/	}					// gestite da tutti gli Slave

	switch (CV_Offset) {																			// Devo controllare se la CV richiesta e' di quelle contenenti informazioni quali produttore o versione
		case 0:		CV_Value = _slaveAddress;	break;												// 897 + 0 = 897
		case 3:		CV_Value = MANUFACTER_ID;	break;												// 897 + 3 = 900
		case 4:		CV_Value = SUSI_VER;		break;												// 897 + 4 = 901
		case 43:	CV_Value = MANUFACTER_ID;	break;												// 897 + 3 + 40 = 940
		case 44:	CV_Value = SUSI_VER;		break;												// 897 + 4 + 40 = 941
		case 83:	CV_Value = MANUFACTER_ID;	break;												// 897 + 3 + 80 = 980
		case 84:	CV_Value = SUSI_VER;		break;												// 897 + 4 + 80 = 981
		default: {
			if (notifySusiCVRead) {																	// Se e' presente il sistema di memorizzazione CV, leggo il valore della CV memorizzata
				CV_Value = notifySusiCVRead(FIRST_CV + CV_Offset);
			}
			else {
				CV_Value = 255;
			}
		}
	}

	switch (CvMessage.Byte[0]) {
		case 119: {
			/*	"CV-Manipulation Byte prüfen" : 0111-0111 (0x77 = 119) | 1 V6 V5 V4 - V3 V2 V1 V0 | D7 D6 D5 D4 - D3 D2 D1 D0
			*
			*	DCC-Befehl Byte Prüfen im Service- und Betriebsmodus
			*	V = CV-Nummer 897 .. 1024 (Wert 0 = CV 897, Wert 127 = CV 1024)
			*	D = Vergleichswert zum Prüfen. Wenn D dem gespeicherten CV-Wert
			*	entspricht, antwortet der Slave mit einem Acknowledge.
			*	Dieser und die beiden folgenden Befehle sind die in Abschnitt 4 genannten
			*	3 Byte Pakete entsprechend [RCN-214]
			*
			*	Controllo Byte di comando DCC in modalita' di servizio e di funzionamento
			*	V = numero CV 897 .. 1024 (valore 0 = CV 897, valore 127 = CV 1024)
			*	D = valore di confronto per il controllo. Se D corrisponde al valore CV memorizzato
			*	lo Slave risponde con un riconoscimento.
			*	Questo e i due comandi seguenti sono quelli menzionati nella sezione 4
			*	Pacchetti da 3 byte secondo [RCN-214]
			*/

			/* Confronto fra il valore memorizzato e quello ipotizzato dal master */
			if (CV_Value == CvMessage.Byte[2]) {													// Controllo il valore ipotizzato dal master
				DATA_ACK;																			// se il valore corrisponde, eseguo un ACK
			}

			break;
		}
		case 123: {
			/* "CV-Manipulation Bit manipulieren" : 0111-1011 (0x7B = 123) | 1 V6 V5 V4 - V3 V2 V1 V0 | 1 1 1 K - D B2 B1 B0
			*
			* DCC-Befehl Bit Manipulieren im Service- und Betriebsmodus
			* V = CV-Nummer 897 ... 1024 (Wert 0 = CV 897, Wert 127 = CV 1024)
			* K = 0: Bit Prüfen. Wenn D mit dem Bitzustand an der Bitstelle B der CV übereinstimmt,
			* wird mit einem Acknowledge geantwortet.
			* K = 1: Bit Schreiben. D wird in Bitstelle B der CV geschrieben.
			* Der Slave bestätigt das Schreiben mit einem Acknowledge.
			*
			* Manipolazione dei bit di comando DCC in modalita' di servizio e operativa
			* V = numero CV 897 ... 1024 (valore 0 = CV 897, valore 127 = CV 1024)
			* K = 0: bit di controllo. Se D corrisponde allo stato del bit nella posizione del bit B del CV,
			* viene dato un riconoscimento.
			* K = 1: scrivi bit. D e' scritto nella posizione di bit B del CV.
			* Lo slave conferma la scrittura con un riconoscimento.
			*/
			uint8_t	bitValue = (CvMessage.Byte[2] & 0b00001000);									// leggo il valore del bit da confrontare/scrivere
			uint8_t	bitPosition = (CvMessage.Byte[2] & 0b00000111);									// leggo in quale posizione si trova il bit su cui fare il confronto/scrittura

			//in base all'operazione richiesta eseguiro' un'azione
			if (CvMessage.Byte[2] & 0b00010000) {													// se 1 scrivo
				switch (CV_Offset) {																// controllo su quale CV si vuole agire: se e' uno NON SCRIVIBILE, non faccio nulla
					case 3:		return;																// 897 + 3 = 900
					case 4:		return;																// 897 + 4 = 901
					case 43:	return;																// 897 + 3 + 40 = 940
					case 44:	return;																// 897 + 4 + 40 = 941
					case 83:	return;																// 897 + 3 + 80 = 980
					case 84:	return;																// 897 + 4 + 80 = 981
					default: {																		// CV scrivibile
						if (notifySusiCVWrite) {
							bitWrite(CV_Value, bitPosition, bitRead(CvMessage.Byte[2], 3));			// scrivo il nuovo valore del bit

							if (notifySusiCVWrite(FIRST_CV + CV_Offset, CV_Value) == CV_Value) {	// memorizzo il nuovo valore della CV
								DATA_ACK;															// Eseguo un ACK come conferma dell'avvenuta operazione
							}
						}
						// else {}																	// nel caso in cui non e' implementato un sistema di memorizzazione CVs, non faccio nulla
					}
				}
			}
			else {																					// se 0 leggo
				if (bitRead(CV_Value, bitPosition) == bitValue) {									// confronto il bit richiesto con quello memorizzato 
					DATA_ACK;																		// se corrisponde eseguo un ACK
				}
			}
			break;
		}
		case 127: {
			/* "CV-Manipulation Byte schreiben" : 0111-1111 (0x7F = 127) | 1 V6 V5 V4 - V3 V2 V1 V0 | D7 D6 D5 D4 - D3 D2 D1 D0
			*
			* DCC-Befehl Byte Schreiben im Service- und Betriebsmodus
			* V = CV-Nummer 897 .. 1024 (Wert 0 = CV 897, Wert 127 = CV 1024)
			* D = Wert zum Schreiben in die CV. Der Slave bestätigt das Schreiben mit
			* einem Acknowledge.
			*
			* Scrittura byte di comando DCC in modalita' di servizio e operativa
			* V = numero CV 897 .. 1024 (valore 0 = CV 897, valore 127 = CV 1024)
			* D = valore per la scrittura nel CV. Lo Slave conferma la scrittura con un riconoscimento.
			*/
			/* Devo controllare se la CV richiesta NON e' di quelle contenenti informazioni quali produttore o versione */

			switch (CV_Offset) {																	// controllo su quale CV si vuole agire: se e' uno NON SCRIVIBILE, non faccio nulla

				// CVs che identificano la Versione del bus SUSI: NON SCRIVIBILI
				case 4:		return;																	// 897 + 4 = 901
				case 44:	return;																	// 897 + 4 + 40 = 941
				case 84:	return;																	// 897 + 4 + 80 = 981

				// CVs che identificano il produttore del modulo SUSI: se il valore che si vuole scrivere e' 8 allora si sta facendo un Reset
				case 3:		{}																		// 897 + 3 = 900
				case 43:	{}																		// 897 + 3 + 40 = 940
				case 83:	{																		// 897 + 3 + 80 = 980
					if (CvMessage.Byte[2] == 8) {													// Controllo che il valore che si vuole scrivere sia 8
						if (notifyCVResetFactoryDefault) {											// Se e' presente il sistema di reset delle CVs

							notifyCVResetFactoryDefault();											// Eseguo il reset

							DATA_ACK;																// Riporto un ACK come conferma operazione
						}
					}
					//else {}																		// In caso contrario, non faccio nulla -> ERRORE
					break;
				}
				default: {																			// CV scrivibile
					if (notifySusiCVWrite) {														// Se e' presente il sistema memorizzazione CVs la scrivo
						if (notifySusiCVWrite(FIRST_CV + CV_Offset, CvMessage.Byte[2]) == CvMessage.Byte[2]) {
							DATA_ACK;																// Ad operazione eseguita confermo con un ACK
						}
					}
					//else {}																		//nel caso in cui non e' implementato un sistema di memorizzazione CVs, non faccio nulla
				}
			}

			if (CV_Offset == 0) {																	// Se e' stato cambiato l'indirizzo dello Slave
				if (CvMessage.Byte[2] > MAX_ADDRESS_VALUE) {										// Controllo se il nuovo valore e' consono
					notifySusiCVWrite(ADDRESS_CV, DEFAULT_SLAVE_NUMBER);							// Riporto il valore dell'indirizzo al valore di default
					_slaveAddress = DEFAULT_SLAVE_NUMBER;											// Aggiorno il valore memorizzato dalla Libreria
				}
				else {																				// Se il valore e' consono
					_slaveAddress = CvMessage.Byte[2];												// Aggiorno il valore memorizzato dalla Libreria
				}
			}
			break;
		}
		default: {}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Il seguente metodo e' stato copiato da qua: https://stackoverflow.com/questions/48567214/how-to-convert-a-byte-in-twos-complement-form-to-its-integer-value-c-sharp */
static int ConvertTwosComplementByteToInteger(byte rawValue) {
	// If a positive value, return it
	if ((rawValue & 0x80) == 0) {
		return rawValue;
	}

	// Otherwise perform the 2's complement math on the value
	return (byte)(~(rawValue - 0x01)) * -1;
}

uint8_t Rcn600::process(void) {
	uint8_t processNextMessage = 1;	// Indica se devo processare piu' messaggi

	while (processNextMessage) {
		processNextMessage = 0;

		if (_BufferPointer != NULL) {		//controllo che siano stati ricevuti dei messaggi

			if (notifySusiRawMessage) {
				notifySusiRawMessage(_BufferPointer->Byte[0], _BufferPointer->Byte[1]);
			}

			/* Devo controllare il valore del primo Byte */
			switch (_BufferPointer->Byte[0]) {
				case 96: {
					/* "Funktionsgruppe 1" : 0110-0000 (0x60 = 96) 0 0 0 F0 - F4 F3 F2 F1 */
					if (notifySusiFunc) {
						notifySusiFunc(SUSI_FN_0_4, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 97: {
					/* "Funktionsgruppe 2" : 0110-0001 (0x61 = 97) F12 F11 F10 F9 - F8 F7 F6 F5 */
					if (notifySusiFunc) {
						notifySusiFunc(SUSI_FN_5_12, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 98: {
					/* "Funktionsgruppe 3" : 0110-0010 (0x62 = 98) F20 F19 F18 F17 - F16 F15 F14 F13 */
					if (notifySusiFunc) {
						notifySusiFunc(SUSI_FN_13_20, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 99: {
					/* "Funktionsgruppe 4" : 0110-0011 (0x63 = 99) F28 F27 F26 F25 - F24 F23 F22 F21 */
					if (notifySusiFunc) {
						notifySusiFunc(SUSI_FN_21_28, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 100: {
					/* "Funktionsgruppe 5" : 0110-0100 (0x64 = 100) F36 F35 F34 F33 - F32 F31 F30 F29 */
					if (notifySusiFunc) {
						notifySusiFunc(SUSI_FN_29_36, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 101: {
					/* "Funktionsgruppe 6" : 0110-0101 (0x65 = 101) F44 F43 F42 F41 - F40 F39 F38 F37 */
					if (notifySusiFunc) {
						notifySusiFunc(SUSI_FN_37_44, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 102: {
					/* "Funktionsgruppe 7" : 0110-0110 (0x66 = 102) F52 F51 F50 F49 - F48 F47 F46 F45 */
					if (notifySusiFunc) {
						notifySusiFunc(SUSI_FN_45_52, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 103: {
					/* "Funktionsgruppe 8" : 0110-0111 (0x67 = 103) F60 F59 F58 F57 - F56 F55 F54 F53 */
					if (notifySusiFunc) {
						notifySusiFunc(SUSI_FN_53_60, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 104: {
					/* "Funktionsgruppe 9" : 0110-1000 (0x68 = 104) F68 F67 F66 F65 - F64 F63 F62 F61 */
					if (notifySusiFunc) {
						notifySusiFunc(SUSI_FN_61_68, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 109: {
					/* "Binärzustände kurze Form" :  0110 - 1101 (0x6D = 109) D L6 L5 L4 - L3 L2 L1 L0
					*
					*	D = 0 bedeutet Funktion L ausgeschaltet, D = 1 eingeschaltet
					*	L = Funktionsnummer 1 ... 127
					*	L = 0 (Broadcast) schaltet alle Funktionen 1 bis 127 aus (D = 0) oder an (D = 1)
					*
					*	D = 0 significa funzione L disattivata, D = 1 attivata
					*	L = numero funzione 1 ... 127
					*	L = 0 (trasmissione) disattiva (D = 0) o attiva tutte le funzioni da 1 a 127 (D = 1)
					*/

					static uint8_t functionNumber, funcState;

					funcState = _BufferPointer->Byte[1] & 0x80;					// leggo il valore dello stato 'D' ()

					functionNumber = _BufferPointer->Byte[1] & 0b01111111;		// elimino il bit piu' significativo (bit7)

					if (notifySusiBinaryState) {
						if (functionNumber == 0) {
							// Comanda tutte le funzioni
							uint8_t i;
							if (funcState == 0) {	// disattivo tutte le funzioni
								for (i = 1; i < 128; ++i) {
									notifySusiBinaryState(i, 0);
								}
							}
							else {				// attivo tutte le funzioni 
								for (i = 1; i < 128; ++i) {
									notifySusiBinaryState(i, 1);
								}
							}
						}
						else {
							// Comanda una singola funzione
							notifySusiBinaryState(functionNumber, funcState);
						}
					}
					break;
				}
				case 110: {	// && 111
					/*	"Binärzustände lange Form low Byte" : 0110-1110 (0x6E = 110) D L6 L5 L4 - L3 L2 L1 L0
					*
					*	Befehl wird immer paarweise vor dem Binärzustand lange Form high Byte gesendet.
					*	Folgen die beiden Befehle nicht direkt aufeinander, so sind sie zu ignorieren.
					*	D = 0 bedeutet Binärzustand L ausgeschaltet, D = 1 "eingeschaltet"
					*	L = niederwertige Bits der Binärzustandsnummer 1 ... 32767
					*
					*	Il comando viene sempre inviato in coppie prima che lo stato binario formi lunghi byte alti.
					*	Se i due comandi non seguono direttamente, devono essere ignorati.
					*	D = 0 significa stato binario L spento, D = 1 "acceso"
					*	L = bit di basso valore dello stato binario numero 1 ... 32767
					*
					*
					*	"Binärzustände lange Form high Byte" : 0110-1111 (0x6F = 111) H7 H6 H5 H4 - H3 H2 H1 H0
					*
					*	Befehl wird immer paarweise nach dem Binärzustand lange Form low
					*	Byte gesendet. Folgen die beiden Befehle nicht direkt aufeinander, so sind sie zu ignorieren.
					*	Erst dieser Befehl führt zur Ausführung des Gesamtbefehls.
					*	H = höherwertigen Bits der Binärzustandsnummer high 1 ... 32767
					*	H und L = 0 (Broadcast) schaltet alle 32767 verfügbaren Binärzustände
					*	aus (D = 0) oder an (D = 1)
					*
					*	Il comando viene sempre inviato in coppie dopo che lo stato binario forma lunghi byte bassi. Se i due comandi non si sus seguono direttamente, devono essere ignorati.
					*	Solo dopo questo comando eseguira' l'intero comando.
					*	H = bit di qualita' superiore dello stato binario numero alto 1 ... 32767
					*/

					Rcn600Message* next = _BufferPointer->nextMessage;
					if (next != NULL) {
						if (next->Byte[0] == 111) {					// Posso eseguire il comando solo se ho ricevuto sia il Byte piu' significativo che quello meno significativo
							if (notifySusiBinaryState) {			// Controllo se e' presente il metodo per gestire il comando
								static uint16_t Command;
								static uint8_t State;

								Command = next->Byte[1];				// memorizzo i bit "piu' significativ"
								Command = Command << 7;					// sposto i bit 7 posti a 'sinistra'
								Command |= _BufferPointer->Byte[1];				// aggiungo i 7 bit "meno significativi"

								State = bitRead(_BufferPointer->Byte[1], 7);

								notifySusiBinaryState(Command, State);
							}
						}
					}
					else {
						// Il messaggio successivo NECESSARIO non e' ancora stato ricevuto, esco dal process senza modifiche
						return 1;
					}
					break;
				}
				case 64: {
					/*	"Direktbefehl 1" (2-Byte): 0100-0000 (0x40 = 64) X8 X7 X6 X5 - X4 X3 X2 X1
					*
					*	Die Direktbefehle dienen zur direkten Ansteuerung von Ausgängen und
					*	anderen Funktionen nach der Anwendung der Funktionstabelle im Master.
					*	Ein Bit = 1 bedeutet der entsprechende Ausgang ist eingeschaltet.
					*
					*	I comandi diretti vengono utilizzati per il controllo diretto delle uscite e
					*	altre funzioni dopo aver utilizzato la tabella delle funzioni nel master.
					*	Un bit = 1 significa che l'uscita corrispondente è attivata.
					*/
					if (notifySusiAux) {
						notifySusiAux(SUSI_AUX_1_8, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 65: {
					/*	"Direktbefehl 2" : 0100-0001 (0x41 = 65) X16 X15 X14 X13 - X12 X11 X10 X9 */
					if (notifySusiAux) {
						notifySusiAux(SUSI_AUX_9_16, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 66: {
					/*	"Direktbefehl 3" : 0100-0010 (0x42 = 66) X24 X23 X22 X21 - X20 X19 X18 X17 */
					if (notifySusiAux) {
						notifySusiAux(SUSI_AUX_17_24, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 67: {
					/*	"Direktbefehl 4" : 0100-0011 (0x43 = 67) X32 X31 X30 X29 - X28 X27 X26 X25 */
					if (notifySusiAux) {
						notifySusiAux(SUSI_AUX_25_32, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 33: {
					/*	"Trigger-Puls" : 0010-0001 (0x21 = 33) 0 0 0 0 - 0 0 0 1
					*
					*	Der Befehl dient zur Synchronisation eines Dampfstoßes. Er wird je
					*	Dampfstoß einmal gesendet. Die Bits 1 bis 7 sind für zukünftige
					*	Anwendungen reserviert.
					*
					*
					*	Il comando viene utilizzato per sincronizzare una scarica di vapore.
					*	Lo fara' mai scoppio di vapore inviato una volta.
					*	I bit da 1 a 7 sono per uso futuro, Applicazioni riservate.
					*/
					if (notifySusiTriggerPulse) {
						notifySusiTriggerPulse(_BufferPointer->Byte[1]);
					}
					break;
				}
				case 35: {
					/*	"Strom" : 0010-0011 (0x23 = 35) S7 S6 S5 S4 - S3 S2 S1 S0
					*
					*	Vom Motor verbrauchter Strom. Der Wert hat einen Bereich von -128 bis
					*	127, wird im 2er-Komplement übertragen und wird über eine
					*	herstellerspezifische CV im Lokdecoder kalibriert. Negative Werte
					*	bedeuten ein Rückspeisen wie es bei modernen E-Loks möglich ist.
					*
					*	Elettricita' consumata dal motore. Il valore ha un intervallo da -128 a
					*	127, viene trasmesso in complemento a 2 e viene trasmesso tramite a
					*	CV specifico del costruttore calibrato nel decoder della locomotiva. Valori negativi
					*	significa un feed back come e' possibile con le moderne locomotive elettriche.
					*/
					if (notifySusiMotorCurrent) {
						notifySusiMotorCurrent(ConvertTwosComplementByteToInteger(_BufferPointer->Byte[1]));
					}
					break;
				}
				case 36: {	/* DEPRECATO DALLA REVISIONE 6 / 12 / 2020 */
					/*	" "Ist" Lok-Fahrstufe " : 0010-0100 (0x24 = 36) R G6 G5 G4 - G3 G2 G1 G0
					*
					*	Die Fahrstufe und Richtung entsprechen dem realen Zustand am Motor.
					*	Der Wert G ist als 0...127 auf die im Modell eingestellte Vmax normiert zu
					*	übertragen. G = 0 bedeutet Lok steht, G = 1 ... 127 ist die normierte
					*	Geschwindigkeit R = Fahrtrichtung mit R = 0 für rückwärts und R = 1
					*	für vorwärts.
					*	Dieser und der folgende Befehl werden nicht für neue Implementierungen
					*	empfohlen. Slaves sollten nach Möglichkeit die Befehle 0x50 bis 0x52
					*	auswerten. Master, die aus Gründen der Kompatibilität zu existierenden
					*	Produkten abweichende und/oder unterschiedliche Umsetzungen bei den
					*	Befehlen 0x24 und 0x25 verwenden, sind normkonform.
					*
					*	Il livello di velocita' e la direzione corrispondono allo stato reale del motore.
					*	Il valore G e' normalizzato come 0 ... 127 alla Vmax impostata nel modello
					*	Trasferimento. G = 0 significa che la locomotiva e' ferma, G = 1 ... 127 e' quella standardizzata
					*	Velocita' R = senso di marcia con R = 0 all'indietro e R = 1 per avanti.
					*	Questo e il seguente comando non vengono utilizzati per le nuove implementazioni
					*	consigliato. Se possibile, gli slave dovrebbero utilizzare i comandi da 0x50 a 0x52
					*	valutare. Master, che per motivi di compatibilita' con gli esistenti
					*	Prodotti che deviano e / o implementazioni differenti nel
					*	L'uso dei comandi 0x24 e 0x25 e' conforme allo standard.
					*/
					if (notifySusiRealSpeed) {
						if (bitRead(_BufferPointer->Byte[1], 7) == 1) {
							notifySusiRealSpeed(_BufferPointer->Byte[1] - 128, SUSI_DIR_FWD);
						}
						else {
							notifySusiRealSpeed(_BufferPointer->Byte[1], SUSI_DIR_REV);
						}
					}
					break;
				}
				case 37: {	/* DEPRECATO DALLA REVISIONE 6 / 12 / 2020 */
					/*	" "Soll" Lok-Fahrstufe " : 0010-0101 (0x25 = 37) R G6 G5 G4 - G3 G2 G1 G0
					*
					*	Empfangene Fahrstufe des "Masters" auf 127 Fahrstufen normiert.
					*	G = 0 bedeutet Lok hält, G = 1 ... 127 ist die normierte Geschwindigkeit
					*	R = Fahrtrichtung mit R = 0 für rückwärts und R = 1 für vorwärts
					*
					*	Passo di velocita' ricevuto del "master" normalizzato a 127 passi di velocita'.
					*	G = 0 significa che la locomotiva si ferma, G = 1 ... 127 e' la velocita' normalizzata
					*	R = senso di marcia con R = 0 indietro e R = 1 avanti
					*/
					if (notifySusiRequestSpeed) {
						if (bitRead(_BufferPointer->Byte[1], 7) == 1) {
							notifySusiRequestSpeed(_BufferPointer->Byte[1] - 128, SUSI_DIR_FWD);
						}
						else {
							notifySusiRequestSpeed(_BufferPointer->Byte[1], SUSI_DIR_REV);
						}
					}
					break;
				}
				case 38: {
					/*	"Lastausregelung" : 0010-0110 (0x26 = 38) P7 P6 P5 P4 - P3 P2 P1 P0
					*
					*	Die Erfassung des Lastzustandes kann über Motorspannung, -strom oder -
					*	leistung erfolgen. 0 = keine Last, 127 = maximale Last. Es sind auch
					*	negative Werte möglich, die im 2er-Komplement übertragen werden.
					*	Diese bedeuten weniger Last als Fahren in der Ebene.
					*
					*	Lo stato del carico puo' essere registrato tramite tensione del motore, corrente o prestazione.
					*	0 = nessun carico, 127 = carico massimo.
					*	Ci sono anche possibili valori negativi, che vengono trasferiti nel complemento di 2.
					*	Significa meno carico rispetto alla guida in piano.
					*/

					if (notifySusiMotorLoad) {
						notifySusiMotorLoad(ConvertTwosComplementByteToInteger(_BufferPointer->Byte[1]));
					}
					break;
				}
				case 80: {
					/*	" "Ist" Geschwindigkeit " : 0101-0000 (0x50 = 80) R G6 G5 G4 - G3 G2 G1 G0
					*
					*	Die Geschwindigkeit und Richtung entsprechen dem realen Zustand am
					*	Motor. Dieses ist ein Regelwert in Bezug auf die "Soll"- Geschwindigkeit,
					*	d.h. dass nach Nachführung der Geschwindigkeitsrampe Ist und Soll gleich
					*	sein sollen (ausgeregelter Zustand). Der Wert G ist als 0...127 auf die im
					*	Modell eingestellte Vmax normiert zu übertragen. G = 0 bedeutet Lok steht, G
					*	= 1 ... 127 ist die normiert Geschwindigkeit R = Fahrtrichtung mit R = 0
					*	für rückwärts und R = 1 für vorwärts
					*
					*	La velocita' e la direzione corrispondono alla situazione reale del Motore.
					*	Questo e' un valore di controllo in relazione alla velocita' "target",
					*	ovvero che dopo aver tracciato la rampa di velocita', i valori effettivi e target sono gli stessi, dovrebbe essere (stato stabile).
					*	Il valore G deve essere trasferito come 0 ... 127 standardizzato alla Vmax impostata nel modello.
					*	G = 0 significa che la locomotiva e' ferma, G = 1 ... 127 e' la velocita' normalizzata, R = senso di marcia
					*	con R = 0 per indietro e R = 1 per avanti
					*/
					if (notifySusiRealSpeed) {
						if (bitRead(_BufferPointer->Byte[1], 7) == 1) {
							notifySusiRealSpeed(_BufferPointer->Byte[1] - 128, SUSI_DIR_FWD);
						}
						else {
							notifySusiRealSpeed(_BufferPointer->Byte[1], SUSI_DIR_REV);
						}
					}
					break;
				}
				case 81: {
					/*	" "Soll" Geschwindigkeit " : 0101-0001 (0x51 = 81) R G6 G5 G4 - G3 G2 G1 G0
					*
					*	Interne Fahrstufe des "Masters" entsprechend der über die Kennlinie (CVs
					*	67 bis 94, CVs 2, 6 und 5) und sonstiger CVs, die die Geschwindigkeit des
					*	Fahrzeugs bestimmen, umgerechnete empfangene Fahrstufe auf 127
					*	Fahrstufen normalisiert. D.h. der höchste aufgrund der CVs 94 und/oder
					*	CV5 bzw. sonstiger entsprechender CVs erreichbare Wert wird auf 127
					*	normiert. CVs für Beschleunigung und Bremsen wie die CVs 3, 4, 23 und
					*	24 gehen in die Berechnung nicht ein. G = 0 bedeutet Lok hält, G = 1 ...
					*	127 ist die Geschwindigkeit R = Fahrtrichtung mit R = 0 für rückwärts und
					*	R = 1 für vorwärts.
					*	Da die Decoder unterschiedliche Verfahren zur Bestimmung der
					*	Höchstgeschwindigkeit verwenden, kann es hier leicht unterschiedliche
					*	Implementationen geben. Wichtig ist vor allem, dass sich die Befehle für
					*	Ist und Soll-Geschwindigkeit gleich verhalten.
					*
					*	Livello di guida interno del "Master" in base al controllo della caratteristica (CV)
					*	da 67 a 94, CV 2, 6 e 5) e altri CV che determinano la velocita' della normalizzazione del veicolo convertito hanno ricevuto un livello di guida a 127 livelli di guida.
					*	Cio' significa che il valore piu' alto ottenibile grazie ai CV 94 e/o CV5 o ad altri CV corrispondenti e' normalizzato a 127.
					*	I CV per l'accelerazione e la frenata come i CV 3, 4, 23 e 24 non vengono utilizzati nel calcolo.
					*	G = 0 significa appigli per locomotive, G = 1 ... 127 e' la velocita' R = direzione di marcia con R = 0 per l'indietro e R = 1 per l'avanti.
					*	Poiche' i decodificatori utilizzano metodi diversi per determinare la velocita' massima, potrebbero esserci implementazioni leggermente diverse.
					*	Soprattutto, e' importante che i comandi per la velocita' effettiva e di destinazione si comportino allo stesso modo.
					*/
					if (notifySusiRequestSpeed) {
						if (bitRead(_BufferPointer->Byte[1], 7) == 1) {
							notifySusiRequestSpeed(_BufferPointer->Byte[1] - 128, SUSI_DIR_FWD);
						}
						else {
							notifySusiRequestSpeed(_BufferPointer->Byte[1], SUSI_DIR_REV);
						}
					}
					break;
				}
				case 82: {	/* NON IMPLEMENTATO */
					/*	"DCC-Fahrstufe" : 0101-0010 (0x52 = 82) R G6 G5 G4 - G3 G2 G1 G0
					*
					*	Dieser Wert ist nur ggf. von 14 oder 28 Fahrstufen auf 127 Fahrstufen normiert.
					*	Es findet keine Anpassung durch irgendwelches CVs statt.
					*
					*	Questo valore e' standardizzato solo da 14 o 28 passaggi di guida a 127 passaggi di guida.
					*	Non vi e' alcuna regolazione da parte di CV.
					*/
					break;
				}
				case 40: {
					/*	"Analogfunktionsgruppe 1" : 0010-1000 (0x28 = 40) A7 A6 A5 A4 - A3 A2 A1 A0
					*
					*	Die acht Befehle dieser Gruppe erlauben die Übertragung von acht
					*	verschiedenen Analogwerten im Digitalbetrieb.
					*
					*	Gli otto comandi di questo gruppo consentono la trasmissione di otto diversi valori analogici in modalita' digitale.
					*/
					if (notifySusiAnalogFunction) {
						notifySusiAnalogFunction(SUSI_AN_FN_0_7, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 41: {
					/*	"Analogfunktionsgruppe 2" : 0010-1001 (0x29 = 41) A15 A14 A13 A12 - A11 A10 A9 A8 */
					if (notifySusiAnalogFunction) {
						notifySusiAnalogFunction(SUSI_AN_FN_8_15, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 42: {
					/*	"Analogfunktionsgruppe 3" : 0010-1010 (0x2A = 42) A23 A22 A21 A20 - A19 A18 A17 A16 */
					if (notifySusiAnalogFunction) {
						notifySusiAnalogFunction(SUSI_AN_FN_16_23, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 43: {
					/*	"Analogfunktionsgruppe 4" : 0010-1011 (0x2B = 43) A31 A30 A29 A28 - A27 A26 A25 A24 */
					if (notifySusiAnalogFunction) {
						notifySusiAnalogFunction(SUSI_AN_FN_24_31, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 44: {
					/*	"Analogfunktionsgruppe 5" : 0010-1100 (0x2C = 44) A39 A38 A37 A36 - A35 A34 A33 A32 */
					if (notifySusiAnalogFunction) {
						notifySusiAnalogFunction(SUSI_AN_FN_32_39, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 45: {
					/*	"Analogfunktionsgruppe 6" : 0010-1101 (0x2D = 45) A47 A46 A45 A44 - A43 A42 A42 A40 */
					if (notifySusiAnalogFunction) {
						notifySusiAnalogFunction(SUSI_AN_FN_40_47, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 46: {
					/*	"Analogfunktionsgruppe 7" : 0010-1110 (0x2E = 46) A55 A54 A53 A52 - A51 A50 A49 A48 */
					if (notifySusiAnalogFunction) {
						notifySusiAnalogFunction(SUSI_AN_FN_48_55, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 47: {
					/*	"Analogfunktionsgruppe 8" : 0010-1111 (0x2F = 47) A63 A62 A61 A60 - A59 A58 A57 A56 */
					if (notifySusiAnalogFunction) {
						notifySusiAnalogFunction(SUSI_AN_FN_56_63, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 48: {
					/*	"Direktbefehl 1 für Analogbetrieb" : 0011-0000 (0x30 = 48) D7 D6 D5 D4 - D3 D2 D1 D0
					*
					*	Einstellung von Grundfunktionen im Analogbetrieb unter Umgehung einer Funktionszuordnung.
					*	- Bit 0: Sound an/aus
					*	- Bit 1: Auf-/Abrüsten
					*	- Bit 2-6: reserviert
					*	- Bit 7: Reduzierte Lautstärke
					*
					*	Impostazione delle funzioni di base in modalità analogica ignorando un'assegnazione di funzione.
					*	- Bit 0: Suono on/off
					*	- Bit 1: Aggiornamento/Disarmo
					*	- Bit 2-6: riservato
					*	- Bit 7: Volume ridotto
					*/
					if (notifySusiAnalogDirectCommand) {
						notifySusiAnalogDirectCommand(1, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 49: {
					/*	"Direktbefehl 2 für Analogbetrieb" : 0011-0000 (0x31 = 49) D7 D6 D5 D4 - D3 D2 D1 D0
					*
					*	Einstellung von Grundfunktionen im Analogbetrieb unter Umgehung einer Funktionszuordnung.
					*	- Bit 0: Spitzenlicht
					*	- Bit 1: Schlusslicht
					*	- Bit 2: Standlicht
					*	- Bit 3-7: reserviert
					*
					*	Impostazione delle funzioni di base in modalità analogica ignorando un'assegnazione di funzione.
					*	- Bit 0: Luce di picco
					*	- Bit 1: Fanale posteriore
					*	- Bit 2: Stand light
					*	- Bit 3-7: riservato
					*/
					if (notifySusiAnalogDirectCommand) {
						notifySusiAnalogDirectCommand(2, _BufferPointer->Byte[1]);
					}
					break;
				}
				case 0: {
					/*	"No Operation" : 0000-0000 (0x00 = 0) X X X X - X X X X
					*
					*	Der Befehl bewirkt keine Aktion im Slave. Die Daten können einen
					*	beliebigen Wert haben. Der Befehl kann als Lückenfüller oder zu
					*	Testzwecken verwendet werden.
					*
					*	Il comando non esegue alcuna azione nello slave.
					*	I dati possono avere qualsiasi valore. Il comando puo' essere utilizzato come gap filler o a scopo di test.
					*/
					if (notifySusiNoOperation) {
						notifySusiNoOperation(_BufferPointer->Byte[1]);
					}
					break;
				}
				case 94: {	//&& 95
					/*	"Moduladresse low" : 0101-1110 (0x5E = 94) A7 A6 A5 A4 - A3 A2 A1 A0
					*
					*	Übermittelt die niederwertigen Bits der aktiven Digitaladresse des "Masters", wenn er sich in einer digitalen Betriebsart befindet.
					*	Der Befehl wird immer paarweise vor der Adresse high Byte gesendet.
					*	Folgen die beiden Befehle nicht direkt aufeinander, so sind sie zu ignorieren.
					*
					*	Invia i bit di basso valore dell'indirizzo digitale attivo del "master" quando e' in modalita' digitale.
					*	Il comando viene sempre inviato in coppie in coppia prima dell'indirizzo high bytes.
					*	Se i due comandi non si susseguono direttamente, devono essere ignorati.
					*
					*
					*	"Moduladresse high" : 0101-1111 (0x5F = 95) A15 A14 A13 A12 - A11 A10 A9 A8
					*
					*	Übermittelt die höherwertigen Bits der aktiven Digitaladresse des "Masters", wenn er sich in einer digitalen Betriebsart befindet.
					*	Der Befehl wird immer paarweise nach der Adresse low Byte gesendet.
					*	Folgen die beiden Befehle nicht direkt aufeinander, so sind sie zu ignorieren.
					*
					*	Invia i bit di qualita' superiore dell'indirizzo digitale attivo del "master" quando e' in modalita' digitale.
					*	Il comando viene sempre inviato a coppie in base all'indirizzo a byte basso.
					*	Se i due comandi non si susseguono direttamente, devono essere ignorati.
					*/

					Rcn600Message* next = _BufferPointer->nextMessage;
					if (next != NULL) {
						if (next->Byte[0] == 95) {							//i byte di comando devono susseguirsi
							if (notifySusiMasterAddress) {					// Controllo se e' presente il metodo per gestire il comando
								static uint16_t MasterAddress;

								MasterAddress = next->Byte[1];				//memorizzo i bit "piu' significativ"
								MasterAddress = MasterAddress << 8;			//sposto i bit 7 posti a 'sinistra'
								MasterAddress |= _BufferPointer->Byte[1];	//aggiungo i 7 bit "meno significativi"

								notifySusiMasterAddress(MasterAddress);
							}
						}
					}
					else {
						// Il messaggio successivo NECESSARIO non e' ancora stato ricevuto, esco dal process senza modifiche
						return 1;
					}
					break;
				}
				case 108: {
					/*	"Modul-Steuerbyte" : 0110-1100 (0x6C = 108) B7 B6 B5 B4 - B3 B2 B1 B0
					*
					*	- Bit 0 = Buffer Control: 0 = Puffer aus, 1 = Puffer an
					*	- Bit 1 = Rücksetzfunktion: 0 = alle Funktionen auf "Aus" setzen, 1 = normaler Betrieb
					*	Alle weiteren Bits von der RailCommunity reserviert.
					*	Falls implementiert sind nach einem Reset die Bits 0 und 1 im Slave auf 1 zu setzen.
					*
					*	- Bit 0 = Controllo buffer: 0 = Buffer off, 1 = Buffer on
					*	- Bit 1 = Funzione di ripristino: 0 = impostare tutte le funzioni su "Off", 1 = normale funzionamento
					*	Tutti gli altri bit riservati dalla RailCommunity.
					*	Se implementati, i bit 0 e 1 nello slave devono essere impostati su 1 dopo un ripristino.
					*/

					if (notifySusiControllModule) {
						notifySusiControllModule(_BufferPointer->Byte[1]);
					}
					break;
				}
				default: {}
			}

			// Aggiorno il puntatore del Buffer
			Rcn600Message* p = _BufferPointer->nextMessage;
			_BufferPointer->nextMessage = FREE_MESSAGE_SLOT;
			_BufferPointer = p;

			// Messaggio decodificato correttamente
			return 1;
		}
		else {
			// Nessun messaggio da decodificare
			return 0;
		}
	}

	// Nessun messaggio da decodificare
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
