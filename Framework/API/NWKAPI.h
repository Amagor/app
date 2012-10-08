/**
 * @file NWKAPI.h
 * NWK API.
 * @author Nezametdinov I.E.
 */

#ifndef __NWK_API_H__
#define __NWK_API_H__

#include "../PIL/Defs.h"

#define BROADCAST_ADDRESS 0xFFFFFFFFFFFFFFFFll




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*******************************************************************************//**
NWK Layer
ZigBee Specification стр 260



 **********************************************************************************/
 
uint8_t NWK_Join(uint16_t PANID, uint8_t Channel,uint8_t Duration,
EVENT (*JDone)(uint8_t status, uint16_t NetAdd, uint8_t Hello, uint8_t Module),
EVENT (*NWK_RxDone)(uint16_t DstAddr, uint16_t SrcAddr, uint8_t NsduLength, uint8_t *NsduData,
uint8_t LinkQuality,uint64_t RxTime ));


uint8_t NWK_StartCrd(uint16_t PANID,uint8_t Channel,uint8_t HelloInterval,uint8_t Module,
EVENT (*NWK_RxDone)(uint16_t DstAddr, uint16_t SrcAddr, uint8_t NsduLength, uint8_t *NsduData,
uint8_t LinkQuality,uint64_t RxTime ));
 

RESULT NWK_Data_Tx(uint16_t DstAddr, uint8_t NsduLength, uint8_t NsduHandle, uint8_t *NsduData,
 EVENT (*NWK_TxDone)(BOOL status, uint8_t NsduHandle, uint64_t TxTime));




// установка уровня мощности

RESULT NWK_Set_TxPower(uint8_t TxPower);


// отключение сети
RESULT NWK_Leave(void);

// Управление режимом дебага

RESULT NWK_DebugOn(void);
RESULT NWK_DebugOff(void);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++







/// socket handle
typedef uint8_t HSocket;

/*******************************************************************************//**
 * sets NWK params
 * @param[in] Address device address
 * @param[in] PanID   NWK PAN ID
 * @param[in] Channel NWK channel
 * @return SUCCESS if NWK params successfully set
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT NWK_SetParams(uint64_t Address,uint8_t SrcAddrMode,uint16_t PanID,uint8_t Channel);

/*******************************************************************************//**
 * starts NWK
 * @param[in] Started NWK "started" event handler
 * @return SUCCESS if NWK starting successfully began
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT NWK_Start(EVENT (*Started)(void));

/*******************************************************************************//**
 * stops NWK
 * @param[in] Stopped NWK "stopped" event handler
 * @return SUCCESS if NWK stopping successfully began
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT NWK_Stop(EVENT (*Stopped)(void));

/*******************************************************************************//**
 * returns device extended MAC address
 * @return device extended MAC address
 **********************************************************************************/
MAC_EXTENDED_ADDR NWK_GetExtAddr(void);

/*******************************************************************************//**
 * returns values for link quality calculation
 * @return values for link quality calculation
 **********************************************************************************/
uint16_t NWK_GetLQVals(void);

/*******************************************************************************//**
 * creates socket
 * @param[in] Port    source port
 * @param[in] TxDone "data transmitted" event handler
 * @param[in] RxDone "data received" event handler
 * @return valid socket handle if socket successfully created
 * @return INVALID_HANDLE      otherwise
 **********************************************************************************/
HSocket Socket_Create(uint8_t Port,EVENT (*TxDone)(RESULT Result),
                      EVENT (*RxDone)(uint8_t Length,uint8_t *Data,uint8_t *SrcAddr,
                      uint8_t SrcAddrMode,uint8_t SrcPort,uint8_t LQI));

/*******************************************************************************//**
 * destroyes socket
 * @param[in] Socket socket handle
 * @return SUCCESS if socket successfully destroyed
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Socket_Destroy(HSocket Socket);

/*******************************************************************************//**
 * sends data via socket
 * @param[in] Socket      socket handle
 * @param[in] Length      data length
 * @param[in] Data        data
 * @param[in] DestAddress destination address
 * @param[in] DestPort    destination port
 * @param[in] TxPower     transmission power
 * @return SUCCESS if data transmission successfully started
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Socket_Tx(HSocket Socket,uint8_t Length,uint8_t *Data,
                 uint8_t *DestAddress,uint8_t DstAddrMode,uint8_t DestPort,uint8_t TxPower);

/*******************************************************************************//**
 * this is an example of how to use NWK API
 * @example BlinkToRadio/app.c
 **********************************************************************************/

#endif
