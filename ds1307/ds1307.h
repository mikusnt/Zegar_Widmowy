/*
ds1307 lib 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References: parts of the code taken from https://github.com/adafruit/RTClib
*/


#ifndef DS1307_H
#define DS1307_H

#include <avr/io.h>
#include "../I2C_TWI/i2c_twi.h"
#include <string.h>
#include <stdlib.h>
#include <avr/wdt.h>
#include <util/delay.h>

/*
 *
 * 		Makroinstrukcje
 *
 */

#define DS1307 0xD0 // I2C bus address of DS1307 RTC  0xD0
#define CH_MASK 0x80

#define SECONDS_REGISTER 0x00
#define MINUTES_REGISTER 0x01
#define HOURS_REGISTER 0x02
#define DAYOFWK_REGISTER 0x03
#define DAYS_REGISTER 0x04
#define MONTHS_REGISTER 0x05
#define YEARS_REGISTER 0x06
#define CONTROL_REGISTER 0x07
#define RAM_BEGIN 0x08
#define RAM_END 0x3F
#define SQWE 4
#define RS0 0

/*
 *
 * 		Funkcje
 *
 */

extern uint8_t DS1307_init(void);
extern uint8_t DS1307_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds);
extern uint8_t DS1307_GetDate(uint8_t *days, uint8_t *months, uint8_t *years);
extern uint8_t  DS1307_SetTime(uint8_t set_hour, uint8_t set_minute, uint8_t set_second); //time set
extern uint8_t  DS1307_SetDate(uint8_t set_day, uint8_t set_month, uint8_t set_year);  //set date
extern void DS1307_Test(); // test poprawnosci komunikacji/dzialania, resetuja WATCHDoga

#endif

