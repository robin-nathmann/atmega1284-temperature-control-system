/*
 * buttons.c
 *
 * Created: 22.04.2026 19:35:02
 *  Author: Robin Nathmann
 */ 


#include "buttons.h"
#include "../config.h"
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>


// Zustand der Initialisierung
static bool init = false;


/*
 * Initialisiert den Treiber für die Taster.
 */
void buttons_init(void)
{
	if (init) return;	// Wenn schon initialsiert, abbrechen
	
	BUTTONS_DDR &= ~((1 << BUTTONS_UP_PIN) | (1 << BUTTONS_DOWN_PIN));	// ALs Eingänge konfigurieren
	
	BUTTONS_PORT |= (1 << BUTTONS_UP_PIN) | (1 << BUTTONS_DOWN_PIN);	// Interne Pull-ups aktivieren
	
	init = true;	// Initialisierung bestätigen
}


/*
 * Gibt zurück, ob der UP Taster gedrückt ist.
 *
 * @return true = Taster ist gedrückt, false = Taster ist nicht gedrückt
 */
bool buttons_up_is_pressed(void)
{
	return !(BUTTONS_PIN & (1 << BUTTONS_UP_PIN));	// Taster ist bei Low Pegel gedrückt
}


/*
 * Gibt zurück, ob der DOWN Taster gedrückt ist.
 *
 * @return true = Taster ist gedrückt, false = Taster ist nicht gedrückt
 */
bool buttons_down_is_pressed(void)
{
	return !(BUTTONS_PIN & (1 << BUTTONS_DOWN_PIN));	// Taster ist bei Low Pegel gedrückt
}


/*
 * Gibt den Zustand der beiden Taster zurück.
 *
 * @return Kein Taster gedrückt:  0b00000000,
 *		   UP Taster gedrückt:	  0b00000010,
 *         DOWN Taster gedrückt:  0b00000001,
 *         Beide Taster gedrückt: 0b00000011
 */
uint8_t buttons_get_state(void)
{
	uint8_t state = 0;	// Hilfsvariabel

	if (buttons_up_is_pressed()) {
		state |= (1 << 1);	// Button UP -> Bit 1
	}

	if (buttons_down_is_pressed()) {
		state |= (1 << 0);	// Button DOWN -> Bit 0
	}

	return state;
}
