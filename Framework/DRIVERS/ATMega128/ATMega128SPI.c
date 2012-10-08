/**
 * @file ATMega128SPI.c
 * ATMega128 SPI implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/SPI/SPI.h"
#include "../../API/SPIAPI.h"
#include "../../PIL/Guard.h"
#include <avr/interrupt.h>

#define DDR_SPI  DDRB
#define PORT_SPI PORTB
#define PIN_SPI  PINB
#define DD_SS    0
#define DD_SCK   1
#define DD_MOSI  2
#define DD_MISO  3

#define SPI_IS_SYSTEM_SPI         ((SPIDefs.SPIState)&2)
#define SPI_IS_IN_ASYNC_MODE      ((SPIDefs.SPIState)&1)
#define SPI_SET_ASYNC_MODE        {SPIDefs.SPIState |= 1;}
#define SPI_SET_SYNC_MODE         {SPIDefs.SPIState &= ~1;}
#define SPI_SET_SYS_ACCESS_RIGHTS {SPIDefs.SPIState |= 2;}
#define SPI_SET_APP_ACCESS_RIGHTS {SPIDefs.SPIState &= ~2;}

/// structure defines SPI
typedef struct
{
	/// SPI "byte transmitted" event handler
	EVENT (*TxRxDone)(uint8_t Byte);
	
	/// SPI state
	uint8_t SPIState;
}SPIDefsStruct;
static volatile SPIDefsStruct SPIDefs;

/// SPI interrupt handler
ISR(SIG_SPI)
{
	// read byte
	uint8_t Byte = SPDR;
	
	if(SPIDefs.TxRxDone!=NULL)
	{
		// save current guard state
		SAVE_GUARD_STATE
		
		// if current SPI is a not a system SPI, then
		// guard should watch for it
		if(!SPI_IS_SYSTEM_SPI)
			Guard_Watch();
		else
			Guard_Idle();
		
		// signal SPI "byte transmitted" event
		SPIDefs.TxRxDone(Byte);
		
		// restore previous guard state
		RESTORE_GUARD_STATE
		
	}
	
}

/*******************************************************************************//**
 * @implements SPI_Init
 **********************************************************************************/
RESULT SPI_Init(void)
{
	// init SPI
	SPCR &= ~(1<<SPE);
	SPIDefs.TxRxDone = NULL;
	SPIDefs.SPIState = 0;
	
	// return success
	return SUCCESS;
}

#ifdef USE_PWR
/// saved SPI state
static volatile uint8_t SavedState[2];

/*******************************************************************************//**
 * @implements SPI_PowerSave
 **********************************************************************************/
void SPI_PowerSave(void)
{
	SavedState[0] = SPCR;
	SavedState[1] = DDR_SPI;
	DDR_SPI &= 0xF0;
	
}

/*******************************************************************************//**
 * @implements SPI_Restore
 **********************************************************************************/
void SPI_Restore(void)
{
	SPCR = SavedState[0];
	DDR_SPI |= SavedState[1] & 0x0F;
	DDR_SPI &= SavedState[1] | 0xF0;
	
}
#endif

/*******************************************************************************//**
 * @implements SPI_Open
 **********************************************************************************/
