/**
 * @file SHT11.c
 * SHT11 implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../DRIVERS/SHT11/SHT11.h"
#include "../../PIL/Timers/Timers.h"
#include "../../API/TWIAPI.h"
#include "../../PIL/Guard.h"

#ifndef SHT11_TWI_CHANNEL
#define SHT11_TWI_CHANNEL 0
#endif

#define SHT11_SET_BUSY                 {SHT11Defs.State |=  1;}
#define SHT11_SET_NOT_BUSY             {SHT11Defs.State &= ~1;}
#define SHT11_IS_BUSY                  (SHT11Defs.State&1)
#define SHT11_MEASURING_TEMPERATURE    {SHT11Defs.State |=  2;}
#define SHT11_MEASURING_HUMIDITY       {SHT11Defs.State &= ~2;}
#define SHT11_IS_MEASURING_TEMPERATURE (SHT11Defs.State&2)

#define MAX_SHT11_MEASUREMENT_TIME 320

// this module needs TWI and Timers
#ifndef USE_TWI
#error TWI needed but not used
#endif

#ifndef USE_TIMERS
#error Timers needed but not used
#endif

/// structure defines SHT11
typedef struct
{
	/// SHT11 timer handle
	HTimer Timer;
	
	/// SHT11 TWI handle
	HTWI TWI;
	
	/// SHT11 state
	uint8_t State;
	
	/// SHT11 last measured value
	uint16_t Value;
}SHT11DefsStruct;
static volatile SHT11DefsStruct SHT11Defs;

/*******************************************************************************//**
 * SHT11 timer "fired" event
 **********************************************************************************/
EVENT SHT11_TimerFired(PARAM Param)
{
	uint8_t Value = 0;
	SHT11_QUANTITY Quantity;
	
	// check quantity
	if(SHT11_IS_MEASURING_TEMPERATURE)
		Quantity = SHT11_TEMPERATURE;
	else
		Quantity = SHT11_HUMIDITY;
	
	// get higher byte
	if(TWI_Rx(SHT11Defs.TWI,&Value)!=SUCCESS)
	{
		TWI_Stop(SHT11Defs.TWI);
		SHT11_SET_NOT_BUSY
		SHT11_MeasurementDone(Quantity,0,FAIL);
		return;
	}
	SHT11Defs.Value = Value;
	SHT11Defs.Value = SHT11Defs.Value<<8;
	
	// get lower byte
	if(TWI_Rx(SHT11Defs.TWI,&Value)!=SUCCESS)
	{
		TWI_Stop(SHT11Defs.TWI);
		SHT11_SET_NOT_BUSY
		SHT11_MeasurementDone(Quantity,0,FAIL);
		return;
	}
	SHT11Defs.Value |= Value;
	
	// get CRC8
	if(TWI_Rx(SHT11Defs.TWI,&Value)!=SUCCESS)
	{
		TWI_Stop(SHT11Defs.TWI);
		SHT11_SET_NOT_BUSY
		SHT11_MeasurementDone(Quantity,0,FAIL);
		return;
	}
	
	// signal "measurement done" event
	SHT11_SET_NOT_BUSY
	TWI_Stop(SHT11Defs.TWI);
	SHT11_MeasurementDone(Quantity,SHT11Defs.Value,SUCCESS);
}

/*******************************************************************************//**
 * @implements SHT11_Init
 **********************************************************************************/
RESULT SHT11_Init(void)
{
	// init SHT11
	SHT11Defs.State = 0;
	
	// create timer
	SHT11Defs.Timer = Timer_Create(SHT11_TimerFired,NULL);
	if(IS_INVALID_HANDLE(SHT11Defs.Timer))
		return FAIL;
	
	// open TWI
	SHT11Defs.TWI = TWI_Open(SHT11_TWI_CHANNEL,TWI_UNIQUE_SHT);
	if(IS_INVALID_HANDLE(SHT11Defs.TWI))
		return FAIL;
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements SHT11_RequestMeasurement
 **********************************************************************************/
RESULT SHT11_RequestMeasurement(SHT11_QUANTITY Quantity)
{
	// if SHT11 is already busy then return fail
	if(SHT11_IS_BUSY)
		return FAIL;
	
	// check quantity
	if(Quantity!=SHT11_TEMPERATURE&&Quantity!=SHT11_HUMIDITY)
		return FAIL;
	
	// save current guard state
	SAVE_GUARD_STATE
	
	// change SHT11 state
	SHT11_SET_BUSY
	if(Quantity==SHT11_TEMPERATURE)
		SHT11_MEASURING_TEMPERATURE
	else
		SHT11_MEASURING_HUMIDITY
	
	// this is a system block of code, so guard may idle
	Guard_Idle();
	
	// start TWI transmission
	if(TWI_Start(SHT11Defs.TWI)!=SUCCESS)
	{
		// restore previous guard state
		RESTORE_GUARD_STATE
		
		SHT11_SET_NOT_BUSY
		return FAIL;
	}
	
	// transmit command
	if(TWI_Tx(SHT11Defs.TWI,Quantity)==SUCCESS)
		Timer_Start(SHT11Defs.Timer,TIMER_ONE_SHOT_MODE,MS(MAX_SHT11_MEASUREMENT_TIME));
	else
	{
		// restore previous guard state
		RESTORE_GUARD_STATE
		
		SHT11_SET_NOT_BUSY
		return FAIL;
	}
	
	// restore previous guard state
	RESTORE_GUARD_STATE
	
	// return success
	return SUCCESS;
}
