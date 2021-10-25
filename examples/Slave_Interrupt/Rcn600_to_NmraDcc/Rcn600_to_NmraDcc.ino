// Questo esempio mostra come interfacciare la libreria RCN600 alla libreria NmraDcc per sfruttarne i suoi metodi

#include <NmraDcc.h>
#include <Rcn600.h>

#define This_Decoder_Address 3

Rcn600 SUSI(2, 3);  //Definisco i pin a cui e' collegato il Bus SUSI

/* Metodi Libreria NmraDcc */

// Uncomment the #define below to print all Speed Packets
#define NOTIFY_DCC_SPEED
#ifdef  NOTIFY_DCC_SPEED
void notifyDccSpeed(uint16_t Addr, DCC_ADDR_TYPE AddrType, uint8_t Speed, DCC_DIRECTION Dir, DCC_SPEED_STEPS SpeedSteps) {
    Serial.print("notifyDccSpeed: Addr: ");
    Serial.print(Addr, DEC);
    Serial.print((AddrType == DCC_ADDR_SHORT) ? "-S" : "-L");
    Serial.print(" Speed: ");
    Serial.print(Speed, DEC);
    Serial.print(" Steps: ");
    Serial.print(SpeedSteps, DEC);
    Serial.print(" Dir: ");
    Serial.println((Dir == DCC_DIR_FWD) ? "Forward" : "Reverse");
};
#endif

// Uncomment the #define below to print all Function Packets
#define NOTIFY_DCC_FUNC
#ifdef  NOTIFY_DCC_FUNC
void notifyDccFunc(uint16_t Addr, DCC_ADDR_TYPE AddrType, FN_GROUP FuncGrp, uint8_t FuncState) {
    Serial.print("notifyDccFunc: Addr: ");
    Serial.print(Addr, DEC);
    Serial.print((AddrType == DCC_ADDR_SHORT) ? 'S' : 'L');
    Serial.print("  Function Group: ");
    Serial.print(FuncGrp, DEC);

    switch (FuncGrp) {
    case FN_0_4:
        Serial.print(" FN 0: ");
        Serial.print((FuncState & FN_BIT_00) ? "1  " : "0  ");
        Serial.print(" FN 1-4: ");
        Serial.print((FuncState & FN_BIT_01) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_02) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_03) ? "1  " : "0  ");
        Serial.println((FuncState & FN_BIT_04) ? "1  " : "0  ");
        break;

    case FN_5_8:
        Serial.print(" FN 5-8: ");
        Serial.print((FuncState & FN_BIT_05) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_06) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_07) ? "1  " : "0  ");
        Serial.println((FuncState & FN_BIT_08) ? "1  " : "0  ");
        break;

    case FN_9_12:
        Serial.print(" FN 9-12: ");
        Serial.print((FuncState & FN_BIT_09) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_10) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_11) ? "1  " : "0  ");
        Serial.println((FuncState & FN_BIT_12) ? "1  " : "0  ");
        break;

    case FN_13_20:
        Serial.print(" FN 13-20: ");
        Serial.print((FuncState & FN_BIT_13) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_14) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_15) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_16) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_17) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_18) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_19) ? "1  " : "0  ");
        Serial.println((FuncState & FN_BIT_20) ? "1  " : "0  ");
        break;

    case FN_21_28:
        Serial.print(" FN 21-28: ");
        Serial.print((FuncState & FN_BIT_21) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_22) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_23) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_24) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_25) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_26) ? "1  " : "0  ");
        Serial.print((FuncState & FN_BIT_27) ? "1  " : "0  ");
        Serial.println((FuncState & FN_BIT_28) ? "1  " : "0  ");
        break;
    }
}
#endif

/* FINE Metodi Libreria NmraDcc */


/* Codice Di INTERFACCIAMENTO alla Libreria NmraDcc */
void notifySusiRealSpeed(uint8_t Speed, SUSI_DIRECTION Dir) {
    if (Dir == SUSI_DIR_FWD) {
        notifyDccSpeed(This_Decoder_Address, DCC_ADDR_SHORT, Speed, DCC_DIR_FWD, SPEED_STEP_128);
    }
    else {
        notifyDccSpeed(This_Decoder_Address, DCC_ADDR_SHORT, Speed, DCC_DIR_REV, SPEED_STEP_128);
    }
}

void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState) {
    switch (SUSI_FuncGrp) {
    case SUSI_FN_0_4: {
        notifyDccFunc(This_Decoder_Address, DCC_ADDR_SHORT, FN_0_4, SUSI_FuncState);
        break;
    }
    case SUSI_FN_5_12: {
        notifyDccFunc(This_Decoder_Address, DCC_ADDR_SHORT, FN_5_8, SUSI_FuncState);
        notifyDccFunc(This_Decoder_Address, DCC_ADDR_SHORT, FN_9_12, (SUSI_FuncState >> 4));
        break;
    }
    case SUSI_FN_13_20: {
        notifyDccFunc(This_Decoder_Address, DCC_ADDR_SHORT, FN_13_20, SUSI_FuncState);
        break;
    }
    case SUSI_FN_21_28: {
        notifyDccFunc(This_Decoder_Address, DCC_ADDR_SHORT, FN_21_28, SUSI_FuncState);
        break;
    }
    default: {}
    }
}
/* FINE Codice Di INTERFACCIAMENTO alla Libreria NmraDcc */


void setup() {
	Serial.begin(500000);   // Avvio la comunicazione Seriale
    	Serial.println("Interfacciamento Libreria RCN600 a NmraDcc");

    	SUSI.init();
}

void loop() {
    SUSI.process();
}
