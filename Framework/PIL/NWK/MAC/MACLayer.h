/**
 * @file MACLayer.h
 * MAC layer implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __MAC_LAYER_H__
#define __MAC_LAYER_H__

#include "../../PIL/Defs.h"

/// MAC broadcast extended 64 bit address
#define MAC_EXTENDED_BROADCAST_ADDR 0xFFFFFFFFFFFFFFFF

/// MAC enumerations IEEE802.15.4 table - 64
typedef enum
{
	/// SUCCESS
	MAC_SUCCESS                = 0x00,
	/// CHANNEL_ACCESS_FAILURE
	MAC_CHANNEL_ACCESS_FAILURE = 0xE1,
	/// FRAME_TOO_LONG
	MAC_FRAME_TOO_LONG         = 0xE5,
	/// INVALID_PARAMETER
	MAC_INVALID_PARAMETER      = 0xE8,
	/// NO_ACK
	MAC_NO_ACK                 = 0xE9,
	/// UNSUPPORTED_ATTRIBUTE
	MAC_UNSUPPORTED_ATTRIBUTE  = 0xF4
}MAC_ENUM;

/// MAC PIB attributes IEEE802.15.4 table - 71
typedef enum
{
	/// ACK wait duration
	MAC_ACK_WAIT_DURATION = 0x40,
	/// DSN
	MAC_MAC_DSN           = 0x4C,
	/// max CSMA backoffs
	MAC_MAX_CSMA_BACKOFFS = 0x4E,
	/// min BE
	MAC_MIN_BE            = 0x4F,
	/// PAN ID
	MAC_PAN_ID            = 0x50,
	/// rx on when idle
	MAC_RX_ON_WHEN_IDLE   = 0x52,
	/// short address
	MAC_SHORT_ADDRESS     = 0x53
}MAC_PIB_ATTRIBUTE_PARAM;

/// MAC sublayer constants IEEE802.15.4 table - 70
enum
{
	/// max frame overhead
	MAC_A_MAX_FRAME_OVERHEAD    = 25,
	/// max MAC frame size
	MAC_A_MAX_MAC_FRAME_SIZE    = 102,
	/// max frame retries
	MAC_A_MAX_FRAME_RETRIES     = 3,
	/// max BE
	MAC_A_MAX_BE                = 5,
	/// unit backoff period
	MAC_A_UNIT_BACKOFF_PERIOD   = 20
};

/// MAC layer frame
typedef struct
{
	/// the source addressing mode
	uint8_t  SrcAddrMode;
	
	/// the 16 bit PAN identifier of the entity from which the
	/// MSDU is being transferred
	uint16_t SrcPanID;
	
	/// the individual device address of the entity from
	/// which the MSDU is being transferred
	uint8_t  *SrcAddr;
	
	/// the destination addressing mode
	uint8_t  DstAddrMode;
	
	/// the 16 bit PAN identifier of the entity to which the
	/// MSDU is being transferred
	uint16_t DstPanID;
	
	/// the individual device address of the entity to which
	/// the MSDU is being transferred
	uint8_t  *DstAddr;
	
	/// MSDU length
	uint8_t  Length;
	
	/// MSDU
	uint8_t  *Data;
}MACLayerFrame;

/*******************************************************************************//**
 * inits MAC layer
 * @return SUCCESS if MAC layer successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT MACLayer_Init(void);

/*******************************************************************************//**
 * MCPS-DATA.request
 * IEEE802.15.4 paragraph - 7.1.1.1
 * @param[in] Frame       MSDU frame
 *                        IEEE802.15.4 paragraph - 7.1.1.1.1
 * @param[in] Handle      the handle associated with the MSDU to be
 *                        transmitted by the MAC sublayer entity
 *                        IEEE802.15.4 paragraph - 7.1.1.1.1
 * @param[in] TxOptions   the transmission options for this MSDU
 *                        IEEE802.15.4 paragraph - 7.1.1.1.1
 * @return SUCCESS if request successfully accepted
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT MACLayer_DATA_Request(MACLayerFrame *Frame,uint8_t Handle,
                             uint8_t TxOptions);

/*******************************************************************************//**
 * MCPS-DATA.confirm
 * IEEE802.15.4 paragraph - 7.1.1.2
 * @param[in] Handle the handle associated with the MSDU
 *                   being confirmed
 *                   IEEE802.15.4 paragraph - 7.1.1.2.1
 * @param[in] Status the result of the request to transmit a packet
 *                   IEEE802.15.4 paragraph - 7.1.1.2.1
 **********************************************************************************/
EVENT MACLayer_DATA_Confirm(uint8_t Handle,MAC_ENUM Status);

/*******************************************************************************//**
 * MCPS-DATA.indication
 * IEEE802.15.4 paragraph - 7.1.1.3
 * @param[in] Frame       MSDU frame
 *                        IEEE802.15.4 paragraph - 7.1.1.3.1
 * @param[in] LinkQuality LQ value measured during reception of the MPDU.
 *                        Lower values represent lower LQ
 *                        IEEE802.15.4 paragraph - 7.1.1.3.1
 * @param[in] SecurityUse an indication of whether the received data frame is
 *                        using security
 *                        IEEE802.15.4 paragraph - 7.1.1.3.1
 * @param[in] ACLEntry    the macSecurityMode parameter value from the ACL
 *                        entry associated with the sender of the data frame
 *                        IEEE802.15.4 paragraph - 7.1.1.3.1
 **********************************************************************************/
EVENT MACLayer_DATA_Indication(MACLayerFrame *Frame,uint8_t LinkQuality,
                               BOOL SecurityUse,uint8_t ACLEntry);

/*******************************************************************************//**
 * returns HW extended MAC address of current device
 * @return HW extended MAC address
 **********************************************************************************/
MAC_EXTENDED_ADDR MACLayer_GetHWAddr(void);

#endif
