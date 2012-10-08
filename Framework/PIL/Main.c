/**
 * @file Main.c
 * Main module.
 * @author Nezametdinov I.E.
 */

#include "../PIL/Scheduler/Scheduler.h"
#include "../PIL/Components.h"
#include "../API/CommonAPI.h"
#include "../PIL/Hardware.h"
#include "../PIL/MCU/MCU.h"
#include "../PIL/Booted.h"
#include "../PIL/Guard.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

void read_MAC();

/// OS start point
int main(void)
{	
	read_MAC(&MAC); //читаем мак
	// init hardware
	if(InitHardware()==SUCCESS)
	{
		// init OS components
		if(InitComponents()==SUCCESS)
		{
			// signal that OS is booted - this is application entry point
			Guard_Watch();
			Booted();
			
			// enable interrupts
			MCU_EnableInterrupts();
			
			// eternal sunshine of spotless mind ^_^
			Scheduler_RunThreads();
			
		}
		
	}
	
	// OS init failed
	while(TRUE);
	return 0;
}
