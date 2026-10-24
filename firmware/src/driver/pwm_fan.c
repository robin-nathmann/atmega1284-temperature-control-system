/*
 * pwm_fan.c
 *
 * Created: 15.05.2026 12:36:17
 *  Author: Robin Nathmann
 */ 


#include "pwm_fan.h"
#include "../config.h"
#include <avr/io.h>
#include <stdbool.h>


// Zustand der Initialisierung
static bool init = false;

// Zustand der PWM Generierung
static bool pwm_on = false;


/*
 * Initialisiert den PWM Treiber für den Lüfter (Timer2).
 */
void pwm_fan_init(void)
{	
	if (init) return;	// Wenn schon Initialsiert, abbrechen
	
	DDRD |= (1 << PD7);		// PD7 (OC2A) als Ausgang
	
	PORTD &= ~(1 << PD7);	// PD7 (OC2A) auf LOW setzen
	
	// Timer2 zurücksetzen
	TCCR2A = 0;
	TCCR2B = 0;
	
	OCR2A = 0;		// Duty Cycle zurücksetzen
	
	init = true;	// Initialisierung bestätigen
}


/*
 * Aktiviert die PWM Generierung für den Lüfter (Timer2).
 */
void pwm_fan_enable(void)
{	
	if (pwm_on) return;		// Wenn PWM schon an ist, abbrechen
	
	// Timer2 zurücksetzen
	TCCR2A = 0;
	TCCR2B = 0;
	
	OCR2A = 0;	// Duty Cycle zurücksetzen
	
	// Wenn Lüfter auf 100% laufen soll
	if (FAN_PWM_PERCENTAGE >= 100.0) {
		
		PORTD |= (1 << PD7);	// PD7 (OC2A) auf HIGH setzen
		
	} else if (FAN_PWM_PERCENTAGE <= 0.0) {
		
		PORTD &= ~(1 << PD7);	// PD7 (OC2A) auf LOW setzen
		
	} else {
			
		TCCR2A = (1 << WGM20) | (1 << COM2A1);	// Phase Correct PWM Mode, nicht-invertierend
			
		// PWM Frequenz = F_CPU / (Prescaler*255*2) = 39,22kHz
		TCCR2B = (1 << CS20);	// Prescaler = 1
			
		TIMSK2 = 0;		// Ohne Interrupts
			
		OCR2A = (255UL * FAN_PWM_PERCENTAGE) / 100;		// Duty Cycle setzen
		
	}
	
	pwm_on = true;		// Aktivierung bestätigen
}


/*
 * Deaktiviert die PWM Generierung für den Lüfter (Timer2).
 */
void pwm_fan_disable(void)
{
	if (!pwm_on) return;	// Wenn PWM schon resettet wurde, abbrechen
	
	// Timer2 zurücksetzen
	TCCR2A = 0;
	TCCR2B = 0;

	OCR2A = 0;		// Duty Cycle zurücksetzen

	PORTD &= ~(1 << PD7);	// PD7 (OC2A) auf LOW setzen
	
	pwm_on = false;		// Aktivierung aufheben
}