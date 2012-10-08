/**
 * @file Scheduler.h
 * Scheduler implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "../../API/SchedulerAPI.h"

/*******************************************************************************//**
 * inits scheduler
 * @return SUCCESS if scheduler successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Scheduler_Init(void);

/*******************************************************************************//**
 * runs threads
 **********************************************************************************/
void Scheduler_RunThreads(void);

#endif
