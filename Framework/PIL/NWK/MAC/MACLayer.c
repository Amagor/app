/**
 * @file MACLayer.c
 * MAC layer implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/NWK/MAC/MACLayerCSMACA.h"
#include "../../PIL/NWK/MAC/MACLayerDefs.h"
#include "../../PIL/NWK/MAC/MACLayer.h"
#include "../../PIL/NWK/NWKLayer.h"
#include "../../API/CommonAPI.h"
#include "../../PIL/Utils.h"



/// MAC layer frame wich must be transmitted
typedef struct
{
	/// MAC layer frame
	MACLayerFrame *Frame;
	
	/// MAC layer frame handle
	uint8_t Handle;
	
	/// MAC layer transmission options
	uint8_t TxOptions;
}MACLayerTxData;

/// MAC layer tx frame
static volatile MACLayerTxData TxFrame;

/// MAC layer rx frame
static volatile MACLayerFrame RxFrame;

/// MAC layer defs
static volatile MACLayerDefsStruct MACLayerDefs;

/*******************************************************************************//**
 * @implements MACLayerCSMACA_Done
 **********************************************************************************/
EVENT MACLayerCSMACA_Done(RESULT Result)
{
	// if success
	if(Result==SUCCESS)
	{
		// request tx
		MACLayerDefs.State = MAC_LAYER_STATE_TX;
		PHYLayer_SETTRXSTATE_Request(PHY_TX_ON);
		
	}
	// else
	else
	{
		// signal channel access failure
		MACLayer_DATA_Confirm(TxFrame.Handle,MAC_CHANNEL_ACCESS_FAILURE);
		
		// change state to rx
		MACLayerDefs.State = MAC_LAYER_STATE_RX;
		PHYLayer_SETTRXSTATE_Request(PHY_RX_ON);
		
	}
	
}

/*******************************************************************************//**
 * @implements MACLayer_Init
 **********************************************************************************/
RESULT MACLayer_Init(void)
{
	// init MAC layer
	#ifdef USE_HWADDR
	MACLayerDefs.ExtendedAddress = MACLayer_GetHWAddr();
	#else
	MACLayerDefs.ExtendedAddress = MAC_DEFAULT_A_EXTENDED_ADDRESS;
	#endif
	MACLayerDefs.AckWaitDuration = 54;
	MACLayerDefs.ShortAddress    = 0xFFFF;
	MACLayerDefs.PanID           = DEFAULT_PAN_ID;
	MACLayerDefs.TxLen           = 0;
	MACLayerDefs.State           = MAC_LAYER_STATE_RX;
	TxFrame.Frame = NULL;
	
	// init MAC layer CSMA-CA
	return MACLayerCSMACA_Init();
}

/*******************************************************************************//**
 * @implements MACLayer_GetDefs
 **********************************************************************************/
MACLayerDefsStruct* MACLayer_GetDefs(void)
{
	return (MACLayerDefsStruct*)&MACLayerDefs;
}

/*******************************************************************************//**
 * @implements MACLayer_DATA_Request
 **********************************************************************************/
