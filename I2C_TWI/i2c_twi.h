/*
 * i2c_twi.h
 *
 *  Created on: 2010-09-07
 *       Autor: Miros³aw Kardaœ
 */

#ifndef I2C_TWI_H_
#define I2C_TWI_H_

#include <compat/twi.h>

#define ACK 1
#define NACK 0
#define REP_START 1
#define START 0

#define I2C_STARTError 1
#define I2C_RepSTARTError 2
#define I2C_NoNACK 3
#define I2C_NoACK 4
#define I2C_FREQKHZ 10

uint8_t I2C_Error;



// funkcje

void I2C_Init();

void I2C_Start(uint8_t StartMode);
void I2C_InitStart(uint8_t StartMode);
void I2C_FailStop(void);
void I2C_Stop(void);
void I2C_Write(uint8_t byte);
void I2C_WriteAdr(uint8_t adr);
uint8_t I2C_Read(uint8_t ack);
uint8_t I2C_FindDevice(uint8_t start);

uint8_t I2C_write_buf( uint8_t SLA, uint8_t adr, uint8_t len, uint8_t *buf );
uint8_t I2C_read_buf(uint8_t SLA, uint8_t adr, uint8_t len, uint8_t *buf);

#endif /* I2C_TWI_H_ */
