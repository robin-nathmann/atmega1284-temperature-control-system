/*
 * millis_timer3.h
 *
 * Created: 09.04.2026 23:02:02
 *  Author: Robin Nathmann
 */ 


#pragma once

/*
 * Initialisiert den Timer3 für die Systemzeitbasis.
 */
void millis_timer3_init(void);

/*
 * Gibt die aktuelle Systemzeit für Zeitvergleiche zurück.
 * Die Systemzeit springt nach etwa 65,5 s wieder auf 0.
 * Es sind nur Zeitvergleiche bis zu 65,5 s Differenz möglich.
 *
 * @return Systemzeit in Millisekunden (max 65535 ms)
 */
uint16_t millis(void);