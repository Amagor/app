/**
 * @file Timers.h
 * Timers implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __TIMERS_H__
#define __TIMERS_H__

#include "../../API/TimersAPI.h"

/*******************************************************************************//**
 * inits timers
 * @return SUCCESS if timers successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Timers_Init(void);

/*******************************************************************************//**
 * updates system time
 * @param[in] Delta value to be added to the system time in micro seconds
 **********************************************************************************/
void Timers_UpdateClock(TIME Delta);

/*******************************************************************************//**
 * forces timers to enter power save mode
 **********************************************************************************/
void Timers_PowerSave(void);

/*******************************************************************************//**
 * restores timers after leaving power save mode
 **********************************************************************************/
void Timers_Restore(void);

#endif
