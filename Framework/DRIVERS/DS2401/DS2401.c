/**
 * @file DS2401.c
 * DS2401 implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../DRIVERS/DS2401/DS2401.h"
#include "../API/OWIAPI.h"
#include "../PIL/Guard.h"

#ifndef DS2401_OWI_CHANNEL
#define DS2401_OWI_CHANNEL 0
#endif

/// this module needs OWI
#ifndef USE_OWI
#error OWI needed but not used
#endif

#define DS2401_ROM_READ 0x0F

/// DS2401 OWI
static volatile HOWI OWI = INVALID_HANDLE;

/// DS2401 busy state
static volatile BOOL Busy = FALSE;

/*******************************************************************************//**
 * @implements DS2401_Init
 **********************************************************************************/
/*RESULT DS2401_Init(void)
{
	// init DS2401
	Busy = FALSE;
	
	// open OWI
	OWI = OWI_Open(DS2401_OWI_CHANNEL);
	if(IS_INVALID_HANDLE(OWI))
		return FAIL;
	
	// return success
	return SUCCESS;
}
*/
/*******************************************************************************//**
 * @implements DS2401_ReadROM
 **********************************************************************************/
/*
RESULT DS2401_ReadROM(uint64_t *ROM)
{
	uint8_t* TmpROM = (uint8_t*)ROM;
	uint8_t i;
	
	// if DS2401 busy return failure
	if(Busy)
		return FAIL;
	
	// change DS2401 state
	Busy = TRUE;
	
	// save current guard state
	SAVE_GUARD_STATE
	
	// this is a system block of code, so guard may idle
	Guard_Idle();
	
	// start OWI transmission
	if(OWI_Start(OWI)!=SUCCESS)
	{
		// restore previous guard state
		RESTORE_GUARD_STATE
		
		// restore DS2401 state
		Busy = FALSE;
		
		// return failure
		return FAIL;
	}
	
	// read ROM
	OWI_Tx(OWI,DS2401_ROM_READ);
	for(i=0;i<8;++i)
		OWI_Rx(OWI,&TmpROM[i]);
	OWI_Stop(OWI);
	
	// restore previous guard state
	RESTORE_GUARD_STATE
	
	// restore DS2401 state
	Busy = FALSE;
	
	// return success
	return SUCCESS;
}
*/