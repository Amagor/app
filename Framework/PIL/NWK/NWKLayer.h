/**
 * @file NWKLayer.h
 * NWK layer implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __NWK_LAYER_H__
#define __NWK_LAYER_H__

#include "../../PIL/Defs.h"
#include "../../PIL/NWK/MAC/MACLayerDefs.h"

/// radio transceiver states
typedef enum
{
	/// power up
	RADIO_STATE_POWER_UP,
	/// power down
	RADIO_STATE_POWER_DOWN
}RADIO_TRANSCEIVER_STATE;

/*******************************************************************************//**
 * returns MAC layer definition data
 * @return MAC layer definition data
 **********************************************************************************/
MACLayerDefsStruct* MACLayer_GetDefs(void);

/*******************************************************************************//**
 * inits NWK layer
 * @return SUCCESS if NWK layer successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT NWKLayer_Init(void);

/*******************************************************************************//**
 * turns on or off the radio transceiver
 * @param[in] State state of the radio transceiver
 * @return SUCCESS if radio transceiver state changing successfully started
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Radio_SetState(RADIO_TRANSCEIVER_STATE State);

/*******************************************************************************//**
 * returns state of the radio transceiver
 * @param[in] State state of the radio transceiver
 * @return RADIO_STATE_POWER_UP   if radio transceiver is
 *                                switched on
 * @return RADIO_STATE_POWER_DOWN otherwise
 **********************************************************************************/
RADIO_TRANSCEIVER_STATE Radio_GetState(void);

/*******************************************************************************//**
 * forces radio transceiver to enter power save mode
 **********************************************************************************/
void Radio_PowerSave(void);

/*******************************************************************************//**
 * restores radio transceiver after leaving power save mode
 **********************************************************************************/
void Radio_Restore(void);

/*******************************************************************************//**
 * "radio state changed" event
 * @param[in] NewState new state of radio transceiver
 **********************************************************************************/
EVENT Radio_StateChanged(RADIO_TRANSCEIVER_STATE NewState);

#endif
