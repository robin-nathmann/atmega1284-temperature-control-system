/*
 * uart.c
 *
 * Created: 04.04.2026 18:44:04
 *  Author: Robin Nathmann
 */ 


#include "uart.h"
#include "../config.h"
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <stdlib.h>
#include <util/setbaud.h>
#include <avr/interrupt.h>


// Zustand der Initialisierung
static bool init = false;

// Ringpuffer und Indizes
static volatile uint8_t tx_buffer[UART_TX_BUFFER_SIZE];		// Ringpuffer
static volatile uint8_t tx_head = 0;						// Schreibindex
static volatile uint8_t tx_tail = 0;						// Leseindex


/*
 * Initialisiert den UART Treiber.
 */
void uart_init(void)
{	
	if (init) return;		// Wenn schon initialsiert, abbrechen
	
	UBRR0H = UBRRH_VALUE;	// USART Baud Rate Register (High Byte): Wird auf setbaud.h Makro UBRRH_VALUE gesetzt
	UBRR0L = UBRRL_VALUE;	// USART Baud Rate Register (Low Byte): Wird auf setbaud.h Makro UBRRL_VALUE gesetzt
	
	// Double Speed falls setbaud.h das möchte
	#if USE_2X
		UCSR0A |= (1 << U2X0);	// Doppelte Geschwindigkeit: U2X0 = 1
	#else
		UCSR0A &= ~(1 << U2X0);	// Normale Geschwindigkeit: U2X0 = 0
	#endif
	
	UCSR0B = (1 << TXEN0);						// Senden aktivieren
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);		// 8 Datenbits, 1 Stoppbit und keine Parität
	
	init = true;	// Initialisierung bestätigen
}


/*
 * Gibt eine Zeichenkette über die UART Schnittstelle aus.
 *
 * @param str Zeichenkette (String)
 */
void uart_print_string(const char* str)
{	
	while(*str) {															// Läuft bis zum letzten Zeichen
		uint8_t next_head = (tx_head + 1) & (UART_TX_BUFFER_SIZE - 1);		// Nächste Schreibposition im Ringpuffer berechnen
		while (next_head == tx_tail);										// Falls der Ringpuffer voll ist: Warten, bis die ISR Platz geschaffen hat
		tx_buffer[next_head] = (uint8_t)*str++;								// Schreibt Zeichen in Ringpuffer und springt zum nächsten Zeichen
		tx_head = next_head;												// Puffer Kopf aktualisieren
		UCSR0B |= (1 << UDRIE0);											// Data Register Empty Interrupt aktivieren (startet Übertragung)								
	}	
}


/*
 * Gibt eine vorzeichenbehaftete 32 Bit Ganzzahl über die UART Schnittstelle aus.
 *
 * @param val Vorzeichenbehaftete 32 Bit Ganzzahl
 */
void uart_print_int32(int32_t val)
{
	char buf[12];				// 12 Byte Zeichenarray als Buffer
	ltoa(val, buf, 10);			// Ganzzahl zu String konvertieren und in Buffer schreiben
	uart_print_string(buf);		// String über UART ausgeben
}


/*
 * Gibt eine nicht vorzeichenbehaftete 16 Bit Ganzzahl über die UART Schnittstelle aus.
 *
 * @param val Nicht vorzeichenbehaftete 16 Bit Ganzzahl
 */
void uart_print_uint16(uint16_t val)
{
	char buf[6];				// 6 Byte Zeichenarray als Buffer
	utoa(val, buf, 10);			// Unsigned Int zu String konvertieren und in Buffer schreiben
	uart_print_string(buf);		// String über UART ausgeben
}


/*
 * Gibt eine Fließkommazahl über die UART Schnittstelle aus.
 *
 * @param val Fließkommazahl
 */
void uart_print_double(double val)
{
	char buf[16];										// 16 Byte Zeichenarray als Buffer
	dtostrf(val, 0, UART_DOUBLE_DECIMAL_PLACES, buf);	// Double zu String konvertieren mit min. 0 Zeichen, UART_DOUBLE_DECIMAL_PLACES Nachkommastellen und in Buffer schreiben
	uart_print_string(buf);								// String über UART ausgeben
}


/*
 * Interrupt Service Routine für UART Senderegister.
 * Löst aus wenn Senderegister (UDR0) bereit für nächstes Byte ist.
 */
ISR(USART0_UDRE_vect)
{
    if (tx_head != tx_tail) {									// Wenn Zeichen im Ringpuffer sind
        tx_tail = (tx_tail + 1) & (UART_TX_BUFFER_SIZE - 1);	// Nächsten Index berechnen
        UDR0 = tx_buffer[tx_tail];								// Zeichen aus Ringpuffer holen und senden
    } else {
        UCSR0B &= ~(1 << UDRIE0);								// Puffer ist leer, UDRE-Interrupt deaktivieren
    }
}