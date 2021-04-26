#ifndef Rcn600Master_Timer_h
#define Rcn600Master_Timer_h

#include <stdint.h>

#include "arduino.h"

/*
*	setTimer() imposta il Timer per creare il Clock
*	Input:
*		- il prescaler del Timer
*	Restituisce:
*		- Nulla
*/
void setTimer(uint8_t prescaler);
/*
*	startTimer() collega il Timer al ISR quando si verifica Overflow del Counter
*	Input:
*		- Nulla
*	Restituisce:
*		- Nulla
*/
void startTimer(void);
/*
*	setTimerRetard() scrive nel registro del Timer un ritardo facendo partire il Timer non da 0 ma dal valore inserito
*	Input:
*		- il numero da cui il Counter inizia il conteggio
*	Restituisce:
*		- Nulla
*/
void setTimerRetard(uint8_t retard);
/*
*	stopTimer() scollega il ISR dal Overflow del Counter
*	Input:
*		- Nulla
*	Restituisce:
*		- Nulla
*/
void stopTimer(void);
/*
*	disableTimer() imposta i registri del Timer ad una condizione di Default
*	Input:
*		- Nulla
*	Restituisce:
*		- Nulla
*/
void disableTimer(void);

#endif