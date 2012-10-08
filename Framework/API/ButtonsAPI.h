/**
 * @file ButtonsAPI.h
 * Buttons API.
 * @author Nezametdinov I.E.
 */

#ifndef __BUTTONS_API_H__
#define __BUTTONS_API_H__

#include "../PIL/Defs.h"

/*******************************************************************************//**
 * opens buttons
 * @param[in] Pressed  buttons "pressed" event handler
 * @param[in] Released buttons "released" event handler
 * @return SUCCESS if buttons successfully opened
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Buttons_Open(EVENT (*Pressed)(uint8_t Buttons),
                    EVENT (*Released)(uint8_t Buttons));

/*******************************************************************************//**
 * closes buttons
 * @return SUCCESS if buttons successfully closed
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Buttons_Close(void);

/*******************************************************************************//**
 * returns state of the buttons
 * @return bit mask of pressed buttons
 **********************************************************************************/
uint8_t Buttons_Get(void);

#endif
