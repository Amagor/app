/**
 * @file SchedulerAPI.h
 * Scheduler API.
 * @author Nezametdinov I.E.
 */

#ifndef __SCHEDULER_API_H__
#define __SCHEDULER_API_H__

#include "../PIL/Defs.h"

/// thread mode
enum
{
	/// task
	THREAD_TASK_MODE    = 0x00,
	/// process
	THREAD_PROCESS_MODE = 0x01
};

/// thread handle
typedef uint8_t HThread;

/*******************************************************************************//**
 * creates a new thread
 * @param[in] Proc  thread proc
 * @param[in] Param thread parameter
 * @return valid thread handle if thread successfully created
 * @return INVALID_HANDLE      otherwise
 **********************************************************************************/
HThread Thread_Create(PROC (*Proc)(PARAM Param),PARAM Param);

/*******************************************************************************//**
 * destroyes the thread
 * @param[in] Thread thread handle
 * @return SUCCESS if thread successfully destroyed
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Thread_Destroy(HThread Thread);

/*******************************************************************************//**
 * starts the thread
 * @param[in] Thread thread handle
 * @param[in] Params thread parameters
 * @return SUCCESS if thread successfully started
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Thread_Start(HThread Thread,uint8_t Params);

/*******************************************************************************//**
 * stops the thread
 * @param[in] Thread thread handle
 * @return SUCCESS if thread successfully stopped
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Thread_Stop(HThread Thread);

/*******************************************************************************//**
 * returns thread parameter
 * @param[in] Thread thread handle
 * @return thread parameter if thread parameter successfully returned
 * @return NULL             otherwise
 **********************************************************************************/
PARAM Thread_GetParam(HThread Thread);

/*******************************************************************************//**
 * sets thread parameter
 * @param[in] Thread thread handle
 * @param[in] Param  thread parameter
 * @return SUCCESS if thread parameter successfully set
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Thread_SetParam(HThread Thread,PARAM Param);

/*******************************************************************************//**
 * returns state of the thread
 * @param[in] Thread thread handle
 * @return TRUE  if thread is active
 * @return FALSE otherwise
 **********************************************************************************/
BOOL Thread_IsActive(HThread Thread);

/*******************************************************************************//**
 * checks thread existance
 * @param[in] Thread thread handle
 * @return TRUE  if thread exists
 * @return FALSE otherwise
 **********************************************************************************/
BOOL Thread_Exists(HThread Thread);

/*******************************************************************************//**
 * returns the handle of currently running thread
 * @return handle of the thread wich is active right now
 **********************************************************************************/
HThread Thread_GetHandle(void);

/*******************************************************************************//**
 * this is an example of how to use scheduler API
 * @example BlinkWithThreads/app.c
 **********************************************************************************/

#endif
