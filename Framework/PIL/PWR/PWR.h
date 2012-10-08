/**
 * @file PWR.h
 * Power management implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __PWR_H__
#define __PWR_H__

#include "../../PIL/Defs.h"

/*******************************************************************************//**
 * inits power management system
 * @return SUCCESS if power management system successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT PWR_Init(void);

/*******************************************************************************//**
 * forces OS to enter power save mode
 * @param[in] Period power save period
 **********************************************************************************/
void PWR_PowerSave(TIME Period);

#endif
