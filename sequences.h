/*
 * sequences.h
 *
 *  Created on: 18 lip 2016
 *      Author: MS-1
 */

#ifndef SEQUENCES_H_
#define SEQUENCES_H_

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "io_functions.h"
#include "alphabet.h"
#include <stdio.h>

/*
 *
 * 		Wewnetrzne typy danych
 *
 */

// nr aktualnie zaladowanej sekwencji
uint8_t uiActualSEQ;

/*
 *
 * 		Makroinstrukcje
 *
 */

#define SEQ_NUMBER 2
#define FRONT_POS 240
#define TEXT_TIME_LENGTH 9
#define TEXT_DATE_LENGTH 11
#define TEXT_DATE_TIME_LENGTH 20
//#define TIME_MASK 0x81

/*
 *
 * 		Sekwencje
 *
 */

// czas w formacie gg:mm:ss, data w formacie dd-mm-rrrr
extern void TimeToBuffer(Time tCzas, Date dData, volatile uint8_t uiBuffer[], volatile uint16_t *ui16_Buffer_Mov_1ms, volatile uint32_t *ui32_1ms_Time, volatile uint16_t *ui16Buffer_Mov, bool *bNewSeq);
// wszystkie teksty z aplhabet
extern void TextsToBuffer(Time tCzas, Date dData, volatile uint8_t uiBuffer[], volatile uint16_t *ui16_Buffer_Mov_1ms, volatile uint32_t *ui32_1ms_Time, volatile uint16_t *ui16Buffer_Mov, bool *bNewSeq);

/*
 *
 * 		Tablica Sekwencji
 *
 */

extern void (*TABLE_SEQ[SEQ_NUMBER])(Time tCzas, Date dData, volatile uint8_t uiBuffer[], volatile uint16_t *ui16_Buffer_Mov_1ms, volatile uint32_t *ui32_1ms_Time, volatile uint16_t *ui16Buffer_Mov, bool *bNewSeq);


#endif /* SEQUENCES_H_ */
