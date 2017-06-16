/*
ds1307 lib 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include "ds1307.h"

/*
 *
 * 		Funkcje wewnetrzne
 *
 */

/* konwertuje liczbe z zapisu dziesietnego na bcd */
static inline uint8_t dec2bcd(uint8_t dec) {
	return ((dec / 10) << 4) | (dec % 10);
} // END uint8_t dec2bcd

/* konwertuje liczbe z zapisu bcd na dziesietny */
static inline uint8_t bcd2dec(uint8_t bcd) {
	return (((bcd >> 4) & 0x0F) * 10) + (bcd & 0x0F);
} // END uint8_t bcd2dec

/* funkcja zapisu 8 bitow danych po I2C, 0 gdy b³¹d */
static uint8_t I2C_WriteRegister(uint8_t busAddr, uint8_t deviceRegister, uint8_t data) {
        I2C_InitStart(START);  //start I2C
        //if (I2C_Error) { I2C_FailStop(); return 0; }

        I2C_WriteAdr(busAddr);  //wyslij adres DS1307
        if (I2C_Error) { I2C_FailStop(); return 0; }

        I2C_Write(deviceRegister);
        if (I2C_Error) { I2C_FailStop(); return 0; }

        I2C_Write(data);  //wysy³¹ 3 ardumenty funkcji I2C_WriteRegister
        if (I2C_Error) { I2C_FailStop(); return 0; }

        I2C_Stop();   //stop I2C
        return 1;
} // END void I2C_WriteRegister

/* funkcja odczytu 8 bitow danych po I2C, 0 gdy b³¹d */
static uint8_t I2C_ReadRegister(uint8_t busAddr, uint8_t deviceRegister, uint8_t *data) {

        I2C_InitStart(START);  //start I2C
        //if (I2C_Error) { I2C_FailStop(); return 0; }

        I2C_WriteAdr(busAddr);  //wyslij adres DS1307
        if (I2C_Error) { I2C_FailStop(); return 0; }

        I2C_Write(deviceRegister);
        if (I2C_Error) { I2C_FailStop(); return 0; }

        I2C_Start(REP_START);  // 2 start I2C, musi byæ!!!
        if (I2C_Error) { I2C_FailStop(); return 0; }

        I2C_WriteAdr(busAddr + 1); // 1 to bit odczytu
        if (I2C_Error) { I2C_FailStop(); return 0; }

        *data = I2C_Read(NACK); // odczytaj rejestr ze zgloszeniem ze to ostatni
        if (I2C_Error) { I2C_FailStop(); return 0; }

        I2C_Stop();   //stop I2C
        return 1;
} // END uint8_t I2C_ReadRegister

/*
 *
 *		Funkcje z pliku h
 *
 */

/* inicjalizacja modulu zegara, wybudzenie z uspienia */
uint8_t DS1307_init(void){
    uint8_t seconds, control;
	if (I2C_ReadRegister(DS1307, SECONDS_REGISTER, &seconds)) {
		seconds &= ~CH_MASK; // usuniecie bitu ch czyli obudzenie ukladu
		control = (1 << SQWE) | (1 << RS0); // ustawienie czestotliwosci wyjscia na 4096 Hz
		if (I2C_WriteRegister(DS1307, SECONDS_REGISTER, seconds) && I2C_WriteRegister(DS1307, CONTROL_REGISTER, control)) return 1;
		else return 0;
	}
	else return 0;


} // END void DS1307_init

