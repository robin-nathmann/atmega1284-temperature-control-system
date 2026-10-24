/*
 * states.h
 *
 * Created: 24.04.2026 20:30:55
 *  Author: Robin Nathmann
 */ 


#pragma once
#include <stdbool.h>
#include <stdint.h>


/*
 * Initialisiert den Zustandsautomaten.
 */
void states_init(void);

/*
 * Gibt zurück, ob im aktuellen Zustand das Heizelement eingeschaltet sein soll.
 *
 * @return true = Heizelement soll an sein, false = Heizelement soll aus sein
 */
bool states_heating_is_enabled(void);

/*
 * Gibt den aktuellen Zustand als Zeichenkette zurück.
 *
 * @return Zustand als String
 */
const char* states_get_current_state_as_string(void);

/*
 * Gibt den aktuellen Sollwert zurück.
 *
 * @return Sollwert in Grad Celsius
 */
uint8_t states_get_target_value(void);

/*
 * Versetzt den Zustandsautomat in den Fehlerzustand (ERROR).
 * Der Fehlerzustand bleibt bis zum Programmende bestehen.
 */
void states_set_error(void);

/*
 * Aktualisiert den Zustandsautomat.
 * Es wird die Übergangslogik und die Ausgangslogik ausgeführt und schließlich das Display aktualisiert.
 *
 * Diese Funktion muss ständig in einer Schleife aufgerufen werden.
 */
void states_refresh(void);