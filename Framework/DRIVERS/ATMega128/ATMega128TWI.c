/**
 * @file ATMega128TWI.c
 * ATMega128 TWI implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/TWI/TWI.h"
#include "../../API/TWIAPI.h"
#include "../../PIL/Guard.h"
#include <avr/io.h>

#define ATMEGA128_MAX_TWI_CHANNELS 8

/// check for needed defines
#ifndef TWI_PORT
#error TWI_PORT should be defined
#endif

#ifndef TWI_PIN
#error TWI_PIN should be defined
#endif

#ifndef TWI_DDR
#error TWI_DDR should be defined
#endif

#ifndef TWI_SCL
#error TWI_SCL should be defined
#endif

#ifndef TWI_SDA
#error TWI_SDA should be defined
#endif

/// helper macroses
#define TWI_SCL_HIGH    TWI_PORT |=  TWI_SCL;
#define TWI_SCL_LOW     TWI_PORT &= ~TWI_SCL;
#define TWI_SCL_OUT     TWI_DDR  |=  TWI_SCL;
#define TWI_SCL_IN      TWI_DDR  &= ~TWI_SCL;
#define TWI_SDA_HIGH    TWI_PORT |=  TWI_SDA;
#define TWI_SDA_LOW     TWI_PORT &= ~TWI_SDA;
#define TWI_SDA_OUT     TWI_DDR  |=  TWI_SDA;
#define TWI_SDA_IN      TWI_DDR  &= ~TWI_SDA;
#define TWI_SDA_IS_HIGH (TWI_PIN&TWI_SDA)

#define TWI_IS_SYSTEM_TWI(TWI)         ((TWIDefs.TWIOwners)&(1<<TWI))
#define TWI_SET_SYS_ACCESS_RIGHTS(TWI) {TWIDefs.TWIOwners |= (1<<TWI);}
#define TWI_SET_APP_ACCESS_RIGHTS(TWI) {TWIDefs.TWIOwners &= ~(1<<TWI);}

/// structure defines TWI
typedef struct
{
	/// type of interface
	uint8_t Type;
	
	/// status of TWI
	uint8_t Status;
	
	/// TWI active channels bit mask
	uint8_t TWIChannels;
	
	/// TWI owners
	uint8_t TWIOwners;
}TWIDefsStruct;
static volatile TWIDefsStruct TWIDefs;

/*******************************************************************************//**
 * @implements TWI_Init
 **********************************************************************************/
RESULT TWI_Init(void)
{
	// init status, active channels mask and type
	TWIDefs.Type        = 0;
	TWIDefs.Status      = 0;
	TWIDefs.TWIChannels = 0;
	
	// hardware TWI disabled
	TWCR &= ~(1<<TWEN);
	
	// return success
	return SUCCESS;
}

#ifdef USE_PWR
/*******************************************************************************//**
 * @implements TWI_PowerSave
 **********************************************************************************/
void TWI_PowerSave(void)
{
	// hardware TWI disabled
	TWCR &= ~(1<<TWEN);
	TWI_SCL_IN
	TWI_SDA_IN
	
}

/*******************************************************************************//**
 * @implements TWI_Restore
 **********************************************************************************/
void TWI_Restore(void)
{
	// empty
}
#endif

/*******************************************************************************//**
 * @implements TWI_Open
 **********************************************************************************/
HTWI TWI_Open(uint8_t Channel,uint8_t Params)
{
	// check channel
	if(Channel>=ATMEGA128_MAX_TWI_CHANNELS)
		return INVALID_HANDLE;
	
	// check if this channel is not already used
	if(TWIDefs.TWIChannels&(1<<Channel))
		return INVALID_HANDLE;
	
	// set TWI access rights
	if(!Guard_IsWatching())
		TWI_SET_SYS_ACCESS_RIGHTS(Channel)
	else
		TWI_SET_APP_ACCESS_RIGHTS(Channel)
	
	// mark channel as active and return TWI handle
	TWIDefs.TWIChannels |= (1<<Channel);
	
	// set interface
	if(Params&TWI_UNIQUE_SHT)
		TWIDefs.Type |=  (1<<Channel);
	else
		TWIDefs.Type &= ~(1<<Channel);
	
	return Channel;
}

/*******************************************************************************//**
 * @implements TWI_Close
 **********************************************************************************/
