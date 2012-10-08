/**
 * @file PlatformHardwareTimer.c
 * Hardware timer implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/Timers/HardwareTimer.h"
#include "../../API/CommonAPI.h"
#include <avr/interrupt.h>
#include <avr/io.h>

/// timeout
static volatile PERIOD HWTimeout = 0;

/// time left
static volatile PERIOD HWTimeLeft = 0;

/// ticks per interrupt
static volatile PERIOD HWTPI = 0;

/*******************************************************************************//**
 * configures hardware timer
 * @param[in] Timeout timeout
 **********************************************************************************/
void HardwareTimer_Configure(PERIOD Timeout)
{
	// stop timer
	TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10));
	TCNT1 = 0;
	
	if(Timeout<=65535)
	{
		OCR1A = Timeout;
		HWTPI = Timeout;
	}
	else
	{
		OCR1A = 65535;
		HWTPI = 65535;
	}
	
	// set prescaler 8
	TCCR1B &= ~((1<<CS12)|(1<<CS10));
	TCCR1B |=  (1<<CS11);
	
}

/// interrupt handler
ISR(SIG_OUTPUT_COMPARE1A)
{
	// recompute time left
	HWTimeLeft -= HWTPI;
	
	// check timeout
	if(HWTimeLeft<=0)
	{
		// signal hardware timer "fired" event
		HardwareTimer_Fired();
		
	}
	else
	{
		// reconfigure hardware timer
		if(HWTimeLeft>65535&&HWTimeLeft<=81919)
			HardwareTimer_Configure(32768);
		else
			HardwareTimer_Configure(HWTimeLeft);
		
	}
	
}

/*******************************************************************************//**
 * @implements HardwareTimer_Init
 **********************************************************************************/
RESULT HardwareTimer_Init(void)
{
	// set CTC mode
	TCCR1A  =  0;
	TCCR1B  = (1<<WGM12);
	
	HardwareTimer_Start(MAX_TIMEOUT);
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements HardwareTimer_Start
 **********************************************************************************/
void HardwareTimer_Start(PERIOD Timeout)
{
	HWTimeout  = Timeout;
	HWTimeLeft = Timeout;
	HardwareTimer_Configure(Timeout);
	TIMSK |= (1<<OCIE1A);
}

/*******************************************************************************//**
 * @implements HardwareTimer_Stop
 **********************************************************************************/
void HardwareTimer_Stop(void)
{
	// stop timer
	TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10));
	
	// disable interrupt
	TIMSK &= ~(1<<OCIE1A);
}

/*******************************************************************************//**
 * @implements HardwareTimer_Update
 **********************************************************************************/
void HardwareTimer_Update(PERIOD Delta)
{
	// stop timer
	TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10));
	
	// update timer
	HWTimeLeft += Delta;
	HWTimeLeft -= TCNT1;
	HWTimeout += Delta;
	
	// recofigure timer
	HardwareTimer_Configure(HWTimeLeft);
}

/*******************************************************************************//**
 * @implements HardwareTimer_GetTimeElapsed
 **********************************************************************************/
PERIOD HardwareTimer_GetTimeElapsed(void)
{
	return (HWTimeout-(HWTimeLeft-TCNT1));
}

/*******************************************************************************//**
 * @implements HardwareTimer_GetTimeLeft
 **********************************************************************************/
PERIOD HardwareTimer_GetTimeLeft(void)
{
	return (HWTimeLeft-TCNT1);
}

/*******************************************************************************//**
 * @implements HardwareTimer_GetInterval
 **********************************************************************************/
PERIOD HardwareTimer_GetTimeout(void)
{
	return HWTimeout;
}

#ifdef USE_PWR
/*******************************************************************************//**
 * @implements HardwareTimer_PowerSave
 **********************************************************************************/
void HardwareTimer_PowerSave(void)
{
	// stop timer
	TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10));
	
	// disable interrupt
	TIMSK &= ~(1<<OCIE1A);
	
}

/*******************************************************************************//**
 * @implements HardwareTimer_Restore
 **********************************************************************************/
void HardwareTimer_Restore(void)
{
	// enable interrupt
	TIMSK |= (1<<OCIE1A);
	
	// start timer and set prescaler 8
	TCCR1B &= ~((1<<CS12)|(1<<CS10));
	TCCR1B |=  (1<<CS11);
	
}
#endif
