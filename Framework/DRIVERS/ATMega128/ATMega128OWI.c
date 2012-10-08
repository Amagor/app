/**
 * @file ATMega128OWI.c
 * ATMega128 OWI implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/OWI/OWI.h"
#include "../../API/OWIAPI.h"
#include "../../PIL/Guard.h"
#include <util/delay.h>
#include <avr/io.h>

/// OWI defines
#ifndef OWI_PORT
#error OWI_PORT should be defined
#endif

#ifndef OWI_PIN
#error OWI_PIN should be defined
#endif

#ifndef OWI_DDR
#error OWI_DDR should be defined
#endif

#ifndef OWI_PIN_NUM
#error OWI_PIN_NUM should be defined
#endif

/// pulls bus low
#define OWI_PULL_BUS_LOW \
	OWI_DDR  |=  (1<<OWI_PIN_NUM);\
	OWI_PORT &= ~(1<<OWI_PIN_NUM);

/// releases bus
#define OWI_RELEASE_BUS \
	OWI_DDR  &= ~(1<<OWI_PIN_NUM);\
	OWI_PORT &= ~(1<<OWI_PIN_NUM);

/// bit timing delays in micro seconds
#define     OWI_DELAY_A_STD_MODE    6
#define     OWI_DELAY_B_STD_MODE    64
#define     OWI_DELAY_C_STD_MODE    60
#define     OWI_DELAY_D_STD_MODE    10
#define     OWI_DELAY_E_STD_MODE    9
#define     OWI_DELAY_F_STD_MODE    55
#define     OWI_DELAY_H_STD_MODE    480
#define     OWI_DELAY_I_STD_MODE    70
#define     OWI_DELAY_J_STD_MODE    410

/// max OWI channels
#define ATMEGA128_MAX_OWI_CHANNELS 8

#define OWI_IS_SYSTEM_OWI(OWI)         ((OWIDefs.OWIOwners)&(1<<OWI))
#define OWI_SET_SYS_ACCESS_RIGHTS(OWI) {OWIDefs.OWIOwners |= (1<<OWI);}
#define OWI_SET_APP_ACCESS_RIGHTS(OWI) {OWIDefs.OWIOwners &= ~(1<<OWI);}

/// structure defines OWI
typedef struct
{
	/// status of OWI
	uint8_t Status;
	
	/// OWI active channels bit mask
	uint8_t OWIChannels;
	
	/// OWI owners
	uint8_t OWIOwners;
}OWIDefsStruct;
static volatile OWIDefsStruct OWIDefs;

/*******************************************************************************//**
 * writes bit
 * @param[in] Bit bit to write
 **********************************************************************************/
void OWI_WriteBit(uint8_t Bit)
{
	if(Bit)
	{
		// drive bus low and delay
		OWI_PULL_BUS_LOW
		_delay_us(OWI_DELAY_A_STD_MODE);
		
		// release bus and delay
		OWI_RELEASE_BUS
		_delay_us(OWI_DELAY_B_STD_MODE);
	}
	else
	{
		// drive bus low and delay
		OWI_PULL_BUS_LOW
		_delay_us(OWI_DELAY_C_STD_MODE);
	
		// release bus and delay
		OWI_RELEASE_BUS
		_delay_us(OWI_DELAY_D_STD_MODE);
	}
	
}

/*******************************************************************************//**
 * reads next bit
 * @return bit
 **********************************************************************************/
uint8_t OWI_ReadBit(void)
{
	BOOL Result = 0;
	
	// drive bus low and delay
	OWI_PULL_BUS_LOW
	_delay_us(OWI_DELAY_A_STD_MODE);
	
	// release bus and delay
	OWI_RELEASE_BUS
	_delay_us(OWI_DELAY_E_STD_MODE);
	
	// sample bus and delay
	if(OWI_PIN & (1<<OWI_PIN_NUM))
		Result = 1;
	_delay_us(OWI_DELAY_F_STD_MODE);
	
	return Result;
}

/*******************************************************************************//**
 * @implements OWI_Init
 **********************************************************************************/
RESULT OWI_Init(void)
{
	OWI_RELEASE_BUS
	// the first rising edge can be interpreted by a slave as the end of a
	// Reset pulse. Delay for the required reset recovery time (H) to be 
	// sure that the real reset is interpreted correctly
	_delay_us(OWI_DELAY_H_STD_MODE);
	
	// init status, active channels mask and type
	OWIDefs.Status      = 0;
	OWIDefs.OWIChannels = 0;
	OWIDefs.OWIOwners   = 0;
	
	return SUCCESS;
}

#ifdef USE_PWR
/*******************************************************************************//**
 * @implements OWI_PowerSave
 **********************************************************************************/
void OWI_PowerSave(void)
{
	// empty
}

/*******************************************************************************//**
 * @implements OWI_Restore
 **********************************************************************************/
void OWI_Restore(void)
{
	// empty
}
#endif

/*******************************************************************************//**
 * @implements OWI_Open
 **********************************************************************************/
