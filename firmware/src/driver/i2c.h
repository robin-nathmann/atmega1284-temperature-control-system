/*
 * i2c.h
 *
 * Created: 06.04.2026 18:54:24
 *  Author: Robin Nathmann
 */ 


#pragma once
#include <stdint.h>


// I2C-Status-Codes
typedef enum {
	I2C_OK						= 0,
	// Timeouts
	I2C_ERR_START_TIMEOUT		= 10,
	I2C_ERR_STOP_TIMEOUT		= 11,
	I2C_ERR_WRITE_TIMEOUT		= 12,
	I2C_ERR_READ_ACK_TIMEOUT	= 13,
	I2C_ERR_READ_NACK_TIMEOUT	= 14,
	// Protokollfehler
	I2C_ERR_START_FAILED		= 20,
	I2C_ERR_SLA_NACK			= 21,	// Slave hat nicht geantwortet
	I2C_ERR_DATA_NACK			= 22,   // Byte wurde mit NACK abgewiesen
	// Busfehler
	I2C_ERR_WRITE_BUS_ERROR		= 30,
	I2C_ERR_READ_ACK_BUS_ERROR	= 31,
	I2C_ERR_READ_NACK_BUS_ERROR = 32,
	// Softwarefehler
	I2C_ERR_DATA_NULL_POINTER	= 90
} i2c_status_t;


/*
 * Initialisiert den I2C Treiber.
 */
void i2c_init(void);

/*
 * Deaktiviert die I2C Schnittstelle.
 */
void i2c_kill(void);

/*
 * Initialisiert eine I2C-Startbedingung.
 *
 * @return I2C-Status
 */
i2c_status_t i2c_start(void);

/*
 * Beendet die I2C- Kommunikation indem eine I2C-Stopbedingung ausgelöst wird.
 *
 * @return I2C-Status
 */
i2c_status_t i2c_stop(void);

/*
 * Sendet ein Byte über I2C.
 *
 * @param data Byte
 *
 * @return I2C-Status
 */
i2c_status_t i2c_write(uint8_t data);

/*
 * Ließt über I2C ein Byte von einem Slave und gibt
 * dem Slave eine Bestätigung (ACK) zurück.
 *
 * @param *data Zeiger auf Adresse, wo Antwort geschrieben werden soll
 *
 * @return I2C-Status
 */
i2c_status_t i2c_read_ack(uint8_t *data);

/*
 * Ließt über I2C ein Byte von einem Slave und gibt
 * dem Slave keine Bestätigung (NACK) zurück.
 *
 * @param *data Zeiger auf Adresse, wo Antwort geschrieben werden soll
 *
 * @return I2C-Status
 */	
i2c_status_t i2c_read_nack(uint8_t *data);