HSPI SPI_Open(uint8_t Channel,uint8_t Params,EVENT (*TxRxDone)(uint8_t Byte))
{
	// check channel
	if(Channel!=0)
		return INVALID_HANDLE;
	
	// check state
	if(SPCR&(1<<SPE))
		return INVALID_HANDLE;
	
	// set SPI access rights
	if(!Guard_IsWatching())
		SPI_SET_SYS_ACCESS_RIGHTS
	else
		SPI_SET_APP_ACCESS_RIGHTS
	
	// set SPI transmission mode
	if(Params&SPI_TRANSMISSION_MODE_ASYNC)
		SPI_SET_ASYNC_MODE
	else
		SPI_SET_SYNC_MODE
	
	// set SPI "byte transmitted" event handler
	SPIDefs.TxRxDone = TxRxDone;
	
	// configure SPI in slave mode
	if(Params&SPI_MODE_SLAVE)
	{
		DDR_SPI |=  (1<<DD_MISO);
		DDR_SPI &= ~((1<<DD_MOSI)|(1<<DD_SCK)|(1<<DD_SS));
		SPCR    |=  (1<<SPE);
		SPCR    &= ~(1<<MSTR);
		SPCR    |=  (1<<SPIE);
		
		// return SPI handle
		return Channel;
	}
	// else configure SPI in master mode
	else
	{
		DDR_SPI  &= ~(1<<DD_MISO);
		DDR_SPI  |=  (1<<DD_MOSI)|(1<<DD_SCK)|(1<<DD_SS);
		PORT_SPI |=  (1<<DD_SS);
		SPCR     |=  (1<<SPE)|(1<<MSTR);
		
		// return SPI handle
		return Channel;
		
	}
	
	// return invalid handle
	return INVALID_HANDLE;
}

/*******************************************************************************//**
 * @implements SPI_Close
 **********************************************************************************/
RESULT SPI_Close(HSPI SPI)
{
	// check SPI handle
	if(SPI!=0)
		return FAIL;
	
	// check state
	if(!(SPCR&(1<<SPE)))
		return FAIL;
	
	// if SPI is a system SPI and guard is watching for a threat
	// then return failure
	if(SPI_IS_SYSTEM_SPI&&Guard_IsWatching())
		return FAIL;
	
	// stop SPI
	SPCR &= ~(1<<SPE);
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements SPI_Start
 **********************************************************************************/
RESULT SPI_Start(HSPI SPI)
{
	// check SPI handle
	if(SPI!=0)
		return FAIL;
	
	// check state
	if(!(SPCR&(1<<SPE)))
		return FAIL;
	
	// if SPI is a system SPI and guard is watching for a threat
	// then return failure
	if(SPI_IS_SYSTEM_SPI&&Guard_IsWatching())
		return FAIL;
	
	// start sransmission by pulling low SS pin
	PORT_SPI &= ~(1<<DD_SS);
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements SPI_Stop
 **********************************************************************************/
RESULT SPI_Stop(HSPI SPI)
{
	// check SPI handle
	if(SPI!=0)
		return FAIL;
	
	// check state
	if(!(SPCR&(1<<SPE)))
		return FAIL;
	
	// if SPI is a system SPI and guard is watching for a threat
	// then return failure
	if(SPI_IS_SYSTEM_SPI&&Guard_IsWatching())
		return FAIL;
	
	// stop sransmission by pulling high the SS pin
	PORT_SPI |= (1<<DD_SS);
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements SPI_TxRx
 **********************************************************************************/
RESULT SPI_TxRx(HSPI SPI,uint8_t TxByte,uint8_t *RxByte)
{
	// check SPI handle
	if(SPI!=0)
		return FAIL;
	
	// check state
	if(!(SPCR&(1<<SPE)))
		return FAIL;
	
	// if SPI is a system SPI and guard is watching for a threat
	// then return failure
	if(SPI_IS_SYSTEM_SPI&&Guard_IsWatching())
		return FAIL;
	
	// if mode is async, then enable SPI interrupt and send byte
	if(SPI_IS_IN_ASYNC_MODE)
	{
		// enable SPI interrupt
		SPCR |= (1<<SPIE);
		
		// send byte
		SPDR = TxByte;
		
	}
	// else send byte and wait
	else
	{
		// disable SPI interrupt
		SPCR &= ~(1<<SPIE);
		
		// send byte
		SPDR = TxByte;
		
		// wait for byte to transmit
		while(!(SPSR&(1<<SPIF)));
		
		// if RxByte is not null,
		// then write received byte to it
		if(RxByte!=NULL)
			(*RxByte) = SPDR;
		
	}
	
	// return success
	return SUCCESS;
}
