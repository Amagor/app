/**
 * @file UARTAPI.h
 * UART API.
 * @author Nezametdinov I.E.
 */

#ifndef __UART_API_H__
#define __UART_API_H__

#include "../PIL/Defs.h"

/// UART baudrate
typedef enum
{
	/// 9600 bps
	UART_BAUDRATE_9600,
	/// 38400 bps
	UART_BAUDRATE_38400,
	/// 115200 bps
	UART_BAUDRATE_115200
}UART_BAUDRATE;

/// UART data length
enum
{
	/// 5 bits
	UART_DATA_LENGTH_5 = 0x00,
	/// 6 bits
	UART_DATA_LENGTH_6 = 0x01,
	/// 7 bits
	UART_DATA_LENGTH_7 = 0x02,
	/// 8 bits
	UART_DATA_LENGTH_8 = 0x03
};

/// UART parity
enum
{
	/// none
	UART_PARITY_NONE = 0x00,
	/// even
	UART_PARITY_EVEN = 0x04,
	/// odd
	UART_PARITY_ODD  = 0x08
};

/// UART stop bits
enum
{
	/// 1 stop bit
	UART_STOP_BITS_1 = 0x00,
	/// 2 stop bits
	UART_STOP_BITS_2 = 0x10
};

/// UART transmission mode
enum
{
	/// sync
	UART_TRANSMISSION_MODE_SYNC  = 0x00,
	/// async
	UART_TRANSMISSION_MODE_ASYNC = 0x20
};

/// UART handle
typedef uint8_t HUART;

/*******************************************************************************//**
 * opens UART
 * @param[in] Channel  UART channel
 * @param[in] Baudrate UART baudrate
 * @param[in] Params   UART parameters
 * @param[in] RxDone   UART "byte received" event handler
 * @param[in] TxDone   UART "data transmitted" event handler
 * @return valid UART handle if UART successfully opened
 * @return INVALID_HANDLE    otherwise
 **********************************************************************************/
HUART UART_Open(uint8_t Channel,UART_BAUDRATE Baudrate,uint8_t Params,
                EVENT (*RxDone)(uint8_t Byte),EVENT (*TxDone)(void));

/*******************************************************************************//**
 * closes UART
 * @param[in] UART UART handle
 * @return SUCCESS if UART successfully closed
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT UART_Close(HUART UART);

/*******************************************************************************//**
 * sends data via UART
 * @param[in] UART   UART handle
 * @param[in] Length data length
 * @param[in] Data   data
 * @return SUCCESS if data transmission successfully started
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT UART_Tx(HUART UART,uint8_t Length,uint8_t *Data);

/*******************************************************************************//**
 * this is an example of how to use UART API
 * @example BlinkWithUART/app.c
 **********************************************************************************/

#endif
