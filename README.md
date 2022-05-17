![GitHub release (latest by date)](https://img.shields.io/github/v/release/TheFidax/Rcn600)
![GitHub Release Date](https://img.shields.io/github/release-date/TheFidax/Rcn600?color=blue&label=last%20release)<br/>
![GitHub commits since latest release (by date)](https://img.shields.io/github/commits-since/TheFidax/Rcn600/latest?color=orange)
[![GitHub stars](https://img.shields.io/github/stars/TheFidax/Rcn600)](https://github.com/TheFidax/Rcn600/stargazers)<br/>
[![arduino-library-badge](https://www.ardu-badge.com/badge/Rcn600.svg)](https://www.ardu-badge.com/Rcn600)
[![License](https://img.shields.io/github/license/TheFidax/Rcn600)](#)

# Indice
* [Introduzione Rcn600 SUSI](#Rcn600-SUSI)
* [Video di Presentazione](#Video-Presentazione)
* [API Libreria](#API-Libreria)
* [Esempi di Utilizzo](#Esempi-di-Utilizzo)

------------

# Rcn600 SUSI
Questa libreria permette di utilizzare una scheda Arduino (o un microcontrollore tramite Arduino IDE) come Slave per l'interfaccia SUSI.<br/>
Libreria testata su Arduino UNO, Arduino NANO (ATmega328P) e ATmega128 (MegaCore).</br>

**ATTENZIONE: Alcune schede Arduino funzionano a 3.3v, le interfacce SUSI, *se non specificato*, SONO A 5 VOLT !** 

Per funzionare sono necessari 2 resistori da **470Ω in serie** sulle linee SUSI (Clock e Dati).<br/>
<img src="https://github.com/TheFidax/Rcn600/blob/master/wiring.png" width="1280">

Maggiori informazioni della specifica nel fle [RCN-600.pdf](https://github.com/TheFidax/Rcn600/blob/master/RCN-600.pdf).

------------

# Video Presentazione
[![Video Presentazione](https://img.youtube.com/vi/VzgkDouOvCY/0.jpg)](http://www.youtube.com/watch?v=VzgkDouOvCY)

------------

# API Libreria
Le Api per la libreria sono nel file "[readme.md](https://github.com/TheFidax/Rcn600/blob/master/src/readme.md)" disponibile sotto la cartella '[src](https://github.com/TheFidax/Rcn600/tree/master/src)'.</br>

------------

# Esempi di Utilizzo
Sotto la cartella "[examples](https://github.com/TheFidax/Rcn600/tree/master/examples)" sono disponibili esempi di utilizzo della libreria.</br>
Sono suddivisi in base alla modalita' di acquisizione del segnale di Clock:</br>
- [Clock Mediante *pin di tipo Interrupt*](https://github.com/TheFidax/Rcn600/tree/master/examples/Slave_Interrupt)
- [Clock Mediante *PortChangeInterrupt*](https://github.com/TheFidax/Rcn600/tree/master/examples/Slave_PortChangeInterrupt)

E' disponibile un [esempio](https://github.com/TheFidax/Rcn600/tree/master/examples/Slave_Rcn600_to_NmraDcc) per *interfacciare la libreria Rcn600 alla libreria [NmraDcc](https://github.com/mrrwa/NmraDcc)*.</br>

------------
