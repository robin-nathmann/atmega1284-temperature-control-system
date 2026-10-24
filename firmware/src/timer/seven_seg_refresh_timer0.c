/*
 * seven_seg_refresh_timer0.c
 *
 * Created: 07.05.2026 19:35:31
 *  Author: Robin Nathmann
 */ 


#include "seven_seg_refresh_timer0.h"
#include "../config.h"
#include "../driver/seven_seg.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>


// Zustand der Initialisierung
static bool init = false;


/*
 * Initialisiert den Timer0 für die Aktualisierung der 7-Segment-Anzeige.
 */
void seven_seg_refresh_timer0_init(void)
{	
	if (init) return;		// Wenn schon initialsiert, abbrechen
	
	TCCR0A = (1 << WGM01);	// Timer0 in CTC Mode (Clear Timer on Compare Match)
	TCCR0B = 0;				// TImer0 resetten
	
	// Compare-Wert berechnen und setzen
	uint32_t ocr = ( ((uint32_t)F_CPU * SEVEN_SEG_REFRESH_TIME_MS) / (1024UL * 1000UL) ) - 1;	//// OCR0A = ( (F_CPU*DISPLAY_REFRESH_TIME_MS) / (Prescaler * 1000) ) - 1
	if (ocr > 0xFF) ocr = 0xFF;																	// Auf 8 Bit sättigen falls Overflow
	OCR0A = ocr;

	TIMSK0 |= (1 << OCIE0A);				// Compare Match A Interrupt aktivieren
	
	TCCR0B |= (1 << CS02) | (1 << CS00);	// Prescaler 1024
	
	sei();									// Globale Interrupts aktivieren
	
	init = true;							// Initialisierung bestätigen
}


/*
 * Interrupt Service Routine für Timer0.
 */
ISR(TIMER0_COMPA_vect)
{
	seven_seg_refresh();	// 7-Segment-Anzeige aktualisieren
}