/**
 * @file PlatformButtons.c
 * Buttons implementation source file.
 * @author Nezametdinov I.E.
 */
 
#include "../../PIL/Scheduler/Scheduler.h"
#include "../../PIL/Buttons/Buttons.h"
#include "../../API/ButtonsAPI.h"
#include "../../API/CommonAPI.h"
#include "../../PIL/Guard.h"
#include <avr/io.h>

#define PIN_BUTTONS PINB
#define DDR_BUTTONS DDRB

#define BTN1 (1<<5)
#define BTN2 (1<<6)

#define BUTTONS_ARE_SYSTEM_BUTTONS    ((ButtonsDefs.Status)&1)
#define BUTTONS_SET_SYS_ACCESS_RIGHTS {ButtonsDefs.Status |=  1;}
#define BUTTONS_SET_APP_ACCESS_RIGHTS {ButtonsDefs.Status &= ~1;}

#define BUTTONS_ARE_OPENED ((ButtonsDefs.Status)&2)
#define BUTTONS_CLOSE  {ButtonsDefs.Status &= ~2;}
#define BUTTONS_OPEN   {ButtonsDefs.Status |=  2;}

// structure defines buttons
typedef struct
{
	/// thread handle
	HThread Thread;
	
	/// status of buttons
	uint8_t Status;
	
	/// saved state of buttons
	uint8_t State;
	
	/// buttons "pressed" event handler
	EVENT (*Pressed)(uint8_t Buttons);
	
	/// buttons "released" event handler
	EVENT (*Released)(uint8_t Buttons);
	
}ButtonsDefsStruct;
static volatile ButtonsDefsStruct ButtonsDefs;

/*******************************************************************************//**
 * buttons thread proc
 **********************************************************************************/
PROC Buttons_ThreadProc(PARAM Param)
{
	// check buttons activity
	if(!BUTTONS_ARE_OPENED)
		return;
	
	if(ButtonsDefs.Pressed==NULL&&ButtonsDefs.Released==NULL)
		return;
	
	// get states of buttons
	uint8_t Released = 0;
	uint8_t Pressed  = 0;
	uint8_t State = Buttons_Get();
	uint8_t i;
	
	// check the buttons
	for(i=1;i<=2;++i)
	{
		if((!(ButtonsDefs.State&i))&&(State&i))
			Pressed  |= i;
		else if((ButtonsDefs.State&i)&&(!(State&i)))
			Released |= i;
	}
	ButtonsDefs.State = State;
	
	// save current guard state
	SAVE_GUARD_STATE
	
	// if buttons are not a system buttons, then
	// guard should watch for them
	if(!BUTTONS_ARE_SYSTEM_BUTTONS)
		Guard_Watch();
	else
		Guard_Idle();
	
	// signal events if needed
	if(Pressed!=0&&ButtonsDefs.Pressed!=NULL)
		SIGNAL_EVENT(ButtonsDefs.Pressed(Pressed));
	
	if(Released!=0&&ButtonsDefs.Released!=NULL)
		SIGNAL_EVENT(ButtonsDefs.Released(Released));
	
	// restore previous guard state
	RESTORE_GUARD_STATE
}

/*******************************************************************************//**
 * @implements Buttons_Init
 **********************************************************************************/
RESULT Buttons_Init(void)
{
	ButtonsDefs.Thread = INVALID_HANDLE;
	ButtonsDefs.Status = 0;
	ButtonsDefs.State  = 0;
	ButtonsDefs.Pressed  = NULL;
	ButtonsDefs.Released = NULL;
	
	// create thread
	ButtonsDefs.Thread = Thread_Create(Buttons_ThreadProc,NULL);
	if(IS_INVALID_HANDLE(ButtonsDefs.Thread))
		return FAIL;
	
	// start thread
	if(Thread_Start(ButtonsDefs.Thread,THREAD_PROCESS_MODE)==FAIL)
		return FAIL;
	
	return SUCCESS;
}

#ifdef USE_PWR
/*******************************************************************************//**
 * @implements Buttons_PowerSave
 **********************************************************************************/
void Buttons_PowerSave(void)
{
	// empty
}

/*******************************************************************************//**
 * @implements Buttons_Restore
 **********************************************************************************/
void Buttons_Restore(void)
{
	// empty
}
#endif

/*******************************************************************************//**
 * @implements Buttons_Open
 **********************************************************************************/
RESULT Buttons_Open(EVENT (*Pressed)(uint8_t Buttons),
                    EVENT (*Released)(uint8_t Buttons))
{
	// check if buttons are already opened
	if(BUTTONS_ARE_OPENED)
		return FAIL;
	
	// open buttons
	DDR_BUTTONS &= ~(BTN1|BTN2);
	BUTTONS_OPEN
	
	ButtonsDefs.Pressed  = Pressed;
	ButtonsDefs.Released = Released;
	
	// set buttons access rights
	if(!Guard_IsWatching())
		BUTTONS_SET_SYS_ACCESS_RIGHTS
	else
		BUTTONS_SET_APP_ACCESS_RIGHTS
	
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Buttons_Close
 **********************************************************************************/
RESULT Buttons_Close(void)
{
	// if buttons are system buttons and guard is watching for a threat
	// then return failure
	if(BUTTONS_ARE_SYSTEM_BUTTONS&&Guard_IsWatching())
		return FAIL;
	
	// close buttons
	BUTTONS_CLOSE
	
	ButtonsDefs.Pressed  = NULL;
	ButtonsDefs.Released = NULL;
	
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Buttons_Get
 **********************************************************************************/
uint8_t Buttons_Get(void)
{
	if(!BUTTONS_ARE_OPENED)
		return 0;
	return ((~(PIN_BUTTONS>>5)) & 0x03);
}
