/**
 * @file PlatformMCU.c
 * MCU functions implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/MCU/MCU.h"
#include <avr/interrupt.h>
#include <avr/io.h>

#ifdef USE_PWR
/// power save period in micro seconds
static volatile TIME PowerSavePeriod = 0;

/// time elapsed since entering power save
static volatile TIME TimeLeft = 0;

/// ticks per interrupt
static volatile uint32_t TicksPerInterrupt = 0;

/// power save flag
static volatile BOOL PowerSave = FALSE;

/*******************************************************************************//**
 * starts power save timer
 **********************************************************************************/
void PowerSaveTimer_Start(void)
{
	// enable timer and set async operation
	ASSR  |= (1<<AS0);
	TCCR0 |= (1<<WGM01) | (0<<WGM00);
	
	// set compare register
	OCR0 = 255;
	while(ASSR & (1<<OCR0UB));
	
	// clear counter register
	TCNT0 = 0;
	while(ASSR & (1<<TCN0UB));
	
	// set 1024 prescaler
	TCCR0 |= (1<<CS02) | (1<<CS01) | (1<<CS00);
	while(ASSR & (1<<TCR0UB));
	
	// enable interrupt
	TIMSK |= (1<<OCIE0);
}

/*******************************************************************************//**
 * stops power save timer
 **********************************************************************************/
void PowerSaveTimer_Stop(void)
{
	// stop timer
	TIMSK &= ~(1<<OCIE0);
	TCCR0 &= ~((1<<CS02) | (1<<CS01) | (1<<CS00));
	while(ASSR & (1<<TCR0UB));
	
	// disable async operation
	ASSR  &= ~(1<<AS0);
	TCCR0 &= ~((1<<WGM01) | (0<<WGM00));
}

/*******************************************************************************//**
 * configures power save timer
 * @param[in] Interval power save timer interval
 **********************************************************************************/
void PowerSaveTimer_Configure(TIME Interval)
{
	// change compare register of the timer
	if(Interval<500000)
	{
		OCR0 = 1;
		TicksPerInterrupt = 31250;
		while(ASSR & (1<<OCR0UB));
	}
	else if(Interval<1000000)
	{
		OCR0 = 16;
		TicksPerInterrupt = 500000;
		while(ASSR & (1<<OCR0UB));
	}
	else if(Interval<4000000)
	{
		OCR0 = 32;
		TicksPerInterrupt = 1000000;
		while(ASSR & (1<<OCR0UB));
	}
	else if(Interval<7968750)
	{
		OCR0 = 128;
		TicksPerInterrupt = 4000000;
		while(ASSR & (1<<OCR0UB));
	}
	else
	{
		OCR0 = 255;
		TicksPerInterrupt = 7968750;
		while(ASSR & (1<<OCR0UB));
	}
	
}

/// interrupt handler
SIGNAL(SIG_OUTPUT_COMPARE0)
{
	// update elapsed time
	TimeLeft -= TicksPerInterrupt;
	
	// check interval
	if(TimeLeft<=0)
	{
		// stop timer
		TIMSK &= ~(1<<OCIE0);
		TCCR0 &= ~((1<<CS02) | (1<<CS01) | (1<<CS00));
		
		PowerSavePeriod -= TimeLeft;
		PowerSave = FALSE;
		
	}
	else
	{
		// if time left is less than min count out interval
		if(TimeLeft<31250)
		{
			// stop timer
			TIMSK &= ~(1<<OCIE0);
			TCCR0 &= ~((1<<CS02) | (1<<CS01) | (1<<CS00));
			
			// update power save time
			PowerSavePeriod -= TimeLeft;
			PowerSave = FALSE;
			return;
			
		}
		
		// reconfigure power save timer
		PowerSaveTimer_Configure(TimeLeft);
		
	}
	
}

/*******************************************************************************//**
 * @implements MCU_PowerSave
 **********************************************************************************/
void MCU_PowerSave(void)
{
	TimeLeft = PowerSavePeriod;
	PowerSave = TRUE;
	
	// start power save timer
	PowerSaveTimer_Start();
	
	// configure power save timer
	PowerSaveTimer_Configure(TimeLeft);
	
	// set power save mode
	MCUCR &= ~((1<<SM2)|(1<<SM1)|(1<<SM0));
	MCUCR |= (1<<SM1)|(1<<SM0);
	
	// enter power save mode
	while(PowerSave)
	{
		MCUCR |= (1<<SE);
		__asm__ __volatile__ ("sleep" "\n\t" :: );
		MCUCR &= ~(1<<SE);
	}
	
	// stop power save timer
	PowerSaveTimer_Stop();
}

/*******************************************************************************//**
 * @implements MCU_SetPowerSavePeriod
 **********************************************************************************/
void MCU_SetPowerSavePeriod(TIME Period)
{
	PowerSavePeriod = Period;
}

/*******************************************************************************//**
 * @implements MCU_GetPowerSavePeriod
 **********************************************************************************/
TIME MCU_GetPowerSavePeriod(void)
{
	return PowerSavePeriod;
}
#endif

/*******************************************************************************//**
 * @implements MCU_Init
 **********************************************************************************/
RESULT MCU_Init(void)
{
	// init power save
	#ifdef USE_PWR
	TimeLeft = 0;
	PowerSavePeriod = 0;
	TicksPerInterrupt = 0;
	PowerSave = FALSE;
	#endif
	
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements MCU_Idle
 **********************************************************************************/
void MCU_Idle(void)
{
	// set idle mode
	MCUCR &= ~((1<<SM2)|(1<<SM1)|(1<<SM0));
	
	// enter idle mode
	MCUCR |= (1<<SE);
	sei();
	__asm__ __volatile__ ("sleep" "\n\t" :: );
	cli();
	MCUCR &= ~(1<<SE);
	
	// wait for interrupt to happen
	__asm__ __volatile__ ("nop" "\n\t" :: );
	__asm__ __volatile__ ("nop" "\n\t" :: );
	
}

/*******************************************************************************//**
 * @implements MCU_EnableInterrupts
 **********************************************************************************/
inline void MCU_EnableInterrupts(void)
{
	sei();
}

/*******************************************************************************//**
 * @implements MCU_DisableInterrupts
 **********************************************************************************/
inline BOOL MCU_DisableInterrupts(void)
{
	BOOL Res = MCU_InterruptsEnabled();
	cli();
	return Res;
}

/*******************************************************************************//**
 * @implements MCU_InterruptsEnabled
 **********************************************************************************/
inline BOOL MCU_InterruptsEnabled(void)
{
	if(SREG&(1<<7))
		return TRUE;
	return FALSE;
}
