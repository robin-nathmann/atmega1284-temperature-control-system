/*
 * pi_controller.h
 *
 * Created: 13.03.2026 13:42:55
 *  Author: Robin Nathmann
 */ 


#pragma once
#include <stdint.h>


/*
 * Initialisiert den PI-Regler
 */
void pi_controller_init(void);

/*
 * Setzt den PI-Regler in den Ausgangszustand zurück
 */
void pi_controller_reset(void);

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
double pi_controller_get_system_input(int32_t target_value, int32_t current_value);