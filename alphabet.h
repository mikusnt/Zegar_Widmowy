/*
 * alphabet.h
 *
 *  Created on: 2 lip 2016
 *      Author: MS-1
 */

#ifndef ALPHABET_H_
#define ALPHABET_H_

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

/*
 *
 *		Makroinstrukcje
 *
 */

#define ALPHABET_SIZE 72
#define MAX_TEXT_SIZE 30
#define TEXT_NUMBER 3

#define ALPHABET_YX(y, x) (pgm_read_byte(&(uiAlphabet[y][x])))
#define AL_LENGTH(y) (pgm_read_byte(&(uiAlLength[y])))

/*
 *
 * 		Dane pamiêci FLASH
 *
 */

extern const uint8_t uiAlphabet[ALPHABET_SIZE][5] PROGMEM;
extern const uint8_t uiAlLength[ALPHABET_SIZE] PROGMEM;

/*
 *
 *		Funkcje
 *
 */

// zwraca pionowa zawartosc okreslonego znaku
extern uint8_t LoadSignByte(char cSign, uint8_t uiByteNr);
// zwraca pionowa dlugosc okreslonego znaku
extern uint8_t LoadSignLength(char cSign);
// laduje tekst z pamiêci proramu do bufora
extern void LoadText(char buffer[], uint8_t uiTextNr );
#endif /* ALPHABET_H_ */
