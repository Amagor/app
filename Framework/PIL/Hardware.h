/**
 * @file Hardware.h
 * Hardware components init header.
 * @author Nezametdinov I.E.
 */

#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "../PIL/Defs.h"

/*******************************************************************************//**
 * inits hardware components
 * @return SUCCESS if hardware components successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT InitHardware(void);

	uint8_t MAC[8];
	MAC_EXTENDED_ADDR HWAddr;
	typedef unsigned char uint8_t;

#define BUF_SIZE  16
#define MASK  (BUF_SIZE-1)
#define B115200 3
#define ERROR_CRC -1
#define OK 1

//Location 1 Wire Net in PORT B pin 3.
#define OW_DQ   PF3
#define OW_PIN  PINF
#define OW_DDR  DDRF
#define OW_PORT PORTF
//#define F_CPU 8000000UL
#define __OPTIMIZE__ 1
#define divisor_1024 0b00000101 //предделитель 1024

// 1 Wire Commands
#define SKIP_ROM  0xCC
#define CONVERT_T 0x44
#define READ_ROM 0x0F

//Прототипы функций
unsigned char OW_ComputeCRC8(unsigned char inData, unsigned char seed);
uint8_t OW_reset(void);
void OW_write_bit(uint8_t bit);
uint8_t OW_read_bit(void);
void OW_write_byte(uint8_t command);
uint8_t OW_read_byte(void);
void convert_MAC(void);

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint8_t delay_done;
volatile uint8_t transmit_buf[BUF_SIZE];
volatile uint8_t transmit_in; //init 0
volatile uint8_t transmit_out; // init 0

#endif
