/*
 * states.c
 *
 * Created: 23.04.2026 20:59:55
 *  Author: Robin Nathmann
 */ 


#include "states.h"
#include "config.h"
#include "hal/display.h"
#include "hal/sensor.h"
#include "driver/buttons.h"
#include "timer/millis_timer3.h"
#include <stdbool.h>
#include <stdint.h>


// Zustände des Automaten
typedef enum {
	ERROR,
	OFF_WAIT,
	OFF,
	ON_WAIT,
	ON,
	SET,
	SET_WAIT,
	UP,
	UP_FAST,
	DOWN,
	DOWN_FAST
} machine_state;

// Zustände der Taster
typedef enum {
	BUTTON_NONE = 0b00,
	BUTTON_UP   = 0b10,
	BUTTON_DOWN = 0b01,
	BUTTON_BOTH = 0b11
} button_state;


// Zustand der Initialisierung
static bool init = false;

// Zustände
static machine_state state = OFF_WAIT;	// Aktueller Zustand und Anfangszustand
static machine_state old_state;			// Vorheriger Zustand zum Vergleichen

// Sollwert Variabeln
static uint8_t states_target_value = STATES_DEFAULT_TARGET_VALUE_IN_CELSIUS;	// Aktueller Sollwert
static uint8_t tmp_target_value = STATES_DEFAULT_TARGET_VALUE_IN_CELSIUS;		// Temporärer Sollwert zur Aktualisierung des aktuellen Sollwerts

// Vorherige Werte
static int32_t old_temp;			// Vorherige Temperatur
static uint8_t old_target_value;	// Vorheriger Sollwert

// Timer Variabel für Zeitmessung
static uint16_t states_timer = 0;


/*
 * Initialisiert den Zustandsautomaten.
 */
void states_init(void)
{
	if (init) return;
	
	// Benötigte Module initialisieren
	buttons_init();
	display_init();
	millis_timer3_init();
	sensor_init();
	
	init = true;
}


/*
 * Gibt zurück, ob im aktuellen Zustand das Heizelement eingeschaltet sein soll.
 *
 * @return true = Heizelement soll an sein, false = Heizelement soll aus sein
 */
bool states_heating_is_enabled(void)
{
	// Das Heizelement soll nur in folgenden Zuständen an sein: ON_WAIT, ON, SET, SET_WAIT, UP, UP_FAST, DOWN, DOWN_FAST
	return (state == ON_WAIT || state == ON || state == SET || state == SET_WAIT || state == UP || state == UP_FAST || state == DOWN || state == DOWN_FAST);
}


/*
 * Gibt den aktuellen Zustand als Zeichenkette zurück.
 *
 * @return Zustand als String
 */
const char* states_get_current_state_as_string(void)
{
	switch(state)
	{
		case ERROR:     return "ERROR";
		case OFF_WAIT:  return "OFF_WAIT";
		case OFF:       return "OFF";
		case ON_WAIT:   return "ON_WAIT";
		case ON:        return "ON";
		case SET:       return "SET";
		case SET_WAIT:  return "SET_WAIT";
		case UP:        return "UP";
		case UP_FAST:   return "UP_FAST";
		case DOWN:      return "DOWN";
		case DOWN_FAST: return "DOWN_FAST";
		default:        return "UNKNOWN";
	}
}


/*
 * Versetzt den Zustandsautomat in den Fehlerzustand (ERROR).
 * Der Fehlerzustand bleibt bis zum Programmende bestehen.
 */
void states_set_error(void) {
	state = ERROR;
}


/*
 * Erhöht den Temporären Sollwert um 1 Grad Celsius.
 * Der Wert kann nur bis zum Maximalwert
 * STATES_MAX_TARGET_VALUE_IN_CELSIUS erhöht werden.
 */
static void increase_target_value(void)
{	
	// Nur ausführen, wenn Sollwertänderung legitim ist
	if (tmp_target_value + 1 <= STATES_MAX_TARGET_VALUE_IN_CELSIUS) {
		 tmp_target_value++;	// Sollwert um 1 erhöhen
	}
}


/*
 * Verringert den Temporären Sollwert um 1 Grad Celsius.
 * Der Wert kann nur bis zum Minimalwert
 * STATES_MIN_TARGET_VALUE_IN_CELSIUS verringert werden.
 */
static void decrease_target_value(void)
{
	// Nur ausführen, wenn Sollwertänderung legitim ist
	if (tmp_target_value - 1 >= STATES_MIN_TARGET_VALUE_IN_CELSIUS) {
		tmp_target_value--;		// Sollwert um 1 verringern
	}
}


/*
 * Gibt den aktuellen Sollwert zurück.
 *
 * @return Sollwert in Grad Celsius
 */
