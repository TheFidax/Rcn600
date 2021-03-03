# Rcn600 SUSI
Questa libreria permette di utilizzare una scheda Arduino (o un microcontrollore tramite Arduino IDE) come Slave per l'interfaccia SUSI.

Libreria testa su Arduino UNO.

**NOTA**: Se attivi i controlli sulla memoria Ram disponibile prima di allocare risorse, e' necessaria la libreria 'freeMemory' disponibile qui: https://github.com/McNeight/MemoryFree

**ATTENZIONE: Alcune schede Arduino funzionano a 3.3v, le interfacce SUSI possono essere a 5v!** 

Per funzionare sono necessari 2 resistori da **470Ω in serie** sulle linee SUSI (Clock e Dati).

Data  ---> 470Ω ---> Pin generico (anche di tipo analogico)

Clock ---> 470Ω ---> Pin Interrupt

Maggiori informazioni della specifiva "*RCN-600.pdf*".

# Metodi Disponibili nella libreria
`Rcn600(uint8_t CLK_pin_i, uint8_t DATA_pin_i);`
Dichiarazione delle libreria nella quale inserire i pin a cui e' collegato il Bus SUSI.
Il pin CLK **deve essere** di tipo ***Interrupt***, il pin Data *puo'* essere di qualsiasi tipo (compresi analogici).

`void init(void);`
E' necessario invocarlo nel 'setup' del codice: avvia la gestione dell'interrupt e inizializza i contatori interni.

`void process(void);`
*E' necessario*  **invocarlo piu' volte possibile** nel 'loop' del codice: decodifica il pacchetto SUSI.

# Metodi Implementabili a Discrezione dell'utente
`void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState);`
Invocato quando: si ricevono i dati dal Master su un gruppo di funzioni digitali:
* Input :
 - il gruppo Funzioni decodificato
 - lo stato del gruppo funzioni
* Restituisce:
 - Nulla
------------
`void notifySusiBinaryState(uint16_t Command, uint8_t CommandState);`
Invocato quando: si ricevono i dati dal Master sullo stato di UNA specifica funzione:
- Input:
 - il numero della funzione (da 1 a 127)
 - lo stato della Funzione (attiva = 1, disattiva = 0)
- Restituisce:
 - Nulla
------------
`void notifySusiAux(SUSI_AUX_GROUP SUSI_auxGrp, uint8_t SUSI_AuxState);`
viene invocato quando: si ricevono i dati dal Master sullo stato di UNA specifica AUX:
- Input:
 - il numero dell'AUX
 - lo stato dell'uscita (attiva = 1, disattiva = 0)
- Restituisce:
 - Nulla
------------
`void notifySusiTriggerPulse(uint8_t state);`