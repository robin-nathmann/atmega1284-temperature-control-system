/*
 * seven_seg.c
 *
 * Created: 08.04.2026 18:16:51
 *  Author: Robin Nathmann
 */ 


#include "seven_seg.h"
#include "../config.h"
#include "../timer/seven_seg_refresh_timer0.h"
#include <stdint.h>
#include <avr/io.h>
#include <stdbool.h>


// Makro Maske für Multiplexer
#define MUX_MASK ((1 << SEVEN_SEG_MUX_PIN1) | (1 << SEVEN_SEG_MUX_PIN2) | (1 << SEVEN_SEG_MUX_PIN3))


// Zustand der Initialisierung
static bool init = false;

// Aktuell aktive Segmente der drei Ziffern
volatile static uint8_t seven_seg_current[3] = {0x00, 0x00, 0x00};

// Zuletzt refreshte Ziffer
static uint8_t last_updated = 0;

// Zustand Anzeige
volatile static bool seven_seg_enabled = true;

// Zuordnung Ziffer zu jeweiligen aktiven Segmenten (1=AN, 0=AUS)
const uint8_t seven_seg_digit_to_seg[10] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3	==========================================================================
    0b01100110, // 4	| 7-Segment Belegung													 |
    0b01101101, // 5	| Pin7 DP | Pin6 g | Pin5 f | Pin4 e | Pin3 d | Pin2 c | Pin1 b | Pin0 a |
    0b01111101, // 6	============================================================== ===========
    0b00000111, // 7
    0b01111111, // 8
    0b01101111  // 9
};


/*
 * Initialisiert den 7-Segment Treiber.
 */
void seven_seg_init(void)
{	
	if (init) return;					// Wenn schon initialsiert, abbrechen
	
	SEVEN_SEG_DDR = 0xFF;				// Alle Pins für Segmente als Ausgang
	SEVEN_SEG_MUX_DDR |= MUX_MASK;		// Multiplexer Pins als Ausgang
	
	seven_seg_refresh_timer0_init();	// Timer für die Aktualisierung der 7-Segment-Anzeige initialisieren
	
	init = true;						// Initialisierung bestätigen
}


/*
 * Gibt die zu einem Zeichen passenden aktiven Segmente
 * der 7-Segment-Anzeige zurück.
 *
 * @param c	Zeichen
 *
 * @return Aktive Segmente als Byte: DP-g-f-e-d-c-b-a
 */
uint8_t seven_seg_char_to_seg(char c)
{
	if (c >= 'a' && c <= 'z') c -= 32;	// Kleinbuchstaben in Großbuchstaben umwandeln

	switch (c)
	{
		case '0': return seven_seg_digit_to_seg[0];
		case '1': return seven_seg_digit_to_seg[1];
		case '2': return seven_seg_digit_to_seg[2];
		case '3': return seven_seg_digit_to_seg[3];
		case '4': return seven_seg_digit_to_seg[4];
		case '5': return seven_seg_digit_to_seg[5];
		case '6': return seven_seg_digit_to_seg[6];
		case '7': return seven_seg_digit_to_seg[7];
		case '8': return seven_seg_digit_to_seg[8];
		case '9': return seven_seg_digit_to_seg[9];
		
		case 'A': return 0b01110111;
		case 'B': return 0b01111100;
		case 'C': return 0b00111001;
		case 'D': return 0b01011110;
		case 'E': return 0b01111001;
		case 'F': return 0b01110001;
		case 'G': return 0b00111101;
		case 'H': return 0b01110110;
		case 'I': return 0b00000110;
		case 'J': return 0b00011110;
		case 'K': return 0b01110110;
		case 'L': return 0b00111000;
		case 'M': return 0b00010101;
		case 'N': return 0b01010100;
		case 'O': return 0b00111111;
		case 'P': return 0b01110011;
		case 'Q': return 0b01100111;
		case 'R': return 0b01010000;
		case 'S': return 0b01101101;
		case 'T': return 0b01111000;
		case 'U': return 0b00111110;
		case 'V': return 0b00111110;
		case 'W': return 0b00101010;
		case 'X': return 0b01110110;
		case 'Y': return 0b01101110;
		case 'Z': return 0b01011011;
		
		case '-': return 0b01000000;

		default: return 0b00000000;		// sonst: alle Segmente aus
	}
}


