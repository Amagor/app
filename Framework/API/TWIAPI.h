/**
 * @file TWIAPI.h
 * TWI API.
 * @author Nezametdinov I.E.
 */

#ifndef __TWI_API_H__
#define __TWI_API_H__

#include "../PIL/Defs.h"

enum
{
	/// TWI mode selectin bit (for Sensirion SHT11)
	TWI_UNIQUE_SHT = 1
};

/// TWI handle
typedef uint8_t HTWI;

/*******************************************************************************//**
 * openes TWI
 * @param[in] Channel TWI channel
 * @param[in] Params  TWI parameters
 * @return valid TWI handle if TWI successfully opened
 * @return INVALID_HANDLE   otherwise
 **********************************************************************************/
HTWI TWI_Open(uint8_t Channel,uint8_t Params);

/*******************************************************************************//**
 * closes TWI
 * @param[in] TWI TWI handle
 * @return SUCCESS if TWI successfully closed
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT TWI_Close(HTWI TWI);

/*******************************************************************************//**
 * starts TWI transmission
 * @param[in] TWI TWI handle
 * @return SUCCESS if TWI transmission successfully started
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT TWI_Start(HTWI TWI);

/*******************************************************************************//**
 * stops TWI transmission
 * @param[in] TWI TWI handle
 * @return SUCCESS if TWI transmission successfully stopped
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT TWI_Stop(HTWI TWI);

/*******************************************************************************//**
 * sends byte via TWI
 * @param[in] TWI  TWI handle
 * @param[in] Byte byte to send
 * @return SUCCESS if byte successfully transmitted
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT TWI_Tx(HTWI TWI,uint8_t Byte);

/*******************************************************************************//**
 * receives byte via TWI
 * @param[in]  TWI  TWI handle
 * @param[out] Byte byte to receive
 * @return SUCCESS if byte successfully received
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT TWI_Rx(HTWI TWI,uint8_t *Byte);

#endif
