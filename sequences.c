/*
 * sequences.c
 *
 *  Created on: 18 lip 2016
 *      Author: MS-1
 */

#include "sequences.h"


/*
 *
 * 		Funkcje pomocnicze
 *
 */

static void TimeDateToText(Time tCzas, Date dDate, char cText[]) {
	sprintf(cText, "%d%d#%d%d#%d%d  %d%d-%d%d-20%d%d", tCzas.OneNumber[h10], tCzas.OneNumber[h0],
			tCzas.OneNumber[m10], tCzas.OneNumber[m0], tCzas.OneNumber[s10], tCzas.OneNumber[s0],
			dDate.OneNumber[d10], dDate.OneNumber[d0], dDate.OneNumber[mon10], dDate.OneNumber[mon0],
			dDate.OneNumber[y10], dDate.OneNumber[y0]);
} // END static void TimeDateToText

static uint16_t TextToBuffer(char cText[], volatile uint8_t uiBuffer[], uint16_t ui16StartPos, bool bSpace) {
	uint8_t uiTextPos = 0;
	uint8_t i;
	uint8_t uiLength;
	uint8_t st, end;

	// wczytywanie tekstu
	if (ui16StartPos >= BUFFER_SIZE) return ui16StartPos - 1;
	uiBuffer[ui16StartPos] = 0;
	if (ui16StartPos + 1 >= BUFFER_SIZE) return ui16StartPos;
	uiBuffer[ui16StartPos+1] = 0;
	ui16StartPos += 2;

	while (cText[uiTextPos]) {
		if (bSpace) {
			if (ui16StartPos >= BUFFER_SIZE) return ui16StartPos - 1;
			uiBuffer[ui16StartPos] = 0;
			if (ui16StartPos + 1 >= BUFFER_SIZE) return ui16StartPos;
			uiBuffer[ui16StartPos+1] = 0;
			if (ui16StartPos + 2 >= BUFFER_SIZE) return ui16StartPos + 1;
			uiBuffer[ui16StartPos+2] = 0;
			ui16StartPos += 3;
		}
		// ustalenie rozmiaru znaku
		uiLength = LoadSignLength(cText[uiTextPos]);
		st = uiLength > 3 ? 0 : (uiLength == 1 ? 2 : 1);
		end = uiLength == 5 ? 5 : (uiLength > 2 ? 4 : 3);

		for (i = st; i < end; i++, ui16StartPos+=3) {
			if (ui16StartPos >= BUFFER_SIZE) return ui16StartPos - 1;
			uiBuffer[ui16StartPos] = LoadSignByte(cText[uiTextPos], i);
			if (ui16StartPos + 1 >= BUFFER_SIZE) return ui16StartPos;
			uiBuffer[ui16StartPos+1] = 0;
			if (ui16StartPos + 2 >= BUFFER_SIZE) return ui16StartPos + 1;
			uiBuffer[ui16StartPos+2] = 0;
		}
		uiTextPos++;
	}
	if (bSpace) {
		if (ui16StartPos >= BUFFER_SIZE) return ui16StartPos - 1;
		uiBuffer[ui16StartPos] = 0;
		if (ui16StartPos + 1 >= BUFFER_SIZE) return ui16StartPos;
		uiBuffer[ui16StartPos+1] = 0;
		if (ui16StartPos + 2 >= BUFFER_SIZE) return ui16StartPos + 1;
		uiBuffer[ui16StartPos+2] = 0;
		ui16StartPos += 3;
	}
	return ui16StartPos - 1;
} // END static void TextToBuffer

static void MaskToBuffer(uint8_t uiMask, volatile uint8_t uiBuffer[], uint16_t ui16StartPos, uint16_t uiEndPos, uint8_t uiPosMul) {
	if ((ui16StartPos <= uiEndPos) && (uiEndPos <= BUFFER_SIZE)) {
		uint16_t i;
		for (i = ui16StartPos; i <= uiEndPos; i++) {
			if ((i % uiPosMul) == 0) uiBuffer[i] = uiMask;
				else uiBuffer[i] = 0;
		}
	}
} // END static void MaskToBuffer

/*
 *
 * 		Sekwencje
 *
 */