uint8_t states_get_target_value(void)
{
	return states_target_value;
}


/*
 * Setzt den internen Timer des Zustandsautomaten zurück.
 */
static void reset_timer(void)
{
	states_timer = millis();	// Internen Timer auf jetziger Zeit setzen
}


/*
 * Gibt die interne vergangene Zeit des Zustandsautomaten
 * seit dem letzten Reset zurück.
 *
 * @return Zeit in ms
 */
static uint16_t get_elapsed_time(void)
{
	return (millis() - states_timer);	// Aktuelle Zeit - interner Timer (vorherige Zeit)
}


/*
 * Führt die Übergangslogik des Zustandsautomaten aus.
 * Dadurch wird der Übergang von einen Zustand in einen anderen Zustand realisiert.
 */
static void states_transition_logic(void)
{	
	uint8_t button = buttons_get_state();		// Zustand der Buttons auslesen
	
	switch (state) {
		
		case ERROR:
			break;
			
		case OFF_WAIT:
			switch (button) {
				case BUTTON_NONE: state = OFF; break;
				case BUTTON_UP: break;
				case BUTTON_DOWN: break;
				case BUTTON_BOTH: break;
				default: break; 
			}
			break;
		
        case OFF:
			switch (button) {
				case BUTTON_NONE: break;
				case BUTTON_UP: state = ON_WAIT; break;
				case BUTTON_DOWN: state = ON_WAIT; break;
				case BUTTON_BOTH: state = ON_WAIT; break;
				default: break;
			}
			break;		
			
		case ON_WAIT:
			switch (button) {
				case BUTTON_NONE: state = ON; break;
				case BUTTON_UP: break;
				case BUTTON_DOWN: break;
				case BUTTON_BOTH: break;
				default: break;
			}
			break;
			
		case ON:
			switch (button) {
				case BUTTON_NONE: break;
				case BUTTON_UP:	state = UP; break;
				case BUTTON_DOWN: state = DOWN; break;
				case BUTTON_BOTH: state = OFF_WAIT; break;
				default: break;
		}
		break;
		
		case SET_WAIT:
			switch (button) {
				case BUTTON_NONE: state = SET; break;
				case BUTTON_UP:	break;
				case BUTTON_DOWN: break;
				case BUTTON_BOTH: state = OFF_WAIT; break;
				default: break;
			}
			break;
			
		case SET:
			switch (button) {
				case BUTTON_NONE: break;
				case BUTTON_UP:	state = UP; increase_target_value(); break;			// Temporären Sollwert erhöhen
				case BUTTON_DOWN: state = DOWN; decrease_target_value(); break;		// Temporären Sollwert verringern
				case BUTTON_BOTH: state = OFF_WAIT; break;
				default: break;
			}
			if (get_elapsed_time() >= STATES_TARGET_TEMP_TIMEOUT_MS) state = ON_WAIT;	// Zustandswechsel bei abgelaufener Zeit
			break;
			
		case UP:
			switch (button) {
				case BUTTON_NONE: state = SET; break;
				case BUTTON_UP:	break;
				case BUTTON_DOWN: state = SET_WAIT; break;
				case BUTTON_BOTH: state = OFF_WAIT; break;
				default: break;
			}
			if (get_elapsed_time() >= STATES_LONG_PRESS_MS) state = UP_FAST;	// Zustandswechsel bei abgelaufener Zeit
			break;
			
		case UP_FAST:
			switch (button) {
				case BUTTON_NONE: state = SET; break;
				case BUTTON_UP:	break;
				case BUTTON_DOWN: state = SET_WAIT; break;
				case BUTTON_BOTH: state = OFF_WAIT; break;
				default: break;
			}
			break;
			
		case DOWN:
			switch (button) {
				case BUTTON_NONE: state = SET; break;
				case BUTTON_UP: state = SET_WAIT; break;
				case BUTTON_DOWN: break;
				case BUTTON_BOTH: state = OFF_WAIT; break;
				default: break;
			}
			if (get_elapsed_time() >= STATES_LONG_PRESS_MS) state = DOWN_FAST;	// Zustandswechsel bei abgelaufener Zeit
			break;
			
		case DOWN_FAST:
			switch (button) {
				case BUTTON_NONE: state = SET; break;
				case BUTTON_UP:	state = SET_WAIT; break;
				case BUTTON_DOWN: break;
				case BUTTON_BOTH: state = OFF_WAIT; break;
				default: break;
			}
			break;
			
		default:
			state = ERROR;
			break;
	}
	
	if(state != old_state) reset_timer();	// Bei ausgeführter Zustandsänderung internen Timer zurücksetzen
}


