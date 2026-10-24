/*
 * sampler.c
 *
 * Created: 06.05.2026 18:27:07
 *  Author: Robin Nathmann
 */ 


#include "sampler.h"
#include "config.h"
#include "states.h"
#include "pi_controller.h"
#include "hal/sensor.h"
#include "hal/debug.h"
#include "driver/pwm_heater.h"
#include "driver/pwm_fan.h"
#include "timer/millis_timer3.h"
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>


// Zustand der Initialisierung
static bool init = false;

// Zeit beim letzten Refresh
static uint16_t last_time = 0;

// Zähler
static uint16_t counter = 0;			// Anzahl an Abtastungen
static uint16_t step_resp_count = 0;	// Anzahl an Abtastungen ab Messung der Sprungantwort


/*
 * Initialisiert den Abtaster.
 */
void sampler_init(void)
{
	if (init) return;
	
	// Benötigte Module initialisieren
	sensor_init();
	states_init();
	pwm_heater_init();
	pi_controller_init();
	millis_timer3_init();
	pwm_fan_init();
	debug_init();
	
	init = true;
}


/*
 * Schaltet das Heizelement und den Lüfter gleichzeitig an oder aus.
 * Beim Ausschalten wird der PI-Regler auf Ausgangszustand gesetzt.
 *
 * @param b true = einschalten, false = ausschalten
 */
static void sampler_set_heater_and_fan(bool b)
{
	if (b) {
		pwm_heater_enable();	// Heizelement einschalten
		pwm_fan_enable();		// Lüfter einschalten
	} else {
		pwm_heater_disable();	// Heizelement ausschalten
		pi_controller_reset();	// PI-Regler zurücksetzen
		pwm_fan_disable();		// Lüfter ausschalten
	}
}


/*
 * Aktualisiert den Abtaster. Der Abtaster stößt die Temperaturmessung an, übermittelt die
 * Regelgrößen, steuert den Treiber für das Heizelement und den Lüfter an, schaltet diese im
 * Fehlerfall aus und übergibt Logging Werte zum Debug Modul.
 *
 * Diese Funktion muss ständig in einer Schleife aufgerufen werden. Die Logik wird nur ausgeführt,
 * wenn die Abtastzeit erreicht ist. Die Abtastzeit wird über PI_CONTROLLER_T0_MS bestimmt.
 */
void sampler_refresh(void)
{
	uint16_t current_time = millis();	// Aktuelle Zeit
	
	// Wenn die Abtastzeit seit der letzten Zeitmessung vorrüber ist
	if (current_time - last_time >= PI_CONTROLLER_T0_MS) {
		
		last_time = current_time;	// Letzte Zeitmessung aktualisieren
		
		int32_t temp = sensor_get_temperature();	// Temperaturmessung anstoßen
		
		// Im Fehlerfall wird Rückgabe auf INT_MAX gesetzt
		if(temp == INT_MAX) {
			sampler_set_heater_and_fan(false);	// Heizelement und Lüfter ausschalten
			states_set_error();					// Zustand auf ERROR setzen
		}
		
		int32_t target = (int32_t)states_get_target_value() * 100;	// Sollwert auslesen und auf selbes Format wie Temperatur bringen (30 Grad Celsius = 3000)
		
		double sys_in = 0.0;	// Stellgröße des Systems
		
		// Wenn die Heizung nach dem Zustandsautomat aktiv sein soll
		if (states_heating_is_enabled()) {
			
			sys_in = pi_controller_get_system_input(target, temp);	// Stellgröße über PI-Regler ermitteln
			
			// Falls Sprungantwort gemessen werden soll
			if (STEP_RESP_ENABLED) {
				// Wenn Abtast Zähler Wert hat, bei dem geheizt werden soll
				if (STEP_RESP_LOW_DURATION <= step_resp_count && (step_resp_count-STEP_RESP_LOW_DURATION) < STEP_RESP_HIGH_DURATION) {
					sys_in = STEP_RESP_DUTY_CYCLE_PERCENT_HIGH; // Heizung auf konfiguriertes HIGH
				} else {
					sys_in = STEP_RESP_DUTY_CYCLE_PERCENT_LOW;	// Heizung auf konfiguriertes LOW
				}
				step_resp_count++;	// Abtast Zähler für Sprungantwort erhöhen
			}
			
			// Falls Zweipunkt-Regler (zum Totzeit messen) aktiv sein soll
			if (TWO_POINT_CONTROLLER_ENABLED) {
				// Wenn Temperatur kleiner als Sollwert
				if (temp < target) {
					sys_in = 100;	// Heizung auf 100%
				} else {
					sys_in = 0;		// Heizung aus
				}
			}
			
			sampler_set_heater_and_fan(true);				// Heizelement und Lüfter an
			pwm_heater_set_duty_cycle_percent(sys_in);		// Duty Cycle des Heizelement setzen
			
		} else {
			sampler_set_heater_and_fan(false);				// Heizelement und Lüfter aus
		}
		
		/////////////////////////////
		// PROTOKOLLIERUNG & DEBUG //
		/////////////////////////////
		
		uint16_t end_time = millis();	// Aktuelle Zeit auslesen (zur Ermittlung der Dauer des Algorithmus)

		uint16_t pwm_dc = pwm_heater_get_duty_cycle();			// Duty Cycle als Registerwert auslesen
		uint16_t pwm_dc_max = pwm_heater_get_duty_cycle_max();	// Maximalen Duty Cycle Registerwert auslesen
		
		// Alle Werte über Debug Modul protokollieren
		debug_print_telemetry(current_time,
							  counter,
							  step_resp_count,
							  temp,
							  target,
							  sys_in,
							  pwm_dc,
							  pwm_dc_max,
							  states_get_current_state_as_string(),
							  end_time);
		
		// Abtastzähler erhöhen
		counter++;
	}
}
