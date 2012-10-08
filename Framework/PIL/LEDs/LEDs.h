/**
 * @file LEDs.h
 * LEDs implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __LEDS_H__
#define __LEDS_H__

#include "../../PIL/Defs.h"

/*******************************************************************************//**
 * inits LEDs
 * @return SUCCESS if LEDs successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT LEDs_Init(void);

/*******************************************************************************//**
 * forces LEDs to enter power save mode
 **********************************************************************************/
void LEDs_PowerSave(void);

/*******************************************************************************//**
 * restores LEDs after leaving power save mode
 **********************************************************************************/
void LEDs_Restore(void);

#endif
