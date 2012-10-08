/**
 * @file MACLayerDefs.h
 * MAC layer definitions.
 * @author Nezametdinov I.E.
 */

#ifndef __MAC_LAYER_DEFS_H__
#define __MAC_LAYER_DEFS_H__

#include "../../PIL/NWK/PHY/PHYLayer.h"
#include "../../PIL/Defs.h"

/// MAC layer states
typedef enum
{
	/// rx
	MAC_LAYER_STATE_RX             = 0,
	/// tx
	MAC_LAYER_STATE_TX             = 1,
	/// tx CSMA-CA
	MAC_LAYER_STATE_TX_CSMA_CA     = 2,
	/// tx waiting ACK
	MAC_LAYER_STATE_TX_WAITING_ACK = 3
}MAC_LAYER_STATE;

/// structure defines MAC layer
typedef struct
{
	/// MAC layer state
	MAC_LAYER_STATE State;
	
	/// the 64 bit (IEEE) address assigned to the device.
	/// though this is MAC constant, it is defined as variable,
	/// but it may only be changed when radio transciever is
	/// switched off
	MAC_EXTENDED_ADDR ExtendedAddress;
	
	MAC_EXTENDED_ADDR ShortAddres;
	
	/// ack wait duration
	uint16_t AckWaitDuration;
	
	/// short address
	MAC_SHORT_ADDR ShortAddress;
	
	/// pan ID
	uint16_t PanID;
	
	/// DSN
	uint8_t DSN;
	
	/// tx data
	uint8_t TxData[PHY_A_MAX_PHY_PACKET_SIZE];
	
	/// tx data length
	uint8_t TxLen;
}MACLayerDefsStruct;

#endif
