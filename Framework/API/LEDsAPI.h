/**
 * @file LEDsAPI.h
 * LEDs API.
 * @author Nezametdinov I.E.
 */

#ifndef __LEDS_API_H__
#define __LEDS_API_H__

#include "../PIL/Defs.h"

/*******************************************************************************//**
 * opens LEDs
 * @return SUCCESS if LEDs successfully opened
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT LEDs_Open(void);

/*******************************************************************************//**
 * closes LEDs
 * @return SUCCESS if LEDs successfully closed
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT LEDs_Close(void);

/*******************************************************************************//**
 * switches the LEDs on
 * @param[in] LEDs LEDs bit mask
 * @return SUCCESS if LEDs successfully switched on
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT LEDs_SwitchOn(uint8_t LEDs);

/*******************************************************************************//**
 * switches the LEDs off
 * @param[in] LEDs LEDs bit mask
 * @return SUCCESS if LEDs successfully switched off
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT LEDs_SwitchOff(uint8_t LEDs);

/*******************************************************************************//**
 * toggles the LEDs
 * @param[in] LEDs LEDs bit mask
 * @return SUCCESS if LEDs successfully toggLED
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT LEDs_Toggle(uint8_t LEDs);

/*******************************************************************************//**
 * returns state of the LEDs
 * @return bit mask of glowing LEDs
 **********************************************************************************/
uint8_t LEDs_GetGlowing(void);

/*******************************************************************************//**
 * returns state of the LEDs
 * @return bit mask of not glowing LEDs
 **********************************************************************************/
uint8_t LEDs_GetNotGlowing(void);

/*******************************************************************************//**
 * this is an example of how to use LEDs API
 * @example Blink/app.c
 **********************************************************************************/

#endif
