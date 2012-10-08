/**
 * @file MACLayerCSMACA.c
 * MAC layer CSMA-CA implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/NWK/MAC/MACLayerCSMACA.h"
#include "../../PIL/NWK/MAC/MACLayer.h"
#include "../../PIL/NWK/PHY/PHYLayer.h"
#include "../../PIL/Timers/Timers.h"
#include "../../PIL/utils.h"

/// MAC layer frame wich must be transmitted
typedef struct
{
	/// MAC layer CSMA-CA timer
	HTimer Timer;
	
	/// NB
	uint8_t NB;
	
	/// BE
	uint8_t BE;
	
	/// max CSMA backoffs
	uint8_t MaxCSMABackoffs;
	
	/// minBE
	uint8_t MinBE;
}MACLayerCSMACADefsStruct;
static volatile MACLayerCSMACADefsStruct MACLayerCSMACADefs;

/*******************************************************************************//**
 * MAC layer CSMA-CA timer "fired" event
 **********************************************************************************/
EVENT MACLayer_CSMACATimerFired(PARAM Param)
{
	// inc NB
	++MACLayerCSMACADefs.NB;
	
	// inc BE
	++MACLayerCSMACADefs.BE;
	if(MACLayerCSMACADefs.BE>MAC_A_MAX_BE)
		MACLayerCSMACADefs.BE = MAC_A_MAX_BE;
	
	// perform CCA
	PHYLayer_CCA_Request();
	
}

/*******************************************************************************//**
 * @implements MACLayer_TimerFired
 **********************************************************************************/
RESULT MACLayerCSMACA_Init(void)
{
	// init CSMA-CA
	MACLayerCSMACADefs.MaxCSMABackoffs = 4;
	MACLayerCSMACADefs.MinBE           = 3;
	
	// create timer
	MACLayerCSMACADefs.Timer = Timer_Create(MACLayer_CSMACATimerFired,NULL);
	if(IS_INVALID_HANDLE(MACLayerCSMACADefs.Timer))
		return FAIL;
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements MACLayerCSMACA_Start
 **********************************************************************************/
void MACLayerCSMACA_Start(void)
{
	uint32_t WaitInterval;
	
	MACLayerCSMACADefs.NB = 0;
	MACLayerCSMACADefs.BE = MACLayerCSMACADefs.MinBE;
	
	// start timer
	WaitInterval = (uint32_t)Utils_Rand( ( (1<<MACLayerCSMACADefs.BE) - 1) );
	WaitInterval *= MAC_A_UNIT_BACKOFF_PERIOD;
	WaitInterval = WaitInterval<<4;
	
	if(WaitInterval==0)
	{
		MACLayer_CSMACATimerFired(NULL);
		
	}
	else
		Timer_Start(MACLayerCSMACADefs.Timer,TIMER_ONE_SHOT_MODE,WaitInterval);
	
}

/*******************************************************************************//**
 * @implements PHYLayer_CCA_Confirm
 **********************************************************************************/
EVENT PHYLayer_CCA_Confirm(PHY_ENUM Status)
{
	// if channel is idle then start transmission
	if(Status==PHY_IDLE)
	{
		// signal CSMA-CA "done" event with success
		MACLayerCSMACA_Done(SUCCESS);
		
	}
	// if channel is busy, then wait for random(2^BE-1) backoff periods
	else if(Status==PHY_BUSY)
	{
		// if num tries is less or equal than max backoffs, then try again
		if(MACLayerCSMACADefs.NB<=MACLayerCSMACADefs.MaxCSMABackoffs)
		{
			uint32_t WaitInterval;
			
			// start timer
			WaitInterval = (uint32_t)Utils_Rand( ( (1<<MACLayerCSMACADefs.BE) - 1) );
			WaitInterval *= MAC_A_UNIT_BACKOFF_PERIOD;
			WaitInterval = WaitInterval<<4;
			
			if(WaitInterval==0)
			{
				MACLayer_CSMACATimerFired(NULL);
				
			}
			else
				Timer_Start(MACLayerCSMACADefs.Timer,TIMER_ONE_SHOT_MODE,WaitInterval);
			
		}
		// else
		else
		{
			// signal CSMA-CA "done" event with fail
			MACLayerCSMACA_Done(FAIL);
			
		}
		
	}
	// else
	else
	{
		// signal CSMA-CA "done" event with fail
		MACLayerCSMACA_Done(FAIL);
		
	}
	
}
