#include "Rcn600.h"

SUSI_t SusiData;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void read_bit(void) {
	bitWrite(SusiData.MessageByte[SusiData.ByteCounter], SusiData.bitCounter, digitalRead(SusiData.DATA_pin));		//salvo il valore della linea DATA
	++SusiData.bitCounter;																							//incremento il contatore dei bit per la prossima lettura
	SusiData.lastbit_time = micros();																				//memorizzo l'istante in cui e' stato letto il bit
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

			/* Controllo se ho letto un Byte (8 bit) */
			if (SusiData.bitCounter == 8) {
				/* Se ho letto un Byte, memorizzo il momento in cui la lettura è avvenuta e resetto il contatore dei bit */
				SusiData.lastByte_time = millis();
				SusiData.bitCounter = 0;

				if (SusiData.ByteCounter == 0) {	/* Se e' il primo Byte letto devo leggerne un altro */
					SusiData.ByteCounter = 1;
				}
				else if (SusiData.ByteCounter == 1) {	/* Se e' il secondo Byte letto, devo controllare se il un messaggio richiede 3 byte o 4 Byte */
					if (SusiData.MessageByte[0] == 110 || SusiData.MessageByte[0] == 94) {	/* comandi in sequenza che richiedono 4 Byte */
						SusiData.ByteCounter = 2;
					}
					else if (SusiData.MessageByte[0] == 119 || SusiData.MessageByte[0] == 123 || SusiData.MessageByte[0] == 127) {	/* Comandi manipolazione CV: 3 Byte*/
						SusiData.ByteCounter = 2;
					}
					else { /* Messaggio 'normale': sono sufficienti due Byte */
						SusiData.MessageComplete = true;
					}
				}
				else if (SusiData.ByteCounter == 2) {	// Controllo se sono messaggi in sequenza oppure comandi per le CV
					if (SusiData.MessageByte[0] == 110 || SusiData.MessageByte[0] == 94) {	/* comandi in sequenza che richiedono 4 Byte */
						SusiData.ByteCounter = 3;
					}
					else {	// comandi CV a cui bastano 3 Byte
						SusiData.MessageComplete = true;
					}
				}
				else {	/* Ho letto 4 Byte, non ci sono messaggi di lunghezza maggiore */
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
	/* Imposto l'indirizzo dello Slave */
	if (notifySusiCVRead) {						/* Se e' presente il sistema di memorizzazione CV, leggo da tale sistema il numero dello Slave*/
		SlaveNumber = notifySusiCVRead(897);
	}
	else {										/* in caso contrario imposto il valore 1 */
		SlaveNumber = DEFAULT_SLAVE_NUMBER;
	}

	SusiData.bitCounter = 0;
	SusiData.ByteCounter = 0;
	SusiData.MessageComplete = false;

	SusiData.lastByte_time = SusiData.lastbit_time = 0;

	attachInterrupt(digitalPinToInterrupt(SusiData.CLK_pin), ISR_SUSI, FALLING);	//da normativa i dati fanno letti sul "fronte di discesa" del Clock
}

void Rcn600::init(uint8_t SlaveAddress) {		/* Inizializzazione con indirizzo scelto dall'utente nel codice */
	if ( (SlaveAddress > 0) && (SlaveAddress < 4) ) {
		SlaveNumber = SlaveAddress;
	}
	else {										/* In caso di indirizzo passato non conforme alla normativa SUSI imposto il valore di Default 1 */
		SlaveNumber = DEFAULT_SLAVE_NUMBER;
	}

	SusiData.bitCounter = 0;
	SusiData.ByteCounter = 0;
	SusiData.MessageComplete = false;

	SusiData.lastByte_time = SusiData.lastbit_time = 0;

	attachInterrupt(digitalPinToInterrupt(SusiData.CLK_pin), ISR_SUSI, FALLING);	//da normativa i dati fanno letti sul "fronte di discesa" del Clock
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Rcn600::Data_ACK(void) {	//impulso ACK sulla linea Data
	/* La normativa prevede che come ACK la linea Data venga messa a livello logico LOW per almeno 1ms (max 2ms) */
	pinMode(SusiData.DATA_pin, OUTPUT);
	digitalWrite(SusiData.DATA_pin, LOW);
	
	delay(1);

	digitalWrite(SusiData.DATA_pin, HIGH);
	pinMode(SusiData.DATA_pin, INPUT); //rimetto la linea a INPUT (alta impedenza), per leggere un nuovo bit
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Rcn600::isCVvalid(uint16_t CV) {
	/* Questa funzione permette di determinare se il numero della CV e' valida per questo modulo SUSI
	* Slave 1: 900 - 939
	* Slave 2: 940 - 979
	* Slave 3: 980 - 1019
	* Per tutti: 1020 - 1024 */

	if ((SlaveNumber == 1) && ((CV >= 900) && (CV <= 939))) {
		return true;
	}
	else if ((SlaveNumber == 2) && ((CV >= 940) && (CV <= 979))) {
		return true;
	}
	else if ((SlaveNumber == 3) && ((CV >= 980) && (CV <= 1019))) {
		return true;
	}
	else if ( CV == 897 || (CV <= 1024 && CV >= 1020)) {	//CV valide per tutti i moduli; le CV 898 e 899 sono Riservate
		return true;
	}
	else {
		return false;
	}
	return false;
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

void Rcn600::process(void) {
	if (SusiData.MessageComplete) {		//controllo che sia stato ricevuto un messaggio completo
		/* Devo controllare il valore del primo Byte */

#ifdef NOTIFY_RAW_MESSAGE
		if (notifySusiRawMessage) {
			notifySusiRawMessage(SusiData.MessageByte, (SusiData.ByteCounter + 1));
		}
#endif // NOTIFY_RAW_MESSAGE

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

			funcState = bitRead(SusiData.MessageByte[1], 7);	// leggo il valore dello stato 'D'

			bitWrite(SusiData.MessageByte[1], 7, 0);			// elimino il valore dello stato

			functionNumber = SusiData.MessageByte[1];			// i restanti bit identificano la Funzione 'L'

			if (notifySusiBinaryState) {
				if (functionNumber == 0) {
					// Comanda tutte le funzioni
					static uint8_t i;
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

			if (SusiData.MessageByte[2] == 111) {					// Posso eseguire il comando solo se ho ricevuto sia il Byte piu' significativo che quello meno significativo
				if (notifySusiBinaryState) {						// Controllo se e' presente il metodo per gestire il comando
					static uint16_t Command;
					static uint8_t State;

					Command = SusiData.MessageByte[3];				// memorizzo i bit "piu' significativ"
					Command = Command << 7;							// sposto i bit 7 posti a 'sinistra'
					Command |= SusiData.MessageByte[1];				// aggiungo i 7 bit "meno significativi"

					State = bitRead(SusiData.MessageByte[1], 7);

					notifySusiBinaryState(Command, State);
				}
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
				notifySusiAux(SUSI_AUX_1_8, SusiData.MessageByte[1]);
			}
			break;
		}
		case 65: {
			/*	"Direktbefehl 2" : 0100-0001 (0x41 = 65) X16 X15 X14 X13 - X12 X11 X10 X9 */
			if (notifySusiAux) {
				notifySusiAux(SUSI_AUX_9_16, SusiData.MessageByte[1]);
			}
			break;
		}
		case 66: {
			/*	"Direktbefehl 3" : 0100-0010 (0x42 = 66) X24 X23 X22 X21 - X20 X19 X18 X17 */
			if (notifySusiAux) {
				notifySusiAux(SUSI_AUX_17_24, SusiData.MessageByte[1]);
			}
			break;
		}
		case 67: {
			/*	"Direktbefehl 4" : 0100-0011 (0x43 = 67) X32 X31 X30 X29 - X28 X27 X26 X25 */
			if (notifySusiAux) {
				notifySusiAux(SUSI_AUX_25_32, SusiData.MessageByte[1]);
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
				notifySusiTriggerPulse(SusiData.MessageByte[1]);
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
				notifySusiMotorCurrent(ConvertTwosComplementByteToInteger(SusiData.MessageByte[1]));
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
				if (bitRead(SusiData.MessageByte[1], 7) == 1) {
					notifySusiRealSpeed(SusiData.MessageByte[1] - 128, SUSI_DIR_FWD);
				}
				else {
					notifySusiRealSpeed(SusiData.MessageByte[1] - 128, SUSI_DIR_REV);
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
				notifySusiMotorLoad(ConvertTwosComplementByteToInteger(SusiData.MessageByte[1]));
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
				if (bitRead(SusiData.MessageByte[1], 7) == 1) {
					notifySusiRealSpeed(SusiData.MessageByte[1] - 128, SUSI_DIR_FWD);
				}
				else {
					notifySusiRealSpeed(SusiData.MessageByte[1] - 128, SUSI_DIR_REV);
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
				if (bitRead(SusiData.MessageByte[1], 7) == 1) {
					notifySusiRequestSpeed(SusiData.MessageByte[1] - 128, SUSI_DIR_FWD);
				}
				else {
					notifySusiRequestSpeed(SusiData.MessageByte[1] - 128, SUSI_DIR_REV);
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
				notifySusiAnalogFunction(SUSI_AN_FN_0_7, SusiData.MessageByte[1]);
			}
			break;
		}
		case 41: {
			/*	"Analogfunktionsgruppe 2" : 0010-1001 (0x29 = 41) A15 A14 A13 A12 - A11 A10 A9 A8 */
			if (notifySusiAnalogFunction) {
				notifySusiAnalogFunction(SUSI_AN_FN_8_15, SusiData.MessageByte[1]);
			}
			break;
		}
		case 42: {
			/*	"Analogfunktionsgruppe 3" : 0010-1010 (0x2A = 42) A23 A22 A21 A20 - A19 A18 A17 A16 */
			if (notifySusiAnalogFunction) {
				notifySusiAnalogFunction(SUSI_AN_FN_16_23, SusiData.MessageByte[1]);
			}
			break;
		}
		case 43: {
			/*	"Analogfunktionsgruppe 4" : 0010-1011 (0x2B = 43) A31 A30 A29 A28 - A27 A26 A25 A24 */
			if (notifySusiAnalogFunction) {
				notifySusiAnalogFunction(SUSI_AN_FN_24_31, SusiData.MessageByte[1]);
			}
			break;
		}
		case 44: {
			/*	"Analogfunktionsgruppe 5" : 0010-1100 (0x2C = 44) A39 A38 A37 A36 - A35 A34 A33 A32 */
			if (notifySusiAnalogFunction) {
				notifySusiAnalogFunction(SUSI_AN_FN_32_39, SusiData.MessageByte[1]);
			}
			break;
		}
		case 45: {
			/*	"Analogfunktionsgruppe 6" : 0010-1101 (0x2D = 45) A47 A46 A45 A44 - A43 A42 A42 A40 */
			if (notifySusiAnalogFunction) {
				notifySusiAnalogFunction(SUSI_AN_FN_40_47, SusiData.MessageByte[1]);
			}
			break;
		}
		case 46: {
			/*	"Analogfunktionsgruppe 7" : 0010-1110 (0x2E = 46) A55 A54 A53 A52 - A51 A50 A49 A48 */
			if (notifySusiAnalogFunction) {
				notifySusiAnalogFunction(SUSI_AN_FN_48_55, SusiData.MessageByte[1]);
			}
			break;
		}
		case 47: {
			/*	"Analogfunktionsgruppe 8" : 0010-1111 (0x2F = 47) A63 A62 A61 A60 - A59 A58 A57 A56 */
			if (notifySusiAnalogFunction) {
				notifySusiAnalogFunction(SUSI_AN_FN_56_63, SusiData.MessageByte[1]);
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
				notifySusiAnalogDirectCommand(1, SusiData.MessageByte[1]);
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
				notifySusiAnalogDirectCommand(2, SusiData.MessageByte[1]);
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


			if (SusiData.MessageByte[2] == 95) {				//i byte di comando devono susseguirsi
				if (notifySusiMasterAddress) {					// Controllo se e' presente il metodo per gestire il comando
					static uint16_t MasterAddress;

					MasterAddress = SusiData.MessageByte[3];	//memorizzo i bit "piu' significativ"
					MasterAddress = MasterAddress << 8;			//sposto i bit 7 posti a 'sinistra'
					MasterAddress |= SusiData.MessageByte[1];	//aggiungo i 7 bit "meno significativi"

					notifySusiMasterAddress(MasterAddress);
				}
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
				notifySusiControllModule(SusiData.MessageByte[1]);
			}
			break;
		}


		/* METODI MANIPOLAZIONE CVs */

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
			*	Questo ei due comandi seguenti sono quelli menzionati nella sezione 4
			*	Pacchetti da 3 byte secondo [RCN-214]
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
						CV_Value = SUSI_VER;
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
			* K = 0: Bit Prüfen. Wenn D mit dem Bitzustand an der Bitstelle B der CV
			* übereinstimmt, wird mit einem Acknowledge geantwortet.
			* K = 1: Bit Schreiben. D wird in Bitstelle B der CV geschrieben.
			* Der Slave bestätigt das Schreiben mit einem Acknowledge.
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
					CV_Value = SUSI_VER;
				}
				else {
					if (notifySusiCVRead) {
						CV_Value = notifySusiCVRead(CV_Number);									// Leggo il valore della CV sulla quale manipolare i bit
					}
					else {
						CV_Value = 255;		//Se non e' implementato un sistema di memorizzazione CV utilizzo il valore simbolico di 255
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
						} 
						/* nel caso in cui non e' implementato un sistema di memorizzazione CVs, non faccio nulla
						else { }
						*/
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
			* D = Wert zum Schreiben in die CV. Der Slave bestätigt das Schreiben mit
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

				if (!((CV_Number == 901) || (CV_Number == 941) || (CV_Number == 981))) {
					if (((CV_Number == 900) || (CV_Number == 940) || (CV_Number == 980)) && (SusiData.MessageByte[2] == 8)) {	//si vuole eseguire un Reset delle CVs
						if (notifyCVResetFactoryDefault) {
							notifyCVResetFactoryDefault();

							Data_ACK();
						}
					}
					else {	//scrittura CVs
						if (notifySusiCVWrite) {
							if (notifySusiCVWrite(CV_Number, SusiData.MessageByte[2]) == SusiData.MessageByte[2]) {
								Data_ACK();
							}
						}
						/* nel caso in cui non e' implementato un sistema di memorizzazione CVs, non faccio nulla
						else { }
						*/
					}
				}

				if (CV_Number == 897) {		/* Se e' stato cambiato l'indirizzo dello Slave aggiorno il valore memorizzato */
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