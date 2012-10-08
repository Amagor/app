/**
 * @file OWI.h
 * OWI implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __OWI_H__
#define __OWI_H__

#include "../../API/TWIAPI.h"

/*******************************************************************************//**
 * inits OWI
 * @return SUCCESS if OWI successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT OWI_Init(void);

/*******************************************************************************//**
 * forces OWI to enter power save mode
 **********************************************************************************/
void OWI_PowerSave(void);

/*******************************************************************************//**
 * restores OWI after leaving power save mode
 **********************************************************************************/
void OWI_Restore(void);

#endif
