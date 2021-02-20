# Rcn600 SUSI
Questa libreria permette di utilizzare una scheda Arduino (o un microcontrollore tramite Arduino IDE) come Slave per l'interfaccia SUSI.

Libreria testa su Arduino UNO.

ATTENZIONE: Alcune schede Arduino funzionano a 3.3v, le interfacce SUSI possono essere a 5v! 

Per funzionare sono necessari 2 resistori da 470Ω in serie sulle linee SUSI (Clock e Dati); la linea Clock va collegata ad un pin di tipo Interrupt.

Data  ---> 470Ω ---> Pin generico (anche di tipo analogico)
Clock ---> 470Ω ---> Pin Interrupt
