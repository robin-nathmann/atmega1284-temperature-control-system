/*
 * config.h
 *
 * Created: 04.04.2026 19:13:01
 *  Author: Robin Nathmann
 */ 


#pragma once
#include <avr/io.h>


/*============================================================================*/
/*                                    SYSTEM                                  */
/*============================================================================*/

#define F_CPU 20000000UL			// Systemtakt

#define WATCHDOG_ENABLED true		// Watchdog aktivieren
#define WATCHDOG_TIMEOUT WDTO_2S	// Watchdog Timeout

#define DEBUG_ENABLED true			// Debug über UART einschalten


/*============================================================================*/
/*                     REGELUNG / SYSTEMIDENTIFIKATION                        */
/*============================================================================*/

// Parameter der Stecke des Systems als PT1-System						|   Übertragungsfunktion der Strecke:
#define SYSTEM_T1 2011.56476832113		// Zeitkonstante der Strecke	|				   K
#define SYSTEM_K 101.08569699424		// Verstärkung der Strecke		|		Gs(s) = -------
#define SYSTEM_Tt 31.0					// Totzeit der Strecke			|				T1*s+1

// PI Regler Konfiguration
#define PI_CONTROLLER_T0_MS 1000									// Abtastzeit des Reglers in ms
#define PI_CONTROLLER_KP ((0.9*SYSTEM_T1)/(SYSTEM_K*SYSTEM_Tt))		// Proportional-Anteil nach Ziegler-Nichols
#define PI_CONTROLLER_KI (PI_CONTROLLER_KP/(3.33*SYSTEM_Tt))		// Integral-Anteil nach Ziegler-Nichols

// Zwei-Punkt Regler (zum Messen der Totzeit, überschreibt PI Regler)
#define TWO_POINT_CONTROLLER_ENABLED false

// Sprungantwort messen
#define STEP_RESP_ENABLED false
#define STEP_RESP_DUTY_CYCLE_PERCENT_LOW 0.0
#define STEP_RESP_DUTY_CYCLE_PERCENT_HIGH 50.0
#define STEP_RESP_LOW_DURATION 10000
#define STEP_RESP_HIGH_DURATION 20000

// Alternativ zu Ziegler-Nichols: Kompensationsregler 
// (Führt hier je nach Tw entweder zu bleibender schwingender Regelabweichung oder erreicht Sollwert garnicht!)		| Übertragungsfunktion eines Kompensationsreglers:
//#define PI_CONTROLLER_TW 100.0										// Zeitkonstante des Kompensationsreglers	|				      1     1    T1
//#define PI_CONTROLLER_KP (SYSTEM_T1 / (SYSTEM_K * PI_CONTROLLER_TW))	// Proportional-Anteil Kompensationsregler	|			GR(s) = ----- * - + -----
//#define PI_CONTROLLER_KI (1.0 / (SYSTEM_K * PI_CONTROLLER_TW))		// Integral-Anteil Kompensationsregler		|			   	    K*Tw    s    K*Tw


/*============================================================================*/
/*                                SCHNITTSTELLEN                              */
/*============================================================================*/

// PWM
#define HEATER_F_PWM 160UL				// PWM Frequenz des Heizelements
#define FAN_PWM_PERCENTAGE 100.0		// Duty-Cycle des Lüfters in %

// I2C
#define F_I2C 100000UL					// I2C Frequenz
#define I2C_TIMEOUT_MS 10				// Maximale Zeit bis einzelne I2C Operation abgeschlossen sein muss

// UART
#define BAUD 625000						// BAUD Rate
#define UART_DOUBLE_DECIMAL_PLACES 2	// Anzahl Nachkommastellen von doubles über UART
#define UART_TX_BUFFER_SIZE 128			// Größe des Ringpuffers -> MUSS eine Zweierpotenz sein (z. B. 32, 64, 128)

