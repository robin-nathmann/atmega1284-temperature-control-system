# Temperaturregelsystem (ATmega1284)

Entwicklung eines mikrocontrollerbasiertes Temperaturregelsystem auf einem ATmega1284 im Rahmen einer Studienarbeit.

## Überblick

Das System erfasst die Ist-Temperatur einer Regelstrecke (PT1 mit Totzeit) über einen digitalen Temperatursensor und regelt ein Heizelement über eine extern aufgebaute Leistungsstufe.
Zudem umfasst das System eine Benutzerschnittstelle zur Bedienung des Reglers.

## Hardware-Setup

* **Mikrocontroller:** AVR ATmega1284P (auf STK600 Entwicklungsboard)
* **Sensorik:** Bosch BME280 (angebunden über I2C / TWI)
* **Leistungsstufe:** Externe MOSFET-Schaltstufe zur Ansteuerung des Heizelements
* **Benutzerschnittstelle:** 3-stellige 7-Segment-Anzeige sowie zwei Taster 
* **Diagnose:** UART-Schnittstelle zu externen Rechner für Telemetrie und Fehlercodes

## Software-Features

* **Regelung:** Zeitdiskreter Regler zum Halten des Soll-Wertes
* **Aktuatorik:** PWM-Treiber zur Ansteuerung der elektrischen Komponenten
* **Sensorik:** BME280- und I2C-Treiber mit Timeout-Überwachung und Plausibilitätsprüfung der Messdaten
* **Benutzerschnittstelle:** Zustandsautomat und Display-Treiber
* **Kommunikation:** UART-Treiber zur Übermittlung der Mess- und Systemdaten
* **Safety:** Hardware-Watchdog(WDT)
* **Konfiguration:** Parameter lassen sich über die `config.h` konfigurieren

## Mitgeliefert

* **Python Skript:** Zur Ermittlung der Totzeit
* **MATLAB Skript:** Zur Ermittlung der weiteren Systemparameter
* **Simulink Modell:** Simulation des Regelkreises
* **Rohdaten:** Messdaten bei einer Thermobox

## Build & Flash

1. Repository klonen.
2. Microchip Studio und ggf. Python und MATLAB/Simulink installieren.
3. `/firmware/Temperaturregelsystem.atsln` mit Microchip Studio öffnen.
1. Projekt über Microchip Studio kompilieren.
2. Output über Microchip Studio auf den ATmega1284 flashen.

## Pin Belegung

* **PWM Heizelement:** PD5
* **PWM Lüfter:** PD7
* **I2C:** PC0 (SCL) & PC1 (SDA)
* **UART:** PD0 (RX) & PD1 (TX)
* **GPIO für Display/Taster:** Lassen sich in der `config.h` frei konfigurieren