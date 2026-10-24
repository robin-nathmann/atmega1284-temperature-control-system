/*
 * pwm_fan.h
 *
 * Created: 15.05.2026 13:32:35
 *  Author: robin
 */ 


#pragma once

/*
 * Initialisiert den PWM Treiber für den Lüfter (Timer2).
 */
void pwm_fan_init(void);

/*
 * Aktiviert die PWM Generierung für den Lüfter (Timer2).
 */
void pwm_fan_enable(void);

/*
 * Deaktiviert die PWM Generierung für den Lüfter (Timer2).
 */
void pwm_fan_disable(void);