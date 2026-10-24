/*
 * pwm_heater.h
 *
 * Created: 04.04.2026 18:32:21
 *  Author: Robin Nathmann
 */ 


#pragma once
#include <stdint.h>


/*
 * Initialisiert den PWM Treiber für das Heizelement (Timer1).
 */
void pwm_heater_init(void);

/*
 * Aktiviert die PWM Generierung für das Heizelement (Timer1).
 */
void pwm_heater_enable(void);

/*
 * Deaktiviert die PWM Generierung für das Heizelement (Timer1).
 */
void pwm_heater_disable(void);

/*
 * Setzt den Duty-Cycle des PWM-Signals für das Heizelement.
 *
 * @param duty_cycle_percent Duty-Cycle in Prozent (0%-100%)
 */
void pwm_heater_set_duty_cycle_percent(double duty_cycle_percent);

/*
 * Gibt den momentanen Duty-Cycle des PWM-Signals für das Heizelement als Registerwert zurück.
 *
 * @return Duty-Cycle als Registerwert
 */
uint16_t pwm_heater_get_duty_cycle(void);

/*
 * Gibt den maximalen Duty-Cycle des PWM-Signals für das Heizelement als Registerwert zurück.
 * Der maximale Duty-Cycle Registerwert entspricht 100% Einschaltzeit.
 *
 * @return Maximaler Duty-Cycle als Registerwert
 */
uint16_t pwm_heater_get_duty_cycle_max(void);