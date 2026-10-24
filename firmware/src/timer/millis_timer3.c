/*
 * millis_timer3.c
 *
 * Created: 09.04.2026 22:45:28
 *  Author: Robin Nathmann
 */ 


#include "../config.h"
#include "millis_timer3.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>


// Zustand der Initialisierung
static bool init = false;

// Interne Variabel für die Systemzeit
static volatile uint16_t millis_timer = 0;


/*
 * Initialisiert den Timer3 für die Systemzeitbasis.
 */
void millis_timer3_init(void)
{	
	if (init) return;		// Wenn schon initialsiert, abbrechen
	
    TCCR3A = 0;				// Timer3 resetten und Pins deaktivieren
    TCCR3B = (1 << WGM32);	// Timer3 in CTC Mode (Clear Timer on Compare Match)

    // Compare-Wert berechnen und setzen -> für ISR jede 1ms (1000 Hz)
    uint32_t ocr = ((uint32_t)F_CPU / (8UL * 1000UL)) - 1;	// OCR3A = (F_CPU / (Prescaler * 1000)) - 1
	if (ocr > 0xFFFF) ocr = 0xFFFF;							// Auf 16 Bit sättigen falls Overflow
    OCR3A = ocr;

    TIMSK3 |= (1 << OCIE3A);	// Compare Match A Interrupt aktivieren
	
	TCCR3B |= (1 << CS31);		// Prescaler = 8
	
	sei();						// Globale Interrupts aktivieren

	init = true;				// Initialisierung bestätigen
}


/*
 * Interrupt Service Routine für Timer3.
 */
ISR(TIMER3_COMPA_vect)
{
    millis_timer++;		// Systemzeit inkrementieren
}


/*
 * Gibt die aktuelle Systemzeit für Zeitvergleiche zurück.
 * Die Systemzeit springt nach etwa 65,5 s wieder auf 0.
 * Es sind nur Zeitvergleiche bis zu 65,5 s Differenz möglich.
 *
 * @return Systemzeit in Millisekunden (max 65535 ms)
 */
uint16_t millis(void)
{
	uint16_t a, b;	// Hilfsvariabeln

	// Atomares sicheres Auslesen der Systemzeit.
	// Da ein uint16 zwei Byte entspricht kann Wert durch ISR während des Auslesens verändert werden.
	// Hier wird sichergestellt, dass der Wert nicht zwischendurch manipuliert wurde.
	do {
		a = millis_timer;	// Auslesen der Systemzeit
		b = millis_timer;	// Erneutes auslesen
	} while (a != b);		// Solange auslesen bis ausgelesene Werte identisch sind
	
	return a;	// Rückgabe Systemzeit
}