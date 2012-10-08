/**
 * @file Timers.c
 * Timers implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/Timers/HardwareTimer.h"
#include "../../PIL/Timers/Timers.h"
#include "../../API/CommonAPI.h"
#include "../../API/TimersAPI.h"
#include "../../PIL/Guard.h"

#define TIMER_ACTIVITY      0x01
#define TIMER_ACTIVITY_MODE 0x02
#define TIMER_ACCESS_RIGHTS 0x04

#define TIMER_EXISTS(Timer)                      ((Timer.TimerIndex)<MAX_TIMERS)
#define TIMER_IS_SYSTEM_TIMER(Timer)             ((Timer.TimerState)&TIMER_ACCESS_RIGHTS)
#define TIMER_IS_IN_ONE_SHOT_MODE(Timer)         ((Timer.TimerState)&TIMER_ACTIVITY_MODE)
#define TIMER_IS_ACTIVE(Timer)                   ((Timer.TimerState)&TIMER_ACTIVITY)
#define TIMER_ACTIVATE(Timer)                    {Timer.TimerState |= TIMER_ACTIVITY;}
#define TIMER_DEACTIVATE(Timer)                  {Timer.TimerState &= ~TIMER_ACTIVITY;}
#define TIMER_SET_ONE_SHOT_MODE(Timer)           {Timer.TimerState |= TIMER_ACTIVITY_MODE;}
#define TIMER_SET_CYCLIC_MODE(Timer)             {Timer.TimerState &= ~TIMER_ACTIVITY_MODE;}
#define TIMER_SET_SYS_ACCESS_RIGHTS(Timer)       {Timer.TimerState |= TIMER_ACCESS_RIGHTS;}
#define TIMER_SET_APP_ACCESS_RIGHTS(Timer)       {Timer.TimerState &= ~TIMER_ACCESS_RIGHTS;}

/// structure defines timer
typedef struct
{
	/// timer "fired" event handler
	EVENT (*Fired)(PARAM Param);
	
	/// timer parameter
	PARAM Param;
	
	/// time left
	PERIOD TimeLeft;
	
	/// timeout
	PERIOD Timeout;
	
	/// index in array
	uint8_t  TimerIndex;
	
	/// state of timer
	uint8_t  TimerState;
}TimerDefsStruct;

/// array of timers
static volatile TimerDefsStruct TimersDefs[MAX_TIMERS];

/// min timeout of all active timers in micro seconds
static volatile PERIOD MinTimeout = MAX_TIMEOUT;

/// array of timers
static volatile uint8_t TimersArray[MAX_TIMERS];

/// index of the first not existing timer in array of timers
static volatile uint8_t CurrentTimerIndex = 0;

/// handle of the timer wich is processed right now
static volatile HTimer CurrentTimer = INVALID_HANDLE;

/// system time
static volatile TIME SystemTime = 0;

/*******************************************************************************//**
 * @implements HardwareTimer_Fired
 **********************************************************************************/
EVENT HardwareTimer_Fired(void)
{
	uint8_t i;
	PERIOD Timeout = MinTimeout;
	
	// save current guard state
	SAVE_GUARD_STATE
	
	// update system time
	SystemTime += HardwareTimer_GetTimeout();
	
	// begin recomputation of min timeout of all active timers
	MinTimeout = MAX_TIMEOUT;
	
	// loop through all timers
	for(i=0;i<CurrentTimerIndex;++i)
	{
		// this is a system block of code, so guard may idle
		Guard_Idle();
		
		// get timer from array of timers
		CurrentTimer = TimersArray[i];
		
		// check timer activity
		if(!TIMER_IS_ACTIVE(TimersDefs[CurrentTimer]))
			continue;
		
		// update timer
		TimersDefs[CurrentTimer].TimeLeft -= HardwareTimer_GetTimeout();
		
		// if timeout happened
		if(TimersDefs[CurrentTimer].TimeLeft<=0)
		{
			// clear time
			TimersDefs[CurrentTimer].TimeLeft += TimersDefs[CurrentTimer].Timeout;
			
			// if timer is in one shot mode then deactivate it
			if(TIMER_IS_IN_ONE_SHOT_MODE(TimersDefs[CurrentTimer]))
				TIMER_DEACTIVATE(TimersDefs[CurrentTimer])
			
			// if event handler is specified run it
			if(TimersDefs[CurrentTimer].Fired==NULL)
				continue;
			
			// if current timer is a not a system timer, then
			// guard should watch for it
			if(!TIMER_IS_SYSTEM_TIMER(TimersDefs[CurrentTimer]))
				Guard_Watch();
			
			// signal timer "fired" event
			TimersDefs[CurrentTimer].Fired(TimersDefs[CurrentTimer].Param);
			
		}
		// else compute new timeout
		else
			Timeout = TimersDefs[CurrentTimer].TimeLeft<Timeout?TimersDefs[CurrentTimer].TimeLeft:Timeout;
		
		// update min timeout
		if(TIMER_IS_ACTIVE(TimersDefs[CurrentTimer])&&TimersDefs[CurrentTimer].Timeout<MinTimeout)
			MinTimeout = TimersDefs[CurrentTimer].Timeout;
		
	}
	// update hardware timer
	HardwareTimer_Start(Timeout);
	
	// restore previous guard state
	RESTORE_GUARD_STATE
}

