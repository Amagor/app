/**
 * @file TimersAPI.h
 * Timers API.
 * @author Nezametdinov I.E.
 */

#ifndef __TIMERS_API_H__
#define __TIMERS_API_H__

#include "../PIL/Defs.h"

/// timer mode
enum
{
	/// one shot
	TIMER_ONE_SHOT_MODE = 0x00,
	/// cyclic
	TIMER_CYCLIC_MODE   = 0x01
};

/// timer handle
typedef uint8_t HTimer;

/*******************************************************************************//**
 * creates a new timer
 * @param[in] Fired timer "fired" event handler
 * @param[in] Param timer parameter
 * @return valid timer handle if timer successfully created
 * @return INVALID_HANDLE     otherwise
 **********************************************************************************/
HTimer Timer_Create(EVENT (*Fired)(PARAM Param),PARAM Param);

/*******************************************************************************//**
 * destroyes the timer
 * @param[in] Timer timer handle
 * @return SUCCESS if timer successfully destroyed
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Timer_Destroy(HTimer Timer);

/*******************************************************************************//**
 * starts the timer
 * @param[in] Timer   timer handle
 * @param[in] Params  timer parameters
 * @param[in] Timeout timer timeout
 * @return SUCCESS if timer successfully started
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Timer_Start(HTimer Timer,uint8_t Params,PERIOD Timeout);

/*******************************************************************************//**
 * stops the timer
 * @param[in] Timer timer handle
 * @return SUCCESS if timer successfully stopped
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Timer_Stop(HTimer Timer);

/*******************************************************************************//**
 * returns timer parameter
 * @param[in] Timer timer handle
 * @return timer parameter if timer parameter successfully returned
 * @return NULL            otherwise
 **********************************************************************************/
PARAM Timer_GetParam(HTimer Timer);

/*******************************************************************************//**
 * sets timer parameter
 * @param[in] Timer timer handle
 * @param[in] Param timer parameter
 * @return SUCCESS if timer parameter successfully set
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Timer_SetParam(HTimer Timer,PARAM Param);

/*******************************************************************************//**
 * returns state of the timer
 * @param[in] Timer timer handle
 * @return TRUE  if timer is active
 * @return FALSE otherwise
 **********************************************************************************/
BOOL Timer_IsActive(HTimer Timer);

/*******************************************************************************//**
 * checks timer existance
 * @param[in] Timer timer handle
 * @return TRUE  if timer exists
 * @return FALSE otherwise
 **********************************************************************************/
BOOL Timer_Exists(HTimer Timer);

/*******************************************************************************//**
 * returns the handle of currently fired timer
 * @return handle of the timer wich is fired right now
 **********************************************************************************/
HTimer Timer_GetHandle(void);

/*******************************************************************************//**
 * returns system time in micro seconds
 * @return system time
 **********************************************************************************/
TIME GetTime(void);

/*******************************************************************************//**
 * this is an example of how to use timers API
 * @example Blink/app.c
 **********************************************************************************/

#endif
