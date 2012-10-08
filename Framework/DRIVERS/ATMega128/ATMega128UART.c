/**
 * @file ATMega128UART.c
 * ATMega128 UART implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/UART/UART.h"
#include "../../API/UARTAPI.h"
#include "../../PIL/Guard.h"
#include <avr/interrupt.h>

#define UART_IS_SYSTEM_UART(UART)        ((UART.UARTState)&4)
#define UART_IS_IN_ASYNC_MODE(UART)      ((UART.UARTState)&2)
#define UART_IS_ACTIVE(UART)             ((UART.UARTState)&1)
#define UART_ACTIVATE(UART)              {UART.UARTState |= 1;}
#define UART_DEACTIVATE(UART)            {UART.UARTState &= ~1;}
#define UART_SET_ASYNC_MODE(UART)        {UART.UARTState |= 2;}
#define UART_SET_SYNC_MODE(UART)         {UART.UARTState &= ~2;}
#define UART_SET_SYS_ACCESS_RIGHTS(UART) {UART.UARTState |= 4;}
#define UART_SET_APP_ACCESS_RIGHTS(UART) {UART.UARTState &= ~4;}

/// structure defines UART
typedef struct
{
	/// tx buffer
	uint8_t *TxData;
	
	/// tx length
	uint8_t TxLen;
	
	/// current tx byte index
	uint8_t TxCur;
	
	/// UART "byte received" event handler
	EVENT (*RxDone)(uint8_t Byte);
	
	/// UART "data transmitted" event handler
	EVENT (*TxDone)(void);
	
	/// UART state
	uint8_t UARTState;
}UARTDefsStruct;
static volatile UARTDefsStruct UARTsDefs[2];

/// UART0 tx interrupt handler
ISR(SIG_USART0_TRANS)
{
	if(UARTsDefs[0].TxCur<UARTsDefs[0].TxLen)
	{
		UDR0 = UARTsDefs[0].TxData[UARTsDefs[0].TxCur];
		++UARTsDefs[0].TxCur;
	}
	else
	{
		if(UARTsDefs[0].TxDone!=NULL)
		{
			// save current guard state
			SAVE_GUARD_STATE
			
			// if current UART is a not a system UART, then
			// guard should watch for it
			if(!UART_IS_SYSTEM_UART(UARTsDefs[0]))
				Guard_Watch();
			else
				Guard_Idle();
			
			// signal UART "data transmitted" event
			UARTsDefs[0].TxDone();
			
			// restore previous guard state
			RESTORE_GUARD_STATE
			
		}
		UARTsDefs[0].TxCur = 0;
		
	}
	
}

/// UART1 tx interrupt handler
ISR(SIG_USART1_TRANS)
{
	if(UARTsDefs[1].TxCur<UARTsDefs[1].TxLen)
	{
		UDR1 = UARTsDefs[1].TxData[UARTsDefs[1].TxCur];
		++UARTsDefs[1].TxCur;
	}
	else
	{
		if(UARTsDefs[1].TxDone!=NULL)
		{
			// save current guard state
			SAVE_GUARD_STATE
			
			// if current UART is a not a system UART, then
			// guard should watch for it
			if(!UART_IS_SYSTEM_UART(UARTsDefs[1]))
				Guard_Watch();
			else
				Guard_Idle();
			
			// signal UART "data transmitted" event
			UARTsDefs[1].TxDone();
			
			// restore previous guard state
			RESTORE_GUARD_STATE
			
		}
		UARTsDefs[1].TxCur = 0;
		
	}
	
}

/// UART0 rx interrupt handler
ISR(SIG_USART0_RECV)
{
	uint8_t Byte = UDR0;
	if(UARTsDefs[0].RxDone!=NULL)
	{
		// save current guard state
		SAVE_GUARD_STATE
		
		// if current UART is a not a system UART, then
		// guard should watch for it
		if(!UART_IS_SYSTEM_UART(UARTsDefs[0]))
			Guard_Watch();
		else
			Guard_Idle();
		
		// signal UART "byte received" event
		UARTsDefs[0].RxDone(Byte);
		
		// restore previous guard state
		RESTORE_GUARD_STATE
		
	}
	
}

/// UART1 rx interrupt handler
ISR(SIG_USART1_RECV)
{
	uint8_t Byte = UDR1;
	if(UARTsDefs[1].RxDone!=NULL)
	{
		// save current guard state
		SAVE_GUARD_STATE
		
		// if current UART is a not a system UART, then
		// guard should watch for it
		if(!UART_IS_SYSTEM_UART(UARTsDefs[1]))
			Guard_Watch();
		else
			Guard_Idle();
		
		// signal UART "byte received" event
		UARTsDefs[1].RxDone(Byte);
		
		// restore previous guard state
		RESTORE_GUARD_STATE
		
	}
	
}

/*******************************************************************************//**
 * @implements UART_Init
 **********************************************************************************/
