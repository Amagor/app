/**
 * @file PlatformSensors.c
 * Sensors implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/Sensors/Sensors.h"
#include "../../DRIVERS/SHT11/SHT11.h"
#include "../../PIL/Guard.h"

#define SENSOR_IS_SYSTEM_SENSOR(Sensor)      ((SensorsDefs.SensorsOwners)&(1<<Sensor))
#define SENSOR_SET_SYS_ACCESS_RIGHTS(Sensor) {SensorsDefs.SensorsOwners |= (1<<Sensor);}
#define SENSOR_SET_APP_ACCESS_RIGHTS(Sensor) {SensorsDefs.SensorsOwners &= ~(1<<Sensor);}

/// structure defines sensor
typedef struct
{
	EVENT (*MeasurementDone[NUM_SENSORS])(uint32_t Value,RESULT Result);
	uint8_t SensorsOwners;
}SensorsDefsStruct;
static volatile SensorsDefsStruct SensorsDefs;

/*******************************************************************************//**
 * @implements SHT11_MeasurementDone
 **********************************************************************************/
EVENT SHT11_MeasurementDone(SHT11_QUANTITY Quantity,uint16_t Value,RESULT Result)
{
	// save current guard state
	SAVE_GUARD_STATE
	
	if(Quantity==SHT11_TEMPERATURE)
	{
		// if sensor is not active then no need to signal "measurement done" event
		if(SensorsDefs.MeasurementDone[TEMPERATURE_SENSOR]==NULL)
		{
			// restore previous guard state
			RESTORE_GUARD_STATE
			return;
		}
		
		// if current sensor is a not a system sensor, then
		// guard should watch for it
		if(!SENSOR_IS_SYSTEM_SENSOR(TEMPERATURE_SENSOR))
			Guard_Watch();
		else
			Guard_Idle();
		
		// signal "measurement done" event
		SensorsDefs.MeasurementDone[TEMPERATURE_SENSOR]((uint32_t)Value,Result);
	}
	else
	{
		// if sensor is not active then no need to signal "measurement done" event
		if(SensorsDefs.MeasurementDone[HUMIDITY_SENSOR]==NULL)
		{
			// restore previous guard state
			RESTORE_GUARD_STATE
			return;
		}
		
		// if current sensor is a not a system sensor, then
		// guard should watch for it
		if(!SENSOR_IS_SYSTEM_SENSOR(HUMIDITY_SENSOR))
			Guard_Watch();
		else
			Guard_Idle();
		
		// signal "measurement done" event
		SensorsDefs.MeasurementDone[HUMIDITY_SENSOR]((uint32_t)Value,Result);
	}
	
	// restore previous guard state
	RESTORE_GUARD_STATE
}

/*******************************************************************************//**
 * @implements Sensors_Init
 **********************************************************************************/
RESULT Sensors_Init(void)
{
	uint8_t i;
	
	// init sensors
	for(i=0;i<NUM_SENSORS;++i)
	{
		SensorsDefs.MeasurementDone[i] = NULL;
	}
	
	// init SHT11
	return SHT11_Init();
}

#ifdef USE_PWR
/*******************************************************************************//**
 * @implements Sensors_PowerSave
 **********************************************************************************/
void Sensors_PowerSave(void)
{
	// empty
}

/*******************************************************************************//**
 * @implements Sensors_Restore
 **********************************************************************************/
void Sensors_Restore(void)
{
	// empty
}
#endif

/*******************************************************************************//**
 * @implements Sensor_Open
 **********************************************************************************/
HSensor Sensor_Open(uint8_t Sensor,EVENT (*MeasurementDone)
                    (uint32_t Value,RESULT Result))
{
	// check sensor
	if(Sensor!=TEMPERATURE_SENSOR&&Sensor!=HUMIDITY_SENSOR)
		return INVALID_HANDLE;
	
	// check "measurement done" event handler
	if(MeasurementDone==NULL)
		return INVALID_HANDLE;
	
	// check sensor to be not active
	if(SensorsDefs.MeasurementDone[Sensor]!=NULL)
		return INVALID_HANDLE;
	
	// activate sensor
	SensorsDefs.MeasurementDone[Sensor] = MeasurementDone;
	
	// set sensor access rights
	if(!Guard_IsWatching())
		SENSOR_SET_SYS_ACCESS_RIGHTS(Sensor)
	else
		SENSOR_SET_APP_ACCESS_RIGHTS(Sensor)
	
	// return sensor handle
	return Sensor;
}

/*******************************************************************************//**
 * @implements Sensor_Close
 **********************************************************************************/
RESULT Sensor_Close(HSensor Sensor)
{
	// check sensor
	if(Sensor!=TEMPERATURE_SENSOR&&Sensor!=HUMIDITY_SENSOR)
		return FAIL;
	
	// if sensor is a system sensor and guard is watching for a threat
	// then return failure
	if(SENSOR_IS_SYSTEM_SENSOR(Sensor)&&Guard_IsWatching())
		return FAIL;
	
	// stop sensor
	SensorsDefs.MeasurementDone[Sensor] = NULL;
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements Sensor_RequestMeasurement
 **********************************************************************************/
RESULT Sensor_RequestMeasurement(HSensor Sensor)
{
	// check sensor
	if(Sensor!=TEMPERATURE_SENSOR&&Sensor!=HUMIDITY_SENSOR)
		return FAIL;
	
	// if sensor is not active then return failure
	if(SensorsDefs.MeasurementDone[Sensor]==NULL)
		return FAIL;
	
	// if sensor is a system sensor and guard is watching for a threat
	// then return failure
	if(SENSOR_IS_SYSTEM_SENSOR(Sensor)&&Guard_IsWatching())
		return FAIL;
	
	// request measurement
	if(Sensor==TEMPERATURE_SENSOR)
		return SHT11_RequestMeasurement(SHT11_TEMPERATURE);
	
	return SHT11_RequestMeasurement(SHT11_HUMIDITY);
}
