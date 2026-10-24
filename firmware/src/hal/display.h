/*
 * display.h
 *
 * Created: 20.04.2026 21:38:19
 *  Author: Robin Nathmann
 */ 


#pragma once
#include <stdint.h>
#include <stdbool.h>

/*
 * Initialisiert das Display HAL.
 */
void display_init(void);

/*
 * Gibt die Temperatur auf dem 7-Segment-Display mit drei Ziffern aus.
 * Die Temperatur wird passend gerundet.
 * Die Darstellungsform ist von der Temperatur abhängig:
 * -X.Y | -XX | X.YY | XX.Y | XXX
 *
 * @param temp Temperatur (Format: 30 Grad Celsius = 3000)
 */
void display_set_temp(int32_t temp);

/*
 * Gibt eine Zeichenkette auf dem 7-Segment-Display mit drei Ziffern aus.
 * Es werden nur die ersten drei Zeichen berücksichtigt.
 *
 * @param str Zeichenkette (String)
 */
void display_set_string(const char* str);

/*
 * Gibt eine Ganzzahl aus zwei Ziffern mit abschließenden konfigurierten
 * Einheitszeichen (Suffix) auf dem 7-Segment-Display mit drei Ziffern aus.
 *
 * @param i Ganzzahl
 */
void display_set_uint8_with_suffix(uint8_t i);

/*
 * Gibt den konfigurierten Text für den ausgeschalteten Zustand 
 * auf dem 7-Segment-Display mit drei Ziffern aus.
 */
void display_set_off(void);

/*
 * Gibt den konfigurierten Text für den Fehler Zustand 
 * auf dem 7-Segment-Display mit drei Ziffern aus.
 */
void display_set_error(void);

/*
 * Schaltet das Blinken des Displays an oder aus.
 *
 * @param b true = einschalten, false = ausschalten
 */
void display_set_blinking_enabled(bool b);

/*
 * Aktualisiert das Display. Ist notwendig für die Blinkfunktion.
 *
 * Diese Funktion muss ständig in einer Schleife aufgerufen werden.
 */
void display_refresh(void);