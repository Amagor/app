/**
 * @file Guard.c
 * OS guard source file.
 * @author Nezametdinov I.E.
 */

#include "../PIL/Guard.h"

static volatile BOOL GuardIsWatching = FALSE;

/*******************************************************************************//**
 * @implements Guard_Init
 **********************************************************************************/
RESULT Guard_Init(void)
{
	GuardIsWatching = FALSE;
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Guard_Watch
 **********************************************************************************/
inline void Guard_Watch(void)
{
	GuardIsWatching = TRUE;
}

/*******************************************************************************//**
 * @implements Guard_Idle
 **********************************************************************************/
inline void Guard_Idle(void)
{
	GuardIsWatching = FALSE;
}

/*******************************************************************************//**
 * @implements Guard_IsWatching
 **********************************************************************************/
inline BOOL Guard_IsWatching(void)
{
	return GuardIsWatching;
}
