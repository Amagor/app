/**
 * @file UART.h
 * UART implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __UART_H__
#define __UART_H__

#include "../../API/UARTAPI.h"

/*******************************************************************************//**
 * inits UART
 * @return SUCCESS if UART successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT UART_Init(void);

/*******************************************************************************//**
 * forces UART to enter power save mode
 **********************************************************************************/
void UART_PowerSave(void);

/*******************************************************************************//**
 * restores UART after leaving power save mode
 **********************************************************************************/
void UART_Restore(void);

#endif
