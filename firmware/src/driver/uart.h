/*
 * uart.h
 *
 * Created: 04.04.2026 18:53:17
 *  Author: Robin Nathmann
 */ 


#pragma once
#include <stdint.h>
#include <stdbool.h>


/*
 * Initialisiert den UART Treiber.
 */
void uart_init(void);

/*
 * Gibt eine Zeichenkette über die UART Schnittstelle aus.
 *
 * @param str Zeichenkette (String)
 */
void uart_print_string(const char* s);

/*
 * Gibt eine vorzeichenbehaftete 32 Bit Ganzzahl über die UART Schnittstelle aus.
 *
 * @param val Vorzeichenbehaftete 32 Bit Ganzzahl
 */
void uart_print_int32(int32_t val);

/*
 * Gibt eine nicht vorzeichenbehaftete 16 Bit Ganzzahl über die UART Schnittstelle aus.
 *
 * @param val Nicht vorzeichenbehaftete 16 Bit Ganzzahl
 */
void uart_print_uint16(uint16_t val);

/*
 * Gibt eine Fließkommazahl über die UART Schnittstelle aus.
 *
 * @param val Fließkommazahl
 */
void uart_print_double(double val);