RESULT TWI_Close(HTWI TWI)
{
	// check TWI handle
	if(TWI>=ATMEGA128_MAX_TWI_CHANNELS)
		return FAIL;
	
	if(!(TWIDefs.TWIChannels&(1<<TWI)))
		return FAIL;
	
	// if TWI is a system TWI and guard is watching for a threat
	// then return failure
	if(TWI_IS_SYSTEM_TWI(TWI)&&Guard_IsWatching())
		return FAIL;
	
	// mark channel as inactive and return success
	TWIDefs.TWIChannels &= ~(1<<TWI);
	
	// if there are no active TWI handles then
	// make input at SCL and SDA
	if(TWIDefs.TWIChannels==0)
	{
		TWI_SCL_IN
		TWI_SDA_IN
	}
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements TWI_Start
 **********************************************************************************/
RESULT TWI_Start(HTWI TWI)
{
	uint8_t i;
	
	// check TWI handle
	if(TWI>=ATMEGA128_MAX_TWI_CHANNELS)
		return FAIL;
	
	if(!(TWIDefs.TWIChannels&(1<<TWI)))
		return FAIL;
	
	// if TWI is a system TWI and guard is watching for a threat
	// then return failure
	if(TWI_IS_SYSTEM_TWI(TWI)&&Guard_IsWatching())
		return FAIL;
	
	// check status
	if(TWIDefs.Status&&!(TWIDefs.Status&(1<<TWI)))
		return FAIL;
	
	// send start
	TWIDefs.Status |= (1<<TWI);
	
	// send start sequence
	// make output at SCL and SDA
	TWI_SCL_OUT
	TWI_SDA_OUT
	
	// now check what TWI interface is used
	if(TWIDefs.Type&(1<<TWI))
	{
		// it is Sensirion SHT11 TWI, so it has unique start sequence
		TWI_SDA_HIGH
		TWI_SCL_LOW
		for(i=0;i<10;++i)
		{
			TWI_SCL_HIGH
			TWI_SCL_LOW
		}
		TWI_SCL_HIGH
		TWI_SDA_LOW
		TWI_SCL_LOW
		TWI_SCL_HIGH
		TWI_SDA_HIGH
		TWI_SCL_LOW
	}
	else
	{
		// it is common TWI
		// while SCL is high set low level at SDA
		TWI_SDA_HIGH
		TWI_SCL_HIGH
		TWI_SDA_LOW
		TWI_SCL_LOW
	}
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements TWI_Stop
 **********************************************************************************/
RESULT TWI_Stop(HTWI TWI)
{
	// check TWI handle
	if(TWI>=ATMEGA128_MAX_TWI_CHANNELS)
		return FAIL;
	
	if(!(TWIDefs.TWIChannels&(1<<TWI)))
		return FAIL;
	
	// if TWI is a system TWI and guard is watching for a threat
	// then return failure
	if(TWI_IS_SYSTEM_TWI(TWI)&&Guard_IsWatching())
		return FAIL;
	
	// check status
	if(!(TWIDefs.Status&(1<<TWI)))
		return FAIL;
	
	// send stop
	// make output at SCL and SDA
	TWI_SCL_OUT
	TWI_SDA_OUT
	
	// while SCL is high
	TWI_SDA_LOW
	TWI_SCL_HIGH
	
	// set high level at SDA
	TWI_SDA_HIGH
	TWI_SCL_LOW
	
	TWIDefs.Status &= ~(1<<TWI);
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements TWI_Tx
 **********************************************************************************/
RESULT TWI_Tx(HTWI TWI,uint8_t Byte)
{
	uint8_t i;
	
	// check TWI handle
	if(TWI>=ATMEGA128_MAX_TWI_CHANNELS)
		return FAIL;
	
	if(!(TWIDefs.TWIChannels&(1<<TWI)))
		return FAIL;
	
	// if TWI is a system TWI and guard is watching for a threat
	// then return failure
	if(TWI_IS_SYSTEM_TWI(TWI)&&Guard_IsWatching())
		return FAIL;
	
	// check status
	if(!(TWIDefs.Status&(1<<TWI)))
		return FAIL;
	
	// send byte
	// make output at SCL and SDA
	TWI_SCL_OUT
	TWI_SDA_OUT
	
	// send byte MSB to LSB
	for(i=0;i<8;++i)
	{
		if(Byte&0x80)
			TWI_SDA_HIGH
		else
			TWI_SDA_LOW
		TWI_SCL_HIGH
		TWI_SCL_LOW
		Byte = Byte << 1;
	}
	
	// wait for ACK
	TWI_SDA_IN
	TWI_SDA_HIGH
	TWI_SCL_HIGH
	
	// if ACK is not received then return failure
	if(TWI_PIN&TWI_SDA)
	{
		return FAIL;
	}
	TWI_SCL_LOW
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements TWI_Rx
 **********************************************************************************/
RESULT TWI_Rx(HTWI TWI,uint8_t *Byte)
{
	uint8_t i;
	
	// check byte
	if(Byte==NULL)
		return FAIL;
	
	// check TWI handle
	if(TWI>=ATMEGA128_MAX_TWI_CHANNELS)
		return FAIL;
	
	if(!(TWIDefs.TWIChannels&(1<<TWI)))
		return FAIL;
	
	// if TWI is a system TWI and guard is watching for a threat
	// then return failure
	if(TWI_IS_SYSTEM_TWI(TWI)&&Guard_IsWatching())
		return FAIL;
	
	// check status
	if(!(TWIDefs.Status&(1<<TWI)))
		return FAIL;
	
	// receive byte
	// make output at SCL and SDA
	TWI_SCL_OUT
	TWI_SDA_IN
	
	// receive byte MSB to LSB
	*Byte = 0;
	for(i=0;i<8;++i)
	{
		TWI_SCL_HIGH
		TWI_SCL_LOW
		if(TWI_SDA_IS_HIGH)
			*Byte |= 0x01;
		if(i!=7)
			*Byte = *Byte << 1;
	}
	
	// send ACK
	TWI_SDA_OUT
	TWI_SDA_LOW
	TWI_SCL_HIGH
	TWI_SCL_LOW
	TWI_SDA_IN
	TWI_SDA_HIGH
	
	// return success
	return SUCCESS;
}
