/*
 * bme280.c
 *
 * Created: 06.04.2026 19:21:49
 *  Author: Robin Nathmann
 */ 


#include "bme280.h"
#include "../config.h"
#include "i2c.h"
#include "../timer/millis_timer3.h"
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>


// Zustand der Initialisierung
static bool init = false;

// Letzter I2C Status
static i2c_status_t bme280_last_i2c_status = I2C_OK;

// Globale Variablen für Kalibrierungsdaten
static uint16_t dig_T1;
static int16_t  dig_T2, dig_T3;
static int32_t t_fine;


// Makro für Errorhandling
// Merkt sich I2C Status, sendet Stop-Bedingung im Fehlerfall und gibt Fehler zurück
#define I2C_CHECK(i2c_func, bme_err) \
do { \
	bme280_last_i2c_status = (i2c_func); \
	if (bme280_last_i2c_status != I2C_OK) { \
		i2c_stop(); \
		return (bme_err); \
	} \
} while(0)


/*
 * Letzten I2C-Status-Code auslesen.
 *
 * @return I2C-Status
 */
i2c_status_t bme280_get_last_i2c_error(void)
{
	return bme280_last_i2c_status;
}


/*
 * Ließt den aktuellen Status des BME280-Sensors ein.
 *
 * @return BME280-Status
 */
static bme280_status_t bme280_get_status(uint8_t *status)
{
	// Status-Daten WRITE
	I2C_CHECK(i2c_start(), BME280_ERR_STATUS_READ);					// Start-Bedingung senden
	I2C_CHECK(i2c_write(BME280_ADDR << 1), BME280_ERR_STATUS_READ);	// Adresse des BME280 mit Schreibabsicht senden
	I2C_CHECK(i2c_write(0xF3), BME280_ERR_STATUS_READ);				// 0xF3 auf Bus schreiben -> Register: Status
	
	// Status-Daten READ
	I2C_CHECK(i2c_start(), BME280_ERR_STATUS_READ);							// Repeated-Start-Bedingung senden
	I2C_CHECK(i2c_write((BME280_ADDR << 1) | 1), BME280_ERR_STATUS_READ);	// Adresse des BME280 mit Leseabsicht senden
	I2C_CHECK(i2c_read_nack(status), BME280_ERR_STATUS_READ);				// Ließt Status Byte von Sensor ein und meldet NACK (Ende Lesevorgang)
	I2C_CHECK(i2c_stop(), BME280_ERR_STATUS_READ);							// STOP-Bedingung senden
	
	return BME280_OK;	// Erfolg
}


/*
 * Blockiert so lange, bis der BME280-Sensor hochgefahren und bereit für Befehle ist.
 *
 * @return BME280-Status
 */
static bme280_status_t bme280_wait_till_booted(void)
{	
	uint16_t start = millis();								// Systemzeit speichern
	while ((millis() - start) < BME280_BOOT_DELAY_MS) {}	// Warten bis Sensor korrekte Antwort geben kann
	
	while (1) {												// Endlosschleife
		uint8_t status;
		bme280_status_t err = bme280_get_status(&status);	// Ließt Statusregister von BME280 aus
		if (err != BME280_OK) return err;					// Fehler durchreichen
		
		if ((status & 0b00000001) == 0) {					// Wenn Bit 0 (im_update) LOW ist, ist Sensor bereit
			return BME280_OK;								// Erfolg
		}

		if ((millis() - start) >= BME280_BOOT_TIMEOUT_MS + BME280_BOOT_DELAY_MS) {	// Falls maximale Wartezeit überschritten	
			return BME280_ERR_BOOT_TIMEOUT;											// Fehlercode
		}
	}
}


/*
 * Startet den BME280 Sensor neu und setzt ihn zurück.
 *
 * @return BME280-Status
 */
static bme280_status_t bme280_reset(void)
{
	I2C_CHECK(i2c_start(), BME280_ERR_RESET);					// Start-Bedingung senden
	I2C_CHECK(i2c_write(BME280_ADDR << 1), BME280_ERR_RESET);	// Adresse des BME280 mit Schreibabsicht senden
	I2C_CHECK(i2c_write(0xE0), BME280_ERR_RESET);				// 0xE0 auf Bus schreiben -> Register: Reset
	I2C_CHECK(i2c_write(0xB6), BME280_ERR_RESET);				// 0xB6 auf Bus schreiben -> Schreibt B6 in Reset Register
	I2C_CHECK(i2c_stop(), BME280_ERR_RESET);					// STOP-Bedingung senden
	
	return BME280_OK;
}



