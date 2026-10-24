/*
 * debug.c
 *
 * Created: 09.06.2026 23:29:30
 *  Author: Robin Nathmann
 */ 


#include "debug.h"
#include "../config.h"
#include "../driver/uart.h"
#include <stdint.h>


// Zustand der Initialisierung
static bool init = false;


/*
 * Initialisiert das Debug HAL.
 */
void debug_init(void)
{
	if(!DEBUG_ENABLED) return;	// Wenn Debug Output ausgeschaltet ist, abbrechen
	if (init) return;
	
	uart_init();	// Benötigt den UART Treiber
	
	init = true;
}


/*
 * Gibt eine Fehlerbezeichnung oder einen Fehlercode zusammen mit einem
 * fehlerbehafteten Wert über die Debug-Schnittstelle mittels UART aus.
 * Formatierung: [FEHLERCODE:WERT]
 *
 * @param str Fehlerbezeichnung oder Fehlercode als Zeichenkette (String)
 * @param val Fehlerhafter Wert
 */
void debug_print_value_error(const char* str, int32_t val)
{	
	if(!DEBUG_ENABLED) return;	// Wenn Debug Output ausgeschaltet ist, abbrechen
	
	// Ausgabe über UART-Treiber
	uart_print_string("[");
	uart_print_string(str);
	uart_print_string(":");
	uart_print_int32(val);
	uart_print_string("]\r\n");
}


/*
 * Gibt einen Bus-Error zusammen mit den Fehlercodes für BME und
 * ggf. I2C über die Debug-Schnittstelle mittels UART aus.
 * Formatierung: [BUS_ERROR BME:FEHLERCODE | I2C:FEHLERCODE]
 *
 * @param bme_err Fehlercode für BME
 * @param i2c_err Fehlercode für I2C (bei 0 wird kein I2C Fehlercode ausgegeben)
 */
void debug_print_bus_error(uint16_t bme_err, uint16_t i2c_err)
{
	if(!DEBUG_ENABLED) return;	// Wenn Debug Output ausgeschaltet ist, abbrechen
	
	// Ausgabe über UART-Treiber
	uart_print_string("[BUS_ERROR BME:");
	uart_print_uint16(bme_err);
	
	// I2C Error nur ausgeben wenn er nicht 0 ist
	if (i2c_err != 0) {
		uart_print_string(" | I2C:");
		uart_print_uint16(i2c_err);
	}
	
	uart_print_string("]\r\n");
}


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
 * @param state           Zustand des Zustandsautomats als Zeichenkette (String)
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
						   uint16_t end_time) {
	
	if(!DEBUG_ENABLED) return;
	
	// Ausgabe über UART-Treiber
	uart_print_string("Time:");
	uart_print_uint16(current_time);
			
	uart_print_string(" Count:");
	uart_print_uint16(counter);
	
	// Abtastzähler für Sprungantwort nur ausgeben, wenn Sprungantwort gemessen wird		
	if (STEP_RESP_ENABLED) {
		uart_print_string(" StepResp:");
		uart_print_uint16(step_resp_count-1);	// -1, da der Zähler bereits vor Ausgabe inkrementiert wird
	}
			
	uart_print_string(" Ist:");
	uart_print_int32(temp);
			
	uart_print_string(" Soll:");
	uart_print_int32(target);
			
	uart_print_string(" ");
	uart_print_double(sys_in);
			
	uart_print_string("% PWM:");
	uart_print_uint16(pwm_dc);
	uart_print_string("/");
	uart_print_uint16(pwm_dc_max);

	uart_print_string(" State:");
	uart_print_string(state);
			
	uart_print_string(" Duration:");
	uart_print_uint16(end_time - current_time);		// Dauer der Abtastung
			
	uart_print_string("\r\n");
	
}