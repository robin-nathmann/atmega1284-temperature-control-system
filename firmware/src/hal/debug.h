/*
 * sensor.h
 *
 * Created: 03.06.2026 20:37:09
 *  Author: Robin Nathmann
 */ 


#pragma once
#include <stdint.h>

/*
 * Initialisiert das Debug HAL.
 */
void debug_init(void);

/*
 * Gibt eine Fehlerbezeichnung oder einen Fehlercode zusammen mit einem
 * fehlerbehafteten Wert über die Debug-Schnittstelle mittels UART aus.
 * Formatierung: [FEHLERCODE:WERT]
 *
 * @param s	  Fehlerbezeichnung oder Fehlercode als Zeichenkette
 * @param val Fehlerhafter Wert
 */
void debug_print_value_error(const char* str, int32_t temp);

/*
 * Gibt einen Bus-Error zusammen mit den Fehlercodes für BME und
 * ggf. I2C über die Debug-Schnittstelle mittels UART aus.
 * Formatierung: [BUS_ERROR BME:FEHLERCODE | I2C:FEHLERCODE]
 *
 * @param bme_err Fehlercode für BME
 * @param i2c_err Fehlercode für I2C (bei 0 wird kein I2C Fehlercode ausgegeben)
 */
void debug_print_bus_error(uint16_t bme_err, uint16_t i2c_err);

/*
 * Gibt alle relevanten Systemdaten über die Debug-Schnittstelle mittels UART aus.
 * Verwendung ist für den Abtaster angedacht. Formatierung mit Beispielwerten:
 * Time:123 Count:123 StepResp:123 Ist:123 Soll:123 12% PWM:123/456 State:OFF Duration:123
 *
 * @param current_time    Zeit zu Beginn der Abtastung
 * @param counter         Abtastzähler
 * @param step_resp_count Abtastzähler ab Beginn der Sprungantwortmessung
 * @param temp			  Ist-Temperatur (Format: 30 Grad Celsius = 3000)
 * @param target          Soll-Temperatur (Format: 30 Grad Celsius = 3000)
 * @param sys_in          Stellgröße in %
 * @param pwm_dc          Duty-Cycle von Heiz-PWM als Registerwert
 * @param pwm_dc_max      Maximalen Duty-Cycle von Heiz-PWM als Registerwert
 * @param state           Zustand des Zustandsautomats als Zeichenkette
 * @param end_time        Zeit zum Ende des Abtastzyklus
 */
void debug_print_telemetry(uint16_t current_time,
						   uint16_t counter,
						   uint16_t step_resp_count,
						   int32_t temp,
						   int32_t target,
						   double sys_in,
						   uint16_t pwm_dc,
						   uint16_t pwm_dc_max,
						   const char* state,
						   uint16_t end_time);