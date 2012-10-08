/**
 * @file TWI.h
 * TWI implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __TWI_H__
#define __TWI_H__

#include "../../API/TWIAPI.h"

/*******************************************************************************//**
 * inits TWI
 * @return SUCCESS if TWI successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT TWI_Init(void);

/*******************************************************************************//**
 * forces TWI to enter power save mode
 **********************************************************************************/
void TWI_PowerSave(void);

/*******************************************************************************//**
 * restores TWI after leaving power save mode
 **********************************************************************************/
void TWI_Restore(void);

#endif
