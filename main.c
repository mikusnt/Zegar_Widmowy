/*
 * main.c
 *
 *  Created on: 2 lip 2016
 *      Author: MS-1
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "io_functions.h"
#include "sequences.h"
#include <avr/wdt.h>
#include "ds1307/ds1307.h"
#include "I2C_TWI/i2c_twi.h"

/*
 *
 * 		Definicje funkcji inicjalizuj¹cych
 *
 */

static inline void IO_Init();
static inline void PCINT_INIT();
static inline void Main_PWM_Init();
//static void Reset();

//ISR(TIMER1_COMPA_vect);
//ISR(PCINT0_vect);
//ISR(PCINT1_vect);

/*
 *
 * 		Zmienne globalne volatile
 *
 */

volatile uint8_t uiBuffer[BUFFER_SIZE]; // glowny bufor matrycy
volatile uint16_t ui16Buffer_Pos; // aktualna pozycja w buforze [0..BUFFER_SIZE], inkrementowana w timerze
volatile uint16_t ui16Buffer_Mov = FRONT_POS; // przesuniecie bufora [0..BUFFER_SIZE], zmieniana przez sekwencje

volatile uint16_t ui16_30us; // podstawowe odniesienie czasu aktualnego obrotu bufora
volatile uint16_t ui16_30us_Circle_Time; // czas pelnego obrotu
volatile uint16_t ui16_30us_Max_Inc; // czas po ktorym nastepuje inkrementacja pozycji zwielokrotnionego bufora
volatile uint16_t ui16_30us_Inc; // licznik ktory dazy do ui16_30us_Max_Inc czyli do zmiany pozycji bufora

volatile uint8_t ui30us_Time; // podstawowe odniesienie czasu [0..34]
volatile uint32_t ui32_1ms_Time; // podstawowe odniesienie czasu
volatile uint16_t ui16_Buffer_Mov_1ms; // czas po ktorym nastepuje przesuniecie ui16Buffer_Mov, 0 - brak przesuniecia
volatile uint16_t ui16_Buffer_Mov_1ms_Inc; // licznik dazacy do ui16_Buffer_Mov_1ms

volatile uint16_t ui16S0Counter; // licznik opoznienia dla reakcji na S0 [0..FAST_KEY..SLOW_KEY], 1 - oczekiwanie na puszczenie przecisku
volatile uint16_t ui16S1Counter; // licznik opoznienia dla reakcji na S1 [0..FAST_KEY..SLOW_KEY]
volatile uint8_t uiS1FastMode; // licznik krotkoœci przyciœniêcia klawisza S1 za jednym razem [0..FAST_AFTER_SLOW]

volatile SetTime eSetTime; // tryb ustawiania czasu 0 - off, 1 - h, 2 - min, 3 - wyslanie do DS
volatile bool bNewTime; // flaga zg³oszenia nowej sekundy przez przerwanie DS
bool bNewSeq; // flaga zmiany sekwencji
uint8_t uiSeqNr; // numer aktualnej sekqwncji

volatile uint8_t uiTimeDiv; // dzielnik czêstoœci przerwania DS [0..32]
volatile uint8_t uiTimeDiv1s; // dzielnik czêstoœci przerwania DS [0..128]
Time tCzas; // aktualny czas
Date dData; // aktualna data

/*
 *
 * 		Funkcja glowna
 *
 */

