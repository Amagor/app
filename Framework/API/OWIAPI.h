/**
 * @file OWIAPI.h
 * OWI API.
 * @author Nezametdinov I.E.
 */

#ifndef __OWI_API_H__
#define __OWI_API_H__

#include "../PIL/Defs.h"

/// OWI handle
typedef uint8_t HOWI;

/*******************************************************************************//**
 * openes OWI
 * @param[in] Channel OWI channel
 * @return valid OWI handle if OWI successfully opened
 * @return INVALID_HANDLE   otherwise
 **********************************************************************************/
HOWI OWI_Open(uint8_t Channel);

/*******************************************************************************//**
 * closes OWI
 * @param[in] OWI OWI handle
 * @return SUCCESS if OWI successfully closed
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT OWI_Close(HOWI OWI);

/*******************************************************************************//**
 * starts OWI transmission
 * @param[in] OWI OWI handle
 * @return SUCCESS if OWI transmission successfully started
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT OWI_Start(HOWI OWI);

/*******************************************************************************//**
 * stops OWI transmission
 * @param[in] OWI OWI handle
 * @return SUCCESS if OWI transmission successfully stopped
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT OWI_Stop(HOWI OWI);

/*******************************************************************************//**
 * sends byte via OWI
 * @param[in] OWI  OWI handle
 * @param[in] Byte byte to send
 * @return SUCCESS if byte successfully transmitted
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT OWI_Tx(HOWI OWI,uint8_t Byte);

/*******************************************************************************//**
 * receives byte via OWI
 * @param[in]  OWI  OWI handle
 * @param[out] Byte byte to receive
 * @return SUCCESS if byte successfully received
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT OWI_Rx(HOWI OWI,uint8_t *Byte);

#endif
