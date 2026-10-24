/*
 * pwm_heater.c
 *
 * Created: 04.04.2026 18:14:53
 *  Author: Robin Nathmann
 */ 


#include "pwm_heater.h"
#include "../config.h"
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>


// Zustand der Initialisierung
static bool init = false;

// Zustand der PWM Generierung
static bool pwm_on = false;


/*
 * Initialisiert den PWM Treiber für das Heizelement (Timer1).
 */
void pwm_heater_init(void)
{
	if (init) return;	// Wenn schon initialsiert, abbrechen
	
	DDRD |= (1 << PD5);		// PD5 (OC1A) als Ausgang
	
	PORTD &= ~(1 << PD5);	// PD5 (OC1A) auf LOW setzen
	
	// Timer1 zurücksetzen
	TCCR1A = 0;
	TCCR1B = 0;

	OCR1A = 0;	// Duty Cycle zurücksetzen
	
	init = true;	// Initialisierung bestätigen
}


/*
 * Aktiviert die PWM Generierung für das Heizelement (Timer1).
 */
void pwm_heater_enable(void)
{	
	if (pwm_on) return;		// Wenn PWM schon aktiviert wurde, abbrechen
	
	// Timer1 zurücksetzen
	TCCR1A = 0;
	TCCR1B = 0;
	
	TCCR1B |= (1 << WGM13);		// Phase & Frequency Correct PWM, Mode 8
	
	TCCR1A |= (1 << COM1A1);	// Nicht-invertierend
	
	TCCR1B |= (1 << CS10);		// Prescaler 1
	
	// Top Wert berechnen und setzen
	uint32_t top = (uint32_t)F_CPU/(2UL*(uint32_t)HEATER_F_PWM);	// TOP = F_CPU/(2*HEATER_F_PWM)
	if (top > 0xFFFF) top = 0xFFFF;									// Auf 16 Bit sättigen falls Overflow
	ICR1 = top;
	
	TIMSK1 = 0;		// Ohne Interrupts
	
	OCR1A = 0;		// Duty Cycle auf 0 setzen
	
	pwm_on = true;	// Aktivierung bestätigen
}


/*
 * Deaktiviert die PWM Generierung für das Heizelement (Timer1).
 */
void pwm_heater_disable(void)
{	
	if (!pwm_on) return;	// Wenn PWM schon deaktiviert wurde, abbrechen
	
	// Timer1 zurücksetzen
	TCCR1A = 0;
	TCCR1B = 0;

	OCR1A = 0;				// Duty Cycle zurücksetzen

	PORTD &= ~(1 << PD5);	// PD5 (OC1A) auf LOW setzen
	
	pwm_on = false;			// Aktivierung aufheben
}


/*
 * Setzt den Duty-Cycle des PWM-Signals für das Heizelement.
 *
 * @param duty_cycle_percent Duty-Cycle in Prozent (0%-100%)
 */
void pwm_heater_set_duty_cycle_percent(double duty_cycle_percent)
{
	// Sättigen bei Werten kleiner 0 oder größer 100
	if(duty_cycle_percent < 0.0) duty_cycle_percent = 0.0;
	if(duty_cycle_percent > 100.0) duty_cycle_percent = 100.0;
	
	// Register Wert als Double berechnen
	double ocr = ((double)ICR1 * duty_cycle_percent) / 100.0;	// OCR1A = TOP * Duty Cycle / 100
	
	OCR1A = (uint16_t)(ocr + 0.5);		// Wert auf nächste Ganzzahl runden und Register darauf setzen
}


/*
 * Gibt den momentanen Duty-Cycle des PWM-Signals für das Heizelement als Registerwert zurück.
 *
 * @return Duty-Cycle als Registerwert
 */
uint16_t pwm_heater_get_duty_cycle(void)
{
	return OCR1A;
}


/*
 * Gibt den maximalen Duty-Cycle des PWM-Signals für das Heizelement als Registerwert zurück.
 * Der maximale Duty-Cycle Registerwert entspricht 100% Einschaltzeit.
 *
 * @return Maximaler Duty-Cycle als Registerwert
 */
uint16_t pwm_heater_get_duty_cycle_max(void)
{
	return ICR1;
}