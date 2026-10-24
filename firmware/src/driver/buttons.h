/*
 * buttons.h
 *
 * Created: 23.04.2026 20:29:34
 *  Author: Robin Nathmann
 */ 


#pragma once
#include <stdbool.h>
#include <stdint.h>


/*
 * Initialisiert den Treiber für die Taster.
 */
void buttons_init(void);

/*
 * Gibt zurück, ob der UP Taster gedrückt ist.
 *
 * @return true = Taster ist gedrückt, false = Taster ist nicht gedrückt
 */
bool buttons_up_is_pressed(void);

/*
 * Gibt zurück, ob der DOWN Taster gedrückt ist.
 *
 * @return true = Taster ist gedrückt, false = Taster ist nicht gedrückt
 */
bool buttons_down_is_pressed(void);

/*
 * Gibt den Zustand der beiden Taster zurück.
 *
 * @return Kein Taster gedrückt:  0b00000000,
 *		   UP Taster gedrückt:	  0b00000010,
 *         DOWN Taster gedrückt:  0b00000001,
 *         Beide Taster gedrückt: 0b00000011
 */
uint8_t buttons_get_state(void);