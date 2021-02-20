#include "Rcn600.h"

SUSI_t SusiData;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void read_bit(void) {
	bitWrite(SusiData.MessageByte[SusiData.ByteCounter], SusiData.bitCounter, digitalRead(SusiData.DATA_pin));		//salvo il valore della linea DATA
	++SusiData.bitCounter;																							//incremento il contatore dei bit per la prossima lettura
	SusiData.lastbit_time = micros();																				//memorizzo l'istante in cui + stato letto il bit
}

void ISR_SUSI() {
	//controllo che l'ultimo messaggio ricevuto sia stato processato: in caso positivo procedo con la lettura di uno nuvo

	if (!SusiData.MessageComplete) {
		if (millis() - SusiData.lastByte_time > SYNC_TIME) {	//se sono passati piu' di 9ms dall'ultimo Byte ricevuto, devo resettare la lettura dei dati
			SusiData.bitCounter = 0;							//dopo il SYNC leggero' il primo bit
			SusiData.ByteCounter = 0;
			SusiData.lastByte_time = millis();					//imposto questo istante come ultimo Byte letto

			read_bit();
		}
		else if (((micros() - SusiData.lastbit_time) > 10) && ((micros() - SusiData.lastbit_time) < 500)) { //se non sono passati ancora 9ms, devo controllare che la durata del bit sia valida: dall'ultimo bit letto devono essere passati almeno 10us e meno di 500us
			read_bit();

			/* Controllo se ho letto un Byte */
			if (SusiData.bitCounter == 8) {
				/* Se ho letto un Byte, memorizzo il momento in cui la lettura � avvenuta e resetto il contatore dei bit */
				SusiData.lastByte_time = millis();
				SusiData.bitCounter = 0;

				if (SusiData.ByteCounter == 0) {	/* Se e' il primo Byte letto devo leggerne un altro */
					SusiData.ByteCounter = 1;
				}
				else if (SusiData.ByteCounter == 1) {	/* Se e' il secodno Byte letto, devo controllare se e' un emssaggio che richiede 3 byte */
					if (SusiData.MessageByte[0] == 119 || SusiData.MessageByte[0] == 123 || SusiData.MessageByte[0] == 127) {
						SusiData.ByteCounter = 2;
					}
					else { /* o se sonop sufficienti due Byte */
						SusiData.MessageComplete = true;
					}
				}
				else {	/* Ho letto 3 byte, non ci sono messaggi di lunghezza maggiore */
					SusiData.MessageComplete = true;
				}
			}

		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Rcn600::Rcn600(uint8_t CLK_pin_i, uint8_t DATA_pin_i) {
	SusiData.CLK_pin = CLK_pin_i;
	SusiData.DATA_pin = DATA_pin_i;

	pinMode(SusiData.CLK_pin, INPUT);
	pinMode(SusiData.DATA_pin, INPUT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Rcn600::init(void) {
	SusiData.bitCounter = 0;
	SusiData.ByteCounter = 0;
	SusiData.MessageComplete = false;

	SusiData.lastByte_time = SusiData.lastbit_time = 0;

	attachInterrupt(digitalPinToInterrupt(SusiData.CLK_pin), ISR_SUSI, FALLING);	//da normativa i dati fanno letti sul "fronte di discesa" del Clock

	if (notifySusiCVRead) {	/* Se e' presente il sistema di memorizzazione CV, leggo da tale sistema il numero dello Slave*/
		SlaveNumber = notifySusiCVRead(897);
	}
	else {	//in caso contrario imposto il valore 1
		SlaveNumber = DEFAULT_SLAVE_NUMBER;
	}

#ifdef DEBUG_RCN_LIBRARY
	Serial.begin(115200);	// Avvio la comunicazione Seriale

	while (!Serial) {}		// Attendo che la comunicazione seriale sia disponibile

	Serial.print("SUSI Debug Modulo ");	//Informo l'utente che e' pronto a leggere i Byte
	Serial.print(SlaveNumber);
	Serial.println(": ");
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Rcn600::Data_ACK(void) {	//impulso ACK sulla linea Data
	/* La normativa prevede che come ACK la linea Data venga messa a livello logico LOW per almeno 1ms (max 2ms) */
	pinMode(SusiData.DATA_pin, OUTPUT); //un pin impostao come OUTPUT � messo in automatico a LOW (GND)
	digitalWrite(SusiData.DATA_pin, LOW);
	
	delay(1);

	digitalWrite(SusiData.DATA_pin, HIGH);
	pinMode(SusiData.DATA_pin, INPUT); //rimetto la linea a INPUT (alta impedenza), per leggere un nuovo bit

#ifdef DEBUG_RCN_LIBRARY
	Serial.println("SUSI ACK");
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Rcn600::isCVvalid(uint16_t CV) {
	/* Questa funzione permette di determinare se il numero della CV e' valida per questo modulo SUSI
	* Slave 1: 900 - 939
	* Slave 2: 940 - 979
	* Slave 3: 980 - 1019
	* Per tutti: 1020 - 1024 */

#ifdef DEBUG_RCN_LIBRARY
	Serial.print("isCVvalid: ");
	Serial.print("CV da controllare: ");
	Serial.print(CV);
	Serial.print(" ; CV valida per: ");
#endif

	if ((SlaveNumber == 1) && ((CV >= 900) && (CV <= 939))) {
#ifdef DEBUG_RCN_LIBRARY
		Serial.println("Slave 1");
#endif
		return true;
	}
	else if ((SlaveNumber == 2) && ((CV >= 940) && (CV <= 979))) {
#ifdef DEBUG_RCN_LIBRARY
		Serial.println("Slave 2");
#endif
		return true;
	}
	else if ((SlaveNumber == 3) && ((CV >= 980) && (CV <= 1019))) {
#ifdef DEBUG_RCN_LIBRARY
		Serial.println("Slave 3");
#endif
		return true;
	}
	else if ( CV == 897 || (CV <= 1024 && CV >= 1020)) {	//CV valide per tutti i moduli
#ifdef DEBUG_RCN_LIBRARY
		Serial.println("Tutti gli Slave");
#endif
		return true;
	}
	else {
#ifdef DEBUG_RCN_LIBRARY
		Serial.println("CV NON VALIDA");
#endif
		return false;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int ConvertTwosComplementByteToInteger(byte rawValue) {
	// If a positive value, return it
	if ((rawValue & 0x80) == 0) {
		return rawValue;
	}

	// Otherwise perform the 2's complement math on the value
	return (byte)(~(rawValue - 0x01)) * -1;
}

void Rcn600::process(void) {
	if (SusiData.MessageComplete) {		//controllo che sia stato ricevuto un messaggio completo
#ifdef DEBUG_RCN_LIBRARY
		Serial.print(SusiData.MessageByte[0]);
		Serial.print(" - ");
		Serial.print(SusiData.MessageByte[1]);
		if (SusiData.ByteCounter == 2) {
			Serial.print(" - ");
			Serial.print(SusiData.MessageByte[2]);
		}
		Serial.println();
#endif // DEBUG_RCN_LIBRARY

		/* Devo controllare il valore del primo Byte */
		switch (SusiData.MessageByte[0]) {
		case 96: {
			/* "Funktionsgruppe 1" : 0110-0000 (0x60 = 96) 0 0 0 F0 - F4 F3 F2 F1 */
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_0_4, SusiData.MessageByte[1]);
			}
			break;
		}
		case 97: {
			/* "Funktionsgruppe 2" : 0110-0001 (0x61 = 97) F12 F11 F10 F9 - F8 F7 F6 F5 */
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_5_12, SusiData.MessageByte[1]);
			}
			break;
		}
		case 98: {
			/* "Funktionsgruppe 3" : 0110-0010 (0x62 = 98) F20 F19 F18 F17 - F16 F15 F14 F13 */
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_13_20, SusiData.MessageByte[1]);
			}
			break;
		}
		case 99: {
			/* "Funktionsgruppe 4" : 0110-0011 (0x63 = 99) F28 F27 F26 F25 - F24 F23 F22 F21 */
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_21_28, SusiData.MessageByte[1]);
			}
			break;
		}
		case 100: {
			/* "Funktionsgruppe 5" : 0110-0100 (0x64 = 100) F36 F35 F34 F33 - F32 F31 F30 F29 */
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_29_36, SusiData.MessageByte[1]);
			}
			break;
		}
		case 101: {
			/* "Funktionsgruppe 6" : 0110-0101 (0x65 = 101) F44 F43 F42 F41 - F40 F39 F38 F37 */
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_37_44, SusiData.MessageByte[1]);
			}
			break;
		}
		case 102: {
			/* "Funktionsgruppe 7" : 0110-0110 (0x66 = 102) F52 F51 F50 F49 - F48 F47 F46 F45 */
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_45_52, SusiData.MessageByte[1]);
			}
			break;
		}
		case 103: {
			/* "Funktionsgruppe 8" : 0110-0111 (0x67 = 103) F60 F59 F58 F57 - F56 F55 F54 F53 */
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_53_60, SusiData.MessageByte[1]);
			}
			break;
		}
		case 104: {
			/* "Funktionsgruppe 9" : 0110-1000 (0x68 = 104) F68 F67 F66 F65 - F64 F63 F62 F61 */
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_61_68, SusiData.MessageByte[1]);
			}
			break;
		}
		case 109: {													
			/* "Bin�rzust�nde kurze Form" :  0110 - 1101 (0x6D = 109) D L6 L5 L4 - L3 L2 L1 L0 */
			static uint8_t functionNumber, funcState;
			funcState = bitRead(SusiData.MessageByte[1], 7);	//leggo il valore dello stato 'D'
			bitWrite(SusiData.MessageByte[1], 7, 0);		//elimino il valore dello stato
			functionNumber = SusiData.MessageByte[1];		//i restanti bit identificano la Funzione 'L'

			/*
			*	D = 0 bedeutet Funktion L ausgeschaltet, D = 1 eingeschaltet	
			*	L = Funktionsnummer 1 ... 127
			*	L = 0 (Broadcast) schaltet alle Funktionen 1 bis 127 aus (D = 0) oder an (D = 1) 
			* 
			*	D = 0 significa funzione L disattivata, D = 1 attivata
			*	L = numero funzione 1 ... 127
			*	L = 0 (trasmissione) disattiva (D = 0) o attiva tutte le funzioni da 1 a 127 (D = 1)
			*/

			if (notifySusiSingleFunc) {
				if (functionNumber == 0) {
					// Comanda tutte le funzioni
					static uint8_t i;
					if (funcState == 0) {	// disattivo tutte le funzioni
						for (i = 1; i < 128; ++i) {
							notifySusiSingleFunc(i, 0);
						}
					}
					else {				// attivo tutte le funzioni 
						for (i = 1; i < 128; ++i) {
							notifySusiSingleFunc(i, 1);
						}
					}
				}
				else {
					// Comanda una singola funzione
					notifySusiSingleFunc(functionNumber, funcState);
				}
			}
			break;
		}
		case 64: {
			/* "Direktbefehl 1" (2-Byte): 0100-0000 (0x40 = 64) X8 X7 X6 X5 - X4 X3 X2 X1
			* 
			* Die Direktbefehle dienen zur direkten Ansteuerung von Ausg�ngen und
			* anderen Funktionen nach der Anwendung der Funktionstabelle im Master.
			* Ein Bit = 1 bedeutet der entsprechende Ausgang ist eingeschaltet.
			* 
			* I comandi diretti vengono utilizzati per il controllo diretto delle uscite e
			* altre funzioni dopo aver utilizzato la tabella delle funzioni nel master.
			* Un bit = 1 significa che l'uscita corrispondente � attivata.
			*/
			if (notifySusiAux) {
				notifySusiAux(SUSI_AUX_1_8, SusiData.MessageByte[1]);
			}
			break;
		}
		case 65: {
			/* "Direktbefehl 2" : 0100-0001 (0x41 = 65) X16 X15 X14 X13 - X12 X11 X10 X9 */
			if (notifySusiAux) {
				notifySusiAux(SUSI_AUX_9_16, SusiData.MessageByte[1]);
			}
			break;
		}
		case 66: {
			/* "Direktbefehl 3" : 0100-0010 (0x42 = 66) X24 X23 X22 X21 - X20 X19 X18 X17 */
			if (notifySusiAux) {
				notifySusiAux(SUSI_AUX_17_24, SusiData.MessageByte[1]);
			}
			break;
		}
		case 67: {
			/* "Direktbefehl 4" : 0100-0011 (0x43 = 67) X32 X31 X30 X29 - X28 X27 X26 X25 */
			if (notifySusiAux) {
				notifySusiAux(SUSI_AUX_25_32, SusiData.MessageByte[1]);
			}
			break;
		}
		case 33: {
			/* "Trigger-Puls" : 0010-0001 (0x21 = 33) 0 0 0 0 - 0 0 0 1 
			*
			* Der Befehl dient zur Synchronisation eines Dampfsto�es. Er wird je
			* Dampfsto� einmal gesendet. Die Bits 1 bis 7 sind f�r zuk�nftige
			* Anwendungen reserviert.
			*
			* 
			* Il comando viene utilizzato per sincronizzare una scarica di vapore.
			* Lo fara' mai scoppio di vapore inviato una volta. 
			* I bit da 1 a 7 sono per uso futuro, Applicazioni riservate.
			*/
			if (notifySusiTriggerPulse) {
				notifySusiTriggerPulse(SusiData.MessageByte[1]);
			}
			break;
		}
		case 35: {
			/* "Strom" : 0010-0011 (0x23 = 35) S7 S6 S5 S4 - S3 S2 S1 S0
			*
			* Vom Motor verbrauchter Strom. Der Wert hat einen Bereich von -128 bis
			* 127, wird im 2er-Komplement �bertragen und wird �ber eine
			* herstellerspezifische CV im Lokdecoder kalibriert. Negative Werte
			* bedeuten ein R�ckspeisen wie es bei modernen E-Loks m�glich ist.
			*
			* Elettricita' consumata dal motore. Il valore ha un intervallo da -128 a
			* 127, viene trasmesso in complemento a 2 e viene trasmesso tramite a
			* CV specifico del costruttore calibrato nel decoder della locomotiva. Valori negativi
			* significa un feed back come e' possibile con le moderne locomotive elettriche.
			*/
			if (notifySusiMotorCurrent) {
				notifySusiMotorCurrent(ConvertTwosComplementByteToInteger(SusiData.MessageByte[1]));
			}
			break;
		}
		case 36: {
			if (notifySusiRealSpeed) {
				if (bitRead(SusiData.MessageByte[1], 7) == 1) {
					notifySusiRealSpeed(SusiData.MessageByte[1] - 128, SUSI_DIR_FWD);
				}
				else {
					notifySusiRealSpeed(SusiData.MessageByte[1] - 128, SUSI_DIR_REV);
				}
			}
			break;
		}
		case 37: {
			if (notifySusiRequestSpeed) {
				if (bitRead(SusiData.MessageByte[1], 7) == 1) {
					notifySusiRequestSpeed(SusiData.MessageByte[1] - 128, SUSI_DIR_FWD);
				}
				else {
					notifySusiRequestSpeed(SusiData.MessageByte[1] - 128, SUSI_DIR_REV);
				}
			}
			break;
		}
		case 38: {
			/* "Lastausregelung" : 0010-0110 (0x26 = 38) P7 P6 P5 P4 - P3 P2 P1 P0
			*
			* Die Erfassung des Lastzustandes kann �ber Motorspannung, -strom oder -
			* leistung erfolgen. 0 = keine Last, 127 = maximale Last. Es sind auch
			* negative Werte m�glich, die im 2er-Komplement �bertragen werden.
			* Diese bedeuten weniger Last als Fahren in der Ebene.
			* 
			* Lo stato del carico puo' essere registrato tramite tensione del motore, corrente o prestazione. 
			* 0 = nessun carico, 127 = carico massimo. 
			* Ci sono anche possibili valori negativi, che vengono trasferiti nel complemento di 2.
			* Significa meno carico rispetto alla guida in piano.
			*/

			if (notifySusiMotorLoad) {
				notifySusiMotorLoad(ConvertTwosComplementByteToInteger(SusiData.MessageByte[1]));
			}
			break;
		}


		/* METODI MANIPOLAZIONE CVs */

		case 119: {	
			/* "CV-Manipulation Byte pr�fen" : 0111-0111 (0x77 = 119) | 1 V6 V5 V4 - V3 V2 V1 V0 | D7 D6 D5 D4 - D3 D2 D1 D0
			*
			* DCC-Befehl Byte Pr�fen im Service- und Betriebsmodus
			* V = CV-Nummer 897 .. 1024 (Wert 0 = CV 897, Wert 127 = CV 1024)
			* D = Vergleichswert zum Pr�fen. Wenn D dem gespeicherten CV-Wert
			* entspricht, antwortet der Slave mit einem Acknowledge. 
			* Dieser und die beiden folgenden Befehle sind die in Abschnitt 4 genannten
			* 3 Byte Pakete entsprechend [RCN-214] 
			* 
			* Controllo byte di comando DCC in modalita' di servizio e di funzionamento
			* V = numero CV 897 .. 1024 (valore 0 = CV 897, valore 127 = CV 1024)
			* D = valore di confronto per il controllo. Se D corrisponde al valore CV memorizzato
			* lo Slave risponde con un riconoscimento.
			* Questo ei due comandi seguenti sono quelli menzionati nella sezione 4
			* Pacchetti da 3 byte secondo [RCN-214]
			*/
			static uint16_t CV_Number;
			static uint8_t CV_Value;

			CV_Number = 897 + (SusiData.MessageByte[1] - 128);

			if (isCVvalid(CV_Number)) {
				/* Devo controllare se la CV richiesta e' di quelle contenenti informazioni quali produttore o versione */

				if ((CV_Number == 897) || (CV_Number == 900) || (CV_Number == 901) || (CV_Number == 940) || (CV_Number == 941) || (CV_Number == 980) || (CV_Number == 981)) {
					if (CV_Number == 897) {
						CV_Value = SlaveNumber;
					}
					else if ((CV_Number == 900) || (CV_Number == 940) || (CV_Number == 980)) {	//identificano il produttore dello Slave
						CV_Value = MANUFACTER_ID;
					}
					else { //identificano la versione software
						CV_Value = LIB_VER;
					}
				}
				else {
					if (notifySusiCVRead) { //altre CV disponibili per il modulo
						CV_Value = notifySusiCVRead(CV_Number);
					}
					else {
						CV_Value = 255;		//Se non e' implementato un sistema di memorizzazione CV utilizzo il valore simbolico di 255
					}
				}

				/* Confronto fra il valore memorizzato e quello ipotizzato dal master */
				if (CV_Value == SusiData.MessageByte[2]) {
					Data_ACK();
				}
			}
			break;
		}
		case 123: {	
			/* "CV-Manipulation Bit manipulieren" : 0111-1011 (0x7B = 123) | 1 V6 V5 V4 - V3 V2 V1 V0 | 1 1 1 K - D B2 B1 B0
			*
			* DCC-Befehl Bit Manipulieren im Service- und Betriebsmodus
			* V = CV-Nummer 897 ... 1024 (Wert 0 = CV 897, Wert 127 = CV 1024)
			* K = 0: Bit Pr�fen. Wenn D mit dem Bitzustand an der Bitstelle B der CV
			* �bereinstimmt, wird mit einem Acknowledge geantwortet.
			* K = 1: Bit Schreiben. D wird in Bitstelle B der CV geschrieben.
			* Der Slave best�tigt das Schreiben mit einem Acknowledge.
			* 
			* Manipolazione dei bit di comando DCC in modalita' di servizio e operativa
			* V = numero CV 897 ... 1024 (valore 0 = CV 897, valore 127 = CV 1024)
			* k = 0: bit di controllo. Se D con lo stato del bit alla posizione del bit B del CV
			* partite, viene dato un riconoscimento.
			* K = 1: scrivi bit. D e' scritto nella posizione di bit B del CV.
			* Lo slave conferma la scrittura con un riconoscimento.
			*/
			static uint16_t CV_Number;

			CV_Number = 897 + (SusiData.MessageByte[1] - 128);

			if (isCVvalid(CV_Number)) {
				static uint8_t CV_Value, operation, bitValue, bitPosition;

				operation = bitRead(SusiData.MessageByte[2], 4);																									// leggo quale operazione sui bit e' richiesta
				bitValue = bitRead(SusiData.MessageByte[2], 3);																										// leggo il valore del bit da confrontare/scrivere
				bitPosition = ((bitRead(SusiData.MessageByte[2], 0) * 1) + (bitRead(SusiData.MessageByte[2], 1) * 2) + (bitRead(SusiData.MessageByte[2], 2) * 4));	// leggo in quale posizione si trova il bit su cui fare il confronto/scrittura

				if ((CV_Number == 900) || (CV_Number == 940) || (CV_Number == 980)) {			//identificano il produttore dello Slave
					CV_Value = MANUFACTER_ID;
				}
				else if ((CV_Number == 901) || (CV_Number == 941) || (CV_Number == 981)) {		//identificano la versione software
					CV_Value = LIB_VER;
				}
				else {
					if (notifySusiCVRead) {
						CV_Value = notifySusiCVRead(CV_Number);									// Leggo il valore della CV sulla quale manipolare i bit
					}
					else {
						CV_Value = 254;		//Se non e' implementato un sistema di memorizzazione CV utilizzo il valore simbolico di 254
					}
				}

				//in base all'operazione richiesta eseguiro' un'azione
				switch (operation) {
				case 0: {	//confronto dei bit
					if (bitRead(CV_Value, bitPosition) == bitValue) {	//confronto il bit richiesto con quello memorizzato 
						Data_ACK();
					}
					break;
				}
				case 1: {	//scrittura del bit
					if (!((CV_Number == 900) || (CV_Number == 901) || (CV_Number == 940) || (CV_Number == 941) || (CV_Number == 980) || (CV_Number == 981))) {
						if (notifySusiCVWrite) {
							bitWrite(CV_Value, bitPosition, bitRead(SusiData.MessageByte[2], 3));	//scrivo il nuovo valore del bit
							if (notifySusiCVWrite((897 + (SusiData.MessageByte[1] - 128)), CV_Value) == CV_Value) {	//memorizzo il nuovo valore della CV
								Data_ACK();
							}
							//nel caso in cui non e' implementato un sistema di memorizzazione CVs, non faccio nulla
						}
					}
					break;
				}
				}

			}
			break;
		}
		case 127: {	
			/* "CV-Manipulation Byte schreiben" : 0111-1111 (0x7F = 127) | 1 V6 V5 V4 - V3 V2 V1 V0 | D7 D6 D5 D4 - D3 D2 D1 D0 
			*
			* DCC-Befehl Byte Schreiben im Service- und Betriebsmodus
			* V = CV-Nummer 897 .. 1024 (Wert 0 = CV 897, Wert 127 = CV 1024)
			* D = Wert zum Schreiben in die CV. Der Slave best�tigt das Schreiben mit
			* einem Acknowledge. 
			* 
			* Scrittura byte di comando DCC in modalita' di servizio e operativa
			* V = numero CV 897 .. 1024 (valore 0 = CV 897, valore 127 = CV 1024)
			* D = valore per la scrittura nel CV. Lo Slave conferma la scrittura con un riconoscimento.
			*/
			static uint16_t CV_Number;

			CV_Number = 897 + (SusiData.MessageByte[1] - 128);

			if (isCVvalid(CV_Number)) {
				/* Devo controllare se la CV richiesta NON e' di quelle contenenti informazioni quali produttore o versione */

				if ( !( (CV_Number == 900) || (CV_Number == 901) || (CV_Number == 940) || (CV_Number == 941) || (CV_Number == 980) || (CV_Number == 981) ) ) {
					if (notifySusiCVWrite) {
						if (notifySusiCVWrite(CV_Number, SusiData.MessageByte[2]) == SusiData.MessageByte[2]) {
							Data_ACK();
						}
					}
					//in caso di sistema di memorizzazione CVs non implementato non eseguo l'ACK
				}

				if (CV_Number == 897) {
					if (notifySusiCVRead) {
						SlaveNumber = notifySusiCVRead(CV_Number);
					}
				}
			}
			break;
		}
		default: {}
		}

		/* Resetto il buffer e i contatori per decodificre un nuovo Comando */
		SusiData.bitCounter = 0;
		SusiData.ByteCounter = 0;
		SusiData.MessageComplete = false;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////