/*******************************************************************************//**
 * @implements Timers_Init
 **********************************************************************************/
RESULT Timers_Init(void)
{
	uint8_t i;
	
	// init timers
	for(i=0;i<MAX_TIMERS;++i)
	{
		TimersDefs[i].Fired       = NULL;
		TimersDefs[i].Param       = NULL;
		TimersDefs[i].TimeLeft    = 0;
		TimersDefs[i].Timeout     = 0;
		TimersDefs[i].TimerIndex  = MAX_TIMERS;
		TimersDefs[i].TimerState  = 0;
		TimersArray[i] = i;
	}
	
	MinTimeout = MAX_TIMEOUT;
	CurrentTimerIndex = 0;
	CurrentTimer = INVALID_HANDLE;
	SystemTime = 0;
	
	// init hardware timer
	return HardwareTimer_Init();
}

/*******************************************************************************//**
 * @implements Timers_UpdateClock
 **********************************************************************************/
void Timers_UpdateClock(TIME Delta)
{
	SystemTime += Delta;
}

#ifdef USE_PWR
/*******************************************************************************//**
 * @implements Timers_PowerSave
 **********************************************************************************/
void Timers_PowerSave(void)
{
	HardwareTimer_PowerSave();
}

/*******************************************************************************//**
 * @implements Timers_Restore
 **********************************************************************************/
void Timers_Restore(void)
{
	HardwareTimer_Restore();
}
#endif

/*******************************************************************************//**
 * @implements Timer_Create
 **********************************************************************************/
HTimer Timer_Create(EVENT (*Fired)(PARAM Param),PARAM Param)
{
	HTimer Timer = INVALID_HANDLE;
	
	// if timer fire event handler is not specified
	// return invalid handle
	if(Fired==NULL)
		return INVALID_HANDLE;
	
	// try to create a timer
	BEGIN_CRITICAL_SECTION
	{
		// if there are not existing timers create timer
		if(CurrentTimerIndex<MAX_TIMERS)
		{
			// get not used timer handle
			Timer = (HTimer)TimersArray[CurrentTimerIndex];
			
			// set event handler
			TimersDefs[Timer].Fired = Fired;
			
			// set timer parameter
			TimersDefs[Timer].Param = Param;
			
			// clear elapsed time
			TimersDefs[Timer].TimeLeft = 0;
			
			// set zero timeout
			TimersDefs[Timer].Timeout = 0;
			
			// set timer access rights
			if(!Guard_IsWatching())
				TIMER_SET_SYS_ACCESS_RIGHTS(TimersDefs[Timer])
			else
				TIMER_SET_APP_ACCESS_RIGHTS(TimersDefs[Timer])
			
			// deactivate the timer
			TIMER_DEACTIVATE(TimersDefs[Timer])
			
			// set index in array
			TimersDefs[Timer].TimerIndex = CurrentTimerIndex;
			
			// inc index of the next timer in array of timers
			++CurrentTimerIndex;
			
		}
		
	}
	END_CRITICAL_SECTION
	
	// return timer handle
	return Timer;
}

/*******************************************************************************//**
 * @implements Timer_Destroy
 **********************************************************************************/
