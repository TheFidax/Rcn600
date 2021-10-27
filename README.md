**!!! README PER 1.5.0 IN LAVORAZIONE !!!**

![GitHub release (latest by date)](https://img.shields.io/github/v/release/TheFidax/Rcn600)
![GitHub Release Date](https://img.shields.io/github/release-date/TheFidax/Rcn600?color=blue&label=last%20release)
![GitHub commits since latest release (by date)](https://img.shields.io/github/commits-since/TheFidax/Rcn600/latest?color=orange)
[![arduino-library-badge](https://www.ardu-badge.com/badge/Rcn600.svg)](https://www.ardu-badge.com/Rcn600)
[![License](https://img.shields.io/github/license/TheFidax/Rcn600)](#)

# Indice
* [Introduzione Rcn600 SUSI](#Rcn600-SUSI)
* [API Libreria](#API-Libreria)

------------

# Rcn600 SUSI
Questa libreria permette di utilizzare una scheda Arduino (o un microcontrollore tramite Arduino IDE) come Slave per l'interfaccia SUSI.<br/>
Libreria testata su Arduino UNO, Arduino NANO (ATmega328P) e ATmega128 (MegaCore).</br></br>

**ATTENZIONE: Alcune schede Arduino funzionano a 3.3v, le interfacce SUSI, *se non specificato*, SONO A 5 VOLT !** 

Per funzionare sono necessari 2 resistori da **470Ω in serie** sulle linee SUSI (Clock e Dati).<br/>
Data  ---> 470Ω ---> Pin generico (anche di tipo analogico)<br/>
Clock ---> 470Ω ---> Pin Interrupt (o PortChangeInterrupt)<br/>

Maggiori informazioni della specifica nel fle [RCN-600.pdf](https://github.com/TheFidax/Rcn600/blob/master/RCN-600.pdf).

**Video Presentazione del Progetto:**</br>

[![Video Presentazione](https://img.youtube.com/vi/VzgkDouOvCY/0.jpg)](http://www.youtube.com/watch?v=VzgkDouOvCY)

------------

#API Libreria
Le Api per la libreria sono nel file "readme.md" disponibile sotto la cartella 'src' *oppure* [tramite il seguente Link Rapido](https://github.com/TheFidax/Rcn600/tree/master/src)

------------