/* pobranie czasu w formacie dziesietnym, 0 gdy b³¹d */
uint8_t DS1307_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
	uint8_t temp_hours, temp_minutes, temp_seconds;
	if (I2C_ReadRegister(DS1307,HOURS_REGISTER, &temp_hours)
			&& I2C_ReadRegister(DS1307,MINUTES_REGISTER, &temp_minutes)
			&& I2C_ReadRegister(DS1307,SECONDS_REGISTER, &temp_seconds)) {

		//temp_hours = bcd2dec(temp_hours);
		temp_minutes = bcd2dec(temp_minutes);
		temp_seconds = bcd2dec(temp_seconds);

		if (temp_hours & 0x40) // 12hr mode:
			temp_hours &= 0x1F; // use bottom 5 bits (pm bit = temp & 0x20)
		else temp_hours &= 0x3F; // 24hr mode: use bottom 6 bits
		temp_hours = bcd2dec(temp_hours);

		/* kontrola poprawnosci wartosci */
		if (temp_hours < 24) *hours = temp_hours;
		if (temp_minutes < 60) *minutes = temp_minutes;
		if (temp_seconds < 60) *seconds = temp_seconds;
		return 1;
	}
	else {
		return 0;
	}

} // END void DS1307_GetTime

/* pobranie daty w formacie dziesietnym */
uint8_t DS1307_GetDate(uint8_t *days, uint8_t *months, uint8_t *years)
{
	if (I2C_ReadRegister(DS1307,MONTHS_REGISTER, months)
			&& I2C_ReadRegister(DS1307,DAYS_REGISTER, days)
			&& I2C_ReadRegister(DS1307,YEARS_REGISTER, years)) {
		*months = bcd2dec(*months);
		*days = bcd2dec(*days);
		*years = bcd2dec(*years);
		return 1;
	}
	else return 0;

} // END void DS1307_GetDate

/* ustawienie czasu w formacie dziesietnym */
uint8_t  DS1307_SetTime(uint8_t set_hour, uint8_t set_minute, uint8_t set_second) {
	if ((set_hour > 23) || (set_minute > 59) || (set_second > 59)) return 0;

	if (I2C_WriteRegister(DS1307,HOURS_REGISTER, dec2bcd(set_hour))
			&& I2C_WriteRegister(DS1307,MINUTES_REGISTER, dec2bcd(set_minute))
			&& I2C_WriteRegister(DS1307,SECONDS_REGISTER, dec2bcd(set_second)))
		return 1;
	else return 0;

} // END void  DS1307_SetTime

/* ustawienie daty w formacie dziesietnym */
uint8_t  DS1307_SetDate(uint8_t set_day, uint8_t set_month, uint8_t set_year) {
	if ((set_day > 31) || (set_day < 1) || (set_month > 12) || (set_month < 1) || (set_year > 99)) return 0;

	if (I2C_WriteRegister(DS1307,MONTHS_REGISTER, dec2bcd(set_month))
			&& I2C_WriteRegister(DS1307,DAYS_REGISTER, dec2bcd(set_day))
			&& I2C_WriteRegister(DS1307,YEARS_REGISTER, dec2bcd(set_year)))
		return 1;
	else return 0;
} // END void  DS1307_SetDate

void DS1307_Test() {
	/*  Test poprawnej pracy RTC, resetuje WATCHDoga  */
	uint8_t hour, min, sec, nhour, nmin, nsec;
	uint8_t count = 10; // maksymalna ilosc powtorzen
	DS1307_GetTime(&hour, &min, &sec);
	_delay_ms(1200);
	DS1307_GetTime(&nhour, &nmin, &nsec);


	/*  Jeœli RTC nie pracuje to wgraj now¹ godzinê, jeœli nie pracuje po wgraniu nowego czasu to daj sygna³ awaryjny  */
	while ((sec == nsec) && (count--)) {
		wdt_disable();
		wdt_enable(WDTO_2S);
		DS1307_init();
		DS1307_SetTime(12, 0, 0);
		DS1307_SetDate(6,5,17);
		DS1307_GetTime(&hour, &min, &sec);
		_delay_ms(1200);
		DS1307_GetTime(&nhour, &nmin, &nsec);
		wdt_reset();
	}
	wdt_disable();
	if (!count) {
		wdt_enable(WDTO_15MS);
		_delay_ms(50);
	}
}
