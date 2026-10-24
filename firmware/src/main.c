/*
 * Temperaturregler.c
 *
 * Created: 22.02.2026 22:11:30
 * Author : Robin Nathmann
 */ 


#include "config.h"
#include "sampler.h"
#include "states.h"
#include <avr/wdt.h>


/*
 * Hauptfunktion, wird bei Programmstart ausgeführt.
 */
int main(void) {
	
	// Initialisierung der Hauptmodule
	sampler_init();		// Abtaster
	states_init();		// Zustandsautomat
	
	if (WATCHDOG_ENABLED) wdt_enable(WATCHDOG_TIMEOUT); // Watchdog nach Konfiguration aktivieren
	
    while (1) {		// Hauptschleife
		
		sampler_refresh();	// Abtaster refreshen
		states_refresh();	// Zustandsautomat refreshen
		
		if (WATCHDOG_ENABLED) wdt_reset();	// Watchdog zurücksetzen

	}
}