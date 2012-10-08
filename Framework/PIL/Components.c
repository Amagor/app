/**
 * @file Components.c
 * OS components init source file.
 * @author Nezametdinov I.E.
 */

#include "../PIL/Scheduler/Scheduler.h"
#include "../PIL/Sensors/Sensors.h"
#include "../PIL/Buttons/Buttons.h"
#include "../PIL/Timers/Timers.h"
#include "../PIL/NWK/NWKLayer.h"
#include "../PIL/Components.h"
#include "../PIL/LEDs/LEDs.h"
#include "../PIL/UART/UART.h"
#include "../PIL/MCU/MCU.h"
#include "../PIL/PWR/PWR.h"
#include "../PIL/SPI/SPI.h"
#include "../PIL/TWI/TWI.h"
#include "../PIL/OWI/OWI.h"
#include "../PIL/Guard.h"
#include "../PIL/Utils.h"

/*******************************************************************************//**
 * @implements InitComponents
 **********************************************************************************/
RESULT InitComponents(void)
{
	// init guard
	if(Guard_Init()==FAIL)
		return FAIL;
	
	// init utils
	if(Utils_Init()==FAIL)
		return FAIL;
	
	// init MCU
	if(MCU_Init()==FAIL)
		return FAIL;
	
	// init scheduler
	if(Scheduler_Init()==FAIL)
		return FAIL;
	
	// init power management system
	#ifdef USE_PWR
	if(PWR_Init()==FAIL)
		return FAIL;
	#endif
	
	// init timers
	#ifdef USE_TIMERS
	if(Timers_Init()==FAIL)
		return FAIL;
	#endif
	
	// init LEDs
	#ifdef USE_LEDS
	if(LEDs_Init()==FAIL)
		return FAIL;
	#endif
	
	// init buttons
	#ifdef USE_BUTTONS
	if(Buttons_Init()==FAIL)
		return FAIL;
	#endif
	
	// init UART
	#ifdef USE_UART
	if(UART_Init()==FAIL)
		return FAIL;
	#endif
	
	// init SPI
	#ifdef USE_SPI
	if(SPI_Init()==FAIL)
		return FAIL;
	#endif
	
	// init TWI
	#ifdef USE_TWI
	if(TWI_Init()==FAIL)
		return FAIL;
	#endif
	
	// init OWI
	#ifdef USE_OWI
	if(OWI_Init()==FAIL)
		return FAIL;
	#endif
	
	// init other components
	if(InitOther()==FAIL)
		return FAIL;
	
	// init sensors
	#ifdef USE_SENSORS
	if(Sensors_Init()==FAIL)
		return FAIL;
	#endif
	
	// init NWK
	#ifdef USE_NWK
	if(NWKLayer_Init()==FAIL)
		return FAIL;
	#endif
	
	// return success
	return SUCCESS;
}
