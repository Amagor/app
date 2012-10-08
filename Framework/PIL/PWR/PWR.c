/**
 * @file PWR.c
 * Power management implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/Scheduler/Scheduler.h"
#include "../../PIL/Sensors/Sensors.h"
#include "../../PIL/Buttons/Buttons.h"
#include "../../PIL/Timers/Timers.h"
#include "../../PIL/NWK/NWKLayer.h"
#include "../../PIL/LEDs/LEDs.h"
#include "../../PIL/UART/UART.h"
#include "../../PIL/MCU/MCU.h"
#include "../../PIL/PWR/PWR.h"
#include "../../PIL/SPI/SPI.h"
#include "../../PIL/TWI/TWI.h"
#include "../../PIL/OWI/OWI.h"

/// thread for handling power management
static volatile HThread Thread = INVALID_HANDLE;

/*******************************************************************************//**
 * power management thread proc
 **********************************************************************************/
PROC PWR_ThreadProc(PARAM Param)
{
	// prepare all OS components to enter power save mode
	// radio
	#ifdef USE_NWK
	Radio_PowerSave();
	#endif
	
	// sensors
	#ifdef USE_SENSORS
	Sensors_PowerSave();
	#endif
	
	// UART
	#ifdef USE_UART
	UART_PowerSave();
	#endif
	
	// SPI
	#ifdef USE_SPI
	SPI_PowerSave();
	#endif
	
	// TWI
	#ifdef USE_TWI
	TWI_PowerSave();
	#endif
	
	// OWI
	#ifdef USE_OWI
	OWI_PowerSave();
	#endif
	
	// LEDs
	#ifdef USE_LEDS
	LEDs_PowerSave();
	#endif
	
	// buttons
	#ifdef USE_BUTTONS
	Buttons_PowerSave();
	#endif
	
	// timers
	#ifdef USE_TIMERS
	Timers_PowerSave();
	#endif
	
	// enter power save mode
	MCU_PowerSave();
	
	// restore all OS components after leaving power save mode
	// timers
	#ifdef USE_TIMERS
	Timers_Restore();
	Timers_UpdateClock(MCU_GetPowerSavePeriod());
	#endif
	
	// UART
	#ifdef USE_UART
	UART_Restore();
	#endif
	
	// SPI
	#ifdef USE_SPI
	SPI_Restore();
	#endif
	
	// TWI
	#ifdef USE_TWI
	TWI_Restore();
	#endif
	
	// OWI
	#ifdef USE_OWI
	OWI_Restore();
	#endif
	
	// LEDs
	#ifdef USE_LEDS
	LEDs_Restore();
	#endif
	
	// buttons
	#ifdef USE_BUTTONS
	Buttons_Restore();
	#endif
	
	// radio
	#ifdef USE_NWK
	Radio_Restore();
	#endif
	
	// sensors
	#ifdef USE_SENSORS
	Sensors_Restore();
	#endif
	
}

/*******************************************************************************//**
 * @implements PWR_Init
 **********************************************************************************/
RESULT PWR_Init(void)
{
	// create thread
	Thread = Thread_Create(PWR_ThreadProc,NULL);
	if(IS_INVALID_HANDLE(Thread))
		return FAIL;
	
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements PWR_PowerSave
 **********************************************************************************/
void PWR_PowerSave(TIME Period)
{
	MCU_SetPowerSavePeriod(Period);
	Thread_Start(Thread,THREAD_TASK_MODE);
}
