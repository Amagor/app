/**
 * @file DS2401.h
 * DS2401 implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __DS2401_H__
#define __DS2401_H__

#include "../../PIL/Defs.h"

/*******************************************************************************//**
 * inits DS2401
 * @return SUCCESS if DS2401 successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT DS2401_Init(void);

/*******************************************************************************//**
 * reads DS2401 ROM
 * @param[out] ROM DS2401 ROM
 * @return SUCCESS if DS2401 ROM has been successfully read
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT DS2401_ReadROM(uint64_t *ROM);

#endif
