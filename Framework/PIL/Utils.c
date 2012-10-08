/**
 * @file Utils.c
 * Utils source file.
 * @author Nezametdinov I.E.
 */

#include "../API/CommonAPI.h"
#include "../PIL/MCU/MCU.h"
#include "../PIL/Utils.h"

/// seed for random value generator
static volatile uint8_t CurrentRandValue = 0;

/// saved global interrupts state
BOOL InterruptsState = FALSE;

/// number of times critical section has been entered
uint8_t NTCSE = 0;

/*******************************************************************************//**
 * @implements BeginCriticalSection
 **********************************************************************************/
void BeginCriticalSection(void)
{
	if(NTCSE==0)
		InterruptsState = MCU_DisableInterrupts();
	++NTCSE;
	
}

/*******************************************************************************//**
 * @implements EndCriticalSection
 **********************************************************************************/
void EndCriticalSection(void)
{
	--NTCSE;
	if(NTCSE==0&&InterruptsState)
		MCU_EnableInterrupts();
	
}

/*******************************************************************************//**
 * @implements Utils_Init
 **********************************************************************************/
RESULT Utils_Init(void)
{
	CurrentRandValue = 0;
	InterruptsState  = FALSE;
	NTCSE = 0;
	
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Utils_Seed
 **********************************************************************************/
void Utils_Seed(uint8_t Seed)
{
	CurrentRandValue = Seed;
}

/*******************************************************************************//**
 * @implements Utils_Rand
 **********************************************************************************/
uint8_t Utils_Rand(uint8_t MaxValue)
{
	// generate random value
	if(MaxValue!=0)
	{
		CurrentRandValue = CurrentRandValue * 87 + 39;
		
		return (CurrentRandValue % MaxValue);
	}
	
	// return 0 if max value is 0
	return 0;
}

/*******************************************************************************//**
 * @implements Utils_ITUTCRC16
 **********************************************************************************/
uint16_t Utils_ITUTCRC16(uint8_t Length,uint8_t *Data)
{
	uint16_t CRC = 0x0000;
	uint8_t i;
	
	while(Length--)
	{
		CRC ^= *Data++;
		for(i=0;i<8;++i)
			CRC = (CRC&0x0001)?(CRC>>1)^0x8408:CRC>>1;
	}
	
	// return CRC
	return CRC;
}

/*******************************************************************************//**
 * @implements Utils_CKSUM
 **********************************************************************************/
uint8_t Utils_CKSUM(uint8_t Length,uint8_t *Data)
{
	uint8_t CheckSum = 0;
	uint8_t i;
	
	for(i=0;i<Length;++i)
		CheckSum ^= Data[i];
	
	return CheckSum;
}
