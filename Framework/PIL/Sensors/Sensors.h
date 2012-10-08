/**
 * @file Sensors.h
 * Sensors implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __SENSORS_H__
#define __SENSORS_H__

#include "../../API/SensorsAPI.h"

/*******************************************************************************//**
 * inits sensors
 * @return SUCCESS if sensors successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Sensors_Init(void);

/*******************************************************************************//**
 * forces sensors to enter power save mode
 **********************************************************************************/
void Sensors_PowerSave(void);

/*******************************************************************************//**
 * restores sensors after leaving power save mode
 **********************************************************************************/
void Sensors_Restore(void);

#endif
