/**
 * @file PlatformCC2420.c
 * CC2420 platform implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../DRIVERS/CC2420/CC2420.h"
#include <avr/interrupt.h>
#include <avr/io.h>

#define CC2420_VREN_DDR    DDRF
#define CC2420_VREN_PORT   PORTF
#define CC2420_VREN        0

#define CC2420_FIFO_PIN  PINE
#define CC2420_FIFO_DDR  DDRE
#define CC2420_FIFO      5

#define CC2420_FIFOP_PIN  PINE
#define CC2420_FIFOP_DDR  DDRE
#define CC2420_FIFOP      6

#define CC2420_SFD_PORT  PORTE
#define CC2420_SFD_PIN   PINE
#define CC2420_SFD_DDR   DDRE
#define CC2420_SFD       7

#define CC2420_CCA_PIN  PINB
#define CC2420_CCA_DDR  DDRB
#define CC2420_CCA      4

/// SFD interrupt handler
ISR(SIG_INTERRUPT7)
{
	CC2420_SFDReceived();
	
}

/// FIFOP interrupt handler
ISR(SIG_INTERRUPT6)
{
	CC2420_FIFOPReceived();
	
}

/*******************************************************************************//**
 * @implements CC2420_Init
 **********************************************************************************/
RESULT CC2420_Init(void)
{
	CC2420_FIFO_DDR  &= ~(1<<CC2420_FIFO);
	CC2420_FIFOP_DDR &= ~(1<<CC2420_FIFOP);
	CC2420_SFD_DDR   &= ~(1<<CC2420_SFD);
	CC2420_CCA_DDR   &= ~(1<<CC2420_CCA);
	CC2420_VREN_DDR  |=  (1<<CC2420_VREN);
	CC2420_VREN_PORT &= ~(1<<CC2420_VREN);
	
	EICRB |= (1<<ISC71)|(1<<ISC70)|(1<<ISC61)|(1<<ISC60);
	EIMSK |= (1<<INT7)|(1<<INT6);
	
	return SUCCESS;
}

#ifdef USE_PWR
/*******************************************************************************//**
 * @implements CC2420_PowerSave
 **********************************************************************************/
inline void CC2420_PowerSave(void)
{
	EIMSK &= ~((1<<INT7)|(1<<INT6));
	
}

/*******************************************************************************//**
 * @implements CC2420_Restore
 **********************************************************************************/
inline void CC2420_Restore(void)
{
	EIMSK |= (1<<INT7)|(1<<INT6);
	
}
#endif

/*******************************************************************************//**
 * @implements CC2420_VRegSwitchOn
 **********************************************************************************/
inline void CC2420_VRegSwitchOn(void)
{
	CC2420_VREN_PORT |=  (1<<CC2420_VREN);
}

/*******************************************************************************//**
 * @implements CC2420_VRegSwitchOff
 **********************************************************************************/
inline void CC2420_VRegSwitchOff(void)
{
	CC2420_VREN_PORT &= ~(1<<CC2420_VREN);
}

/*******************************************************************************//**
 * @implements CC2420_RxFIFOOverflow
 **********************************************************************************/
inline BOOL CC2420_GetRxFIFOOverflow(void)
{
	if(!(CC2420_FIFO_PIN&(1<<CC2420_FIFO))&&(CC2420_FIFOP_PIN&(1<<CC2420_FIFOP)))
		return TRUE;
	return FALSE;
}

/*******************************************************************************//**
 * @implements CC2420_GetFIFO
 **********************************************************************************/
inline uint8_t CC2420_GetFIFO(void)
{
	return (CC2420_FIFO_PIN&(1<<CC2420_FIFO));
}

/*******************************************************************************//**
 * @implements CC2420_GetCCA
 **********************************************************************************/
inline uint8_t CC2420_GetCCA(void)
{
	return (CC2420_CCA_PIN&(1<<CC2420_CCA));
}

/*******************************************************************************//**
 * @implements CC2420_GetSFD
 **********************************************************************************/
inline uint8_t CC2420_GetSFD(void)
{
	return (CC2420_SFD_PIN&(1<<CC2420_SFD));
}