HOWI OWI_Open(uint8_t Channel)
{
	// check channel
	if(Channel>=ATMEGA128_MAX_OWI_CHANNELS)
		return INVALID_HANDLE;
	
	// check if this channel is not already used
	if(OWIDefs.OWIChannels&(1<<Channel))
		return INVALID_HANDLE;
	
	// set OWI access rights
	if(!Guard_IsWatching())
		OWI_SET_SYS_ACCESS_RIGHTS(Channel)
	else
		OWI_SET_APP_ACCESS_RIGHTS(Channel)
	
	// mark channel as active and return OWI handle
	OWIDefs.OWIChannels |= (1<<Channel);
	
	return Channel;
}

/*******************************************************************************//**
 * @implements OWI_Close
 **********************************************************************************/
RESULT OWI_Close(HOWI OWI)
{
	// check OWI handle
	if(OWI>=ATMEGA128_MAX_OWI_CHANNELS)
		return FAIL;
	
	if(!(OWIDefs.OWIChannels&(1<<OWI)))
		return FAIL;
	
	// if OWI is a system OWI and guard is watching for a threat
	// then return failure
	if(OWI_IS_SYSTEM_OWI(OWI)&&Guard_IsWatching())
		return FAIL;
	
	// mark channel as inactive and return success
	OWIDefs.OWIChannels &= ~(1<<OWI);
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements OWI_Start
 **********************************************************************************/
RESULT OWI_Start(HOWI OWI)
{
	// check OWI handle
	if(OWI>=ATMEGA128_MAX_OWI_CHANNELS)
		return FAIL;
	
	if(!(OWIDefs.OWIChannels&(1<<OWI)))
		return FAIL;
	
	// if OWI is a system OWI and guard is watching for a threat
	// then return failure
	if(OWI_IS_SYSTEM_OWI(OWI)&&Guard_IsWatching())
		return FAIL;
	
	// check status
	if(OWIDefs.Status&&!(OWIDefs.Status&(1<<OWI)))
		return FAIL;
	
	RESULT Result = FAIL;
	
	// send start
	OWIDefs.Status |= (1<<OWI);
	
	// drive bus low and delay
	OWI_PULL_BUS_LOW
	_delay_us(OWI_DELAY_H_STD_MODE);
	
	// release bus and delay
	OWI_RELEASE_BUS
	_delay_us(OWI_DELAY_I_STD_MODE);
	
	// sample bus to detect presence signal and delay
	if((~OWI_PIN) & (1<<OWI_PIN_NUM))
		Result = SUCCESS;
	else
		OWIDefs.Status &= ~(1<<OWI);
	
	_delay_us(OWI_DELAY_J_STD_MODE);
	
	return Result;
}

/*******************************************************************************//**
 * @implements OWI_Stop
 **********************************************************************************/
RESULT OWI_Stop(HOWI OWI)
{
	// check OWI handle
	if(OWI>=ATMEGA128_MAX_OWI_CHANNELS)
		return FAIL;
	
	if(!(OWIDefs.OWIChannels&(1<<OWI)))
		return FAIL;
	
	// if OWI is a system OWI and guard is watching for a threat
	// then return failure
	if(OWI_IS_SYSTEM_OWI(OWI)&&Guard_IsWatching())
		return FAIL;
	
	// check status
	if(!(OWIDefs.Status&(1<<OWI)))
		return FAIL;
	
	// stop OWI
	OWIDefs.Status &= ~(1<<OWI);
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements OWI_Tx
 **********************************************************************************/
RESULT OWI_Tx(HOWI OWI,uint8_t Byte)
{
	uint8_t i;
	
	// check OWI handle
	if(OWI>=ATMEGA128_MAX_OWI_CHANNELS)
		return FAIL;
	
	if(!(OWIDefs.OWIChannels&(1<<OWI)))
		return FAIL;
	
	// if OWI is a system OWI and guard is watching for a threat
	// then return failure
	if(OWI_IS_SYSTEM_OWI(OWI)&&Guard_IsWatching())
		return FAIL;
	
	// check status
	if(!(OWIDefs.Status&(1<<OWI)))
		return FAIL;
	
	// do once for each bit
	for(i=0;i<8;++i)
	{
		// determine if lsb is '0' or '1' and transmit corresponding
		// waveform on the bus
		OWI_WriteBit(Byte & 0x01);
		
		// right shift the data to get next bit
		Byte >>= 1;
		
	}
	
	return SUCCESS;
	
}

/*******************************************************************************//**
 * @implements OWI_Rx
 **********************************************************************************/
RESULT OWI_Rx(HOWI OWI,uint8_t *Byte)
{
	uint8_t i;
	
	// check byte
	if(Byte==NULL)
		return FAIL;
	
	// check OWI handle
	if(OWI>=ATMEGA128_MAX_OWI_CHANNELS)
		return FAIL;
	
	if(!(OWIDefs.OWIChannels&(1<<OWI)))
		return FAIL;
	
	// if OWI is a system OWI and guard is watching for a threat
	// then return failure
	if(OWI_IS_SYSTEM_OWI(OWI)&&Guard_IsWatching())
		return FAIL;
	
	// check status
	if(!(OWIDefs.Status&(1<<OWI)))
		return FAIL;
	
	// clear the byte
	(*Byte) = 0x00;
	
	// do once for each bit
	for (i = 0; i < 8; ++i)
	{
		// shift temporary input variable right
		(*Byte) >>= 1;
		
		// set the msb if a '1' value is read from the bus
		// leave as it is ('0') else
		if(OWI_ReadBit())
			(*Byte) |= 0x80;
		
	}
	
	return SUCCESS;
}
