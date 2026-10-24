/*
 * i2c.c
 *
 * Created: 06.04.2026 18:23:10
 *  Author: Robin Nathmann
 */ 


#include "i2c.h"
#include "../config.h"
#include "../timer/millis_timer3.h"
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <stdbool.h>
#include <util/twi.h>
#include <stddef.h>


// Zustand der Initialisierung
static bool init = false;


/*
 * Initialisiert den I2C Treiber.
 */
void i2c_init(void)
{
	if (init) return;		// Wenn schon initialsiert, abbrechen
	
	millis_timer3_init();	// Timer3 initialisieren
	
	DDRC &= ~((1 << PC0) | (1 << PC1));		// Pins als Eingang setzen
	PORTC |= (1 << PC0) | (1 << PC1);		// Pullups aktivieren
	
	// TWBR Wert berechnen
	int32_t twbr =((int32_t)F_CPU/(2*(int32_t)F_I2C)) - 8;		// twbr = ( F_CPU/(2*F_I2C) ) - 8
	if (twbr < 0) twbr = 0;										// Auf 0 setzen falls negativ
	if (twbr > 0xFF) twbr = 0xFF;								// Auf 8 Bit sättigen falls Overflow
	TWBR = twbr;												// TWI Register setzen
	
	TWSR = 0;				// Prescaler 1
	TWCR = (1 << TWEN);		// I2C Schnittstelle aktivieren
	
	init = true;			// Initialisierung bestätigen
}


/*
 * Deaktiviert die I2C Schnittstelle.
 */
void i2c_kill(void)
{
	TWCR = 0;		// I2C Schnittstelle deaktivieren
	init = false;	// Initialisierung aufheben
}


/*
 * Blockiert so lange, bis die I2C-Operation angeschlossen ist.
 * Über I2C_TIMEOUT_MS wird ein Timeout definiert, ab welchem
 * das Blockieren beendet wird und Fehler zurückgegeben wird.
 *
 * @return 0 = Erfolg, 1 = Fehler (Timeout)
 */
static uint8_t i2c_wait(void)
{
	uint16_t start = millis();							// Systemzeit speichern
	
	while (!(TWCR & (1 << TWINT))) {					// Läuft so lange, wie I2C-Operation läuft
		if ((millis() - start) >= I2C_TIMEOUT_MS) {		// Falls maximale Wartezeit überschritten	
			TWCR = 0;									// I2C Schnittstelle deaktivieren
			TWCR = (1 << TWEN);							// I2C Schnittstelle aktivieren
			return 1;									// Fehlercode				
		}
	}
	return 0;											// Erfolg
}


/*
 * Initialisiert eine I2C-Startbedingung.
 *
 * @return I2C-Status
 */
i2c_status_t i2c_start(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);	// Löst Startbedingung aus
	
	if (i2c_wait() != 0) return I2C_ERR_START_TIMEOUT;	// Wartet auf Abschluss, sonst Timeout Error
	
	uint8_t status = TW_STATUS;							// Ließt I2C Statuscode aus
	
	// Fehler, wenn weder Erfolg bei normaler Startbedingung oder Repeated Start
	if (status != TW_START && status != TW_REP_START) return I2C_ERR_START_FAILED;
	
	return I2C_OK;	// Erfolg
}


/*
 * Beendet die I2C- Kommunikation indem eine I2C-Stopbedingung ausgelöst wird.
 *
 * @return I2C-Status
 */
i2c_status_t i2c_stop(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);	// Löst STOP-Bedingung aus
	
	uint16_t start = millis();							// Systemzeit speichern
	
	while (TWCR & (1 << TWSTO)) {						// Läuft so lange bis gestoppt wurde
		if ((millis() - start) >= I2C_TIMEOUT_MS) {		// Falls maximale Wartezeit überschritten
			TWCR = 0;									// I2C Schnittstelle deaktivieren
			TWCR = (1 << TWEN);							// I2C Schnittstelle aktivieren
			return I2C_ERR_STOP_TIMEOUT;				// Fehlercode
		}
	}
	return I2C_OK;										// Erfolg
}


/*
 * Sendet ein Byte über I2C.
 *
 * @param data Byte
 *
 * @return I2C-Status
 */
i2c_status_t i2c_write(uint8_t data)
{
	TWDR = data;										// Schreibt Daten in Register für den Bus
	TWCR = (1 << TWINT) | (1 << TWEN);					// Aktiviert Übertragung
	
	if (i2c_wait() != 0) return I2C_ERR_WRITE_TIMEOUT;	// Wartet auf Abschluss, sonst Timeout Error
	
	uint8_t status = TW_STATUS;							// Ließt I2C Statuscode aus
	
	// Überprüft ob Slave Erfolg (ACK) gemeldet hat -> Dann Erfolg
	if (status == TW_MT_SLA_ACK || status == TW_MT_DATA_ACK || status == TW_MR_SLA_ACK) return I2C_OK;
	
	// Überprüft ob Slave Anfrage nicht bestätigt hat (NACK) -> Dann Error
	if (status == TW_MT_SLA_NACK || status == TW_MT_DATA_NACK || status == TW_MR_SLA_NACK) return I2C_ERR_SLA_NACK;
	
	return I2C_ERR_WRITE_BUS_ERROR;		// Busfehler
}


/*
 * Ließt über I2C ein Byte von einem Slave und gibt
 * dem Slave eine Bestätigung (ACK) zurück.
 *
 * @param *data Zeiger auf Adresse, wo Antwort geschrieben werden soll
 *
 * @return I2C-Status
 */	
i2c_status_t i2c_read_ack(uint8_t *data)
{
	if (data == NULL) return I2C_ERR_DATA_NULL_POINTER;		// Error falls Zeiger ungültig
	
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);		// Lesen mit ACK starten
	
	if (i2c_wait() != 0) return I2C_ERR_READ_ACK_TIMEOUT;	// Wartet auf Abschluss, sonst Timeout Error 
	
	if (TW_STATUS != TW_MR_DATA_ACK) return I2C_ERR_READ_ACK_BUS_ERROR;		// Busfehler
	
	*data = TWDR;	// Daten in Adresse schreiben
	
	return I2C_OK;	// Erfolg
}


/*
 * Ließt über I2C ein Byte von einem Slave und gibt
 * dem Slave keine Bestätigung (NACK) zurück.
 *
 * @param *data Zeiger auf Adresse, wo Antwort geschrieben werden soll
 *
 * @return I2C-Status
 */	
i2c_status_t i2c_read_nack(uint8_t *data)
{
	if (data == NULL) return I2C_ERR_DATA_NULL_POINTER;		// Error falls Zeiger ungültig
	
	TWCR = (1 << TWINT) | (1 << TWEN);						// Lesen mit NACK starten (ohne ACK)
	
	if (i2c_wait() != 0) return I2C_ERR_READ_NACK_TIMEOUT;	// Wartet auf Abschluss, sonst Timeout Error
	
	if (TW_STATUS != TW_MR_DATA_NACK) return I2C_ERR_READ_NACK_BUS_ERROR;	// Busfehler
	
	*data = TWDR;	// Daten in Adresse schreiben
	
	return I2C_OK;	// Erfolg
}