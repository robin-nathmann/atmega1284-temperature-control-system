/*
 * sensor.c
 *
 * Created: 03.05.2026 19:59:40
 *  Author: Robin Nathmann
 */ 


#include "sensor.h"
#include "../config.h"
#include "../driver/bme280.h"
#include "../hal/debug.h"
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>


// Zustand der Initialisierung
static bool init = false;

// Anzahl der aufgetretenen fehlerhaften Messungen
static uint8_t error_counter = -1;

// Zuletzt gemessene Temperatur (initialisiert auf INT_MAX)
static int32_t sensor_current_temp = INT_MAX;


/*
 * Initialisiert das Sensor HAL.
 */
void sensor_init(void)
{
	if (init) return;
	
	// Benötigte Module initialisieren
	bme280_init();
	debug_init();
	
	init = true;
}


/*
 * Gibt die Temperatur der letzten Messung zurück.
 *
 * @return Zuletzt gemessene Temperatur (Format: 30 Grad Celsius = 3000)		
 */
int32_t sensor_get_last_temp(void)
{
	return sensor_current_temp;
}


/*
 * Überprüft die Korrektheit einer Temperaturmessung.
 *
 * @param temp Temperatur (Format: 30 Grad Celsius = 3000)
 *
 * @return	   true = Temperatur ist valide, false = Temperatur ist fehlerhaft
 */
static bool sensor_is_value_valid(int32_t temp)
{
	// Wenn Temperatur höher als Max-Wert oder kleiner als Min-Wert
	if (temp >= SENSOR_MAX_VALID_TEMP*100 || temp <= SENSOR_MIN_VALID_TEMP*100) {
		debug_print_value_error("RANGE_ERROR", temp);	// Fehler über Debug Schnittstelle protokollieren
		return false;
	}
	
	// Wenn Abweichung zu letzter Messung größer als
	if (sensor_current_temp != INT_MAX && abs(temp - sensor_current_temp) >= SENSOR_MAX_VALID_DEVIATION*100) {
		debug_print_value_error("DEVIATION_ERROR", temp);	// Fehler über Debug Schnittstelle protokollieren
		return false;
	}
	
	return true;
}


/*
 * Startet eine Temperaturmessung und gibt die gemessene Temperatur zurück.
 * Steuert dafür den BME280-Treiber an und realisiert eine Fehlerbehandlung.
 *
 * @return Temperatur (Format: 30 Grad Celsius = 3000)
 *					  Im wiederholten Fehlerfall wird INT_MAX zurückgegeben,
 *					  ansonsten wird vorherige Temperatur zurückgegeben.				 
 */
int32_t sensor_get_temperature(void)
{
	int32_t temp = INT_MAX;		// Variabel für Temperatumessung
	bool valid = false;			// Variabel für Prüfung auf Korrektheit
	
	bme280_status_t status = bme280_start_measurement();	// Temperaturmessung starten
	
	if (status == BME280_OK) {								// Wenn Temperaturmessung erfolgreich gestartet
		status = bme280_wait_till_measurement_done();		// Warten bis Messung abgeschlossen ist
	}
	
	if (status == BME280_OK) {								// Wenn Temperaturmessung erfolgreich abgeschlossen
		status = bme280_read_temp(&temp);					// Temperatur auslesen
	}
	
	if (status == BME280_OK) {								// Wenn Temperatur erfolgreich ausgelesen
		valid = sensor_is_value_valid(temp);				// Temperatur auf Korrektheit überprüfen
	} else {
		debug_print_bus_error((uint16_t)status, (uint16_t)bme280_get_last_i2c_error());		// Fehler über Debug Schnittstelle protokollieren
	}
	
	if (status == BME280_OK && valid) {		// Wenn Temperaturmessung erfolgreich war und Wert korrekt ist
		sensor_current_temp = temp;			// Aktuelle Temperatur überschreiben
		error_counter = 0;					// Anzahl an Fehlern zurücksetzen
	} else {
		// Wenn Temperaturmessung fehlerhaft und Anzahl an aufeinanderfolgenden Fehlern zu hoch (oder erste Messung bereits fehlerhaft)
		if (error_counter < 0 || error_counter > SENSOR_MAX_CONSECUTIVE_ERRORS) {
			sensor_current_temp = INT_MAX;												// AKtuelle Temperatur auf INT_MAX setzen
		} else {
			error_counter++;		// Fehlerzähler erhöhen
		}
		bme280_restart();			// BME280 neustarten
	}
	
	return sensor_current_temp;		// Aktuelle Temperatur zurückgeben
}