/*
 * Aktualisiert die 7-Segment-Anzeige.
 * Es wird pro Aufruf eine Zifferanzeige aufgefrischt.
 */
void seven_seg_refresh(void)
{
	if(seven_seg_enabled) {		// Nur wenn Anzeige aktiv ist
		switch (last_updated) {
			case 0:																									// Multiplexer 1. Segment
				SEVEN_SEG_MUX_PORT |= MUX_MASK;																		// Alle Mux Pins auf HIGH (Aus, Mux ist Low-Aktiv)
				SEVEN_SEG_PORT = ~seven_seg_current[0];																// Segmentausgänge aktualisieren (Negation, da Segmente Low-Aktiv)
				SEVEN_SEG_MUX_PORT = (SEVEN_SEG_MUX_PORT & ~MUX_MASK) | (MUX_MASK & ~(1 << SEVEN_SEG_MUX_PIN1));	// MUX: Pin 1 LOW, Pin 2 HIGH, Pin 3 HIGH, alle anderen Pins beibehalten (Mux ist Low-Aktiv)
				last_updated = 1;																					// Beim nächsten Aufruf nächste Ziffer
				break;
			case 1:																									// Multiplexer 2. Segment
				SEVEN_SEG_MUX_PORT |= MUX_MASK;																		// Alle Mux Pins auf HIGH (Aus, Mux ist Low-Aktiv)
			    SEVEN_SEG_PORT = ~seven_seg_current[1];																// Segmentausgänge aktualisieren (Negation, da Segmente Low-Aktiv)
			    SEVEN_SEG_MUX_PORT = (SEVEN_SEG_MUX_PORT & ~MUX_MASK) | (MUX_MASK & ~(1 << SEVEN_SEG_MUX_PIN2));	// MUX: Pin 1 HIGH, Pin 2 LOW, Pin 3 HIGH, alle anderen Pins beibehalten (Mux ist Low-Aktiv)
			    last_updated = 2;																					// Beim nächsten Aufruf nächste Ziffer
			    break;
			case 2:																									// Multiplexer 3. Segment
			    SEVEN_SEG_MUX_PORT |= MUX_MASK;																		// Alle Mux Pins auf HIGH (Aus, Mux ist Low-Aktiv)
			    SEVEN_SEG_PORT = ~seven_seg_current[2];																// Segmentausgänge aktualisieren (Negation, da Segmente Low-Aktiv)
			    SEVEN_SEG_MUX_PORT = (SEVEN_SEG_MUX_PORT & ~MUX_MASK) | (MUX_MASK & ~(1 << SEVEN_SEG_MUX_PIN3));	// MUX: Pin 1 HIGH, Pin 2 HIGH, Pin 3 LOW, alle anderen Pins beibehalten (Mux ist Low-Aktiv)
			    last_updated = 0;																					// Beim nächsten Aufruf nächste Ziffer
			    break;
		}
	} else {
		SEVEN_SEG_MUX_PORT &= ~MUX_MASK;	// Alle Mux Pins auf LOW (An, Mux ist Low-Aktiv)
		SEVEN_SEG_PORT = 0xFF;				// Alle 7-Segmente auf HIGH (Aus, Segmente sind Low-Aktiv)
	}
}


/*
 * Schaltet die 7-Segment-Anzeige an oder aus.
 *
 * @param b true = einschalten, false = ausschalten
 */
void seven_seg_set_enabled(bool b)
{
	seven_seg_enabled = b;
}


/*
 * Schaltet die 7-Segment-Anzeige um.
 * Von an nach aus oder von aus nach an.
 */
void seven_seg_toggle(void)
{
	seven_seg_enabled = !seven_seg_enabled;
}


/*
 * Setzt die Segmente der 7-Segment-Anzeige.
 *
 * @param seven_seg[3] Array der aktive Segmente als Byte: DP-g-f-e-d-c-b-a aller Ziffernanzeigen (0;1;2)
 */
void seven_seg_set_segments(uint8_t seven_seg[3])
{
	seven_seg_current[0] = seven_seg[0];
	seven_seg_current[1] = seven_seg[1];
	seven_seg_current[2] = seven_seg[2];
}