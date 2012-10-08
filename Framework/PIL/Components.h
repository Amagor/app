/**
 * @file Components.h
 * OS components init header.
 * @author Nezametdinov I.E.
 */

#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include "../PIL/Defs.h"

/*******************************************************************************//**
 * inits OS components such as timers, scheduler, etc.
 * @return SUCCESS if OS components successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT InitComponents(void);

/*******************************************************************************//**
 * inits other OS components.
 * @return SUCCESS if other components successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT InitOther(void);

#endif
