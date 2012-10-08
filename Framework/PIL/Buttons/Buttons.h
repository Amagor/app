/**
 * @file Buttons.h
 * Buttons implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include "../../PIL/Defs.h"

/*******************************************************************************//**
 * inits buttons
 * @return SUCCESS if buttons successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Buttons_Init(void);

/*******************************************************************************//**
 * forces buttons to enter power save mode
 **********************************************************************************/
void Buttons_PowerSave(void);

/*******************************************************************************//**
 * restores buttons after leaving power save mode
 **********************************************************************************/
void Buttons_Restore(void);

#endif
