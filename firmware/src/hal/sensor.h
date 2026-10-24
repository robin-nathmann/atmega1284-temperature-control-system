/*
 * sensor.h
 *
 * Created: 03.05.2026 20:37:09
 *  Author: Robin Nathmann
 */ 


#pragma once
#include <stdbool.h>
#include <stdint.h>


/*
 * Initialisiert das Sensor HAL.
 */
void sensor_init(void);

/*
 * Gibt die Temperatur der letzten Messung zurück.
 *
 * @return Zuletzt gemessene Temperatur (Format: 30 Grad Celsius = 3000)		
 */
int32_t sensor_get_last_temp(void);

/*
 * Startet eine Temperaturmessung und gibt die gemessene Temperatur zurück.
 * Steuert dafür den BME280-Treiber an und realisiert eine Fehlerbehandlung.
 *
 * @return Temperatur (Format: 30 Grad Celsius = 3000)
 *					  Im wiederholten Fehlerfall wird INT_MAX zurückgegeben,
 *					  ansonsten wird vorherige Temperatur zurückgegeben.				 
 */
int32_t sensor_get_temperature();