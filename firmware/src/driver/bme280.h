/*
 * bme280.h
 *
 * Created: 06.04.2026 19:31:21
 *  Author: Robin Nathmann
 */ 


#pragma once
#include <stdint.h>
#include "i2c.h"


// BME280-Status-Codes
typedef enum {
	BME280_OK = 0,
	// Initialisierung
	BME280_ERR_ALREADY_INIT = 10,
	BME280_ERR_BOOT_TIMEOUT = 11,
	BME280_ERR_TRIMMING_READ = 12,
	// Status
	BME280_ERR_STATUS_READ = 20,
	// Messungen
	BME280_ERR_MEAS_START = 30,
	BME280_ERR_MEAS_TIMEOUT = 31,
	BME280_ERR_TEMP_READ = 32,
	// Reset
	BME280_ERR_RESET = 41
} bme280_status_t;


/*
 * Letzten I2C-Status-Code auslesen.
 *
 * @return I2C-Status
 */
i2c_status_t bme280_get_last_i2c_error(void);

/*
 * Initialisiert den BME280 Sensor.
 *
 * @return BME280-Status
 */
bme280_status_t bme280_init(void);

/*
 * Startet die Messung des BME280 Sensors.
 *
 * @return BME280-Status
 */
bme280_status_t bme280_start_measurement(void);

/*
 * Blockiert so lange, bis die Messung des BME280-Sensors abgeschlossen ist.
 *
 * @return BME280-Status
 */
bme280_status_t bme280_wait_till_measurement_done(void);

/*
 * Lieﬂt das Ergebnis der Temperaturmessung des BME280-Sensor ein.
 *
 * @param *T Zieladresse der Messung
 *
 * @return BME280-Status
 */
bme280_status_t bme280_read_temp(int32_t *T);

/*
 * Startet den BME280-Sensor neu.
 *
 * @return BME280-Status
 */
bme280_status_t bme280_restart(void);