/*
 * Initialisiert den BME280 Sensor.
 *
 * @return BME280-Status
 */
bme280_status_t bme280_init(void)
{
	bme280_status_t err;
	
	if (init) return BME280_ERR_ALREADY_INIT;	// Wenn schon initialsiert, abbrechen
	
	i2c_init();									// I2C Treiber initialisieren
	millis_timer3_init();						// Millis-Timer initialisieren
	
	err = bme280_reset();						// BME280 Resetten
	if (err != BME280_OK) goto init_error;		// Fehlerbehandlung
	
	err = bme280_wait_till_booted();			// Warten bis Trimming-Daten vorliegen
	if (err != BME280_OK) goto init_error;		// Fehlerbehandlung
	
	// Trimming-Daten WRITE (0x88 bis 0x8D)
	I2C_CHECK(i2c_start(), BME280_ERR_TRIMMING_READ);					// Start-Bedingung senden
	I2C_CHECK(i2c_write(BME280_ADDR << 1), BME280_ERR_TRIMMING_READ);	// Adresse des BME280 mit Schreibabsicht senden
	I2C_CHECK(i2c_write(0x88), BME280_ERR_TRIMMING_READ);				// 0x088 auf Bus schreiben -> Befehl: Trimming-Daten auslesen
	
	// Trimming-Daten READ
	uint8_t lsb, msb;
	I2C_CHECK(i2c_start(), BME280_ERR_TRIMMING_READ);						// Repeated-Start-Bedingung senden
	I2C_CHECK(i2c_write((BME280_ADDR << 1) | 1), BME280_ERR_TRIMMING_READ);	// Adresse des BME280 mit Leseabsicht senden
	
	// 0x88 & 0x89 für dig_T1
	I2C_CHECK(i2c_read_ack(&lsb), BME280_ERR_TRIMMING_READ);	// Ließt LSB Byte von Sensor ein und meldet ACK
	I2C_CHECK(i2c_read_ack(&msb), BME280_ERR_TRIMMING_READ);	// Ließt MSB Byte von Sensor ein und meldet ACK
	dig_T1 = (uint16_t)lsb | ((uint16_t)msb << 8);				// Bytes in uint16_t entsprechend anordnen
	
	// 0x8A & 0x8B für dig_T2
	I2C_CHECK(i2c_read_ack(&lsb), BME280_ERR_TRIMMING_READ);	// Ließt LSB Byte von Sensor ein und meldet ACK
	I2C_CHECK(i2c_read_ack(&msb), BME280_ERR_TRIMMING_READ);	// Ließt MSB Byte von Sensor ein und meldet ACK
	dig_T2 = (int16_t)lsb | ((int16_t)msb << 8);				// Bytes in uint16_t entsprechend anordnen
	
	// 0x8C & 0x8D für dig_T3
	I2C_CHECK(i2c_read_ack(&lsb), BME280_ERR_TRIMMING_READ);	// Ließt LSB Byte von Sensor ein und meldet ACK
	I2C_CHECK(i2c_read_nack(&msb), BME280_ERR_TRIMMING_READ);	// Ließt MSB Byte von Sensor ein und meldet NACK (Ende des Lesevorgangs)
	dig_T3 = (int16_t)lsb | ((int16_t)msb << 8);				// Bytes in uint16_t entsprechend anordnen
	
	I2C_CHECK(i2c_stop(), BME280_ERR_TRIMMING_READ);			// STOP-Bedingung senden
	
	init = true;		// Initialisierung bestätigen
	
	return BME280_OK;	// Erfolg
	
	init_error:			// Fehlerbehandlung
	return err;			// Fehler zurückgeben
}


/*
 * Startet die Messung des BME280 Sensors.
 *
 * @return BME280-Status
 */
bme280_status_t bme280_start_measurement(void)
{
	I2C_CHECK(i2c_start(), BME280_ERR_MEAS_START);												// Start-Bedingung senden
	I2C_CHECK(i2c_write(BME280_ADDR << 1), BME280_ERR_MEAS_START);								// Adresse des BME280 mit Schreibabsicht senden
	I2C_CHECK(i2c_write(0xF4), BME280_ERR_MEAS_START);											// 0xF4 auf Bus schreiben -> Register: ctrl_meas
	I2C_CHECK(i2c_write((BME280_TEMP_OVERSAMPLING << 5) | BME280_MODE), BME280_ERR_MEAS_START);	// Oversampling und Modus in Register schreiben
	I2C_CHECK(i2c_stop(), BME280_ERR_MEAS_START);												// STOP-Bedingung senden
	
	return BME280_OK;	// Erfolg
}


