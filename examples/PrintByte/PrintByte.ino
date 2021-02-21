/*
Questo esempio permette di vedere, tramite seriale, il gruppo di Byte (con i rispettivi valori) ricveuti dal Master.
Per eseguire tale operazione E' INDISPENSABILE ABILITARE LA MODALITA' DI NOTIFY_RAW_MESSAGE NEL FILE Rcn600.h 
*/
#include <Rcn600.h>			// Includo la libreria per la gestione della SUSI
#include <EEPROM.h>

Rcn600 SUSI(2, 3);			// (CLK pin, DATA pin) il pin di Clock DEVE ESSERE di tipo interrupt, il pin Data puo' essere in pin qualsiasi: compresi gli analogici

uint8_t notifySusiCVRead(uint16_t CV){
  return EEPROM.read(CV);
}

uint8_t notifySusiCVWrite(uint16_t CV, uint8_t Value){
  EEPROM.update(CV, Value);

  return EEPROM.read(CV);
}

void notifySusiRawMessage(uint8_t *rawMessage, uint8_t messageLength) {
  static uint8_t i;
  
  Serial.print("notifySusiRawMessage : ");
  
  for(i = 0; i < messageLength; ++i) {
    Serial.print(rawMessage[i], BIN);
    if(i != messageLength) {
      Serial.print(" - ");
    }
  }

  Serial.print(" ( ");

  for(i = 0; i < messageLength; ++i) {
    Serial.print(rawMessage[i]);
    if(i != messageLength) {
      Serial.print(" - ");
    }
  }

  Serial.println(" )");
}

void setup() {  
  Serial.begin(115200);
  while(!Serial) {}
   
	SUSI.init();			// Avvio la libreria
}

void loop() {
	SUSI.process();			// Elaboro pi� volte possibile i dati acquisiti
}
