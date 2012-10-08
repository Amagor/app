/**
 * @file PlatformLEDs.c
 * LEDs implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../API/LEDsAPI.h"
#include "../../PIL/LEDs/LEDs.h"
#include <avr/io.h>

#define PORT_LEDS PORTE
#define DDR_LEDS  DDRE
#define PIN_LEDS  PINE

/// status of LEDs
static volatile BOOL Opened = FALSE;

/*******************************************************************************//**
 * @implements LEDs_Init
 **********************************************************************************/
RESULT LEDs_Init(void)
{
	DDR_LEDS &= ~0x0E;
	Opened = FALSE;
	return SUCCESS;
}

#ifdef USE_PWR

/// LEDs state
static volatile uint8_t SavedState = 0;

/*******************************************************************************//**
 * @implements LEDs_PowerSave
 **********************************************************************************/
void LEDs_PowerSave(void)
{
	SavedState = PIN_LEDS;
	DDR_LEDS &= ~0x0E;
	
}

/*******************************************************************************//**
 * @implements LEDs_Restore
 **********************************************************************************/
void LEDs_Restore(void)
{
	if(Opened)
	{
		DDR_LEDS  |= 0x0E;
		PORT_LEDS |= SavedState & 0x0E;
		PORT_LEDS &= SavedState | 0xF1;
		
	}
	
}
#endif

/*******************************************************************************//**
 * @implements LEDs_Open
 **********************************************************************************/
RESULT LEDs_Open(void)
{
	DDR_LEDS  |= 0x0E;
	PORT_LEDS |= 0x0E;
	Opened = TRUE;
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements LEDs_Close
 **********************************************************************************/
RESULT LEDs_Close(void)
{
	Opened = FALSE;
	DDR_LEDS &= ~0x0E;
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements LEDs_SwitchOn
 **********************************************************************************/
RESULT LEDs_SwitchOn(uint8_t LEDs)
{
	if(Opened==FALSE)
		return FAIL;
	PORT_LEDS &= (~(LEDs<<1)) | 0xF1;
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements LEDs_SwitchOff
 **********************************************************************************/
RESULT LEDs_SwitchOff(uint8_t LEDs)
{
	if(Opened==FALSE)
		return FAIL;
	PORT_LEDS |= (LEDs<<1) & 0x0E;
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements LEDs_Toggle
 **********************************************************************************/
RESULT LEDs_Toggle(uint8_t LEDs)
{
	if(Opened==FALSE)
		return FAIL;
	PORT_LEDS ^= (LEDs<<1) & 0x0E;
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements LEDs_GetGlowing
 **********************************************************************************/
uint8_t LEDs_GetGlowing(void)
{
	if(Opened==FALSE)
		return 0;
	return ((~(PIN_LEDS>>1)) & 0x07);
}

/*******************************************************************************//**
 * @implements LEDs_GetNotGlowing
 **********************************************************************************/
uint8_t LEDs_GetNotGlowing(void)
{
	if(Opened==FALSE)
		return 0xFF;
	return ((PIN_LEDS>>1) | 0xF8);
}
