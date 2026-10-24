/*
 * seven_seg.h
 *
 * Created: 10.04.2026 12:22:50
 *  Author: Robin Nathmann
 */ 


#pragma once
#include <stdint.h>
#include <stdbool.h>


// Makro für das Dezimalpunkt Segment
#define SEVEN_SEG_DP (1 << 7)


// Zuordnung Ziffer zu jeweiligen aktiven Segmenten (1=AN, 0=AUS)
const uint8_t seven_seg_digit_to_seg[10];

/*
 * Initialisiert den 7-Segment Treiber.
 */
void seven_seg_init(void);

/*
 * Gibt die zu einem Zeichen passenden aktiven Segmente
 * der 7-Segment-Anzeige zurück.
 *
 * @param c	Zeichen
 *
 * @return Aktive Segmente als Byte: DP-g-f-e-d-c-b-a
 */
uint8_t seven_seg_char_to_seg(char c);

/*
 * Aktualisiert die 7-Segment-Anzeige.
 * Es wird pro Aufruf eine Zifferanzeige aufgefrischt.
 */
void seven_seg_refresh(void);

/*
 * Schaltet die 7-Segment-Anzeige an oder aus.
 *
 * @param b true = einschalten, false = ausschalten
 */
void seven_seg_set_enabled(bool b);

/*
 * Schaltet die 7-Segment-Anzeige um.
 * Von an nach aus oder von aus nach an.
 */
void seven_seg_toggle(void);

/*
 * Setzt die Segmente der 7-Segment-Anzeige.
 *
 * @param seven_seg[3] Array der aktive Segmente als Byte: DP-g-f-e-d-c-b-a aller Ziffernanzeigen (0;1;2)
 */
void seven_seg_set_segments(uint8_t seven_seg[3]);