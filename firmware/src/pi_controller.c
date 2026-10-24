/*
 * pi_pontroller.c
 *
 * Created: 12.03.2026 12:52:13
 *  Author: Robin Nathmann
 */ 


#include "pi_controller.h"
#include "config.h"
#include <stdint.h>
#include <stdbool.h>


// Zustand der Initialisierung
static bool init = false;

// Regelparameter
static double Kp;		// Proportional-Teil
static double T0Ki;		// Produkt aus Abtastzeit und Integrations-Teil vorberechnen

// Speicher für den Integralanteil des Reglers
static double I_part = 0.0;


/*
 * Initialisiert den PI-Regler.
 */
void pi_controller_init(void)
{
	// Wenn schon initialsiert, abbrechen
	if (init) return;
	
	Kp = PI_CONTROLLER_KP;
	T0Ki = ((double)PI_CONTROLLER_T0_MS/1000.0) * PI_CONTROLLER_KI;
	
	// Initialisierung bestätigen
	init = true;
}


/*
 * Setzt den PI-Regler in den Ausgangszustand zurück.
 */
void pi_controller_reset(void)
{
	I_part = 0.0;	// I-Anteil auf 0 setzen
}


/*
 * Berechnet die Stellgröße in % als double für die PWM-Regelung anhand eines
 * Temperatur Soll- und Ist-Werts durch eine PI-Regelung mit Anti-Windup
 * (Clamping) mittels der konfigurierten Regelparameter.
 *
 * @param target_value	Temperatur Soll-Wert (Format: 30 Grad Celsius = 3000)
 * @param current_value Temperatur Ist-Wert (Format: 30 Grad Celsius = 3000)
 *
 * @return				Stellgröße des Systems in %
 */
double pi_controller_get_system_input(int32_t target_value, int32_t current_value)
{
	// Regelfehler berechnen (Sollwert - Istwert)
	double error = (double)(target_value - current_value);
	
	// P-Anteil berechnen
	double P_part = Kp * error;
	
	// I-Zuwachs berechnen
	double I_increment = T0Ki * error;
	
	// Unbegrenzten Stellwert berechnen
	double pwm_output = P_part + I_part + I_increment;

	// Anti-Windup (Clamping) 
	if ((pwm_output >= 100.0 && error > 0.0) || (pwm_output <= 0.0 && error < 0.0)) {
	// Wenn Sättigung erreicht ist und Fehler in selbiger Richtung existiert
		// Nichts machen
	} else {
	// Ansonsten
		// I-Anteil inkrementieren
		I_part += I_increment;
	}
	
	// Sättigen bei Werten kleiner 0 oder größer 100
	if(pwm_output < 0.0) pwm_output = 0.0;
	if(pwm_output > 100.0) pwm_output = 100.0;
	
	// PWM Ausgang zurückgeben
	return pwm_output;
}