/*
 * Blockiert so lange, bis die Messung des BME280-Sensors abgeschlossen ist.
 *
 * @return BME280-Status
 */
bme280_status_t bme280_wait_till_measurement_done(void)
{	
	_delay_us(BME280_MEAS_DELAY_US);	// Warten bis Sensor korrekte Antwort geben kann
	uint16_t start = millis();			// Systemzeit speichern
	
	while (1) {
		uint8_t status;
		bme280_status_t err = bme280_get_status(&status);	// Ließt Statusregister von BME280 aus
		if (err != BME280_OK) return err;					// Fehler durchreichen
		
		if ((status & 0b00001001) == 0) {					// Wenn Bit 3 (measuring) und Bit 0 (im_update) LOW ist, ist Messung abgeschlossen
			return BME280_OK;								// Erfolg
		}
		
		if ((millis() - start) >= BME280_MEAS_TIMEOUT_MS) {	// Falls maximale Wartezeit überschritten	
			return BME280_ERR_MEAS_TIMEOUT;					// Fehlercode
		}
	}
}


/*
 * Berechnet die gemessene Temperatur des BME280-Sensors mittels
 * der Bosch Kompensations-Formel und den Trimming-Daten.
 *
 * @param adc_T Ausgelesene Rohdaten des BME280-Sensors
 *
 * @return Temperatur (Format: 30 Grad Celsius = 3000)
 */
static int32_t bme280_compensate_T_int32(int32_t adc_T)
{
	// C Code übernommen aus Datenblatt von BME280
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}


/*
 * Ließt das Ergebnis der Temperaturmessung des BME280-Sensor ein.
 *
 * @param *T Zieladresse der Messung
 *
 * @return BME280-Status
 */
bme280_status_t bme280_read_temp(int32_t *T)
{
	// Temperatur-Daten WRITE
	I2C_CHECK(i2c_start(), BME280_ERR_TEMP_READ);						// Start-Bedingung senden
	I2C_CHECK(i2c_write(BME280_ADDR << 1), BME280_ERR_TEMP_READ);		// Adresse des BME280 mit Schreibabsicht senden
	I2C_CHECK(i2c_write(0xFA), BME280_ERR_TEMP_READ);					// 0xF4 auf Bus schreiben -> Temp MSB Register
	
	// Temperatur-Daten READ
	uint8_t msb, lsb, xlsb;
	I2C_CHECK(i2c_start(), BME280_ERR_TEMP_READ);						// Repeated-Start-Bedingung senden
	I2C_CHECK(i2c_write((BME280_ADDR << 1) | 1), BME280_ERR_TEMP_READ);	// Adresse des BME280 mit Leseabsicht senden
	I2C_CHECK(i2c_read_ack(&msb), BME280_ERR_TEMP_READ);				// Ließt MSB Byte von Sensor ein und meldet ACK
	I2C_CHECK(i2c_read_ack(&lsb), BME280_ERR_TEMP_READ);				// Ließt LSB Byte von Sensor ein und meldet ACK
	I2C_CHECK(i2c_read_nack(&xlsb), BME280_ERR_TEMP_READ);				// Ließt XLSB Byte von Sensor ein und meldet NACK (Ende des Lesevorgangs)
	I2C_CHECK(i2c_stop(), BME280_ERR_TEMP_READ);						// STOP-Bedingung senden

	int32_t adc_T = ((uint32_t)msb << 12) | ((uint32_t)lsb << 4) | ((uint32_t)xlsb >> 4);	// Bytes in int32_t entsprechend Datenblatt anordnen

	*T = bme280_compensate_T_int32(adc_T);	// Bosch Kompensations-Formel anwenden
	
	return BME280_OK;	// Erfolg
}


/*
 * Startet den BME280-Sensor neu.
 *
 * @return BME280-Status
 */
bme280_status_t bme280_restart(void)
{
	i2c_kill();				// Deaktiviert die I2C Schnittstelle
	init = false;			// Initialisierung aufheben
	return bme280_init();	// Initialisiert den BME280 Sensor
}