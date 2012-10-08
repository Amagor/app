/**
 * @file MACLayer.h
 * MAC layer CSMA-CA implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __MAC_LAYER_CSMA_CA_H__
#define __MAC_LAYER_CSMA_CA_H__

#include "../../PIL/Defs.h"

/*******************************************************************************//**
 * inits CSMA-CA
 * @return SUCCESS if CSMA-CA successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT MACLayerCSMACA_Init(void);

/*******************************************************************************//**
 * starts CSMA-CA
 **********************************************************************************/
void MACLayerCSMACA_Start(void);

/*******************************************************************************//**
 * CSMA-CA "done" event
 * @param[in] Result result of CSMA-CA
 **********************************************************************************/
EVENT MACLayerCSMACA_Done(RESULT Result);

#endif