// czas przeskoku o jeden pixel
#define TIME_ONE_MS 30
// sumaryczny czas obrotów
#define TIME_ALL_MS ((uint32_t)TIME_ONE_MS * BUFFER_SIZE * 2 + 1000)
void TimeToBuffer(Time tCzas, Date dData, volatile uint8_t uiBuffer[], volatile uint16_t *ui16_Buffer_Mov_1ms, volatile uint32_t *ui32_1ms_Time, volatile uint16_t *ui16Buffer_Mov, bool *bNewSeq) {
	static Time tCzas0;
	if (*ui32_1ms_Time >= TIME_ALL_MS) {
		*ui32_1ms_Time = 0;
		*bNewSeq = true;
	} else if ((*ui32_1ms_Time % TIME_ALL_MS) < (TIME_ALL_MS - 1000)) {
		uiActualSEQ = 0;
		if (!TimesEquals(tCzas0, tCzas)) {
			//CopyTime(tCzas, &tCzas0);
			char cBuffer[TEXT_DATE_TIME_LENGTH];
			TimeDateToText(tCzas, dData, cBuffer);

			uint16_t uiEnd = TextToBuffer(cBuffer, uiBuffer, 0, true);
			MaskToBuffer(LINE_MASK, uiBuffer, uiEnd + 1, BUFFER_SIZE - 1, 1);
			*ui16_Buffer_Mov_1ms = TIME_ONE_MS;
		}
	} else {
		MaskToBuffer(LINE_MASK, uiBuffer, 0, BUFFER_SIZE - 1, 1);
		*ui16_Buffer_Mov_1ms = 0;
		*ui16Buffer_Mov = FRONT_POS;
	}
} // END void TimeToBuffer

// problem do naprawy - zbyt d³uga pierwsza sekwencja, pozosta³e dwie w normie, raczej naprawiony
// czas przeskoku o jeden pixel
#define TEXT_ONE_MS 20
// sumaryczny czas obrotów
#define TEXT_ALL_MS ((uint32_t)TEXT_ONE_MS * BUFFER_SIZE * 2 + 1000)
void TextsToBuffer(Time tCzas, Date dData, volatile uint8_t uiBuffer[], volatile uint16_t *ui16_Buffer_Mov_1ms, volatile uint32_t *ui32_1ms_Time, volatile uint16_t *ui16Buffer_Mov, bool *bNewSeq) {
	static uint8_t uiActualText = 0xff;
	if (*ui32_1ms_Time >= (TEXT_ALL_MS * TEXT_NUMBER)) {
		*ui32_1ms_Time = 0;
		uiActualText = 0xff;
		*bNewSeq = true;
	} else {
		uiActualSEQ = 1;
		if ((*ui32_1ms_Time % TEXT_ALL_MS) < (TEXT_ALL_MS - 1000)) {
			if (uiActualText != (*ui32_1ms_Time / TEXT_ALL_MS)) {
				char cBuffer[MAX_TEXT_SIZE];
				uiActualText = *ui32_1ms_Time / TEXT_ALL_MS;
				LoadText(cBuffer, uiActualText);

				uint16_t uiEnd = TextToBuffer(cBuffer, uiBuffer, 0, true);
				MaskToBuffer(LINE_MASK, uiBuffer, uiEnd + 1, BUFFER_SIZE - 1, 1);
				*ui16_Buffer_Mov_1ms = TEXT_ONE_MS;
			}

		} else {
			MaskToBuffer(LINE_MASK, uiBuffer, 0, BUFFER_SIZE - 1, 1);
			*ui16_Buffer_Mov_1ms = 0;
			*ui16Buffer_Mov = FRONT_POS;
		}
	}
} // END void TextsToBuffer

//void Text0ToBuffer(Time tCzas, volatile uint8_t uiBuffer[], volatile uint16_t *ui16_Buffer_Mov_1ms, volatile uint32_t *ui32_1ms_Time, volatile uint16_t *ui16Buffer_Mov, bool *bNewSeq) {
//	if (uiActualSEQ != 2) {
//		uiActualSEQ = 2;
//		char cBuffer[MAX_TEXT_SIZE];
//		LoadText(cBuffer, 0);
//		uint16_t uiEnd = TextToBuffer(cBuffer, uiBuffer, 0, true);
//		MaskToBuffer(0, uiBuffer, uiEnd + 1, BUFFER_SIZE - 1, 1);
//		*ui16_Buffer_Mov_1ms = 15;
//	}
//} // END void Text0ToBuffer
//
//void Text1ToBuffer(Time tCzas, volatile uint8_t uiBuffer[], volatile uint16_t *ui16_Buffer_Mov_1ms, volatile uint32_t *ui32_1ms_Time, volatile uint16_t *ui16Buffer_Mov, bool *bNewSeq) {
//	if (uiActualSEQ != 3) {
//		uiActualSEQ = 3;
//		char cBuffer[MAX_TEXT_SIZE];
//		LoadText(cBuffer, 1);
//		uint16_t uiEnd = TextToBuffer(cBuffer, uiBuffer, 0, true);
//		MaskToBuffer(0, uiBuffer, uiEnd + 1, BUFFER_SIZE - 1, 1);
//		*ui16_Buffer_Mov_1ms = 15;
//	}
//} // END void Text1ToBuffer

/*
 *
 * 		Tablica Sekwencji
 *
 */

void (*TABLE_SEQ[SEQ_NUMBER])(Time tCzas, Date dData, volatile uint8_t uiBuffer[], volatile uint16_t *ui16_Buffer_Mov_1ms, volatile uint32_t *ui32_1ms_Time, volatile uint16_t *ui16Buffer_Mov, bool *bNewSeq) = {
		TimeToBuffer, TextsToBuffer
};