RESULT UART_Init(void)
{
	uint8_t i;
	
	// stop UARTs
	UCSR0B &= ~((1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<TXCIE1)|(1<<UDRIE1));
	UCSR1B &= ~((1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<TXCIE1)|(1<<UDRIE1));
	
	// init UARTs
	for(i=0;i<2;++i)
	{
		UARTsDefs[i].TxData = NULL;
		UARTsDefs[i].TxLen  = 0;
		UARTsDefs[i].TxCur  = 0;
		UARTsDefs[i].RxDone = NULL;
		UARTsDefs[i].TxDone = NULL;
		UARTsDefs[i].UARTState = 0;
	}
	
	return SUCCESS;
}

#ifdef USE_PWR
/// saved UART state
static volatile uint8_t SavedState[2];

/*******************************************************************************//**
 * @implements UART_PowerSave
 **********************************************************************************/
void UART_PowerSave(void)
{
	// save UARTs state
	SavedState[0] = UCSR0B;
	SavedState[1] = UCSR1B;
	
	// stop UARTs
	UCSR0B &= ~((1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<TXCIE1)|(1<<UDRIE1));
	UCSR1B &= ~((1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<TXCIE1)|(1<<UDRIE1));
	
}

/*******************************************************************************//**
 * @implements UART_Restore
 **********************************************************************************/
void UART_Restore(void)
{
	// restore UARTs state
	UCSR0B = SavedState[0];
	UCSR1B = SavedState[1];
	
}
#endif

/*******************************************************************************//**
 * @implements UART_Open
 **********************************************************************************/
HUART UART_Open(uint8_t Channel,UART_BAUDRATE Baudrate,uint8_t Params,
                EVENT (*RxDone)(uint8_t Byte),EVENT (*TxDone)(void))
{
	uint8_t *CtrlRegC,*CtrlRegB;
	uint8_t *BaudrateRegL,*BaudrateRegH;
	// check channel
	if(Channel>1)
		return INVALID_HANDLE;
	
	// check state
	if(UART_IS_ACTIVE(UARTsDefs[Channel]))
		return INVALID_HANDLE;
	
	// select control regs depending on UART channel
	switch(Channel)
	{
		case 0:
			BaudrateRegL = (uint8_t*)&UBRR0L;
			BaudrateRegH = (uint8_t*)&UBRR0H;
			CtrlRegB = (uint8_t*)&UCSR0B;
			CtrlRegC = (uint8_t*)&UCSR0C;
			break;
		case 1:
			BaudrateRegL = (uint8_t*)&UBRR1L;
			BaudrateRegH = (uint8_t*)&UBRR1H;
			CtrlRegB = (uint8_t*)&UCSR1B;
			CtrlRegC = (uint8_t*)&UCSR1C;
			break;
		default:
			return INVALID_HANDLE;
	}
	
	// stop UART
	(*CtrlRegB) &= ~((1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<TXCIE1)|(1<<UDRIE1));
	
	// configure stop bits
	if(Params&UART_STOP_BITS_2)
		(*CtrlRegC) |=  (1<<USBS1);
	else
		(*CtrlRegC) &= ~(1<<USBS1);
	
	// configure parity
	switch(Params&0x0C)
	{
		case UART_PARITY_NONE:
			(*CtrlRegC) &= ~((1<<UPM11)|(1<<UPM10));
			break;
		case UART_PARITY_EVEN:
			(*CtrlRegC) |=  (1<<UPM11);
			(*CtrlRegC) &= ~(1<<UPM10);
			break;
		case UART_PARITY_ODD:
			(*CtrlRegC) |=  ((1<<UPM11)|(1<<UPM10));
			break;
		default:
			return INVALID_HANDLE;
	}
	
	// configure data length
	switch(Params&0x03)
	{
		case UART_DATA_LENGTH_5:
			(*CtrlRegB) &= ~(1<<UCSZ12);
			(*CtrlRegC) &= ~((1<<UCSZ11)|(1<<UCSZ10));
			break;
		case UART_DATA_LENGTH_6:
			(*CtrlRegB) &= ~(1<<UCSZ12);
			(*CtrlRegC) |=  (1<<UCSZ10);
			(*CtrlRegC) &= ~(1<<UCSZ11);
			break;
		case UART_DATA_LENGTH_7:
			(*CtrlRegB) &= ~(1<<UCSZ12);
			(*CtrlRegC) |=  (1<<UCSZ11);
			(*CtrlRegC) &= ~(1<<UCSZ10);
			break;
		case UART_DATA_LENGTH_8:
			(*CtrlRegB) &= ~(1<<UCSZ12);
			(*CtrlRegC) |=  ((1<<UCSZ11)|(1<<UCSZ10));
			break;
		default:
			return INVALID_HANDLE;
	}
	
	// configure baudrate
	switch(Baudrate)
	{
		case UART_BAUDRATE_9600:
			(*BaudrateRegL) = 51;
			(*BaudrateRegH) = 0;
			break;
		case UART_BAUDRATE_38400:
			(*BaudrateRegL) = 12;
			(*BaudrateRegH) = 0;
			break;
		case UART_BAUDRATE_115200:
			(*BaudrateRegL) = 3;
			(*BaudrateRegH) = 0;
			break;
		default:
			return INVALID_HANDLE;
	}
	
	// set UART access rights
	if(!Guard_IsWatching())
		UART_SET_SYS_ACCESS_RIGHTS(UARTsDefs[Channel])
	else
		UART_SET_APP_ACCESS_RIGHTS(UARTsDefs[Channel])
	
	// set UART transmission mode
	if(Params&UART_TRANSMISSION_MODE_ASYNC)
		UART_SET_ASYNC_MODE(UARTsDefs[Channel])
	else
		UART_SET_SYNC_MODE(UARTsDefs[Channel])
	
	// start UART
	UARTsDefs[Channel].RxDone = RxDone;
	UARTsDefs[Channel].TxDone = TxDone;
	UART_ACTIVATE(UARTsDefs[Channel])
	(*CtrlRegB) |=  (1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<TXCIE1);
	(*CtrlRegB) &= ~(1<<UDRIE1);
	
	// return UART handle
	return Channel;
}

