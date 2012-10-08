/**
 * @file HardwareTimer.h
 * Hardware timer implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __HARDWARE_TIMER_H__
#define __HARDWARE_TIMER_H__

#include "../../PIL/Defs.h"

/*******************************************************************************//**
 * inits hardware timer
 * @return SUCCESS if hardware timer successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT HardwareTimer_Init(void);

/*******************************************************************************//**
 * starts hardware timer
 * @param[in] Timeout timeout
 **********************************************************************************/
void HardwareTimer_Start(PERIOD Timeout);

/*******************************************************************************//**
 * stops hardware timer
 **********************************************************************************/
void HardwareTimer_Stop(void);

/*******************************************************************************//**
 * updates hardware timer timeout
 * @param[in] Delta value to be added to the timeout
 **********************************************************************************/
void HardwareTimer_Update(PERIOD Delta);

/*******************************************************************************//**
 * returns time elapsed after hardware timer fire
 * @return time elapsed
 **********************************************************************************/
PERIOD HardwareTimer_GetTimeElapsed(void);

/*******************************************************************************//**
 * returns time left before hardware timer fire
 * @return time left
 **********************************************************************************/
PERIOD HardwareTimer_GetTimeLeft(void);

/*******************************************************************************//**
 * returns hardware timer timeout
 * @return hardware timer timeout
 **********************************************************************************/
PERIOD HardwareTimer_GetTimeout(void);

/*******************************************************************************//**
 * forces hardware timer to enter power save mode
 **********************************************************************************/
void HardwareTimer_PowerSave(void);

/*******************************************************************************//**
 * restores hardware timer after leaving power save mode
 **********************************************************************************/
void HardwareTimer_Restore(void);

/*******************************************************************************//**
 * hardware timer "fired" event
 **********************************************************************************/
EVENT HardwareTimer_Fired(void);

#endif
