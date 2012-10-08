/**
 * @file Scheduler.c
 * Scheduler implementation source file.
 * @author Nezametdinov I.E.
 */

#include "Scheduler.h"
#include "../../API/SchedulerAPI.h"
#include "../../API/CommonAPI.h"
#include "../../PIL/MCU/MCU.h"
#include "../../PIL/Guard.h"

#define THREAD_ACTIVITY      0x01
#define THREAD_ACTIVITY_MODE 0x02
#define THREAD_ACCESS_RIGHTS 0x04

#define THREAD_EXISTS(Thread)                ((Thread.ThreadIndex)<MAX_THREADS)
#define THREAD_IS_SYSTEM_THREAD(Thread)      ((Thread.ThreadState)&THREAD_ACCESS_RIGHTS)
#define THREAD_IS_IN_TASK_MODE(Thread)       ((Thread.ThreadState)&THREAD_ACTIVITY_MODE)
#define THREAD_IS_ACTIVE(Thread)             ((Thread.ThreadState)&THREAD_ACTIVITY)
#define THREAD_ACTIVATE(Thread)              {Thread.ThreadState |= THREAD_ACTIVITY;}
#define THREAD_DEACTIVATE(Thread)            {Thread.ThreadState &= ~THREAD_ACTIVITY;}
#define THREAD_SET_TASK_MODE(Thread)         {Thread.ThreadState |= THREAD_ACTIVITY_MODE;}
#define THREAD_SET_PROCESS_MODE(Thread)      {Thread.ThreadState &= ~THREAD_ACTIVITY_MODE;}
#define THREAD_SET_SYS_ACCESS_RIGHTS(Thread) {Thread.ThreadState |= THREAD_ACCESS_RIGHTS;}
#define THREAD_SET_APP_ACCESS_RIGHTS(Thread) {Thread.ThreadState &= ~THREAD_ACCESS_RIGHTS;}

/// structure defines thread
typedef struct
{
	/// thread proc
	PROC (*Proc)(PARAM Param);
	
	/// thread parameter
	PARAM Param;
	
	/// index in array
	uint8_t ThreadIndex;
	
	/// state of thread
	uint8_t ThreadState;
	
}ThreadDefsStruct;

/// array of threads
static volatile ThreadDefsStruct ThreadsDefs[MAX_THREADS];

/// array of existing and not existing threads
static volatile uint8_t ThreadsArray[MAX_THREADS];

/// index of the first not existing thread in array of threads
static volatile uint8_t CurrentThreadIndex = 0;

/// handle of the thread wich is processed right now
static volatile HThread CurrentThread = INVALID_HANDLE;

/*******************************************************************************//**
 * @implements Scheduler_Init
 **********************************************************************************/
