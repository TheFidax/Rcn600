/*
Questo esempio stampa tutti i dati decodificati dalla libreria
*/

#include <Rcn600.h>     // Includo la libreria per la gestione della SUSI
#include <EEPROM.h>     // Includo la libreria per la gestione della EEPROM interna

Rcn600 SusiSlave(2, 3);      // (CLK pin, DATA pin) il pin di Clock DEVE ESSERE di tipo interrupt, il pin Data puo' essere in pin qualsiasi: compresi gli analogici

// Decommentare la #define sotto per stampare lo stato delle Funzioni Digitali
#define NOTIFY_SUSI_FUNC
#ifdef  NOTIFY_SUSI_FUNC
void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState) {
  Serial.print("notifySusiFunc: ");

  switch( SUSI_FuncGrp ) {
     case SUSI_FN_0_4:
       Serial.print(" FN 0: ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_00) ? "1  ": "0  ");
       Serial.print("FN 1-4: ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_01) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_02) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_03) ? "1  ": "0  ");
       Serial.println((SUSI_FuncState & SUSI_FN_BIT_04) ? "1  ": "0  ");
       break;
    
     case SUSI_FN_5_12:
       Serial.print(" FN 5-12: ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_05) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_06) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_07) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_08) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_09) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_10) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_11) ? "1  ": "0  ");
       Serial.println((SUSI_FuncState & SUSI_FN_BIT_12) ? "1  ": "0  ");
       break;

     case SUSI_FN_13_20:
       Serial.print(" FN 13-20: ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_13) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_14) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_15) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_16) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_17) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_18) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_19) ? "1  ": "0  ");
       Serial.println((SUSI_FuncState & SUSI_FN_BIT_20) ? "1  ": "0  ");
       break;
  
     case SUSI_FN_21_28:
       Serial.print(" FN 21-28: ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_21) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_22) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_23) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_24) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_25) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_26) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_27) ? "1  ": "0  ");
       Serial.println((SUSI_FuncState & SUSI_FN_BIT_28) ? "1  ": "0  ");
       break;  
     
     case SUSI_FN_29_36:
       Serial.print(" FN 29-36: ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_29) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_30) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_31) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_32) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_33) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_34) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_35) ? "1  ": "0  ");
       Serial.println((SUSI_FuncState & SUSI_FN_BIT_36) ? "1  ": "0  ");
       break;  

     case SUSI_FN_37_44:
       Serial.print(" FN 37-44: ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_37) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_38) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_39) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_40) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_41) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_42) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_43) ? "1  ": "0  ");
       Serial.println((SUSI_FuncState & SUSI_FN_BIT_44) ? "1  ": "0  ");
       break;  

     case SUSI_FN_45_52:
       Serial.print(" FN 45-52: ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_45) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_46) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_47) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_48) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_49) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_50) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_51) ? "1  ": "0  ");
       Serial.println((SUSI_FuncState & SUSI_FN_BIT_52) ? "1  ": "0  ");
       break;  

     case SUSI_FN_53_60:
       Serial.print(" FN 53-60: ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_53) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_54) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_55) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_56) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_57) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_58) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_59) ? "1  ": "0  ");
       Serial.println((SUSI_FuncState & SUSI_FN_BIT_60) ? "1  ": "0  ");
       break; 

     case SUSI_FN_61_68:
       Serial.print(" FN 61-68: ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_61) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_62) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_63) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_64) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_65) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_66) ? "1  ": "0  ");
       Serial.print((SUSI_FuncState & SUSI_FN_BIT_67) ? "1  ": "0  ");
       Serial.println((SUSI_FuncState & SUSI_FN_BIT_68) ? "1  ": "0  ");
       break; 
   }
}
#endif

// Decommentare la #define sotto per stampare quando e' comandata una singola funzione
#define NOTIFY_SUSI_BINARY_STATE
#ifdef  NOTIFY_SUSI_BINARY_STATE
void notifySusiBinaryState(uint16_t Command, uint8_t CommandState) {
  Serial.print("notifySusiBinaryState: ");
  Serial.print(Command);
  Serial.print(" ; State: ");
  Serial.println(CommandState);
};
#endif

// Decommentare la #define sotto per stampare lo stato delle AUX comandate direttamente dal Master
#define NOTIFY_SUSI_AUX
#ifdef  NOTIFY_SUSI_AUX
void notifySusiAux(SUSI_AUX_GROUP SUSI_auxGrp, uint8_t SUSI_AuxState) {
  Serial.print("notifySusiAux: ");

  switch( SUSI_auxGrp ) {
     case SUSI_AUX_1_8:
       Serial.print(" AUX 1-8: ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_01) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_02) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_03) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_04) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_05) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_06) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_07) ? "1  ": "0  ");
       Serial.println((SUSI_AuxState & SUSI_AUX_BIT_08) ? "1  ": "0  ");
       break;
    
     case SUSI_AUX_9_16:
       Serial.print(" AUX 9-16: ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_09) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_10) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_11) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_12) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_13) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_14) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_15) ? "1  ": "0  ");
       Serial.println((SUSI_AuxState & SUSI_AUX_BIT_16) ? "1  ": "0  ");
       break;

     case SUSI_AUX_17_24:
       Serial.print(" AUX 17-24: ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_17) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_18) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_19) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_20) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_21) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_22) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_23) ? "1  ": "0  ");
       Serial.println((SUSI_AuxState & SUSI_AUX_BIT_24) ? "1  ": "0  ");
       break;
  
     case SUSI_AUX_25_32:
       Serial.print(" AUX 25-32: ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_25) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_26) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_27) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_28) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_29) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_30) ? "1  ": "0  ");
       Serial.print((SUSI_AuxState & SUSI_AUX_BIT_31) ? "1  ": "0  ");
       Serial.println((SUSI_AuxState & SUSI_AUX_BIT_32) ? "1  ": "0  ");
       break;  
   }
}
#endif

// Decommentare la #define sotto per stampare quando e' richiesto un Trigger / Pulsazione
#define NOTIFY_SUSI_TRIGGER_PULSE
#ifdef  NOTIFY_SUSI_TRIGGER_PULSE
void notifySusiTriggerPulse(uint8_t state) {
  Serial.print("notifySusiTriggerPulse: ");
  Serial.println((state & 1) ? "Pulse!  ": "Not Pulse  ");
};
#endif

// Decommentare la #define sotto per stampare il consumo di corrente del motore
#define NOTIFY_SUSI_MOTOR_CURRENT
#ifdef  NOTIFY_SUSI_MOTOR_CURRENT
void notifySusiMotorCurrent(int current) {
  Serial.print("notifySusiCurrentAbsorption: ");
  Serial.println(current,DEC);
};
#endif

// Decommentare la #define sotto per stampare i dati della Velocita' Richiesta dalla Centrale al Decoder
#define NOTIFY_SUSI_REQUEST_SPEED
#ifdef  NOTIFY_SUSI_REQUEST_SPEED
void notifySusiRequestSpeed(uint8_t Speed, SUSI_DIRECTION Dir) {
  Serial.print("notifySusiRequestSpeed: ");
  Serial.print(" Speed: ");
  Serial.print(Speed,DEC);
  Serial.print(" Dir: ");
  Serial.println( (Dir == SUSI_DIR_FWD) ? "Forward" : "Reverse" );
};
#endif

// Decommentare la #define sotto per stampare i dati della Velocita' Reale
#define NOTIFY_SUSI_REAL_SPEED
#ifdef  NOTIFY_SUSI_REAL_SPEED
void notifySusiRealSpeed(uint8_t Speed, SUSI_DIRECTION Dir) {
  Serial.print("notifySusiRealSpeed: ");
  Serial.print(" Speed: ");
  Serial.print(Speed,DEC);
  Serial.print(" Dir: ");
  Serial.println( (Dir == SUSI_DIR_FWD) ? "Forward" : "Reverse" );
};
#endif

// Decommentare la #define sotto per stampare il Carico del motore
#define NOTIFY_SUSI_MOTOR_LOAD
#ifdef  NOTIFY_SUSI_MOTOR_LOAD
void notifySusiMotorLoad(int load) {
  Serial.print("notifySusiMotorLoad: ");
  Serial.println(load,DEC);
};
#endif

// Decommentare la #define sotto per stampare lo stato dei comandi analogici
#define NOTIFY_SUSI_ANALOG_FUNCTION
#ifdef  NOTIFY_SUSI_ANALOG_FUNCTION
void notifySusiAnalogFunction(SUSI_AN_GROUP SUSI_AnalogGrp, uint8_t SUSI_AnalogState) {
  Serial.print("notifySusiAnalog: ");
  switch( SUSI_AnalogGrp ) {
     case SUSI_AN_FN_0_7:
       Serial.print(" AN 0 - 7 : ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_00) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_01) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_02) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_03) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_04) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_05) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_06) ? "1  ": "0  ");
       Serial.println((SUSI_AnalogState & SUSI_AN_FN_BIT_07) ? "1  ": "0  ");
       break;
    
     case SUSI_AN_FN_8_15:
       Serial.print(" FN 8-15: ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_08) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_09) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_10) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_11) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_12) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_13) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_14) ? "1  ": "0  ");
       Serial.println((SUSI_AnalogState & SUSI_AN_FN_BIT_15) ? "1  ": "0  ");
       break;

     case SUSI_AN_FN_16_23:
       Serial.print(" FN 16-23: ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_16) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_17) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_18) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_19) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_20) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_21) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_22) ? "1  ": "0  ");
       Serial.println((SUSI_AnalogState & SUSI_AN_FN_BIT_23) ? "1  ": "0  ");
       break;
  
     case SUSI_AN_FN_24_31:
       Serial.print(" FN 24-31: ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_24) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_25) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_26) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_27) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_28) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_29) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_30) ? "1  ": "0  ");
       Serial.println((SUSI_AnalogState & SUSI_AN_FN_BIT_31) ? "1  ": "0  ");
       break;  
     
     case SUSI_AN_FN_32_39:
       Serial.print(" FN 32-39: ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_32) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_33) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_34) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_35) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_36) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_37) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_38) ? "1  ": "0  ");
       Serial.println((SUSI_AnalogState & SUSI_AN_FN_BIT_39) ? "1  ": "0  ");
       break;  

     case SUSI_AN_FN_40_47:
       Serial.print(" FN 40-47: ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_40) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_41) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_42) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_43) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_44) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_45) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_46) ? "1  ": "0  ");
       Serial.println((SUSI_AnalogState & SUSI_AN_FN_BIT_47) ? "1  ": "0  ");
       break;  

     case SUSI_AN_FN_48_55:
       Serial.print(" FN 48-55: ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_48) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_49) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_50) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_51) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_52) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_53) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_54) ? "1  ": "0  ");
       Serial.println((SUSI_AnalogState & SUSI_AN_FN_BIT_55) ? "1  ": "0  ");
       break;  

     case SUSI_AN_FN_56_63:
       Serial.print(" FN 56-63: ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_56) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_57) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_58) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_59) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_60) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_61) ? "1  ": "0  ");
       Serial.print((SUSI_AnalogState & SUSI_AN_FN_BIT_62) ? "1  ": "0  ");
       Serial.println((SUSI_AnalogState & SUSI_AN_FN_BIT_63) ? "1  ": "0  ");
       break; 
   }
}
#endif

// Decommentare la #define sotto per stampare i comandi 'Diretti Analogici'
#define NOTIFY_SUSI_ANALOG_DIRECT_COMMAND
#ifdef  NOTIFY_SUSI_ANALOG_DIRECT_COMMAND
void notifySusiAnalogDirectCommand(uint8_t commandNumber, uint8_t Command) {
  Serial.print("notifySusiAnalogDirectCommand: ");
  Serial.print(" Command Number: ");
  Serial.print(commandNumber);
  Serial.print(" Command: ");
  Serial.println(Command);
};
#endif

// Decommentare la #define sotto per stampare i comandi 'No Operation': serve a scopo di test
#define NOTIFY_SUSI_NO_OPERATION
#ifdef  NOTIFY_SUSI_NO_OPERATION
void notifySusiNoOperation(uint8_t commandArgument) {
  Serial.print("notifySusiNoOperation: ");
  Serial.print(" commandArgument: ");
  Serial.println(commandArgument);
};
#endif

// Decommentare la #define sotto per stampare l'indirizzo del Decoder Master
#define NOTIFY_SUSI_MASTER_ADDRESS
#ifdef  NOTIFY_SUSI_MASTER_ADDRESS
void notifySusiMasterAddress(uint16_t MasterAddress) {
  Serial.print("notifySusiMasterAddress: ");
  Serial.println(MasterAddress);
};
#endif

// Decommentare la #define sotto per stampare i comandi 'Controllo Modulo'
#define NOTIFY_SUSI_MODULE_CONTROLL
#ifdef  NOTIFY_SUSI_MODULE_CONTROLL
void notifySusiControllModule(uint8_t ModuleControll) {
  Serial.print("notifySusiControllModule: ");
  Serial.println(ModuleControll);
};
#endif

void setup() {   
  Serial.begin(115200); // Avvio la comunicazione Seriale

  while(!Serial) {}   // Attendo che la comunicazione seriale sia disponibile
  
  SusiSlave.init();      // Avvio la libreria

  Serial.println("SUSI PrintMessage:"); //Informo l'utente che e' pronto a leggere i Byte
}

void loop() {
  SusiSlave.process();     // Elaboro più volte possibile i dati acquisiti
}

/* Metodi Per la Manipolazione delle CV */
// Decommentare la #define sotto per mostrare il comando di Confronto CV
#define NOTIFY_SUSI_CV_READ
uint8_t notifySusiCVRead(uint16_t CV){
#ifdef  NOTIFY_SUSI_CV_READ
  Serial.print("notifySusiCVRead: ");
  Serial.print(" CV: ");
  Serial.print(CV,DEC);
  Serial.print(" Read Value: ");
  Serial.println(EEPROM.read(CV),DEC);
#endif

  return EEPROM.read(CV);
}

// Decommentare la #define sotto per mostrare il comando di Scrittura CV
#define NOTIFY_SUSI_WRITE
uint8_t notifySusiCVWrite(uint16_t CV, uint8_t Value){
#ifdef  NOTIFY_SUSI_WRITE
  Serial.print("notifySusiCVWrite: ");
  Serial.print(" CV: ");
  Serial.print(CV,DEC);
  Serial.print(" Value to Check: ");
  Serial.print(Value,DEC);
#endif

  EEPROM.update(CV, Value);
#ifdef  NOTIFY_SUSI_WRITE 
  Serial.print(" New CV Value: ");
  Serial.println(EEPROM.read(CV),DEC);
#endif

  return EEPROM.read(CV);
}
