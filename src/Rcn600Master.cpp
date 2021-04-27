#include "Rcn600Master.h"

Rcn600Master* pointerToRcn600Master; 

ISR(TIMER2_OVF_vect) { // define global handler
	pointerToRcn600Master->ISR_SUSI(); // calls class member handler
}

void Rcn600Master::printBuffer(void) {
	for (uint8_t i = 0; i < BUFFER_LENGTH; ++i) {
		Serial.print(i); Serial.print(": "); 
		Serial.print(_Buffer[i].Bytes[0]); Serial.print(" - "); 
		Serial.print(_Buffer[i].Bytes[1]); Serial.print(" - "); 
		Serial.print(_Buffer[i].Bytes[2]); Serial.print(" ; "); 
		Serial.print(_Buffer[i].isCvManipulating); Serial.print(" ; "); 
		Serial.println(_Buffer[i].sent);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Rcn600Master::Rcn600Master(uint8_t CLK_pin_i, uint8_t DATA_pin_i) {
#ifdef __AVR__
	_CLK_pin = new digitalPinFast(CLK_pin_i);
	_DATA_pin = new digitalPinFast(DATA_pin_i);
#else
	_CLK_pin = CLK_pin_i;
	_DATA_pin = DATA_pin_i;
#endif // __AVR__
}


Rcn600Master::~Rcn600Master(void) {
	/* Disabilito il Timer */
	disableTimer();

#ifdef __AVR__
	delete _CLK_pin;
	delete _DATA_pin;
#else
	pinMode(_CLK_pin, INPUT);
	pinMode(_DATA_pin, INPUT);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Rcn600Master::init(void) {
	pointerToRcn600Master = this;

	for (uint8_t i = 0; i < BUFFER_LENGTH; ++i) {
		_Buffer[i].sent = true;
	}

#ifdef __AVR__
	_CLK_pin->pinModeFast(OUTPUT);
	_DATA_pin->pinModeFast(OUTPUT);
#else
	pinMode(_CLK_pin, OUTPUT);
	pinMode(_DATA_pin, OUTPUT);
#endif

	modeISR = ClockMode;

	/* Inizializzo il Timer2 per l'utilizzo */
	setTimer(TIMER_PRESCALER);
	startTimer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __AVR__
void Rcn600Master::writeLine(digitalPinFast *fastPin, uint8_t bit) {
	fastPin->digitalWriteFast(bit);
}
#else
void Rcn600Master::writeLine(uint8_t Pin, uint8_t bit) {
	digitalWrite(Pin, bit);
}
#endif

void Rcn600Master::ISR_SUSI(void) {
	switch (modeISR)
	{
	case ClockMode:
		ISR_Clock();
		setTimerRetard(TIMER_RETARD);
		break;
	case SyncMode:
		ISR_Sync();
		break;
	}
}

void Rcn600Master::ISR_Clock(void) {
	static uint8_t ClockState = HIGH;
	static uint8_t bitCounter = 0;		// indica quale bit si deve leggere
	static uint8_t messageCounter = 0;	// indica quale Messaggio sta venendo spedito

	if (ClockState == HIGH) {
		/* Scrivo il valore del bit sulla linea DATA sul fronte di salita */
		if (_Buffer[messageCounter].sent == false) {
			writeLine(_DATA_pin, bitRead(_Buffer[messageCounter].Bytes[bitCounter / 8], (bitCounter%8)));
			//Serial.print(bitRead(_Buffer[messageCounter].Bytes[bitCounter / 8], (bitCounter%8)));
			//Serial.print(" ("); Serial.print(bitCounter / 8); Serial.println(")");
		}
		else {
			writeLine(_DATA_pin, 0);
		}
	}
	else {	//sono sul fronte di discesa, aggiorno i contatori 
		++bitCounter;

		if ((bitCounter % 8) == 0) {
			//Serial.println(_Buffer[messageCounter].Bytes[((bitCounter - 1) / 8)]);
			if (bitCounter == 16) {
				if (!_Buffer[messageCounter].isCvManipulating) {
					_Buffer[messageCounter].sent = true,
					bitCounter = 0;
					++messageCounter;

					if ((messageCounter % MESSAGES_BEFORE_SYNC) == 0) {
						modeISR = SyncMode;

						setTimer(0x07);
						setTimerRetard(90);
					}
				}
			}
			else if (bitCounter == 24) {

			}

			if (messageCounter == BUFFER_LENGTH) {
				messageCounter = 0;
			}
		}
	}

	/* Genero il Clock */
	writeLine(_CLK_pin, ClockState);

	// Cambio lo stato del Clock
	ClockState = !ClockState;
}

void Rcn600Master::ISR_Sync(void) {
	modeISR = ClockMode;
	//Serial.println("sync");
	setTimer(TIMER_PRESCALER);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Rcn600Master::writeBuffer(bool isCvManipulating, uint8_t firstByte, uint8_t secondByte, uint8_t thirdByte) {
	/* Questo metodo inserisce nel buffer un comando */

	for(uint8_t i = 0; i < BUFFER_LENGTH; ++i) {
		if (_Buffer[i].sent == true) {	// struct libera per nuovo comando
			_Buffer[i].isCvManipulating = isCvManipulating;
			_Buffer[i].Bytes[0] = firstByte;
			_Buffer[i].Bytes[1] = secondByte;
			_Buffer[i].Bytes[2] = thirdByte;
			_Buffer[i].sent = false;

			return true;
		}
	}

	// Se arrivo qui non sono riuscito a trovare uno spazio libero, ritorno l'Errore
	if (notifySusiBufferFull) {
		notifySusiBufferFull();
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Rcn600Master::sendSusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState) {
	uint8_t commandByte;

	switch (SUSI_FuncGrp)
	{
	case SUSI_FN_0_4: {
		commandByte = 96;
		break;
	}
	case SUSI_FN_5_12: {
		commandByte = 97;
		break;
	}
	case SUSI_FN_13_20: {
		commandByte = 98;
		break;
	}
	case SUSI_FN_21_28: {
		commandByte = 99;
		break;
	}
	case SUSI_FN_29_36: {
		commandByte = 100;
		break;
	}
	case SUSI_FN_37_44: {
		commandByte = 101;
		break;
	}
	case SUSI_FN_45_52: {
		commandByte = 102;
		break;
	}
	case SUSI_FN_53_60: {
		commandByte = 103;
		break;
	}
	case SUSI_FN_61_68: {
		commandByte = 104;
		break;
	}
	default:
		return -1;
	}

	return writeBuffer(false, commandByte, SUSI_FuncState);
}

bool Rcn600Master::sendSusiRealSpeed(uint8_t realSpeed, SUSI_DIRECTION dir) {
	uint8_t command = realSpeed;

	bitWrite(command, 7, (uint8_t)dir);

	return writeBuffer(false, 36, command);
}

bool Rcn600Master::sendSusiRequestSpeed(uint8_t requestSpeed, SUSI_DIRECTION dir) {
	uint8_t command = requestSpeed;

	bitWrite(command, 7, (uint8_t)dir);

	return writeBuffer(false, 37, command);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////