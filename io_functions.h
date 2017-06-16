/*
 * functions.h
 *
 *  Created on: 2 lip 2016
 *      Author: MS-1
 */

#ifndef IO_FUNCTIONS_H_
#define IO_FUNCTIONS_H_

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>


/*
 *
 * 		Makroinstrukcje
 *
 */

#define D_MS(x) _delay_ms(x)
// w jednostkach podstawy czasu 30us
#define SLOW_KEY 10000
#define FAST_KEY 2000
// ilosc wolnych zmian, po których nastêpuje przejœcie na szybkie
#define FAST_AFTER_SLOW 10

// glowny bufor wyswietlacza
#define BUFFER_SIZE 360
// maska pustego tekstu
#define LINE_MASK 0x81

// bajt diod
#define DIODE_MASK1 0xFF
#define DIODE_MASK0 0x00
#define DIODE_DDR DDRD
#define DIODE_ADR PORTD
#define DIODE_OFF DIODE_ADR = DIODE_MASK0
#define DIODE_ON DIODE_ADR = DIODE_MASK1
#define DIODE_CH DIODE_ADR ^= DIODE_MASK1

// czyjnik halla
#define HALL_ADR (1 << PB0)
#define HALL_DDR DDRB
#define HALL_PORT PORTB
#define HALL_PIN PINB
#define HALL_PRESSED() (!(HALL_PIN & HALL_ADR))
#define HALL_PCINT (1 << PCINT0)

// przelacznik s0
#define S0_ADR (1 << PB1)
#define S0_DDR DDRB
#define S0_PORT PORTB
#define S0_PIN PINB
#define S0_PRESSED() (!(S0_PIN & S0_ADR))
#define S0_PCINT (1 << PCINT1)

// przelacznik s1
#define S1_ADR (1 << PB2)
#define S1_DDR DDRB
#define S1_PORT PORTB
#define S1_PIN PINB
#define S1_PRESSED() (!(S1_PIN & S1_ADR))
#define S1_PCINT (1 << PCINT2)

// cosekundowe zbocze z DS1307
#define DS_ADR (1 << PC3)
#define DS_DDR DDRC
#define DS_PORT PORTC
#define DS_PIN PINC
#define DS_PRESSED() (!(DS_PIN & DS_ADR))
#define DS_PCINT (1 << PCINT11)


/*
 *
 * 		Wlasne typy danych
 *
 */

typedef enum {h10 = 0, h0, m10, m0, s10, s0} TimePos; // pozycja licznika w tablicy OneLamp[6]
typedef struct {
	uint8_t hour, min, sec;
	uint8_t OneNumber[6];
} Time; // reprezentacja czasu

typedef enum {d10 = 0, d0, mon10, mon0, y10, y0} DatePos; // pozycja licznika w tablicy OneLamp[6]
typedef struct {
	uint8_t day, month, year;
	uint8_t OneNumber[6];
} Date; // reprezentacja daty

// tryby pracy uk³adu
typedef enum {NoSet = 0, SetHour, SetMinute, SetDay, SetMonth, SetYear, SetToDs, SetWait} SetTime;

/*
 *
 *		Deklaracja funkcji inline obslugi czasu
 *
 */

inline void UnpackTime(Time *Time);
inline void PackTime(Time *Time);
inline void CopyTime(Time From, Time *To);
inline bool TimesEquals(Time Time0, Time Time1);

inline void UnpackDate(Date *Date);
inline void PackDate(Date *Date);

/*
 *
 *		Definicje funkcji obslugi czasu
 *
 */

inline void UnpackTime(Time *Time) {
	Time->OneNumber[h10] = Time->hour / 10;
	Time->OneNumber[h0] = Time->hour % 10;
	Time->OneNumber[m10] = Time->min / 10;
	Time->OneNumber[m0] = Time->min % 10;
	Time->OneNumber[s10] = Time->sec / 10;
	Time->OneNumber[s0] = Time->sec % 10;
} // END inline void UnpackTime

inline void PackTime(Time *Time) {
	Time->hour = (Time->OneNumber[h10] * 10) + Time->OneNumber[h0];
	Time->min = (Time->OneNumber[m10] * 10) + Time->OneNumber[m0];
	Time->sec = (Time->OneNumber[s10] * 10) + Time->OneNumber[s0];
} // END inline void PackTime

inline void CopyTime(Time From, Time *To) {
	To->hour = From.hour;
	To->min = From.min;
	To->sec = From.sec;
	for(uint8_t i = h10; i <= s0; i++)
		To->OneNumber[i] = From.OneNumber[i];
} // END inline void CopyTime

inline bool TimesEquals(Time Time0, Time Time1) {
	if ((Time0.hour == Time1.hour) && (Time0.min == Time1.min) && (Time0.sec == Time1.sec)) return true;
	else return false;
}

inline void UnpackDate(Date *Date) {
	Date->OneNumber[d10] = Date->day / 10;
	Date->OneNumber[d0] = Date->day % 10;
	Date->OneNumber[mon10] = Date->month / 10;
	Date->OneNumber[mon0] = Date->month % 10;
	Date->OneNumber[y10] = Date->year / 10;
	Date->OneNumber[y0] = Date->year % 10;
} // END inline void UnpackDate

inline void PackDate(Date *Date) {
	Date->day = (Date->OneNumber[d10] * 10) + Date->OneNumber[d0];
	Date->month = (Date->OneNumber[mon10] * 10) + Date->OneNumber[mon0];
	Date->year = (Date->OneNumber[y10] * 10) + Date->OneNumber[y0];
} // END inline void PackDate

#endif /* IO_FUNCTIONS_H_ */
