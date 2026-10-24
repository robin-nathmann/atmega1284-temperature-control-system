/*
 * sampler.h
 *
 * Created: 06.05.2026 19:02:30
 *  Author: Robin Nathmann
 */ 


#pragma once


/*
 * Initialisiert den Abtaster.
 */
void sampler_init(void);

/*
 * Aktualisiert den Abtaster. Der Abtaster stößt die Temperaturmessung an, übermittelt die
 * Regelgrößen, steuert den Treiber für das Heizelement und den Lüfter an, schaltet diese im
 * Fehlerfall aus und übergibt Logging Werte zum Debug Modul.
 *
 * Diese Funktion muss ständig in einer Schleife aufgerufen werden. Die Logik wird nur ausgeführt,
 * wenn die Abtastzeit erreicht ist. Die Abtastzeit wird über PI_CONTROLLER_T0_MS bestimmt.
 */
void sampler_refresh(void);