int main (void) {

	/*
	 *
	 *		Inicjalizacja
	 *
	 */

	IO_Init();
	Main_PWM_Init();
	PCINT_INIT();


	DS1307_init();
	DS1307_Test();

	// ladowanie czasu i daty
	DS1307_GetTime(&tCzas.hour, &tCzas.min, &tCzas.sec);
	DS1307_GetDate(&dData.day, &dData.month, &dData.year);
	UnpackTime(&tCzas);
	UnpackDate(&dData);

	// watchdog na 15ms
	//wdt_enable(WDTO_500MS);
	sei();

	while(1) {

		/*
		 *
		 * Obs³uga timera DS
		 *
		 */

		if (bNewTime) {
			bNewTime = false;

			if (eSetTime == NoSet) {
				if(++uiTimeDiv1s >= 128) {
					uiTimeDiv1s = 0;
					// ladowanie czasu i daty
					DS1307_GetTime(&tCzas.hour, &tCzas.min, &tCzas.sec);
					DS1307_GetDate(&dData.day, &dData.month, &dData.year);
					UnpackTime(&tCzas);
					UnpackDate(&dData);
				}

				// odswiezanie aktualnej sekwencji 128 razy na sekunde
				do {
					if (bNewSeq) {
						bNewSeq = false;
						uiSeqNr++;
						//Reset();
					}

					if (uiSeqNr >= SEQ_NUMBER) {
						uiSeqNr = 0;
					}

					TABLE_SEQ[uiSeqNr](tCzas, dData, uiBuffer, &ui16_Buffer_Mov_1ms, &ui32_1ms_Time, &ui16Buffer_Mov, &bNewSeq);
				} while (bNewSeq);
			}
			wdt_reset();
		}

		/*
		 *
		 *		Obs³uga klawiszy
		 *
		 */

		// zaladowanie licznika czasu S0, gdy licznik wyzerowany
		// gdy przecysik nie wcisniety zezwolenie na ponowne wcisniecie
		if (S0_PRESSED())  {
				if (!ui16S0Counter) ui16S0Counter = FAST_KEY;
		}

		// reakcja na S0 po up³ywie opóŸnienia, przycisk w trybie oczekiwania na puszczenie
		if ((ui16S0Counter == 1) && (!S0_PRESSED())) {
			ui16S0Counter = 0;
			switch(eSetTime) {
				case NoSet: {
					eSetTime = SetHour;
					DIODE_ADR = tCzas.hour;
				} break;
				case SetHour: {
					eSetTime = SetMinute;
					DIODE_ADR = tCzas.min;
				} break;
				case SetMinute: {
					eSetTime = SetDay;
					DIODE_ADR = dData.day;
				} break;
				case SetDay: {
					eSetTime = SetMonth;
					DIODE_ADR = dData.month;
				} break;
				case SetMonth: {
					eSetTime = SetYear;
					DIODE_ADR = dData.year;
				} break;
				case SetYear: {
					eSetTime = SetToDs;
					DIODE_ADR = 0xff;
				} break;
				case SetWait: {
					eSetTime = SetHour;
					DIODE_ADR = tCzas.hour;
				} break;
				case SetToDs: {} break;
			}
			//DIODE_ADR = DIODE_ADR << 1;
		}

		// obsluga klawisza S1
		// zaladowanie licznika czasu S1
		if (S1_PRESSED() && (!ui16S1Counter)) {
			if (uiS1FastMode < 10) {
				ui16S1Counter = SLOW_KEY;
				uiS1FastMode++;
			}
			else ui16S1Counter = FAST_KEY;
		}

		// reakcja na S1 po up³ywie opóŸnienia
		if (ui16S1Counter == 1) {
			ui16S1Counter = 0;
			switch(eSetTime) {
				case NoSet: {
					// obsluga zmiany aktualnej sekwencji
				} break;
				case SetHour: {
					if (++tCzas.hour > 23) tCzas.hour = 0;
					DIODE_ADR = tCzas.hour;
				} break;
				case SetMinute: {
					if (++tCzas.min > 59) tCzas.min = 0;
					DIODE_ADR = tCzas.min;
				} break;
				case SetDay: {
					if (++dData.day > 31) dData.day = 1;
					DIODE_ADR = dData.day;
				} break;
				case SetMonth: {
					if (++dData.month > 12) dData.month = 1;
					DIODE_ADR = dData.month;
				} break;
				case SetYear: {
					if (++dData.year > 99) dData.year = 0;
					DIODE_ADR = dData.year;
				} break;
				case SetWait: {} break;
				case SetToDs: {} break;
			}
			//DIODE_ADR++;
		}

		// obsluga zaladowania czasu do DS
		if (eSetTime == SetToDs) {
			DS1307_SetTime(tCzas.hour, tCzas.min, 0);
			DS1307_SetDate(dData.day, dData.month, dData.year);
			eSetTime = SetWait;
		}
	}
} // END int main (void)

/*
 *
 * 		Deklaracje funkcji inicjalizuj¹cych
 *
 */

static inline void IO_Init() {
	// ustawienie diod na wyjscie
	DIODE_DDR |= DIODE_MASK1;
	DIODE_OFF;
	// ustawienie czujnika halla, switchow, DS na wejscie z podciagnieciem do Vcc
	HALL_PORT |= HALL_ADR;
	S0_PORT |= S0_ADR;
	S1_PORT |= S1_ADR;
	DS_PORT |= DS_ADR;
} // END static inline void IO_Init()

static inline void PCINT_INIT() {
	PCICR |= (1 << PCIE0) // odblokowanie pierwszej i drugiej grupy przerwan
			| (1 << PCIE1);
	PCMSK0 |= HALL_PCINT // odblokowanie przerwan na wlacznikach i hall
			| S0_PCINT
			| S1_PCINT;
	PCMSK1 |= DS_PCINT; // odblokowanie przerwan sygna³u z DS
} // END static inline void PCINT_INIT()

