/**
 * @file MCU.h
 * MCU functions implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __MCU_H__
#define __MCU_H__

#include "../../PIL/Defs.h"

/*******************************************************************************//**
 * inits MCU
 * @return SUCCESS if MCU successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT MCU_Init(void);

/*******************************************************************************//**
 * forces MCU to enter power save mode
 **********************************************************************************/
void MCU_PowerSave(void);

/*******************************************************************************//**
 * sets power save mode period in micro seconds
 * @param[in] Period power save period
 **********************************************************************************/
void MCU_SetPowerSavePeriod(TIME Period);

/*******************************************************************************//**
 * returns power save period in micro seconds
 * @return power save period
 **********************************************************************************/
TIME MCU_GetPowerSavePeriod(void);

/*******************************************************************************//**
 * forces MCU to enter idle mode
 **********************************************************************************/
void MCU_Idle(void);

/*******************************************************************************//**
 * enables interrupts
 **********************************************************************************/
void MCU_EnableInterrupts(void);

/*******************************************************************************//**
 * disables interrupts
 * @return TRUE  if interrupts were enabled before this function was called
 * @return FALSE otherwise
 **********************************************************************************/
BOOL MCU_DisableInterrupts(void);

/*******************************************************************************//**
 * returns global interrupts state
 * @return TRUE  if interrupts are enabled
 * @return FALSE otherwise
 **********************************************************************************/
BOOL MCU_InterruptsEnabled(void);

#endif
