#include "FastIOMode.h"

void pinModeFastInput(volatile uint8_t* PortModeReg, volatile uint8_t* PortOutputReg, uint8_t bitMask) {
	*PortModeReg &= ~bitMask;
	*PortOutputReg &= ~bitMask;
}

void pinModeFastOutput(volatile uint8_t* PortModeReg, uint8_t bitMask) {
	*PortModeReg |= bitMask;
}

void digitalWriteFastHigh(volatile uint8_t* PortOutputReg, uint8_t bitMask) {
	*PortOutputReg |= bitMask;
}

void digitalWriteFastLow(volatile uint8_t* PortOutputReg, uint8_t bitMask) {
	*PortOutputReg &= ~bitMask;
}

uint8_t digitalReadFast(volatile uint8_t* PortInputReg, uint8_t bitMask) {
	return *PortInputReg & bitMask;
}