static inline void Main_PWM_Init() {
	TCCR1B |= (1 << WGM12) // tryb CTC
			| (1 << CS10); // preskaler 1
	TIMSK1 |= (1 << OCIE1A); // odblokowanie przerwania przepelnienia
	OCR1A = 600; // wyzwalanie timera co 1 * 600 taktów czyli 30us
} // END static inline void Main_PWM_Init()

//static void Reset() {
//	ui16Buffer_Pos = 0;
//	ui16Buffer_Mov = FRONT_POS;
//	ui16_30us = 0;
//	ui16_30us_Circle_Time = 0;
//	ui16_30us_Max_Inc = 0;
//	ui16_30us_Inc = 0;
//	ui30us_Time = 0;
//	ui32_1ms_Time = 0;
//	ui16_Buffer_Mov_1ms = 0;
//	ui16_Buffer_Mov_1ms_Inc = 0;
//	ui16S0Counter = 0;
//	ui16S1Counter = 0;
//	uiS1FastMode = 0;
//	bNewTime = false;
//	eSetTime = NoSet;
//}

/*
 *
 *		Przerwanie przepelnienia licznika timera, TCNT1 == OCR1A
 *
 */

ISR(TIMER1_COMPA_vect) {

	// inkrementacja zmiennych czasowych
	ui16_30us++;
	if (++ui30us_Time >= 34) {
		ui30us_Time = 0;
		ui32_1ms_Time++;
		ui16_Buffer_Mov_1ms_Inc++;
	}

	// przesuniecie ui16Buffer_Mov o 1
	if (ui16_Buffer_Mov_1ms && (ui16_Buffer_Mov_1ms_Inc >= ui16_Buffer_Mov_1ms)) {
		ui16_Buffer_Mov_1ms_Inc = 0;
		if (++ui16Buffer_Mov == BUFFER_SIZE) {
			ui16Buffer_Mov = 0;
		}
	}

	// obsluga wyswietlania danych bufora, tylko gdy nieaktywny tryb zmiany czasu
	if ((++ui16_30us_Inc >= ui16_30us_Max_Inc) && (eSetTime == NoSet)) {
		if (ui16Buffer_Pos < BUFFER_SIZE) {
				// realizacja wypisywania danych bufora
				DIODE_ADR = uiBuffer[(ui16Buffer_Pos++ + ui16Buffer_Mov) % BUFFER_SIZE];
				//if (++uiBuffer_Pos >= BUFFER_SIZE) uiBuffer_Pos = 0;
		//	else DIODE_ADR = 0x00;
		} else DIODE_ADR = 0x00;
		ui16_30us_Max_Inc = ((uint32_t)ui16_30us_Circle_Time * (ui16Buffer_Pos + 1)) / BUFFER_SIZE;
	}

	// obsluga trybu zmiany czasu, zmiana gdy brak odpowiedzi od halla
	if ((ui16_30us > 10000) && (eSetTime == NoSet)) {
		eSetTime = SetHour; // tryb zmiany minuty
		DIODE_ADR = tCzas.hour;
	}

	// dekrementacja opóŸnienia czasowego S0
	if ((ui16S0Counter > 1) && (!S0_PRESSED())) {
		ui16S0Counter--;
	}

	// dekrementacja opóŸnienia czasowego S1
	if (ui16S1Counter > 1) {
		ui16S1Counter--;
	} else uiS1FastMode = 0;

	// resetowanie licznika watchdoga

} // END ISR(TIMER1_COMPA_vect)

ISR(PCINT0_vect) {
	if (HALL_PRESSED()) {

		// obliczenia czasu zmiany licznika bufora, srednia ciagniona
		ui16_30us_Circle_Time = ui16_30us;
		ui16_30us_Max_Inc = (uint32_t)ui16_30us_Circle_Time / BUFFER_SIZE;
		ui16Buffer_Pos = 0;
		ui16Buffer_Pos = 0;
		ui16_30us_Inc = 0;
		ui16_30us = 0;
		TCNT1 = 0;

		if ((eSetTime == SetWait) || (eSetTime == SetHour) || (eSetTime == SetMinute))  eSetTime = NoSet;
	}
} // END ISR(PCINT0_vect)

ISR(PCINT1_vect) {
	// obsluga zbocza DS, nawet gdy czas nie jest zmieniany, bNewTime resetuje watchdoga
	//if (DS_PRESSED() && (eSetTime == NoSet)) {
	if (DS_PRESSED()) {
		if(++uiTimeDiv >= 32) {
			uiTimeDiv = 0;
			bNewTime = true;
			//wdt_reset();
		}

	}
} // END ISR(PCINT1_vect)