/*******************************************************************************//**
 * @implements UART_Close
 **********************************************************************************/
RESULT UART_Close(HUART UART)
{
	uint8_t *CtrlRegB;
	
	// check UART handle
	if(UART>1)
		return FAIL;
	
	// check state
	if(!UART_IS_ACTIVE(UARTsDefs[UART]))
		return FAIL;
	
	// if UART is a system UART and guard is watching for a threat
	// then return failure
	if(UART_IS_SYSTEM_UART(UARTsDefs[UART])&&Guard_IsWatching())
		return FAIL;
	
	// select control regs depending on UART channel
	switch(UART)
	{
		case 0:
			CtrlRegB = (uint8_t*)&UCSR0B;
			break;
		case 1:
			CtrlRegB = (uint8_t*)&UCSR1B;
			break;
		default:
			return FAIL;
	}
	
	// stop UART
	(*CtrlRegB) &= ~((1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1)|(1<<TXCIE1)|(1<<UDRIE1));
	UART_DEACTIVATE(UARTsDefs[UART])
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements UART_Tx
 **********************************************************************************/
RESULT UART_Tx(HUART UART,uint8_t Length,uint8_t *Data)
{
	uint8_t i;
	
	// check UART handle
	if(UART>1)
		return FAIL;
	
	// check state
	if(!UART_IS_ACTIVE(UARTsDefs[UART]))
		return FAIL;
	
	// if UART is a system UART and guard is watching for a threat
	// then return failure
	if(UART_IS_SYSTEM_UART(UARTsDefs[UART])&&Guard_IsWatching())
		return FAIL;
	
	// check UART activity
	if(UARTsDefs[UART].TxCur>0)
		return FAIL;
	
	// check data
	if(Data==NULL||Length==0)
		return FAIL;
	
	// if mode is async, then enable UART interrupt and send byte
	if(UART_IS_IN_ASYNC_MODE(UARTsDefs[UART]))
	{
		// prepare transmission
		UARTsDefs[UART].TxData = Data;
		UARTsDefs[UART].TxLen  = Length;
		UARTsDefs[UART].TxCur  = 1;
		
		// send data
		switch(UART)
		{
			// UART 0
			case 0:
				UCSR0B |= (1<<TXCIE1);
				UDR0 = UARTsDefs[UART].TxData[0];
				break;
			
			// UART 1
			case 1:
				UCSR1B |= (1<<TXCIE1);
				UDR1 = UARTsDefs[UART].TxData[0];
				break;
			
			// illegal UART
			default:
				// error happened
				UARTsDefs[UART].TxCur = 0;
				
				// return failure
				return FAIL;
		}
		
	}
	// else send
	else
	{
		// prepare transmission
		UARTsDefs[UART].TxCur = 1;
		
		// send data
		switch(UART)
		{
			// UART 0
			case 0:
				UCSR0B &= ~((1<<TXCIE1)|(1<<UDRIE1));
				for(i=0;i<Length;++i)
				{
					// wait while UDR is not empty
					while(!(UCSR0A&(1<<UDRE)));
					// send next byte
					UDR0 = Data[i];
				}
				break;
			
			// UART 1
			case 1:
				UCSR1B &= ~((1<<TXCIE1)|(1<<UDRIE1));
				for(i=0;i<Length;++i)
				{
					// wait while UDR is not empty
					while(!(UCSR1A&(1<<UDRE)));
					// send next byte
					UDR1 = Data[i];
				}
				break;
			
			// illegal UART
			default:
				// error happened
				UARTsDefs[UART].TxCur = 0;
				
				// return failure
				return FAIL;
		}
		
		// UART transmission ended
		UARTsDefs[UART].TxCur = 0;
		
	}
	
	// return success
	return SUCCESS;
}
