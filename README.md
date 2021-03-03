# Indice
* [Introduzione Rcn600 SUSI](#Rcn600-SUSI)
* [Metodi Della libreria](#Metodi-Della-libreria)
* [Metodi Implementabili a Discrezione dell'utente](#Metodi-Implementabili-a-Discrezione-dell-'-utente)
* [Programmers](#programmers)
* [Write to own flash](#write-to-own-flash)
* **[How to install](#how-to-install)**
  - [Boards Manager Installation](#boards-manager-installation)
  - [Manual Installation](#manual-installation)
  - [PlatformIO](#platformio)
* **[Getting started with MegaCore](#getting-started-with-megacore)**
* [Wiring reference](#wiring-reference)
* **[Pinout](#pinout)**
* **[Minimal setup](#minimal-setup)**

# Rcn600 SUSI
Questa libreria permette di utilizzare una scheda Arduino (o un microcontrollore tramite Arduino IDE) come Slave per l'interfaccia SUSI.<br/>
Libreria testata su Arduino UNO.

**NOTA**: Se attivi i controlli sulla memoria Ram disponibile prima di allocare risorse, e' necessaria la libreria 'freeMemory' disponibile qui: https://github.com/McNeight/MemoryFree

**ATTENZIONE: Alcune schede Arduino funzionano a 3.3v, le interfacce SUSI possono essere a 5v!** 

Per funzionare sono necessari 2 resistori da **470Ω in serie** sulle linee SUSI (Clock e Dati).<br/>
Data  ---> 470Ω ---> Pin generico (anche di tipo analogico)<br/>
Clock ---> 470Ω ---> Pin Interrupt<br/>

Maggiori informazioni della specifiva "*RCN-600.pdf*".

------------

# Metodi Della libreria
```c
Rcn600(uint8_t CLK_pin_i, uint8_t DATA_pin_i);
```
Dichiarazione delle libreria nella quale inserire i pin a cui e' collegato il Bus SUSI.
Il pin CLK **deve essere** di tipo ***Interrupt***, il pin Data *puo'* essere di qualsiasi tipo (compresi analogici).


```c
void init(void);
```
E' necessario invocarlo nel 'setup' del codice: avvia la gestione dell'interrupt e inizializza i contatori interni.


```c
void process(void);
```
*E' necessario*  **invocarlo piu' volte possibile** nel 'loop' del codice: decodifica il pacchetto SUSI.

------------

# Metodi Implementabili a Discrezione dell'utente
```c
void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState);
```
Invocato quando: si ricevono i dati dal Master su un gruppo di funzioni digitali:
* Input: 
 - il gruppo Funzioni decodificato
 - lo stato del gruppo funzioni
* Restituisce:
 - Nulla

------------

```c
void notifySusiBinaryState(uint16_t Command, uint8_t CommandState);
```
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


* A
 - B
* C