RESULT Timer_Destroy(HTimer Timer)
{
	// if timer handle is not valid return failure
	if(Timer>=MAX_TIMERS)
		return FAIL;
	
	// if timer does not exist, then return failure
	if(!TIMER_EXISTS(TimersDefs[Timer]))
		return FAIL;
	
	// if timer is a system timer and guard is watching for a threat
	// then return failure
	if(TIMER_IS_SYSTEM_TIMER(TimersDefs[Timer])&&Guard_IsWatching())
		return FAIL;
	
	// try to destroy timer
	BEGIN_CRITICAL_SECTION
	{
		// if timer exists, then destroy it
		if(TIMER_EXISTS(TimersDefs[Timer]))
		{
			// dec index of next timer in array of existing timers
			--CurrentTimerIndex;
			
			// move the last timer from the array to the place of destroyed timer
			TimersDefs[TimersArray[CurrentTimerIndex]].TimerIndex = TimersDefs[Timer].TimerIndex;
			TimersArray[TimersDefs[Timer].TimerIndex] = TimersArray[CurrentTimerIndex];
			TimersArray[CurrentTimerIndex] = Timer;
			
			// mark timer as not existing
			TimersDefs[Timer].TimerIndex = MAX_TIMERS;
			
		}
		
	}
	END_CRITICAL_SECTION
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Timer_Start
 **********************************************************************************/
RESULT Timer_Start(HTimer Timer,uint8_t Params,PERIOD Timeout)
{
	// if timer handle is not valid return failure
	if(Timer>=MAX_TIMERS)
		return FAIL;
	
	// if no event handler is specified return failure
	if(!TIMER_EXISTS(TimersDefs[Timer]))
		return FAIL;
	
	// if timer is a system timer and guard is watching for a threat
	// then return failure
	if(TIMER_IS_SYSTEM_TIMER(TimersDefs[Timer])&&Guard_IsWatching())
		return FAIL;
	
	// handle small timeout
	if(Timeout<MIN_TIMEOUT)
		Timeout = MIN_TIMEOUT;
	
	// define type of the timer
	if(Params&TIMER_CYCLIC_MODE)
		TIMER_SET_CYCLIC_MODE(TimersDefs[Timer])
	else
		TIMER_SET_ONE_SHOT_MODE(TimersDefs[Timer])
	
	// init timer
	TimersDefs[Timer].TimeLeft = Timeout;
	TimersDefs[Timer].Timeout  = Timeout;
	
	// update min timeout
	if(Timeout<MinTimeout)
		MinTimeout = Timeout;
	
	BEGIN_CRITICAL_SECTION
	{
		// activate the timer
		TIMER_ACTIVATE(TimersDefs[Timer])
		
		// update hardware timer
		PERIOD HWTimeLeft = HardwareTimer_GetTimeLeft();
		PERIOD HWTimeElapsed = HardwareTimer_GetTimeElapsed();
		TimersDefs[Timer].TimeLeft += HWTimeElapsed;
		if(Timeout<HWTimeLeft)
			HardwareTimer_Update(Timeout-HWTimeLeft);
	}
	END_CRITICAL_SECTION
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Timer_Stop
 **********************************************************************************/
RESULT Timer_Stop(HTimer Timer)
{
	// if timer handle is not valid return failure
	if(Timer>=MAX_TIMERS)
		return FAIL;
	
	// if timer is not active or does not exist return failure
	if(!TIMER_EXISTS(TimersDefs[Timer])||!TIMER_IS_ACTIVE(TimersDefs[Timer]))
		return FAIL;
	
	// if timer is a system timer and guard is watching for a threat
	// then return failure
	if(TIMER_IS_SYSTEM_TIMER(TimersDefs[Timer])&&Guard_IsWatching())
		return FAIL;
	
	// deactivate timer
	TIMER_DEACTIVATE(TimersDefs[Timer])
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Timer_GetParam
 **********************************************************************************/
PARAM Timer_GetParam(HTimer Timer)
{
	// if timer handle is not valid return failure
	if(Timer>=MAX_TIMERS)
		return NULL;
	
	// if timer does not exist, then return failure
	if(!TIMER_EXISTS(TimersDefs[Timer]))
		return NULL;
	
	// if timer is a system timer and guard is watching for a threat
	// then return failure
	if(TIMER_IS_SYSTEM_TIMER(TimersDefs[Timer])&&Guard_IsWatching())
		return NULL;
	
	// return timer parameter
	return TimersDefs[Timer].Param;
}

/*******************************************************************************//**
 * @implements Timer_SetParam
 **********************************************************************************/
RESULT Timer_SetParam(HTimer Timer,PARAM Param)
{
	// if timer handle is not valid return failure
	if(Timer>=MAX_TIMERS)
		return FAIL;
	
	// if timer is active or does not exist, then return failure
	if(!TIMER_EXISTS(TimersDefs[Timer])||TIMER_IS_ACTIVE(TimersDefs[Timer]))
		return FAIL;
	
	// if timer is a system timer and guard is watching for a threat
	// then return failure
	if(TIMER_IS_SYSTEM_TIMER(TimersDefs[Timer])&&Guard_IsWatching())
		return FAIL;
	
	// set timer parameter
	TimersDefs[Timer].Param = Param;
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Timer_IsActive
 **********************************************************************************/
BOOL Timer_IsActive(HTimer Timer)
{
	// if timer handle is not valid return false
	if(Timer>=MAX_TIMERS)
		return FALSE;
	
	// if timer does not exist return false
	if(!TIMER_EXISTS(TimersDefs[Timer]))
		return FALSE;
	
	// check timer
	if(TIMER_IS_ACTIVE(TimersDefs[Timer]))
		return TRUE;
	
	// timer is not active
	return FALSE;
}

/*******************************************************************************//**
 * @implements Timer_Exists
 **********************************************************************************/
BOOL Timer_Exists(HTimer Timer)
{
	// if timer handle is not valid return false
	if(Timer>=MAX_TIMERS)
		return FALSE;
	
	// if timer does not exist return false
	if(!TIMER_EXISTS(TimersDefs[Timer]))
		return FALSE;
	
	// timer exists, return true
	return TRUE;
}

/*******************************************************************************//**
 * @implements Timer_GetHandle
 **********************************************************************************/
HTimer Timer_GetHandle(void)
{
	return CurrentTimer;
}

/*******************************************************************************//**
 * @implements GetTime
 **********************************************************************************/
TIME GetTime(void)
{
	return SystemTime + HardwareTimer_GetTimeElapsed();
}
