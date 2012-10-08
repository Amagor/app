/**
 * @file SPIAPI.h
 * SPI API.
 * @author Nezametdinov I.E.
 */

#ifndef __SPI_API_H__
#define __SPI_API_H__

#include "../PIL/Defs.h"

/// SPI mode
enum
{
	/// master
	SPI_MODE_MASTER = 0x00,
	/// slave
	SPI_MODE_SLAVE  = 0x01
};

/// SPI transmission mode
enum
{
	/// sync
	SPI_TRANSMISSION_MODE_SYNC  = 0x00,
	/// async
	SPI_TRANSMISSION_MODE_ASYNC = 0x02
};

/// SPI handle
typedef uint8_t HSPI;

/*******************************************************************************//**
 * opens SPI
 * @param[in] Channel  SPI channel
 * @param[in] Params   SPI parameters
 * @param[in] TxRxDone SPI "byte transmitted" event handler
 * @return valid SPI handle if SPI successfully opened
 * @return INVALID_HANDLE   otherwise
 **********************************************************************************/
HSPI SPI_Open(uint8_t Channel,uint8_t Params,EVENT (*TxRxDone)(uint8_t Byte));

/*******************************************************************************//**
 * closes SPI
 * @param[in] SPI SPI handle
 * @return SUCCESS if SPI successfully closed
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT SPI_Close(HSPI SPI);

/*******************************************************************************//**
 * starts SPI transmission
 * @param[in] SPI SPI handle
 * @return SUCCESS if SPI transmission successfully started
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT SPI_Start(HSPI SPI);

/*******************************************************************************//**
 * stops SPI transmission
 * @param[in] SPI SPI handle
 * @return SUCCESS if SPI transmission successfully stopped
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT SPI_Stop(HSPI SPI);

/*******************************************************************************//**
 * transmits byte via SPI
 * @param[in]  SPI    SPI handle
 * @param[in]  TxByte byte to send
 * @param[out] RxByte received byte if transmission is sync
 * @return SUCCESS if byte successfully transmitted
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT SPI_TxRx(HSPI SPI,uint8_t TxByte,uint8_t *RxByte);

#endif
