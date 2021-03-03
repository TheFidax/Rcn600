# Indice
* [Introduzione Rcn600 SUSI](#Rcn600-SUSI)
* [Metodi Della Libreria](#Metodi-Della-Libreria)
* [Metodi Implementabili a Scelta](#Metodi-Implementabili-a-Scelta)


# Rcn600 SUSI
Questa libreria permette di utilizzare una scheda Arduino (o un microcontrollore tramite Arduino IDE) come Slave per l'interfaccia SUSI.<br/>
Libreria testata su Arduino UNO.

**NOTA**: Se attivi i controlli sulla memoria Ram disponibile prima di allocare risorse, e' necessaria la libreria 'freeMemory' disponibile qui: https://github.com/McNeight/MemoryFree

**ATTENZIONE: Alcune schede Arduino funzionano a 3.3v, le interfacce SUSI possono essere a 5v!** 

Per funzionare sono necessari 2 resistori da **470Ω in serie** sulle linee SUSI (Clock e Dati).<br/>
Data  ---> 470Ω ---> Pin generico (anche di tipo analogico)<br/>
Clock ---> 470Ω ---> Pin Interrupt<br/>

Maggiori informazioni della specifiva "*RCN-600.pdf*".


# Metodi Della Libreria
```c
Rcn600(uint8_t CLK_pin_i, uint8_t DATA_pin_i);
```
Dichiarazione delle libreria nella quale inserire i pin a cui e' collegato il Bus SUSI.<br/>
Il pin CLK **deve essere** di tipo ***Interrupt***, il pin Data *puo'* essere di qualsiasi tipo (compresi analogici).

------------

```c
void init(void);
```
*E' necessario* invocarlo nel 'setup' del codice: avvia la gestione dell'interrupt e inizializza i contatori interni.

------------

```c
void process(void);
```
**E' necessario invocarlo piu' volte possibile** nel 'loop' del codice: decodifica il pacchetto SUSI.


# Metodi Implementabili a Scelta
```c
void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState);
```
*notifySusiFunc()* viene invocato quando: si ricevono i dati dal Master su un gruppo di funzioni digitali:
* Input:
  - il gruppo Funzioni decodificato
  - lo stato del gruppo funzioni
* Restituisce:
  - Nulla

------------

```c
void notifySusiBinaryState(uint16_t Command, uint8_t CommandState);
```
*notifySusiBinaryState()* viene invocato quando: si ricevono i dati dal Master sullo stato di UNA specifica funzione:
- Input:
  - il numero della funzione (da 1 a 127)
  - lo stato della Funzione (attiva = 1, disattiva = 0)
- Restituisce:
  - Nulla

------------

```c
void notifySusiAux(SUSI_AUX_GROUP SUSI_auxGrp, uint8_t SUSI_AuxState);
```
*notifySusiAux()* viene invocato quando: si ricevono i dati dal Master sullo stato di UNA specifica AUX:
- Input:
  - il numero dell'AUX
  - lo stato dell'uscita (attiva = 1, disattiva = 0)
- Restituisce:
  - Nulla

------------

```c
void notifySusiTriggerPulse(uint8_t state);
```
notifySusiTriggerPulse() viene invocato quando: si riceve dal Master il comando di Trigger (o pulsazione) per eventuali sbuffi di vapore
- Input:
  - stato del comando Trigger/Pulse
- Restituisce:
  - Nulla

------------

```c
void notifySusiMotorCurrent(int current);
```
*notifySusiMotorCurrent()* viene invocato quando: si riceve dal Master i dati sull'assorbimento di Corrente da parte del Motore
- Input:
  - Assorbimento di Corrente: da -128 a + 127 (gia' convertita dal Complemento a 2 originale)
- Restituisce:
  - Nulla

------------

```c
void notifySusiRequestSpeed(uint8_t Speed, SUSI_DIRECTION Dir);
```
*notifySusiRequestSpeed()* viene invocato quando: si ricevono i dati sulla Velocita' e sulla Direzione richiesti dalla Centrale al Master
- Input:
  - la velocita' (128 step) richiesta
  - la direzione richiesta
- Restituisce:
  - Nulla

------------

```c
void notifySusiRealSpeed(uint8_t Speed, SUSI_DIRECTION Dir);
```
*notifySusiRealSpeed()* viene invocato quando: si ricevono i dati dal Master sulla Velocita' e sulla Direzione reali 
- Input:
  - la velocita' (128 step) reale
  - la direzione reale
- Restituisce:
  - Nulla

------------

```c
void notifySusiMotorLoad(int load);
```
*notifySusiAnalogFunction()* viene invocato quando: si riceve dal Master i dati sul carico del Motore
- Input:
  - Carico del Motore: da -128 a + 127 (gia' convertita dal Complemento a 2 originale)
- Restituisce:
  - Nulla

------------

```c
void notifySusiAnalogFunction(SUSI_AN_GROUP SUSI_AnalogGrp, uint8_t SUSI_AnalogState);
```
*notifySusiAnalogFunction()* viene invocato quando: si ricevono i dati dal Master su un gruppo di funzioni analogiche
- Input:
  - il gruppo Analogico decodificato
  - lo stato del gruppo
- Restituisce:
  - Nulla

------------

```c
void notifySusiAnalogDirectCommand(uint8_t commandNumber, uint8_t Command);
```
*notifySusiAnalogDirectCommand()* viene invocato quando: si ricevono i dati dal Master i comandi diretti per il funzionamento analogico
- Input:
  - il numero del comando: 1 o 2
  - i bit del comando
- Restituisce:
  - Nulla

------------

```c
void notifySusiMasterAddress(uint16_t MasterAddress);
```
*notifySusiMasterAddress()* viene invocato quando: si riceve l'indirizzo digitale del Master
- Input:
  - l'indirizzo Digitale del Master
- Restituisce:
  - Nulla

------------

```c
void notifySusiControllModule(uint8_t ModuleControll);
```
*notifySusiControlModule()* viene invocato quando: si riceve il comando sul controllo del modulo
- Input:
  - byte contenete il controllo del modulo
- Restituisce:
  - Nulla