/*
 * i2c_twi.c
 *
 *  Created on: 2010-09-07
 *       Autor: Miros³aw Kardaœ
 */
#include <avr/io.h>

#include "i2c_twi.h"

static inline void I2C_WaitForComplete() {
	while (!(TWCR & (1 << TWINT)));
}

static inline void I2C_WaitTillStopWasSent() {
	while (!(TWCR & (1 << TWSTO)));
}

static inline void I2C_SetError(uint8_t ErrorNr) {
	I2C_Error = ErrorNr;
}
static inline void I2C_CleanError() {
	I2C_Error = 0;
}

static uint8_t I2C_Detect(uint8_t addr) {
	I2C_InitStart(START);
	I2C_WriteAdr(addr);
	uint8_t error = I2C_Error;
	I2C_CleanError();
 return (!error); // return 1 if found; 0 otherwise
}

/*
 *
 *		Funkcje z pliku h
 *
 */

void I2C_Init() {
	uint16_t bitrate;
	uint8_t prescaler = 0;
	bitrate = ((F_CPU / 1000 / I2C_FREQKHZ) - 16) / 2;
	while (bitrate > 255) {
		bitrate /= 4;
		prescaler++;
	}
	I2C_Error = 0;
	TWCR = (1 << TWEA) | (1 << TWEN); // uruchomienie interfejsu
	//TWSR = (TWSR & ((1 << TWPS1) | (1 << TWPS0))) | prescaler; // konfiguracja preskalera MK
	TWSR = (TWSR & (~((1 << TWPS1) | (1 << TWPS0)))) | prescaler; // konfiguracja preskalera MK mod
	//TWSR = prescaler;
	TWBR = bitrate; // ustawienie bitrate
}

void I2C_Start(uint8_t StartMode) {
	uint8_t Status;
	if (StartMode) Status = TW_REP_START;
	else Status = TW_START;
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTA);
	I2C_WaitForComplete();
	if (TW_STATUS != Status) { // obs³uga b³êdu
		I2C_SetError(I2C_STARTError);
	} else {
		I2C_CleanError();
	}
}

void I2C_InitStart(uint8_t StartMode) {
	I2C_Init();
	I2C_Start(StartMode);
}

void I2C_FailStop(void) {
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

void I2C_Stop(void) {
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	I2C_WaitTillStopWasSent();
}

void I2C_Write(uint8_t byte) {
	TWDR = byte;
	TWCR = (1<<TWINT)|(1<<TWEN);
	I2C_WaitForComplete();
	if(TW_STATUS != TW_MT_DATA_ACK) { // obs³uga b³êdu
		I2C_SetError(I2C_NoACK);
	} else {
		I2C_CleanError();
	}
}

void I2C_WriteAdr(uint8_t adr) {
	uint8_t Status;
	if (adr & 0x01) Status = TW_MR_SLA_ACK;
		else Status = TW_MT_SLA_ACK;
	TWDR =  adr;
	TWCR = (1 << TWINT) | (1<< TWEN);
	I2C_WaitForComplete();
	if (TW_STATUS != Status) { // obs³uga b³êdu MK
		I2C_SetError(I2C_NoNACK);
	} else {
		I2C_CleanError();
	}
}

uint8_t I2C_Read(uint8_t ack) {
	TWCR = (1<<TWINT)|(ack<<TWEA)|(1<<TWEN);
	I2C_WaitForComplete();
	if (ack) { // obs³uba b³êdu
		if (TW_STATUS != TW_MR_DATA_ACK) { I2C_SetError(I2C_NoACK); return 0; }
	} else {
		if (TW_STATUS != TW_MR_DATA_NACK) { I2C_SetError(I2C_NoNACK); return 0; }
	}

	I2C_CleanError();
	return TWDR;
}

uint8_t I2C_FindDevice(uint8_t start) {
	for (uint8_t addr=start;addr<0xFF;addr++)
		if (I2C_Detect(addr)) return addr;
	return 0;
}

uint8_t I2C_write_buf( uint8_t SLA, uint8_t adr, uint8_t len, uint8_t *buf ) {

	I2C_InitStart(START);

	I2C_WriteAdr(SLA);
	if (I2C_Error) { I2C_FailStop(); return I2C_Error; }

	I2C_Write(adr);
	if (I2C_Error) { I2C_FailStop(); return I2C_Error; }

	while (len--) {
		I2C_Write(*buf++);
		if (I2C_Error) { I2C_FailStop(); return I2C_Error; }
	}

	I2C_Stop();
	return 0;
}



uint8_t I2C_read_buf(uint8_t SLA, uint8_t adr, uint8_t len, uint8_t *buf) {

	I2C_InitStart(START);

	I2C_WriteAdr(SLA);
	if (I2C_Error) { I2C_FailStop(); return I2C_Error; }

	I2C_Write(adr);
	if (I2C_Error) { I2C_FailStop(); return I2C_Error; }

	I2C_Start(REP_START);
	if (I2C_Error) { I2C_FailStop(); return I2C_Error; }

	I2C_WriteAdr(SLA + 1);
	if (I2C_Error) { I2C_FailStop(); return I2C_Error; }

	while (len--) {
		*buf++ = I2C_Read( len ? ACK : NACK );
		if (I2C_Error) { I2C_FailStop(); return I2C_Error; }
	}

	I2C_Stop();
	return 0;
}