RESULT MACLayer_DATA_Request(MACLayerFrame *Frame,uint8_t Handle,
                             uint8_t TxOptions)
{
	uint8_t i;
	MAC_LAYER_STATE State;
	
	// check radio transceiver state
	if(Radio_GetState()==RADIO_STATE_POWER_DOWN)
		return FAIL;
	
	BEGIN_CRITICAL_SECTION
	{
		State = MACLayerDefs.State;
		
		// check current state
		if(State==MAC_LAYER_STATE_RX)
			MACLayerDefs.State = MAC_LAYER_STATE_TX;
	}
	END_CRITICAL_SECTION
	
	if(State!=MAC_LAYER_STATE_RX)
		return FAIL;
	
	// set frame
	TxFrame.Frame     = Frame;
	TxFrame.Handle    = Handle;
	TxFrame.TxOptions = TxOptions;
	
	// check frame
	if(TxFrame.Frame==NULL)
	{
		MACLayerDefs.State = MAC_LAYER_STATE_RX;
		PHYLayer_SETTRXSTATE_Request(PHY_RX_ON);
		
		SIGNAL_EVENT(MACLayer_DATA_Confirm(TxFrame.Handle,MAC_INVALID_PARAMETER))
		
		return SUCCESS;
		
	}
	
	if(TxFrame.Frame->Data==NULL)
	{
		MACLayerDefs.State = MAC_LAYER_STATE_RX;
		PHYLayer_SETTRXSTATE_Request(PHY_RX_ON);
		
		SIGNAL_EVENT(MACLayer_DATA_Confirm(TxFrame.Handle,MAC_INVALID_PARAMETER))
		
		return SUCCESS;
		
	}
	
	// check length
	if(TxFrame.Frame->Length>MAC_A_MAX_MAC_FRAME_SIZE-2)
	{
		MACLayerDefs.State = MAC_LAYER_STATE_RX;
		PHYLayer_SETTRXSTATE_Request(PHY_RX_ON);
		
		SIGNAL_EVENT(MACLayer_DATA_Confirm(TxFrame.Handle,MAC_FRAME_TOO_LONG))
		
	}
	
	// construct MSDU
	MACLayerDefs.TxLen = 0;
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = 0x41;
	if (TxFrame.Frame->SrcAddrMode == 0x02 && TxFrame.Frame->DstAddrMode == 0x02) MACLayerDefs.TxData[MACLayerDefs.TxLen++] = 0x44;
	if (TxFrame.Frame->SrcAddrMode == 0x02 && TxFrame.Frame->DstAddrMode == 0x03) MACLayerDefs.TxData[MACLayerDefs.TxLen++] = 0x4C;
	if (TxFrame.Frame->SrcAddrMode == 0x03 && TxFrame.Frame->DstAddrMode == 0x02) MACLayerDefs.TxData[MACLayerDefs.TxLen++] = 0xC4;
	if (TxFrame.Frame->SrcAddrMode == 0x03 && TxFrame.Frame->DstAddrMode == 0x03) MACLayerDefs.TxData[MACLayerDefs.TxLen++] = 0xCC;
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = MACLayerDefs.DSN++;
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = (uint8_t)TxFrame.Frame->DstPanID;
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = (uint8_t)(TxFrame.Frame->DstPanID>>8);
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->DstAddr[0];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->DstAddr[1];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->DstAddr[2];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->DstAddr[3];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->DstAddr[4];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->DstAddr[5];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->DstAddr[6];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->DstAddr[7];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->SrcAddr[0];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->SrcAddr[1];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->SrcAddr[2];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->SrcAddr[3];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->SrcAddr[4];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->SrcAddr[5];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->SrcAddr[6];
	MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->SrcAddr[7];
	
	for(i=0;i<TxFrame.Frame->Length;++i)
		MACLayerDefs.TxData[MACLayerDefs.TxLen++] = TxFrame.Frame->Data[i];
	
	#ifndef PHY_LAYER_HANDLE_CHECKSUM
	*((uint16_t*)(&MACLayerDefs.TxData[MACLayerDefs.TxLen])) = 
	    Utils_ITUTCRC16(MACLayerDefs.TxLen,(uint8_t*)MACLayerDefs.TxData);
	MACLayerDefs.TxLen += 2;
	#endif
	
	// begin CSMA-CA
	MACLayerDefs.State = MAC_LAYER_STATE_TX_CSMA_CA;
	PHYLayer_SETTRXSTATE_Request(PHY_RX_ON_REJECT_ALL);
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements PHYLayer_DATA_Confirm
 **********************************************************************************/
EVENT PHYLayer_DATA_Confirm(PHY_ENUM Status)
{
	// if success
	if(Status==PHY_SUCCESS)
	{
		// confirm
		MACLayer_DATA_Confirm(TxFrame.Handle,MAC_SUCCESS);
		
		// change state to rx
		MACLayerDefs.State = MAC_LAYER_STATE_RX;
		PHYLayer_SETTRXSTATE_Request(PHY_RX_ON);
		
	}
	// else
	else
	{
		// change state to tx
		PHYLayer_SETTRXSTATE_Request(PHY_TX_ON);
		
	}
	
}

/*******************************************************************************//**
 * @implements PHYLayer_DATA_Indication
 **********************************************************************************/
EVENT PHYLayer_DATA_Indication(uint8_t Length,uint8_t *Data,uint8_t LinkQuality)
{
	// check current state
	if(MACLayerDefs.State!=MAC_LAYER_STATE_RX)
		return;
	
	// check length
	if(Length<=4)
		return;
	
	#ifndef PHY_LAYER_HANDLE_CHECKSUM
	// check CRC
	uint16_t CRC = *((uint16_t*)(&Data[Length-2]));
	if(CRC!=Utils_ITUTCRC16(Length-2,Data))
		return;
	Length -= 2;
	#endif
	
	// check adress
	if(Data[0]!=0x41||Length<19)
		return;
		
	switch (Data[1])
		{
		case 0x44:
		RxFrame.SrcAddrMode = 0x02;
		RxFrame.DstAddrMode = 0x02;
			// check address
			if((*((MAC_EXTENDED_ADDR*)(&Data[5]))!=MACLayerDefs.ShortAddress)&&
			(*((MAC_EXTENDED_ADDR*)(&Data[5]))!=0xFFFF))
			return;
		break;

		case 0x4C:
		RxFrame.SrcAddrMode = 0x02;
		RxFrame.DstAddrMode = 0x03;
					// check address
			if((*((MAC_EXTENDED_ADDR*)(&Data[5]))!=MACLayerDefs.ExtendedAddress)&&
			(*((MAC_EXTENDED_ADDR*)(&Data[5]))!=0xFFFF))
			return;
		break;

		case 0xC4:
		RxFrame.SrcAddrMode = 0x03;
		RxFrame.DstAddrMode = 0x02;
					// check address
			if((*((MAC_EXTENDED_ADDR*)(&Data[5]))!=MACLayerDefs.ShortAddress)&&
			(*((MAC_EXTENDED_ADDR*)(&Data[5]))!=0xFFFF))
			return;
		break;

		case 0xCC:
		RxFrame.SrcAddrMode = 0x03;
		RxFrame.DstAddrMode = 0x03;
					// check address
			if((*((MAC_EXTENDED_ADDR*)(&Data[5]))!=MACLayerDefs.ExtendedAddress)&&
			(*((MAC_EXTENDED_ADDR*)(&Data[5]))!=0xFFFF))
			return;
		break;				
		}
	
	// check pan ID
	if(*((uint16_t*)(&Data[3]))!=MACLayerDefs.PanID)
		return;
	

	
	// set rx frame params
	RxFrame.SrcPanID    = MACLayerDefs.PanID;
	RxFrame.SrcAddr     = (uint8_t*)&Data[13];
	RxFrame.DstPanID    = MACLayerDefs.PanID;
	RxFrame.DstAddr     = (uint8_t*)&Data[5];
	RxFrame.Length      = Length-21;
	RxFrame.Data        = (uint8_t*)&Data[21];
	
	// signal data indication
	MACLayer_DATA_Indication((MACLayerFrame*)&RxFrame,LinkQuality,FALSE,0);
	
}

/*******************************************************************************//**
 * @implements PHYLayer_ED_Confirm
 **********************************************************************************/
EVENT PHYLayer_ED_Confirm(PHY_ENUM Status,
                          int8_t EnergyLevel)
{
	// empty
}

/*******************************************************************************//**
 * @implements PHYLayer_GET_Confirm
 **********************************************************************************/
EVENT PHYLayer_GET_Confirm(PHY_ENUM Status,
                           PHY_PIB_ATTRIBUTE_PARAM PIBAttribute,
                           uint32_t PIBAttributeValue)
{
	// empty
}

/*******************************************************************************//**
 * @implements PHYLayer_SETTRXSTATE_Confirm
 **********************************************************************************/
EVENT PHYLayer_SETTRXSTATE_Confirm(PHY_ENUM Status)
{
	// CSMA-CA
	if(Status!=PHY_SUCCESS&&Status!=PHY_RX_ON_REJECT_ALL&&
	   MACLayerDefs.State==MAC_LAYER_STATE_TX_CSMA_CA)
	{
		// request state change to RX_ON_REJECT_ALL
		PHYLayer_SETTRXSTATE_Request(PHY_RX_ON_REJECT_ALL);
		return;
		
	}
	else if(MACLayerDefs.State==MAC_LAYER_STATE_TX_CSMA_CA)
	{
		// start CSMA-CA
		MACLayerCSMACA_Start();
		return;
		
	}
	
	// receiving frame
	if(Status!=PHY_SUCCESS&&Status!=PHY_RX_ON&&
	   MACLayerDefs.State==MAC_LAYER_STATE_RX)
	{
		PHYLayer_SETTRXSTATE_Request(PHY_RX_ON);
		return;
		
	}
	
	// sending frame
	if(Status!=PHY_SUCCESS&&Status!=PHY_TX_ON&&
	   MACLayerDefs.State==MAC_LAYER_STATE_TX)
	{
		PHYLayer_SETTRXSTATE_Request(PHY_TX_ON);
		
	}
	else if(MACLayerDefs.State==MAC_LAYER_STATE_TX)
	{
		PHYLayer_DATA_Request(MACLayerDefs.TxLen,(uint8_t*)MACLayerDefs.TxData);
		
	}
	
}

/*******************************************************************************//**
 * @implements PHYLayer_SET_Confirm
 **********************************************************************************/
EVENT PHYLayer_SET_Confirm(PHY_ENUM Status,
                           PHY_PIB_ATTRIBUTE_PARAM PIBAttribute)
{
	// empty
}
