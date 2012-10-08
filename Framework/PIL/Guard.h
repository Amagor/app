/**
 * @file Guard.h
 * OS guard header.
 * @author Nezametdinov I.E.
 */

#ifndef __GUARD_H__
#define __GUARD_H__

#include "../PIL/Defs.h"

/*******************************************************************************//**
 * inits OS guard
 * @return SUCCESS if OS guard successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Guard_Init(void);

/*******************************************************************************//**
 * makes OS guard watch for possible threat
 **********************************************************************************/
void Guard_Watch(void);

/*******************************************************************************//**
 * makes OS guard idle
 **********************************************************************************/
void Guard_Idle(void);

/*******************************************************************************//**
 * returns OS guard state
 * @return TRUE  if OS guard is watching
 * @return FALSE otherwise
 **********************************************************************************/
BOOL Guard_IsWatching(void);

/*******************************************************************************//**
 * saves current OS guard state
 **********************************************************************************/
#define SAVE_GUARD_STATE \
	BOOL GuardWasWatching = Guard_IsWatching();

/*******************************************************************************//**
 * restores OS guard state
 **********************************************************************************/
#define RESTORE_GUARD_STATE \
	if(GuardWasWatching)\
		Guard_Watch();\
	else\
		Guard_Idle();

#endif
