/**
 * @file PHYLayer.h
 * PHY layer implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __PHY_LAYER_H__
#define __PHY_LAYER_H__

#include "../../PIL/Defs.h"

/// PHY enumerations IEEE802.15.4 table - 16
typedef enum
{
	/// BUSY
	PHY_BUSY                  = 0x00,
	/// BUSY_RX
	PHY_BUSY_RX               = 0x01,
	/// BUSY_TX
	PHY_BUSY_TX               = 0x02,
	/// FORCE_TRX_OFF
	PHY_FORCE_TRX_OFF         = 0x03,
	/// IDLE
	PHY_IDLE                  = 0x04,
	/// INVALID_PARAMETER
	PHY_INVALID_PARAMETER     = 0x05,
	/// RX_ON
	PHY_RX_ON                 = 0x06,
	/// SUCCESS
	PHY_SUCCESS               = 0x07,
	/// TRX_OFF
	PHY_TRX_OFF               = 0x08,
	/// TX_ON
	PHY_TX_ON                 = 0x09,
	/// UNSUPPORTED_ATTRIBUTE
	PHY_UNSUPPORTED_ATTRIBUTE = 0x0A,
	/// RX_ON_REJECT_ALL
	PHY_RX_ON_REJECT_ALL      = 0x0B
}PHY_ENUM;

/// PHY PIB attributes IEEE802.15.4 table - 19
typedef enum
{
	/// current channel
	PHY_PIB_CURRENT_CHANNEL_ID    = 0x00,
	/// supported channels
	PHY_PIB_CHANNELS_SUPPORTED_ID = 0x01,
	/// tx power
	PHY_PIB_TX_POWER_ID           = 0x02,
	/// CCA mode
	PHY_PIB_CCA_MODE_ID           = 0x03
}PHY_PIB_ATTRIBUTE_PARAM;

/// PHY constants IEEE802.15.4 table - 18
enum
{
	/// max packet size
	PHY_A_MAX_PHY_PACKET_SIZE = 127,
	/// turnaround time
	PHY_A_TURNAROUND_TIME     = 12,
};

/*******************************************************************************//**
 * inits PHY layer
 * @return SUCCESS if PHY layer successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT PHYLayer_Init(void);

/*******************************************************************************//**
 * PD-DATA.request
 * IEEE802.15.4 paragraph - 6.2.1.1
 * @param[in] Length PSDU length
 *                   IEEE802.15.4 paragraph - 6.2.1.1.1
 * @param[in] Data   PSDU
 *                   IEEE802.15.4 paragraph - 6.2.1.1.1
 * @return SUCCESS if request successfully accepted
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT PHYLayer_DATA_Request(uint8_t Length,uint8_t *Data);

/*******************************************************************************//**
 * PD-DATA.confirm
 * IEEE802.15.4 paragraph - 6.2.1.2
 * @param[in] Status the result of the request to transmit a packet
 *                   IEEE802.15.4 paragraph - 6.2.1.2.1
 **********************************************************************************/
EVENT PHYLayer_DATA_Confirm(PHY_ENUM Status);

/*******************************************************************************//**
 * PD-DATA.indication
 * IEEE802.15.4 paragraph - 6.2.1.3
 * @param[in] Length      PSDU length
 *                        IEEE802.15.4 paragraph - 6.2.1.3.1
 * @param[in] Data        PSDU
 *                        IEEE802.15.4 paragraph - 6.2.1.3.1
 * @param[in] LinkQuality LQ value measured during
 *                        reception of the PPDU
 *                        IEEE802.15.4 paragraph - 6.2.1.3.1
 **********************************************************************************/
EVENT PHYLayer_DATA_Indication(uint8_t Length,uint8_t *Data,uint8_t LinkQuality);

/*******************************************************************************//**
 * PLME-CCA.request
 * IEEE802.15.4 paragraph - 6.2.2.1
 * @return SUCCESS if request successfully accepted
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT PHYLayer_CCA_Request(void);

/*******************************************************************************//**
 * PLME-CCA.confirm
 * IEEE802.15.4 paragraph - 6.2.2.2
 * @param[in] Status the result of the request to perform a CCA
 *                   IEEE802.15.4 paragraph - 6.2.2.2.1
 **********************************************************************************/
EVENT PHYLayer_CCA_Confirm(PHY_ENUM Status);

