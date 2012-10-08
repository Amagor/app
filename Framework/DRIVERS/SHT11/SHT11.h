/**
 * @file SHT11.h
 * SHT11 implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __SHT11_H__
#define __SHT11_H__

#include "../../PIL/Defs.h"

/// quantity
typedef enum
{
	/// temperature
	SHT11_TEMPERATURE = 0x03,
	/// humidity
	SHT11_HUMIDITY    = 0x05
}SHT11_QUANTITY;

/*******************************************************************************//**
 * inits SHT11
 * @return SUCCESS if SHT11 successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT SHT11_Init(void);

/*******************************************************************************//**
 * requests measurement
 * @param[in] Quantity physical quantity which will be measured
 * @return SUCCESS if measurement successfully started
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT SHT11_RequestMeasurement(SHT11_QUANTITY Quantity);

/*******************************************************************************//**
 * "measurement done" event
 * @param[in] Quantity physical quantity which has been measured
 * @param[in] Value    measured value
 * @param[in] Result   result of measurement
 **********************************************************************************/
EVENT SHT11_MeasurementDone(SHT11_QUANTITY Quantity,uint16_t Value,RESULT Result);

#endif
