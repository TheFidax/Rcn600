#ifndef FastIOMode_h
#define FastIOMode_h

#include <stdint.h>

/* Funzioni di controllo IO che ricevono in input porta e registro del pin, bypassando tutti i controlli nativi della libreria Wiring */

void pinModeFastInput(volatile uint8_t* PortModeReg, volatile uint8_t* PortOutputReg, uint8_t bitMask);    // Imposta un pin come Input


void pinModeFastOutput(volatile uint8_t * PortModeReg, uint8_t bitMask);                                   // Imposta un pin come Output


void digitalWriteFastHigh(volatile uint8_t* PortOutputReg, uint8_t bitMask);                               // Imposta il pin al valore HIGH


void digitalWriteFastLow(volatile uint8_t * PortOutputReg, uint8_t bitMask);                               // Imposta il pin al valore LOW


uint8_t digitalReadFast(volatile uint8_t * PortInputReg, uint8_t bitMask);                                 // Restituisce il valore del pin

#endif