/*******************************************************************************//**
 * PLME-ED.request
 * IEEE802.15.4 paragraph - 6.2.2.3
 * @return SUCCESS if request successfully accepted
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT PHYLayer_ED_Request(void);

/*******************************************************************************//**
 * PLME-ED.confirm
 * IEEE802.15.4 paragraph - 6.2.2.4
 * @param[in] Status      the result of the request to perform
 *                        an ED measurement
 *                        IEEE802.15.4 paragraph - 6.2.2.4.1
 * @param[in] EnergyLevel ED level for the current channel
 *                        IEEE802.15.4 paragraph - 6.2.2.4.1
 **********************************************************************************/
EVENT PHYLayer_ED_Confirm(PHY_ENUM Status,int8_t EnergyLevel);

/*******************************************************************************//**
 * PLME-GET.request
 * IEEE802.15.4 paragraph - 6.2.2.5
 * @param[in] PIBAttribute the identifier of the
 *                         PHY PIB attribute to get
 *                         IEEE802.15.4 paragraph - 6.2.2.5.1
 * @return SUCCESS if request successfully accepted
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT PHYLayer_GET_Request(PHY_PIB_ATTRIBUTE_PARAM PIBAttribute);

/*******************************************************************************//**
 * PLME-GET.confirm
 * IEEE802.15.4 paragraph - 6.2.2.6
 * @param[in] Status            the result of the request for PHY PIB
 *                              attribute information
 *                              IEEE802.15.4 paragraph - 6.2.2.6.1
 * @param[in] PIBAttribute      the identifier of the PHY PIB attribute
 *                              to get
 *                              IEEE802.15.4 paragraph - 6.2.2.6.1
 * @param[in] PIBAttributeValue the value of the indicated PHY PIB
 *                              attribute to get
 *                              IEEE802.15.4 paragraph - 6.2.2.6.1
 **********************************************************************************/
EVENT PHYLayer_GET_Confirm(PHY_ENUM Status,PHY_PIB_ATTRIBUTE_PARAM PIBAttribute,
                           uint32_t PIBAttributeValue);

/*******************************************************************************//**
 * PLME-SET-TRX-STATE.request
 * IEEE802.15.4 paragraph - 6.2.2.7
 * @param[in] State the new state in which to configure the transceiver
 *                  IEEE802.15.4 paragraph - 6.2.2.7.1
 * @return SUCCESS if request successfully accepted
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT PHYLayer_SETTRXSTATE_Request(PHY_ENUM State);

/*******************************************************************************//**
 * PLME-SET-TRX-STATE.confirm
 * IEEE802.15.4 paragraph - 6.2.2.8
 * @param[in] Status the result of the request to
 *                   change the state of the transceiver
 *                   IEEE802.15.4 paragraph - 6.2.2.8.1
 **********************************************************************************/
EVENT PHYLayer_SETTRXSTATE_Confirm(PHY_ENUM Status);

/*******************************************************************************//**
 * PLME-SET.request
 * IEEE802.15.4 paragraph - 6.2.2.9
 * @param[in] PIBAttribute      the identifier of the
 *                              PHY PIB attribute to set
 *                              IEEE802.15.4 paragraph - 6.2.2.9.1
 * @param[in] PIBAttributeValue the value of the indicated PIB
 *                              attribute to set
 *                              IEEE802.15.4 paragraph - 6.2.2.9.1
 * @return SUCCESS if request successfully accepted
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT PHYLayer_SET_Request(PHY_PIB_ATTRIBUTE_PARAM PIBAttribute,
                            uint32_t PIBAttributeValue);

/*******************************************************************************//**
 * PLME-SET.confirm
 * IEEE802.15.4 paragraph - 6.2.2.10
 * @param[in] Status       the status of the attempt to set the request
 *                         PIB attribute
 *                         IEEE802.15.4 paragraph - 6.2.2.10.1
 * @param[in] PIBAttribute the identifier of the PIB attribute being
 *                         confirmed
 *                         IEEE802.15.4 paragraph - 6.2.2.10.1
 **********************************************************************************/
EVENT PHYLayer_SET_Confirm(PHY_ENUM Status,PHY_PIB_ATTRIBUTE_PARAM PIBAttribute);

/*******************************************************************************//**
 * returns last SFD time in micro seconds
 * @return last SFD time
 **********************************************************************************/
uint64_t PHYLayer_GetLastSFDTime(void);

#endif
