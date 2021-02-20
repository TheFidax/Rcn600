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
				/* Se ho letto un Byte, memorizzo il momento in cui la lettura è avvenuta e resetto il contatore dei bit */
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
		SlaveNumber = 1;
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
	pinMode(SusiData.DATA_pin, OUTPUT); //un pin impostao come OUTPUT è messo in automatico a LOW (GND)
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
		case 96: {														// Gruppo Funzioni Digitali 1: F0 -> F4
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_0_4, SusiData.MessageByte[1]);
			}
			break;
		}
		case 97: {														// Gruppo Funzioni Digitali 2: F5 -> F12
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_5_12, SusiData.MessageByte[1]);
			}
			break;
		}
		case 98: {														// Gruppo Funzioni Digitali 3: F13 -> F20
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_13_20, SusiData.MessageByte[1]);
			}
			break;
		}
		case 99: {														// Gruppo Funzioni Digitali 4: F21 -> F28
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_21_28, SusiData.MessageByte[1]);
			}
			break;
		}
		case 100: {														// Gruppo Funzioni Digitali 5: F29 -> F36
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_29_36, SusiData.MessageByte[1]);
			}
			break;
		}
		case 101: {														// Gruppo Funzioni Digitali 6: F37 -> F44
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_37_44, SusiData.MessageByte[1]);
			}
			break;
		}
		case 102: {														// Gruppo Funzioni Digitali 7: F45 -> F52
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_45_52, SusiData.MessageByte[1]);
			}
			break;
		}
		case 103: {														// Gruppo Funzioni Digitali 8: F53 -> F60
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_53_60, SusiData.MessageByte[1]);
			}
			break;
		}
		case 104: {														// Gruppo Funzioni Digitali 9: F60 -> F61
			if (notifySusiFunc) {
				notifySusiFunc(SUSI_FN_61_68, SusiData.MessageByte[1]);
			}
			break;
		}
		case 33: {														// Trigger / Pulsazione
			if (notifySusiTriggerPulse) {
				notifySusiTriggerPulse(SusiData.MessageByte[1]);
			}
			break;
		}
		case 34: {														// Deviazione fra la velocita' richiesta e la velocita' reale
			if (notifySusiSpeedDeviation) {
				notifySusiSpeedDeviation(ConvertTwosComplementByteToInteger(SusiData.MessageByte[1]));
			}
			break;
		}
		case 35: {														// Consumo di corrente da parte del motore
			if (notifySusiCurrentAbsorption) {
				notifySusiCurrentAbsorption(ConvertTwosComplementByteToInteger(SusiData.MessageByte[1]));
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
		case 119: {														// Controllo Valore CV
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
				else if (notifySusiCVRead) { //altre CV disponibili per il modulo
					CV_Value = notifySusiCVRead(CV_Number);
				}
				 
#ifdef DEBUG_RCN_LIBRARY
				Serial.print("notifySusiCVcheck: ");
				Serial.print(" CV: ");
				Serial.print(CV_Number, DEC);
				Serial.print(" CV_Value: ");
				Serial.print(CV_Value, DEC);
				Serial.print(" CV_Value_toCheck: ");
				Serial.println(SusiData.MessageByte[2], DEC);
#endif // DEBUG_RCN_LIBRARY

				/* Confronto fra il valore memorizzato e quello ipotizzato dal master */
				if (CV_Value == SusiData.MessageByte[2]) {
					Data_ACK();
				}
			}
			break;
		}
		case 123: {														// Manipolazione bit CV
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
				}

#ifdef DEBUG_RCN_LIBRARY
				Serial.print("notifySusibitManipulation: ");
				Serial.print(" CV: ");
				Serial.print(CV_Number, DEC);
				Serial.print(" operation: ");
				Serial.print(operation, DEC);
				Serial.print(" bitValue: ");
				Serial.print(bitValue, DEC);
				Serial.print(" CurrentbitValue: ");
				Serial.print(bitRead(CV_Value, bitPosition), DEC);
				Serial.print(" CV_Value: ");
				Serial.print(CV_Value, BIN);
				Serial.print(" bitPosition: ");
				Serial.println(bitPosition, DEC);
#endif // DEBUG_RCN_LIBRARY

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
						bitWrite(CV_Value, bitPosition, bitRead(SusiData.MessageByte[2], 3));	//scrivo il nuovo valore del bit
						if (notifySusiCVWrite((897 + (SusiData.MessageByte[1] - 128)), CV_Value) == CV_Value) {	//memorizzo il nuovo valore della CV
							Data_ACK();
						}
					}
					break;
				}
				}

			}
			break;
		}
		case 127: {														// Scrittura Valore CV
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