/*
 * Führt die Übergangslogik des Zustandsautomaten aus.
 * Der momentane Zustand bestimmt den Ausgang des Automaten.
 * Dies beeinflusst das Display und den Sollwert.
 */
static void states_output_logic(void)
{
	switch (state) {
		
		case ERROR:
			display_set_error();						// ERROR auf Anzeige ausgeben
			display_set_blinking_enabled(false);		// Anzeige statisch
			break;
			
		case OFF_WAIT:
		case OFF:
			if(state == old_state) return;				// Nur ausführen, wenn sich Zustand verändert hat
			display_set_off();							// OFF auf Anzeige ausgeben
			display_set_blinking_enabled(false);		// Anzeige statisch
			tmp_target_value = states_target_value;
			break;	
			
		case ON_WAIT:
		case ON:
			;
			int32_t temp = sensor_get_last_temp();					// Letztes Ergebnis der Temperaturmessung auslesen
			if(state == old_state && temp == old_temp) return;		// Nur ausführen, wenn sich Zustand oder Temperatur verändert hat
			old_temp = temp;
			display_set_temp(temp);									// Temperatur auf Anzeige ausgeben
			display_set_blinking_enabled(false);					// Anzeige statisch
			states_target_value = tmp_target_value;					// Temporären Sollwert in richtigen Sollwert schreiben
			break;
		
		case SET_WAIT:
		case SET:
			if(state == old_state && tmp_target_value == old_target_value) return;		// Nur ausführen, wenn sich Zustand oder Sollwert verändert hat
			old_target_value = tmp_target_value;
			display_set_uint8_with_suffix(tmp_target_value);							// Temporären Sollwert auf Anzeige ausgeben
			display_set_blinking_enabled(true);											// Anzeige blinken
			break;
			
		case UP:
			if(state == old_state && tmp_target_value == old_target_value) return;		// Nur ausführen, wenn sich Zustand oder Sollwert verändert hat
			old_target_value = tmp_target_value;
			display_set_uint8_with_suffix(tmp_target_value);							// Temporären Sollwert auf Anzeige ausgeben
			display_set_blinking_enabled(false);										// Anzeige statisch
			break;
			
		case UP_FAST:
			if(get_elapsed_time() >= STATES_FAST_MS_PER_STEP) {							// Nur ausführen, wenn Cooldown abgelaufen ist
				reset_timer();															// Interne Zeitmessung zurücksetzen
				increase_target_value();												// Temporären Sollwert erhöhen
			}
			if(state == old_state && tmp_target_value == old_target_value) return;		// Nur ausführen, wenn sich Zustand oder Sollwert verändert hat
			old_target_value = tmp_target_value;
			display_set_uint8_with_suffix(tmp_target_value);							// Temporären Sollwert auf Anzeige ausgeben
			display_set_blinking_enabled(false);										// Anzeige statisch
			break;
			
		case DOWN:
			if(state == old_state && tmp_target_value == old_target_value) return;		// Nur ausführen, wenn sich Zustand oder Sollwert verändert hat
			old_target_value = tmp_target_value;
			display_set_uint8_with_suffix(tmp_target_value);							// Temporären Sollwert auf Anzeige ausgeben
			display_set_blinking_enabled(false);										// Anzeige statisch
			break;
			
		case DOWN_FAST:
			if(get_elapsed_time() >= STATES_FAST_MS_PER_STEP) {							// Nur ausführen, wenn Cooldown abgelaufen ist
				reset_timer();															// Interne Zeitmessung zurücksetzen
				decrease_target_value();												// Temporären Sollwert verringern
			}
			if(state == old_state && tmp_target_value == old_target_value) return;		// Nur ausführen, wenn sich Zustand oder Sollwert verändert hat
			old_target_value = tmp_target_value;
			display_set_uint8_with_suffix(tmp_target_value);							// Temporären Sollwert auf Anzeige ausgeben
			display_set_blinking_enabled(false);										// Anzeige statisch
			break;
			
		default:
			if(state == old_state) return;			// Nur ausführen, wenn sich Zustand verändert hat
			display_set_error();					// ERROR auf Anzeige ausgeben
			display_set_blinking_enabled(false);	// Anzeige statisch
			break;
	}		
}


/*
 * Aktualisiert den Zustandsautomat.
 * Es wird die Übergangslogik und die Ausgangslogik ausgeführt und schließlich das Display aktualisiert.
 *
 * Diese Funktion muss ständig in einer Schleife aufgerufen werden.
 */
void states_refresh(void)
{	
	old_state = state;			// Zustand sichern
	
	states_transition_logic();	// Übergangslogik
	
	states_output_logic();		// Ausgabelogik	
	
	display_refresh();			// Display refreshen
}