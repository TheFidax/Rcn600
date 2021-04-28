#include "Rcn600Master_Timer.h"

void setTimer(uint8_t prescaler) {
	TCCR2A = 0;				//'normal mode': solo incremento
	TCCR2B = prescaler;		//prescaler
	TCNT2 = 0;
}

void startTimer(void) {
	//TIMSK2 = 0x01;
	bitWrite(TIMSK2, TOIE2, 1);
}

void setTimerRetard(uint8_t retard) {
	TCNT2 += retard;
}

void stopTimer(void) {
	TIMSK2 = 0;
}

void disableTimer(void) {
	TIMSK2 = 0;
	TCCR2B = 0;
}