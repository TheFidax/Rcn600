# Indice
* [Introduzione Rcn600 SUSI](#Rcn600-SUSI)
* [Metodi Obbligatori](#Metodi-Obbligatori)
* [Metodi Facoltativi](#Metodi-Facoltativi)
* [Metodi Manipolazione CVs](#Metodi-Manipolazione-CVs)
* [Tipi di Dati](#Tipi-di-Dati)
* [Funzionalita' Extra](#Funzionalita-Extra)

------------

# Rcn600 SUSI
Questa libreria permette di utilizzare una scheda Arduino (o un microcontrollore tramite Arduino IDE) come Slave per l'interfaccia SUSI.<br/>
Libreria testata su Arduino UNO.

**ATTENZIONE: Alcune schede Arduino funzionano a 3.3v, le interfacce SUSI possono essere a 5v!** 

Per funzionare sono necessari 2 resistori da **470Ω in serie** sulle linee SUSI (Clock e Dati).<br/>
Data  ---> 470Ω ---> Pin generico (anche di tipo analogico)<br/>
Clock ---> 470Ω ---> Pin Interrupt<br/>

Maggiori informazioni della specifiva nel fle "*RCN-600.pdf*".


**Video Presentazione del Progetto:**</br>

[![Video Presentazione](https://img.youtube.com/vi/VzgkDouOvCY/0.jpg)](http://www.youtube.com/watch?v=VzgkDouOvCY)

------------

# Metodi Obbligatori
I seguenti metodi sono **obbligatori** per il corretto funzionamento della libreria.

------------

```c
Rcn600(uint8_t CLK_pin_i, uint8_t DATA_pin_i);
```
Dichiarazione delle libreria nella quale inserire i pin a cui e' collegato il Bus SUSI.<br/>
Il pin CLK **deve essere** di tipo ***Interrupt***, il pin Data *puo'* essere di qualsiasi tipo (compresi analogici).

------------

```c
void init(void);
```
*E' necessario* invocarlo nel 'setup' del codice: avvia la gestione dell'interrupt e inizializza i contatori interni.</br>
**N.B.** Se e' presente il metodo 'notifySusiCVRead' la libreria leggera' la CV contenente l'indirizzo dello Slave SUSI, in caso contrario utilizzerà il valore di *default*: 1.</br>

------------

```c
void init(uint8_t SlaveAddress);
```
*E' necessario* invocarlo nel 'setup' del codice: avvia la gestione dell'interrupt e inizializza i contatori interni.</br>
*Permette di specificare l'indirizzo del modulo*, **PUO' AVERE VALORE**: 1, 2, 3. In caso di valore diverso verra' utilizzato il valore di *default*: 1.</br>

------------

```c
void process(void);
```
**E' necessario invocarlo piu' volte possibile** nel 'loop' del codice: decodifica il pacchetto SUSI.

------------

# Metodi Facoltativi
I seguenti metodi sono **facoltativi** (definiti come 'extern' alla libreria), e permettono all'utente di definire il comportamento da adottare in caso di un particolare comando.</br>

------------

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

# Metodi Manipolazione CVs
I seguenti metodi sono **facoltativi** (definiti come 'extern' alla libreria), ma permettono alla libreria di dialogare con il Decoder Master in caso di *Lettura/Scrittura CVs*</br>
La libreria **gestisce l'ACK** che permette al decoder di conoscere l'esito dell'operazione richiesta.

------------

```c
uint8_t notifySusiCVRead(uint16_t CV);
```
*notifySusiCVRead()* viene invocato quando: e' richiesta la lettura di una CV
- Input:
  - il numero della CV da leggere
- Restituisce:
  - il valore della CV letta

------------

```c
uint8_t notifySusiCVWrite(uint16_t CV, uint8_t Value);
```
*notifySusiCVWrite()* viene invocato quando: e' richiesta la Scrittura di una CV.
- Input:
  - il numero della CV richiesta
  - il Nuovo valore della CV
- Restituisce:
  - il valore letto (post scrittura) della CV da scrivere

------------

RESET CVs, viene utilizzato lo *stesso metodo* della Libreria [NmraDcc](https://github.com/mrrwa/NmraDcc):</br>
```c
void notifyCVResetFactoryDefault(void);
```
*notifyCVResetFactoryDefault()* Called when CVs must be reset. This is called when CVs must be reset to their factory defaults.
- Inputs:
  - None                                                                                                       
- Returns:
  - None


# Tipi di Dati
I seguenti tipi di dati vengono utilizzati dai metodi della libreria.
```c
SUSI_DIRECTION
```
Tipo *enum*, identifica *simbolicamente* la direzione trasmessa dal Decoder Master: </br>
- SUSI_DIR_REV : Direzione *reverse*
- SUSI_DIR_FWD : Direzione *forward*

------------

```c
SUSI_FN_GROUP
```
Tipo *enum*, identifica *simbolicamente* il gruppo di Funzioni Digitali trasmesse dal Decoder Master:</br>
- SUSI_FN_0_4 : Funzioni dalla 0 alla 4
- SUSI_FN_5_12 : Funzioni dalla 5 alla 12
- SUSI_FN_13_20 : Funzioni dalla 13 alla 20
- SUSI_FN_21_28 : Funzioni dalla 21 alla 28
- SUSI_FN_29_36 : Funzioni dalla 29 alla 36
- SUSI_FN_37_44 : Funzioni dalla 37 alla 44
- SUSI_FN_45_52 : Funzioni dalla 45 alla 52
- SUSI_FN_53_60 : Funzioni dalla 53 alla 60
- SUSI_FN_61_68 : Funzioni dalla 61 alla 68

------------

```c
SUSI_AUX_GROUP
```
Tipo *enum*, identifica *simbolicamente* il gruppo di AUXs trasmesse dal Decoder Master:</br>
- SUSI_AUX_1_8 : AUX dalla 1 alla 8
- SUSI_AUX_9_16 : AUX dalla 9 alla 16
- SUSI_AUX_17_24 : AUX dalla 17 alla 24
- SUSI_AUX_25_32 : AUX dalla 25 alla 32

------------

```c
SUSI_AN_FN_GROUP
```
Tipo *enum*, identifica *simbolicamente* il gruppo di Funzioni Analogiche trasmesse dal Decoder Master:</br>
- SUSI_AN_FN_0_7 : Funzioni Analogiche dalla 0 alla 7
- SUSI_AN_FN_8_15 : Funzioni Analogiche dalla 8 alla 15
- SUSI_AN_FN_16_23 : Funzioni Analogiche dalla 16 alla 23
- SUSI_AN_FN_24_31 : Funzioni Analogiche dalla 24 alla 31
- SUSI_AN_FN_32_39 : Funzioni Analogiche dalla 32 alla 39
- SUSI_AN_FN_40_47 : Funzioni Analogiche dalla 40 alla 47
- SUSI_AN_FN_48_55 : Funzioni Analogiche dalla 48 alla 55
- SUSI_AN_FN_56_63 : Funzioni Analogiche dalla 56 alla 63

------------

# Funzionalita' Extra
Le seguenti funzionalita' vengono comandate *manualmente* agendo su delle #define nel file **Rcn600.h**

```c
#define	NOTIFY_RAW_MESSAGE
```
Questa define rende disponibile un metodo che permette di *esportare* il messaggio *grezzo* per poterlo manipolare in maniera Esterna alla libreria.</br>
Stato di *default*: **Disattivata**</br>
```c
void notifySusiRawMessage(uint8_t *rawMessage, uint8_t messageLength);
```
*notifySusiRawMessage()* viene invocato ogni volta che è presente un messaggio da decodificare
- Input:
  - i byte che compongono il messaggio acquisito dalla libreria
  - la lunghezza del messaggio
- Restituisce:
  - Nulla
