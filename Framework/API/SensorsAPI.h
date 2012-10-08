/**
 * @file SensorsAPI.h
 * Sensors API.
 * @author Nezametdinov I.E.
 */

#ifndef __SENSORS_API_H__
#define __SENSORS_API_H__

#include "../PIL/Defs.h"

/// sensor handle
typedef uint8_t HSensor;

/*******************************************************************************//**
 * opens sensor
 * @param[in] Sensor          sensor
 * @param[in] MeasurementDone sensor "measurement done" event handler
 * @return valid sensor handle if sensor successfully opened
 * @return INVALID_HANDLE      otherwise
 **********************************************************************************/
HSensor Sensor_Open(uint8_t Sensor,EVENT (*MeasurementDone)
                    (uint32_t Value,RESULT Result));

/*******************************************************************************//**
 * closes sensor
 * @param[in] Sensor sensor handle
 * @return SUCCESS if sensor successfully closed
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Sensor_Close(HSensor Sensor);

/*******************************************************************************//**
 * requests sensor measurement
 * @param[in] Sensor sensor handle
 * @return SUCCESS if measurement successfully started
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Sensor_RequestMeasurement(HSensor Sensor);

/*******************************************************************************//**
 * this is an example of how to use sensors API
 * @example Sensors/app.c
 **********************************************************************************/

#endif