RESULT Scheduler_Init(void)
{
	uint8_t i;
	
	// init scheduler
	for(i=0;i<MAX_THREADS;++i)
	{
		ThreadsDefs[i].Proc        = NULL;
		ThreadsDefs[i].Param       = NULL;
		ThreadsDefs[i].ThreadIndex = MAX_THREADS;
		ThreadsDefs[i].ThreadState = 0;
		ThreadsArray[i] = i;
	}
	
	CurrentThread = INVALID_HANDLE;
	CurrentThreadIndex  = 0;
	
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Scheduler_RunThreads
 **********************************************************************************/
void Scheduler_RunThreads(void)
{
	uint8_t i;
	
	// eternal loop
	while(TRUE)
	{
		// loop through all existing threads
		for(i=0;i<CurrentThreadIndex;++i)
		{
			// this is a system block of code, so guard may idle
			Guard_Idle();
			
			// get thread from threads array
			CurrentThread = ThreadsArray[i];
			
			// if thread is not active or does not exist, then skip it
			if(!THREAD_EXISTS(ThreadsDefs[CurrentThread])||
			   !THREAD_IS_ACTIVE(ThreadsDefs[CurrentThread]))
				continue;
			
			// if thread is in a task mode, then stop it
			if(THREAD_IS_IN_TASK_MODE(ThreadsDefs[CurrentThread]))
				THREAD_DEACTIVATE(ThreadsDefs[CurrentThread])
			
			// if thread proc is not specified, then skip thread
			if(ThreadsDefs[CurrentThread].Proc==NULL)
				continue;
			
			// enable interrupts
			MCU_EnableInterrupts();
			
			// if current thread is a not a system thread, then
			// guard should watch for it
			if(!THREAD_IS_SYSTEM_THREAD(ThreadsDefs[CurrentThread]))
				Guard_Watch();
			
			// process thread proc
			ThreadsDefs[CurrentThread].Proc(ThreadsDefs[CurrentThread].Param);
			
		}
		
	}
	
}

/*******************************************************************************//**
 * @implements Thread_Create
 **********************************************************************************/
HThread Thread_Create(PROC (*Proc)(PARAM Param),PARAM Param)
{
	HThread Thread = INVALID_HANDLE;
	
	// if thread proc is not specified return invalid handle
	if(Proc==NULL)
		return INVALID_HANDLE;
	
	// try to create a thread
	BEGIN_CRITICAL_SECTION
	{
		// if there is free memory
		if(CurrentThreadIndex<MAX_THREADS)
		{
			// get not used thread handle
			Thread = (HThread)ThreadsArray[CurrentThreadIndex];
			
			// set thread proc
			ThreadsDefs[Thread].Proc  = Proc;
			
			// set thread parameter
			ThreadsDefs[Thread].Param = Param;
			
			// set index in array
			ThreadsDefs[Thread].ThreadIndex = CurrentThreadIndex;
			
			// set thread access rights
			if(!Guard_IsWatching())
				THREAD_SET_SYS_ACCESS_RIGHTS(ThreadsDefs[Thread])
			else
				THREAD_SET_APP_ACCESS_RIGHTS(ThreadsDefs[Thread])
			
			// deactivate thread
			THREAD_DEACTIVATE(ThreadsDefs[Thread])
			
			// inc index of the next thread in array of threads
			++CurrentThreadIndex;
			
		}
		
	}
	END_CRITICAL_SECTION
	
	// return thread handle
	return Thread;
}

/*******************************************************************************//**
 * @implements Thread_Destroy
 **********************************************************************************/
RESULT Thread_Destroy(HThread Thread)
{
	// if thread handle is not valid return failure
	if(Thread>=MAX_THREADS)
		return FAIL;
	
	// if thread does not exist, then return failure
	if(!THREAD_EXISTS(ThreadsDefs[Thread]))
		return FAIL;
	
	// if thread is a system thread and guard is watching for a threat
	// then return failure
	if(THREAD_IS_SYSTEM_THREAD(ThreadsDefs[Thread])&&Guard_IsWatching())
		return FAIL;
	
	// try to destroy thread
	BEGIN_CRITICAL_SECTION
	{
		// if thread exists, then destroy it
		if(THREAD_EXISTS(ThreadsDefs[Thread]))
		{
			// dec index of the next thread in array of threads
			--CurrentThreadIndex;
			
			// move the last thread from the array to the place of destroyed thread
			ThreadsDefs[ThreadsArray[CurrentThreadIndex]].ThreadIndex = ThreadsDefs[Thread].ThreadIndex;
			ThreadsArray[ThreadsDefs[Thread].ThreadIndex] = ThreadsArray[CurrentThreadIndex];
			ThreadsArray[CurrentThreadIndex] = Thread;
			
			// mark thread as not existing
			ThreadsDefs[Thread].ThreadIndex = MAX_THREADS;
			
		}
		
	}
	END_CRITICAL_SECTION
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Thread_Start
 **********************************************************************************/
RESULT Thread_Start(HThread Thread,uint8_t Params)
{
	// if thread handle is not valid return failure
	if(Thread>=MAX_THREADS)
		return FAIL;
	
	// if thread does not exist, then return failure
	if(!THREAD_EXISTS(ThreadsDefs[Thread]))
		return FAIL;
	
	// if thread is a system thread and guard is watching for a threat
	// then return failure
	if(THREAD_IS_SYSTEM_THREAD(ThreadsDefs[Thread])&&Guard_IsWatching())
		return FAIL;
	
	// define type of the thread
	if(Params&THREAD_PROCESS_MODE)
		THREAD_SET_PROCESS_MODE(ThreadsDefs[Thread])
	else
		THREAD_SET_TASK_MODE(ThreadsDefs[Thread])
	
	// activate the thread
	THREAD_ACTIVATE(ThreadsDefs[Thread])
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Thread_Stop
 **********************************************************************************/
RESULT Thread_Stop(HThread Thread)
{
	// if thread handle is not valid return failure
	if(Thread>=MAX_THREADS)
		return FAIL;
	
	// if thread is not active or does not exist, then return failure
	if(!THREAD_EXISTS(ThreadsDefs[Thread])||!THREAD_IS_ACTIVE(ThreadsDefs[Thread]))
		return FAIL;
	
	// if thread is a system thread and guard is watching for a threat
	// then return failure
	if(THREAD_IS_SYSTEM_THREAD(ThreadsDefs[Thread])&&Guard_IsWatching())
		return FAIL;
	
	// deactivate thread
	THREAD_DEACTIVATE(ThreadsDefs[Thread])
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Thread_GetParam
 **********************************************************************************/
PARAM Thread_GetParam(HThread Thread)
{
	// if thread handle is not valid return failure
	if(Thread>=MAX_THREADS)
		return NULL;
	
	// if thread does not exist, then return failure
	if(!THREAD_EXISTS(ThreadsDefs[Thread]))
		return NULL;
	
	// if thread is a system thread and guard is watching for a threat
	// then return failure
	if(THREAD_IS_SYSTEM_THREAD(ThreadsDefs[Thread])&&Guard_IsWatching())
		return NULL;
	
	// return thread parameter
	return ThreadsDefs[Thread].Param;
}

/*******************************************************************************//**
 * @implements Thread_SetParam
 **********************************************************************************/
RESULT Thread_SetParam(HThread Thread,PARAM Param)
{
	// if thread handle is not valid return failure
	if(Thread>=MAX_THREADS)
		return FAIL;
	
	// if thread is active or does not exist, then return failure
	if(!THREAD_EXISTS(ThreadsDefs[Thread])||THREAD_IS_ACTIVE(ThreadsDefs[Thread]))
		return FAIL;
	
	// if thread is a system thread and guard is watching for a threat
	// then return failure
	if(THREAD_IS_SYSTEM_THREAD(ThreadsDefs[Thread])&&Guard_IsWatching())
		return FAIL;
	
	// set thread parameter
	ThreadsDefs[Thread].Param = Param;
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Thread_IsActive
 **********************************************************************************/
BOOL Thread_IsActive(HThread Thread)
{
	// if thread handle is not valid return false
	if(Thread>=MAX_THREADS)
		return FALSE;
	
	// if thread does not exist return false
	if(!THREAD_EXISTS(ThreadsDefs[Thread]))
		return FALSE;
	
	// check thread
	if(THREAD_IS_ACTIVE(ThreadsDefs[Thread]))
		return TRUE;
	
	// thread is not active
	return FALSE;
}

/*******************************************************************************//**
 * @implements Thread_Exists
 **********************************************************************************/
BOOL Thread_Exists(HThread Thread)
{
	// if thread handle is not valid return false
	if(Thread>=MAX_THREADS)
		return FALSE;
	
	// if thread does not exist return false
	if(!THREAD_EXISTS(ThreadsDefs[Thread]))
		return FALSE;
	
	// thread exists, return true
	return TRUE;
}

/*******************************************************************************//**
 * @implements Thread_GetHandle
 **********************************************************************************/
HThread Thread_GetHandle(void)
{
	return CurrentThread;
}