// 7-Segment-Anzeige
#define SEVEN_SEG_REFRESH_TIME_MS 5		// Intervall zum Auffrischen der Anzeige (es wird je Zyklus ein Segment aufgefrischt)
#define SEVEN_SEG_DDR DDRB				// Data Direction Register für Segmentanschlüsse
#define SEVEN_SEG_PORT PORTB			// PORT Register für Segmentanschlüsse
#define SEVEN_SEG_MUX_DDR DDRA			// Data Direction Register für Multiplexer
#define SEVEN_SEG_MUX_PORT PORTA		// PORT Register für Multiplexer
#define SEVEN_SEG_MUX_PIN1 PA0			// PIN für Multiplexer der 1. Ziffer
#define SEVEN_SEG_MUX_PIN2 PA1			// PIN für Multiplexer der 2. Ziffer
#define SEVEN_SEG_MUX_PIN3 PA2			// PIN für Multiplexer der 3. Ziffer

// Taster
#define BUTTONS_DDR DDRA				// Data Direction Register für Taster
#define BUTTONS_PORT PORTA				// PORT Register für Taster
#define BUTTONS_PIN PINA				// PIN Register für Taster
#define BUTTONS_UP_PIN PA3				// PIN für UP Taster
#define BUTTONS_DOWN_PIN PA4			// PIN für DOWN Taster


/*============================================================================*/
/*                                  SENSOR                                    */
/*============================================================================*/

// Sensor
#define SENSOR_MAX_CONSECUTIVE_ERRORS 5		// Maximal erlaubte aufeinanderfolgend Auslesefehler des Sensors
#define SENSOR_MAX_VALID_TEMP 100			// Maximal erlaubte gemesessene Temperatur
#define SENSOR_MIN_VALID_TEMP 10			// Minimal erlaubte gemsessene Temperatur
#define SENSOR_MAX_VALID_DEVIATION 5		// Maximale erlaubte Abweichung zwischen zwei Messungen

// BME280 Config
#define BME280_ADDR 0x77				// BME280 Adresse
#define BME280_MODE 0b01				// MODE: (00: Sleep mode, 01/10: Forced mode, 11: Normal mode)
#define BME280_TEMP_OVERSAMPLING 0b101	// OVERSAMPLING: (000: Skipped, 001: 1x, 010: 2x, 011: 4x, 100: 8x, 101: 16x)
#define BME280_BOOT_DELAY_MS 5			// Verzögerung beim Hochfahren um Sensor Zeit zu geben korrekt zu antworten
#define BME280_BOOT_TIMEOUT_MS 100		// Maximale Zeit bis Sensor ansprechbar sein muss
#define BME280_MEAS_DELAY_US 50			// Verzögerung ab Messbeginn um Sensor Zeit zu geben korrekt zu antworten
#define BME280_MEAS_TIMEOUT_MS 50		// Maximale Zeit bis Sensor Messwerte haben muss


/*============================================================================*/
/*                         BENUTZERSCHNITTSTELLE                              */
/*============================================================================*/

// Display
#define DISPLAY_SUFFIX 'C'				// Angezeigtes Einheitszeichen hinter der Temperatur (beim Verstellen des Soll-Werts)
#define DISPLAY_BLINK_TIME_MS 500		// Blinkinterval der Anzeige in ms (beim Verstellen des Soll-Werts)
#define DISPLAY_OFF_TEXT "OFF"			// Angezeigter Text im ausgeschalteten Zustand des Systems
#define DISPLAY_ERROR_TEXT "Err"		// Angezeigter Text falls ein Fehler auftritt

// Zustände
#define STATES_DEFAULT_TARGET_VALUE_IN_CELSIUS 70	// Standardwert des Soll-Werts in Celsius
#define STATES_MIN_TARGET_VALUE_IN_CELSIUS 30		// Minimal einstellbarer Soll-Wert in Celsius
#define STATES_MAX_TARGET_VALUE_IN_CELSIUS 80		// Maximal einstellbarer Soll-Wert in Celsius
#define STATES_TARGET_TEMP_TIMEOUT_MS 5000			// Dauer der Rückkehr von Soll-Wert Anzeige zur Anzeige der aktuellen Temperatur in ms
#define STATES_LONG_PRESS_MS 500					// Dauer einer langen Tastenbestätigung in ms -> Schnelle Veränderung des Soll-Werts
#define STATES_FAST_MS_PER_STEP 50					// Dauer einer Erhöhung/Verkleinerung des Soll-Werts beim gedrückthalten in ms