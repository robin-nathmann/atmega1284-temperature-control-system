/*
 * display.c
 *
 * Created: 20.04.2026 20:32:10
 *  Author: robin
 */ 


#include "display.h"
#include "../config.h"
#include "../driver/seven_seg.h"
#include "../timer/millis_timer3.h"
#include <stdint.h>
#include <stdbool.h>


// Zustand der Initialisierung
static bool init = false;

// Blinken
static bool blinking_enabled = false;	// Zustand des Blinkens
static uint16_t last_blink_time = 0;	// Zeit beim letztem Blinkzyklus


/*
 * Initialisiert das Display HAL.
 */
void display_init(void)
{
	if (init) return;
	
	// Benötigte Module initialisieren
	seven_seg_init();
	millis_timer3_init();
	
	init = true;
}


/*
 * Gibt die Temperatur auf dem 7-Segment-Display mit drei Ziffern aus.
 * Die Temperatur wird passend gerundet.
 * Die Darstellungsform ist von der Temperatur abhängig:
 * -X.Y | -XX | X.YY | XX.Y | XXX
 *
 * @param temp Temperatur (Format: 30 Grad Celsius = 3000)
 */
void display_set_temp(int32_t temp)
{
	// Absolutwert
	int32_t abs_temp = (temp < 0) ? -temp : temp;
	
	// Hilfsvariabeln
	uint8_t seven_seg[3];
	int32_t val;
	int dp_pos = -1;

	if (temp < 0) {	// Negativ
		
		val = (abs_temp + 5) / 10;	// Auf eine Nachkommastelle runden
		
		if (val < 100) {			// -9.9 bis -0.1 -> Format: -X.Y
			dp_pos = 1;				// Dezimalpunkt beim 2. Segment
			
		} else {							// -10 bis -? -> Format: -XX
			
			val = (abs_temp + 50) / 100;	// Auf ganze Grad runden
			
			if (val > 99) val = 99;			// Begrenzung auf -99
			dp_pos = -1;					// Kein Dezimalpunkt
		}

		seven_seg[0] = seven_seg_char_to_seg('-');		// 1. Segment setzen

	} else {	// Positiv
		
		val = abs_temp;
		
		if (val < 1000) {	// 0 bis 9.99 -> Format: X.YY
			dp_pos = 0;		// Dezimalpunkt beim 1. Segment
			
		} else {
			
			val = (abs_temp + 5) / 10;		// Auf eine Nachkommastelle runden
			
			if (val < 1000) {				// 10 bis 99.9 -> Format: XX.Y
				dp_pos = 1;					// Dezimalpunkt beim 2. Segment
				
			} else {								// 100 bis ? -> Format: XXX
				val = (abs_temp + 50) / 100;		// Auf ganze Grad runden
				if (val > 999) val = 999;			// Begrenzung auf 999
				dp_pos = -1;						// Kein Dezimalpunkt
			}
		}

		seven_seg[0] = seven_seg_digit_to_seg[(val / 100) % 10];	// 1. Segment setzen
	}
	
	seven_seg[1] = seven_seg_digit_to_seg[(val / 10) % 10];			// 2. Segment setzen
	seven_seg[2] = seven_seg_digit_to_seg[val % 10];				// 3. Segment setzen
	
	if (dp_pos != -1) {							// Wenn Dezimalpunkt vorhanden ist
		seven_seg[dp_pos] |= SEVEN_SEG_DP;		// Dezimalpunkt setzen
	}
	
	seven_seg_set_segments(seven_seg);		// Segmente auf Display setzen
}


/*
 * Gibt eine Zeichenkette auf dem 7-Segment-Display mit drei Ziffern aus.
 * Es werden nur die ersten drei Zeichen berücksichtigt.
 *
 * @param str Zeichenkette (String)
 */
void display_set_string(const char* str)
{
	if (!str) return;	// Null-Pointer abfangen
	
	// Hilfsvariabeln
	uint8_t seven_seg[3];
	int index = 0;

	while (str[index] != '\0' && index < 3) {		// Über die ersten drei Zeichen iterieren

		seven_seg[index] = seven_seg_char_to_seg(str[index]);	// Aktive Segmente für den Buchstaben ermitteln
		index++;
	}
	
	seven_seg_set_segments(seven_seg);	// Segmente auf Display setzen
}


/*
 * Gibt eine Ganzzahl aus zwei Ziffern mit abschließenden konfigurierten
 * Einheitszeichen (Suffix) auf dem 7-Segment-Display mit drei Ziffern aus.
 *
 * @param i Ganzzahl
 */
void display_set_uint8_with_suffix(uint8_t i)
{
	if (i > 99) {
		i = 99;		// Sättigen
	}
	
	uint8_t seven_seg[3];	// Hilfsvariabel
	
	seven_seg[0] = seven_seg_digit_to_seg[i / 10];			// Erste Ziffer
	seven_seg[1] = seven_seg_digit_to_seg[i % 10];			// Zweite Ziffer
	seven_seg[2] = seven_seg_char_to_seg(DISPLAY_SUFFIX);	// Einheitszeichen (Suffix)
	
	seven_seg_set_segments(seven_seg);	// Segmente auf Display setzen
}


/*
 * Gibt den konfigurierten Text für den ausgeschalteten Zustand 
 * auf dem 7-Segment-Display mit drei Ziffern aus.
 */
void display_set_off(void)
{
	display_set_string(DISPLAY_OFF_TEXT);
}


/*
 * Gibt den konfigurierten Text für den Fehler Zustand 
 * auf dem 7-Segment-Display mit drei Ziffern aus.
 */
void display_set_error(void)
{
	display_set_string(DISPLAY_ERROR_TEXT);
}


/*
 * Schaltet das Blinken des Displays an oder aus.
 *
 * @param b true = einschalten, false = ausschalten
 */
void display_set_blinking_enabled(bool b)
{
	if(blinking_enabled == b) return;	// Nur ändern wenn nötig
	blinking_enabled = b;				// Blinkzustand ändern
}


/*
 * Aktualisiert das Display. Ist notwendig für die Blinkfunktion.
 *
 * Diese Funktion muss ständig in einer Schleife aufgerufen werden.
 */
void display_refresh(void)
{	
	uint16_t current_time = millis();	// Aktuelle Zeit ermitteln
	
	if(blinking_enabled) {				// Falls blinken an ist
		
		if(current_time - last_blink_time >= DISPLAY_BLINK_TIME_MS) {	// Falls Zeit zwischen dem Blinkzyklen erreicht wurde
			last_blink_time = current_time;								// Letzte Blinkzeit überschreiben
			seven_seg_toggle();											// Segmente an/aus toggeln
		}
	} else {								// Falls blinken aus ist
		last_blink_time = current_time;		// Letzte Blinkzeit überschreiben
		seven_seg_set_enabled(true);		// Segmente einschalten
	}
	
}