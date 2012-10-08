/**
 * @file CC2420.c
 * CC2420 implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/Scheduler/Scheduler.h"
#include "../../DRIVERS/CC2420/CC2420.h"
#include "../../PIL/NWK/PHY/PHYLayer.h"
#include "../../PIL/Timers/Timers.h"
#include "../../PIL/NWK/NWKLayer.h"
#include "../../API/SchedulerAPI.h"
#include "../../API/CommonAPI.h"
#include "../../PIL/SPI/SPI.h"

#define CC2420_OPERATION_IS(x)   (CC2420Defs.Operation&(1<<x))
#define CC2420_STOP_OPERATION(x) CC2420Defs.Operation &= ~(1<<x);
#define CC2420_DEFER_OPERATION   CC2420Defs.Operation |= 1<<PHY_OPERATION_DEFERED;

// this module needs SPI and Timers
#ifndef USE_SPI
#error SPI needed but not used
#endif

#ifndef USE_TIMERS
#error Timers needed but not used
#endif

/// PHY layer operations
enum
{
	PHY_OPERATION_CCA           = 0,
	PHY_OPERATION_ED            = 1,
	PHY_OPERATION_GET           = 2,
	PHY_OPERATION_SET_TRX_STATE = 3,
	PHY_OPERATION_SET           = 4,
	PHY_OPERATION_REQUEST_DATA  = 5,
	PHY_OPERATION_DEFERED       = 6,
	PHY_OPERATION_ATTR_VALID    = 7
};

/// CC2420 states
typedef enum
{
	CC2420_STATE_VREG_OFF           = 0,
	CC2420_STATE_VREG_WAITING_ON    = 1,
	CC2420_STATE_VREG_WAITING_OFF   = 2,
	CC2420_STATE_POWER_DOWN         = 3,
	CC2420_STATE_OSC_ENABLE_WAITING = 4,
	CC2420_STATE_IDLE               = 5,
	CC2420_STATE_RX                 = 6,
	CC2420_STATE_RX_GOT_SFD         = 7,
	CC2420_STATE_RX_READING         = 8,
	CC2420_STATE_RX_REJECT_ALL      = 9,
	CC2420_STATE_TX                 = 10,
	CC2420_STATE_TX_GOT_SFD         = 11
}CC2420_STATE;

/// structure defines CC2420 driver
typedef struct
{
	/// SPI handle
	HSPI SPI;
	
	/// timer handle
	HTimer Timer;
	
	/// thread handle
	HThread Thread;
	
	/// CC2420 state
	CC2420_STATE State;
	
	/// current PIB attribute to get
	PHY_PIB_ATTRIBUTE_PARAM GetPIBAttribute;
	
	/// current PIB attribute to set
	PHY_PIB_ATTRIBUTE_PARAM SetPIBAttribute;
	
	/// new state to set
	PHY_ENUM NewTRXState;
	
	/// last SFD time
	uint64_t LastSFDTime;
	
	/// values for LQ calculation
	uint16_t LQValues;
	
	///  PHY layer operation
	uint8_t Operation;
	
	/// channel mask
	uint32_t SupportedChannels;
	
	/// current channel
	uint8_t  Channel;
	
	/// tx power
	uint8_t  TxPower;
	
	/// CCA mode
	uint8_t  CCAMode;
	
	/// received data
	uint8_t RxData[PHY_A_MAX_PHY_PACKET_SIZE];
	uint8_t RxLen;
	
	// data to send
	uint8_t *TxData;
	uint8_t TxLen;
	
}CC2420DefsStruct;
static volatile CC2420DefsStruct CC2420Defs;

/*******************************************************************************//**
 * @implements CC2420_SendCommandStrobe
 **********************************************************************************/
uint8_t CC2420_SendCommandStrobe(CC2420_COMMAND_STROBE CommandStrobe)
{
	uint8_t RxByte;
	SPI_Start(CC2420Defs.SPI);
	SPI_TxRx(CC2420Defs.SPI,CommandStrobe,&RxByte);
	SPI_Stop(CC2420Defs.SPI);
	return RxByte;
}

/*******************************************************************************//**
 * @implements CC2420_WriteRegister
 **********************************************************************************/
void CC2420_WriteRegister(CC2420_REGISTER Register,uint16_t Value)
{
	SPI_Start(CC2420Defs.SPI);
	SPI_TxRx(CC2420Defs.SPI,Register,NULL);
	SPI_TxRx(CC2420Defs.SPI,(uint8_t)(Value>>8),NULL);
	SPI_TxRx(CC2420Defs.SPI,(uint8_t)Value,NULL);
	SPI_Stop(CC2420Defs.SPI);
}

/*******************************************************************************//**
 * @implements CC2420_ReadRegister
 **********************************************************************************/
uint16_t CC2420_ReadRegister(CC2420_REGISTER Register)
{
	uint16_t Result;
	uint8_t Value;
	Register |= (1<<CC2420_READ_WRITE_BIT);
	SPI_Start(CC2420Defs.SPI);
	SPI_TxRx(CC2420Defs.SPI,Register,NULL);
	SPI_TxRx(CC2420Defs.SPI,0,&Value);
	Result = Value<<8;
	SPI_TxRx(CC2420Defs.SPI,0,&Value);
	Result |= Value;
	SPI_Stop(CC2420Defs.SPI);
	return Result;
}

/*******************************************************************************//**
 * configures CC2420
 **********************************************************************************/
void CC2420_Configure(void)
{
	uint16_t Val;
	
	// disable hardware MAC operations
	Val = CC2420_ReadRegister(CC2420_MDMCTRL0);
	#ifdef PHY_LAYER_HANDLE_CHECKSUM
	Val &= ~((1<<CC2420_MDMCTRL0_PAN_COORDINATOR)|(1<<CC2420_MDMCTRL0_ADR_DECODE)|(1<<CC2420_MDMCTRL0_AUTOACK));
	Val |=  (1<<CC2420_MDMCTRL0_AUTOCRC);
	#else
	Val &= ~((1<<CC2420_MDMCTRL0_PAN_COORDINATOR)|(1<<CC2420_MDMCTRL0_ADR_DECODE)|(1<<CC2420_MDMCTRL0_AUTOCRC)|(1<<CC2420_MDMCTRL0_AUTOACK));
	#endif
	CC2420_WriteRegister(CC2420_MDMCTRL0,Val);
	
	// disable hardware security operations
	Val = CC2420_ReadRegister(CC2420_SECCTRL0);
	Val &= ~((1<<CC2420_SECCTRL0_RXFIFO_PROTECTION)|(1<<CC2420_SECCTRL0_SEC_MODE_1)|(1<<CC2420_SECCTRL0_SEC_MODE_0));
	CC2420_WriteRegister(CC2420_SECCTRL0,Val);
	
	// set FIFOP THR value to 127
	Val = CC2420_ReadRegister(CC2420_IOCFG0);
	Val |= 0x007F;
	CC2420_WriteRegister(CC2420_IOCFG0,Val);
	
}

/*******************************************************************************//**
 * CC2420 timer "fired" event
 **********************************************************************************/
EVENT CC2420_TimerFired(PARAM Param)
{
	// if current state is waiting for vreg to become stable
	if(CC2420Defs.State==CC2420_STATE_VREG_WAITING_ON)
	{
		// change state to power down with vreg on
		CC2420Defs.State = CC2420_STATE_POWER_DOWN;
		
	}
	// if current state is waiting for vreg to be turned off
	else if(CC2420Defs.State==CC2420_STATE_VREG_WAITING_OFF)
	{
		// change state to power down with vreg off
		CC2420Defs.State = CC2420_STATE_VREG_OFF;
		
		// signal event "radio state changed"
		Radio_StateChanged(RADIO_STATE_POWER_DOWN);
		
	}
	
}

/*******************************************************************************//**
 * CC2420 thread proc
 **********************************************************************************/
PROC CC2420_ThreadProc(PARAM Param)
{
	uint16_t Val;
	uint8_t i,LQI,Byte;
	int8_t RSSIVal;
	
	// request data
	if(CC2420_OPERATION_IS(PHY_OPERATION_REQUEST_DATA))
	{
		CC2420_STOP_OPERATION(PHY_OPERATION_REQUEST_DATA)
		
		// if transmitter is off signal error
		if(CC2420Defs.State<=CC2420_STATE_IDLE)
		{
			SIGNAL_EVENT(PHYLayer_DATA_Confirm(PHY_TRX_OFF))
			
		}
		// if transmitter is in rx state signal error
		else if(CC2420Defs.State==CC2420_STATE_RX||
		        CC2420Defs.State==CC2420_STATE_RX_GOT_SFD||
		        CC2420Defs.State==CC2420_STATE_RX_REJECT_ALL)
		{
			SIGNAL_EVENT(PHYLayer_DATA_Confirm(PHY_RX_ON))
			
		}
		// everithing is fine, send data
		else
		{
			// if there was tx underflow, then flush tx fifo
			if(CC2420_SendCommandStrobe(CC2420_SNOP)&(1<<CC2420_STATUS_TX_UNDERFLOW))
			{
				// flush tx buffer
				CC2420_SendCommandStrobe(CC2420_SFLUSHTX);
				
			}
			
			// write data to tx fifo
			SPI_Start(CC2420Defs.SPI);
			
			SPI_TxRx(CC2420Defs.SPI,CC2420_TXFIFO,NULL);
			
			#ifdef PHY_LAYER_HANDLE_CHECKSUM
			SPI_TxRx(CC2420Defs.SPI,CC2420Defs.TxLen+2,&Byte);
			#else
			SPI_TxRx(CC2420Defs.SPI,CC2420Defs.TxLen,&Byte);
			#endif
			
			for(i=0;i<CC2420Defs.TxLen;++i)
			{
				SPI_TxRx(CC2420Defs.SPI,CC2420Defs.TxData[i],NULL);
				
			}
			
			SPI_Stop(CC2420Defs.SPI);
			
			// if there was rx fifo overflow, then flush rx fifo twice
			if(CC2420_GetRxFIFOOverflow())
			{
				CC2420_SendCommandStrobe(CC2420_SFLUSHRX);
				CC2420_SendCommandStrobe(CC2420_SFLUSHRX);
				
			}
			
			// begin transmission
			CC2420Defs.State = CC2420_STATE_TX;
			
			CC2420_SendCommandStrobe(CC2420_STXON);
			
		}
		
	}
	
	// request CCA
	if(CC2420_OPERATION_IS(PHY_OPERATION_CCA))
	{
		CC2420_STOP_OPERATION(PHY_OPERATION_CCA)
		
		// if TRX is off, then return this state
		if(CC2420Defs.State<=CC2420_STATE_IDLE)
		{
			SIGNAL_EVENT(PHYLayer_CCA_Confirm(PHY_TRX_OFF))
			
		}
		// if tx is on, then return this state
		else if(CC2420Defs.State==CC2420_STATE_TX||
		        CC2420Defs.State==CC2420_STATE_TX_GOT_SFD)
		{
			SIGNAL_EVENT(PHYLayer_CCA_Confirm(PHY_TX_ON))
			
		}
		// check CCA and return its state
		else if(CC2420_GetCCA())
		{
			SIGNAL_EVENT(PHYLayer_CCA_Confirm(PHY_IDLE))
			
		}
		else
		{
			SIGNAL_EVENT(PHYLayer_CCA_Confirm(PHY_BUSY))
			
		}
		
	}
	
	// request ED
	if(CC2420_OPERATION_IS(PHY_OPERATION_ED))
	{
		CC2420_STOP_OPERATION(PHY_OPERATION_ED)
		
		// if TRX is off, then return this state
		if(CC2420Defs.State<=CC2420_STATE_IDLE)
		{
			SIGNAL_EVENT(PHYLayer_ED_Confirm(PHY_TRX_OFF,0))
			
		}
		// if tx is on, then return this state
		else if(CC2420Defs.State==CC2420_STATE_TX||
		        CC2420Defs.State==CC2420_STATE_TX_GOT_SFD)
		{
			SIGNAL_EVENT(PHYLayer_ED_Confirm(PHY_TX_ON,0))
			
		}
		// else get energy level
		else
		{
			// get energy level
			Val = CC2420_ReadRegister(CC2420_RSSI);
			Val |= 0x00FF;
			RSSIVal = *((int8_t*)(&Val));
			if(RSSIVal<-81)
				RSSIVal = -127;
			else
				RSSIVal -= 45;
			
			SIGNAL_EVENT(PHYLayer_ED_Confirm(PHY_SUCCESS,RSSIVal))
			
		}
		
	}
	
	// request SET_TRX_STATE
	if(CC2420_OPERATION_IS(PHY_OPERATION_SET_TRX_STATE))
	{
		CC2420_STOP_OPERATION(PHY_OPERATION_SET_TRX_STATE)
		CC2420_STOP_OPERATION(PHY_OPERATION_DEFERED)
		
		switch(CC2420Defs.NewTRXState)
		{
			// rx on
			case PHY_RX_ON:
				// it is altready in this state
				if(CC2420Defs.State==CC2420_STATE_RX||
				   CC2420Defs.State==CC2420_STATE_RX_GOT_SFD)
				{
					SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_RX_ON))
					
				}
				// it is transmitting packet
				else if(CC2420Defs.State==CC2420_STATE_TX_GOT_SFD)
				{
					CC2420_DEFER_OPERATION
					SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_BUSY_TX))
					
				}
				// set rx on state
				else
				{
					CC2420_SendCommandStrobe(CC2420_SRXON);
					CC2420Defs.State = CC2420_STATE_RX;
					SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_SUCCESS))
					
				}
				break;
			
			// rx on, reject all frames
			case PHY_RX_ON_REJECT_ALL:
				// if it is already in this state
				if(CC2420Defs.State==PHY_RX_ON_REJECT_ALL)
				{
					SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_RX_ON_REJECT_ALL))
					
				}
				// it is transmitting packet
				else if(CC2420Defs.State==CC2420_STATE_TX_GOT_SFD)
				{
					CC2420_DEFER_OPERATION
					SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_BUSY_TX))
					
				}
				// set rx on reject all state
				else
				{
					CC2420Defs.State = CC2420_STATE_RX_REJECT_ALL;
					CC2420_SendCommandStrobe(CC2420_SRXON);
					SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_SUCCESS))
					
				}
				break;
			
			// tx on
			case PHY_TX_ON:
				// it is altready in this state
				if(CC2420Defs.State==CC2420_STATE_TX||
				   CC2420Defs.State==CC2420_STATE_TX_GOT_SFD)
				{
					SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_TX_ON))
					
				}
				// it is receiving packet
				else if(CC2420Defs.State==CC2420_STATE_RX_GOT_SFD)
				{
					CC2420_DEFER_OPERATION
					SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_BUSY_RX))
					
				}
				//set tx on state
				else
				{
					CC2420_SendCommandStrobe(CC2420_SRFOFF);
					CC2420Defs.State = CC2420_STATE_TX;
					SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_SUCCESS))
					
				}
				break;
			
			// trx off
			case PHY_TRX_OFF:
				// it is altready in this state
				if(CC2420Defs.State==CC2420_STATE_IDLE)
				{
					SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_TRX_OFF))
					
				}
				// it is receiving packet
				else if(CC2420Defs.State==CC2420_STATE_RX_GOT_SFD)
				{
					CC2420_DEFER_OPERATION
					SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_BUSY_RX))
					
				}
				// it is transmitting packet
				else if(CC2420Defs.State==CC2420_STATE_TX_GOT_SFD)
				{
					CC2420_DEFER_OPERATION
					SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_BUSY_TX))
					
				}
				//change state
				else
				{
					CC2420_SendCommandStrobe(CC2420_SRFOFF);
					CC2420Defs.State = CC2420_STATE_IDLE;
					SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_SUCCESS))
					
				}
				break;
			
			// force trx off
			case PHY_FORCE_TRX_OFF:
				CC2420_SendCommandStrobe(CC2420_SRFOFF);
				CC2420Defs.State = CC2420_STATE_IDLE;
				SIGNAL_EVENT(PHYLayer_SETTRXSTATE_Confirm(PHY_SUCCESS))
				break;
			
			// default
			default:
				break;
			
		}
		
	}
	
	// request GET
	if(CC2420_OPERATION_IS(PHY_OPERATION_GET))
	{
		CC2420_STOP_OPERATION(PHY_OPERATION_GET)
		
		switch(CC2420Defs.GetPIBAttribute)
		{
			// get current channel
			case PHY_PIB_CURRENT_CHANNEL_ID:
				SIGNAL_EVENT(PHYLayer_GET_Confirm(PHY_SUCCESS,PHY_PIB_CURRENT_CHANNEL_ID,
				                                     CC2420Defs.Channel))
				break;
			
			// get channels mask
			case PHY_PIB_CHANNELS_SUPPORTED_ID:
				SIGNAL_EVENT(PHYLayer_GET_Confirm(PHY_SUCCESS,PHY_PIB_CHANNELS_SUPPORTED_ID,
				                                     CC2420Defs.SupportedChannels))
				break;
			
			// get tx power
			case PHY_PIB_TX_POWER_ID:
				SIGNAL_EVENT(PHYLayer_GET_Confirm(PHY_SUCCESS,PHY_PIB_TX_POWER_ID,
				                                     CC2420Defs.TxPower))
				break;
			
			// get CCA mode
			case PHY_PIB_CCA_MODE_ID:
				SIGNAL_EVENT(PHYLayer_GET_Confirm(PHY_SUCCESS,PHY_PIB_CCA_MODE_ID,
				                                     CC2420Defs.CCAMode))
				break;
			
			// wrong attribute
			default:
				SIGNAL_EVENT(PHYLayer_GET_Confirm(PHY_UNSUPPORTED_ATTRIBUTE,0,0))
				break;
			
		}
		
	}
	
	// request SET
	if(CC2420_OPERATION_IS(PHY_OPERATION_SET))
	{
		CC2420_STOP_OPERATION(PHY_OPERATION_SET)
		
		if(CC2420Defs.Operation&(1<<PHY_OPERATION_ATTR_VALID))
		{
			// set attribute value
			switch(CC2420Defs.SetPIBAttribute)
			{
				// channel
				case PHY_PIB_CURRENT_CHANNEL_ID:
					
					Val = CC2420_ReadRegister(CC2420_FSCTRL);
					Val &= (~0x01FF);
					Val |= 0x01FF & (357 + 5*(CC2420Defs.Channel - 11));
					
					// turn off the TRX to set new channel
					CC2420_SendCommandStrobe(CC2420_SRFOFF);
					CC2420_WriteRegister(CC2420_FSCTRL,Val);
					
					// if TRX is in rx state, then restore it
					if(CC2420Defs.State==CC2420_STATE_RX||
					   CC2420Defs.State==CC2420_STATE_RX_GOT_SFD)
						CC2420_SendCommandStrobe(CC2420_SRXON);
					
					SIGNAL_EVENT(PHYLayer_SET_Confirm(PHY_SUCCESS,PHY_PIB_CURRENT_CHANNEL_ID))
					
					break;
				
				// supported channels
				case PHY_PIB_CHANNELS_SUPPORTED_ID:
					SIGNAL_EVENT(PHYLayer_SET_Confirm(PHY_SUCCESS,PHY_PIB_CHANNELS_SUPPORTED_ID))
					break;
				
				// tx power
				case PHY_PIB_TX_POWER_ID:
					// change tx power
					Val = CC2420_ReadRegister(CC2420_TXCTRL);
					Val &= 0xFFE0;
					Val |= (CC2420Defs.TxPower&0x1F);
					CC2420_WriteRegister(CC2420_TXCTRL,Val);
					
					SIGNAL_EVENT(PHYLayer_SET_Confirm(PHY_SUCCESS,PHY_PIB_TX_POWER_ID))
					break;
				
				// CCA mode
				case PHY_PIB_CCA_MODE_ID:
					//change CCA mode
					Val = CC2420_ReadRegister(CC2420_MDMCTRL0);
					Val &= 0xFF3F;
					Val |= (CC2420Defs.CCAMode<<6);
					CC2420_WriteRegister(CC2420_MDMCTRL0,Val);
					
					SIGNAL_EVENT(PHYLayer_SET_Confirm(PHY_SUCCESS,PHY_PIB_CCA_MODE_ID))
					
					break;
				
				// unsupported attribute
				default:
					SIGNAL_EVENT(PHYLayer_SET_Confirm(PHY_UNSUPPORTED_ATTRIBUTE,0))
					break;
				
			}
			
		}
		else
			SIGNAL_EVENT(PHYLayer_SET_Confirm(PHY_INVALID_PARAMETER,PHY_PIB_CURRENT_CHANNEL_ID))
		
	}
	
	// handle radio states
	switch(CC2420Defs.State)
	{
		// power down
		case CC2420_STATE_POWER_DOWN:
			// reset
			CC2420_WriteRegister(CC2420_MAIN,0x7800);
			CC2420_WriteRegister(CC2420_MAIN,0xF800);
			
			// switch on osc
			CC2420_SendCommandStrobe(CC2420_SXOSCON);
			
			// change state to power down with vreg on
			CC2420Defs.State = CC2420_STATE_OSC_ENABLE_WAITING;
			break;
		
		// waiting for osc to become stable
		case CC2420_STATE_OSC_ENABLE_WAITING:
			// if XOSC16M_STABLE is set then state of C2420 is IDLE
			if(!(CC2420_SendCommandStrobe(CC2420_SNOP)&(1<<CC2420_STATUS_XOSC16M_STABLE)))
				break;
			
			// configure CC2420
			CC2420_Configure();
			
			// set channel
			Val = CC2420_ReadRegister(CC2420_FSCTRL);
			Val &= (~0x01FF);
			Val |= 0x01FF & (357 + 5*(CC2420Defs.Channel - 11));
			CC2420_SendCommandStrobe(CC2420_SRFOFF);
			CC2420_WriteRegister(CC2420_FSCTRL,Val);
			CC2420_SendCommandStrobe(CC2420_SRXON);
			
			// change state
			CC2420Defs.State = CC2420_STATE_IDLE;
			
			// signal "radio state changed" event
			SIGNAL_EVENT(Radio_StateChanged(RADIO_STATE_POWER_UP))
			
			break;
		
		// receiving
		case CC2420_STATE_RX_READING:
			// get data from rx fifo
			SPI_Start(CC2420Defs.SPI);
			
			SPI_TxRx(CC2420Defs.SPI,(CC2420_RXFIFO|(1<<CC2420_READ_WRITE_BIT)),NULL);
			
			SPI_TxRx(CC2420Defs.SPI,0,&Byte);
			CC2420Defs.RxLen = Byte;
			
			// check data length
			if(CC2420Defs.RxLen>PHY_A_MAX_PHY_PACKET_SIZE)
			{
				SPI_Stop(CC2420Defs.SPI);
				
				// if length is wrong, then flush the RXFIFO
				CC2420_SendCommandStrobe(CC2420_SFLUSHRX);
				CC2420_SendCommandStrobe(CC2420_SFLUSHRX);
				
				// exit
				break;
			}
			
			// get data
			for(i=0;i<CC2420Defs.RxLen;++i)
			{
				SPI_TxRx(CC2420Defs.SPI,0,&Byte);
				CC2420Defs.RxData[i] = Byte;
			}
			
			SPI_Stop(CC2420Defs.SPI);
			
			if(CC2420Defs.RxLen!=0)
			{
				
				#ifdef PHY_LAYER_HANDLE_CHECKSUM
				// get LQI
				LQI = CC2420Defs.RxData[CC2420Defs.RxLen-2];
				if(LQI&0x80)
					LQI = ~LQI;
				CC2420Defs.RxLen -= 2;
				CC2420Defs.LQValues = *((uint16_t*)&CC2420Defs.RxData[CC2420Defs.RxLen]);
				#else
				// get LQI
				LQI = (uint8_t)CC2420_ReadRegister(CC2420_RSSI);
				if(LQI&0x80)
					LQI = ~LQI;
				CC2420Defs.LQValues = LQI;
				#endif
				
				// signal data indication
				SIGNAL_EVENT(PHYLayer_DATA_Indication(CC2420Defs.RxLen,(uint8_t*)CC2420Defs.RxData,LQI))
				
			}
			
			// flush rx fifo
			CC2420_SendCommandStrobe(CC2420_SFLUSHRX);
			CC2420_SendCommandStrobe(CC2420_SFLUSHRX);
			
			// change state
			CC2420Defs.State = CC2420_STATE_RX;
			
			// if there is defered state change, then change the state
			if(CC2420_OPERATION_IS(PHY_OPERATION_DEFERED))
			{
				CC2420_STOP_OPERATION(PHY_OPERATION_DEFERED)
				
				switch(CC2420Defs.NewTRXState)
				{
					case PHY_TX_ON:
						CC2420_SendCommandStrobe(CC2420_SRFOFF);
						CC2420Defs.State = CC2420_STATE_TX;
						break;
					
					case PHY_TRX_OFF:
						CC2420_SendCommandStrobe(CC2420_SRFOFF);
						CC2420Defs.State = CC2420_STATE_IDLE;
						break;
					
					default:
						break;
					
				}
				
			}
			
			break;
		
		// receiving and rejecting all frames
		case CC2420_STATE_RX_REJECT_ALL:
			// flush RXFIFO
			CC2420_SendCommandStrobe(CC2420_SFLUSHRX);
			CC2420_SendCommandStrobe(CC2420_SFLUSHRX);
			break;
		
		// transmitting
		case CC2420_STATE_TX_GOT_SFD:
			if(CC2420_SendCommandStrobe(CC2420_SNOP)&(1<<CC2420_STATUS_TX_ACTIVE))
				break;
			
			CC2420_SendCommandStrobe(CC2420_SRFOFF);
			CC2420Defs.State = CC2420_STATE_TX;
			
			// if there is defered state change then change the state
			if(CC2420_OPERATION_IS(PHY_OPERATION_DEFERED))
			{
				CC2420_STOP_OPERATION(PHY_OPERATION_DEFERED)
				
				switch(CC2420Defs.NewTRXState)
				{
					case PHY_RX_ON:
						CC2420_SendCommandStrobe(CC2420_SRXON);
						CC2420Defs.State = CC2420_STATE_RX;
						break;
					
					case PHY_RX_ON_REJECT_ALL:
						CC2420_SendCommandStrobe(CC2420_SRXON);
						CC2420Defs.State = CC2420_STATE_RX_REJECT_ALL;
						break;
					
					case PHY_TRX_OFF:
						CC2420Defs.State = CC2420_STATE_IDLE;
						break;
					
					default:
						break;
					
				}
				
			}
			
			// confirm data request with status success
			SIGNAL_EVENT(PHYLayer_DATA_Confirm(PHY_SUCCESS))
			
			break;
		
		default:
			break;
		
	}
	
}

/*******************************************************************************//**
 * @implements NWK_GetLQVals
 **********************************************************************************/
uint16_t NWK_GetLQVals(void)
{
	return CC2420Defs.LQValues;
}

/*******************************************************************************//**
 * @implements Radio_SetState
 **********************************************************************************/
RESULT Radio_SetState(RADIO_TRANSCEIVER_STATE State)
{
	// turn on the radio transceiver
	if(State==RADIO_STATE_POWER_UP&&CC2420Defs.State==CC2420_STATE_VREG_OFF)
	{
		// change state
		CC2420Defs.State = CC2420_STATE_VREG_WAITING_ON;
		
		// enable voltage regulator
		CC2420_VRegSwitchOn();
		
		// start timer
		Timer_Start(CC2420Defs.Timer,TIMER_ONE_SHOT_MODE,MS(CC2420_WAIT_TIME));
		
		// return success
		return SUCCESS;
		
	}
	// turn it off
	else if(State==RADIO_STATE_POWER_DOWN&&
	        CC2420Defs.State!=CC2420_STATE_VREG_OFF&&
	        CC2420Defs.State!=CC2420_STATE_VREG_WAITING_OFF)
	{
		// stop timer
		Timer_Stop(CC2420Defs.Timer);
		
		// reset
		CC2420_WriteRegister(CC2420_MAIN,0x7800);
		CC2420_WriteRegister(CC2420_MAIN,0xF800);
		
		// switch off osc
		CC2420_SendCommandStrobe(CC2420_SXOSCOFF);
		
		// disable voltage regulator
		CC2420_VRegSwitchOff();
		
		// change state
		CC2420Defs.State = CC2420_STATE_VREG_WAITING_OFF;
		
		// start timer
		Timer_Start(CC2420Defs.Timer,TIMER_ONE_SHOT_MODE,MS(CC2420_WAIT_TIME));
		
		// return success
		return SUCCESS;
		
	}
	
	// radio transceiver is already in this state
	return FAIL;
}

/*******************************************************************************//**
 * @implements Radio_GetState
 **********************************************************************************/
RADIO_TRANSCEIVER_STATE Radio_GetState(void)
{
	if(CC2420Defs.State<CC2420_STATE_IDLE)
		return RADIO_STATE_POWER_DOWN;
	
	return RADIO_STATE_POWER_UP;
}

#ifdef USE_PWR
/*******************************************************************************//**
 * @implements Radio_PowerSave
 **********************************************************************************/
void Radio_PowerSave(void)
{
	CC2420_PowerSave();
	
	// no need to turn off
	if(Radio_GetState()==RADIO_STATE_POWER_DOWN)
		return;
	
	// reset
	CC2420_WriteRegister(CC2420_MAIN,0x7800);
	CC2420_WriteRegister(CC2420_MAIN,0xF800);
	
	// switch off osc
	CC2420_SendCommandStrobe(CC2420_SXOSCOFF);
	
	// disable voltage regulator
	CC2420_VRegSwitchOff();
	
}

/*******************************************************************************//**
 * @implements Radio_Restore
 **********************************************************************************/
void Radio_Restore(void)
{
	CC2420_Restore();
	
	// no need to turn on
	if(Radio_GetState()==RADIO_STATE_POWER_DOWN)
		return;
	
	// enable voltage regulator
	CC2420_VRegSwitchOn();
	
	// wait some time
	volatile uint16_t i;
	for(i=0;i<2048;++i);
	
	// reset
	CC2420_WriteRegister(CC2420_MAIN,0x7800);
	CC2420_WriteRegister(CC2420_MAIN,0xF800);
	
	// switch on osc
	CC2420_SendCommandStrobe(CC2420_SXOSCON);
	
	// wait for XOSC16M to become stable
	while(!(CC2420_SendCommandStrobe(CC2420_SNOP)&(1<<CC2420_STATUS_XOSC16M_STABLE)));
	
	// configure CC2420
	CC2420_Configure();
	
}
#endif

/*******************************************************************************//**
 * @implements PHYLayer_Init
 **********************************************************************************/
RESULT PHYLayer_Init(void)
{
	// init CC2420
	if(CC2420_Init()==FAIL)
		return FAIL;
	
	CC2420Defs.State              = CC2420_STATE_VREG_OFF;
	
	CC2420Defs.Channel            = DEFAULT_CHANNEL;
	CC2420Defs.SupportedChannels  = 0x07FFF800;
	CC2420Defs.TxPower            = 0xBF;
	CC2420Defs.CCAMode            = 3;
	
	CC2420Defs.GetPIBAttribute    = PHY_PIB_CCA_MODE_ID;
	CC2420Defs.SetPIBAttribute    = PHY_PIB_CCA_MODE_ID;
	
	CC2420Defs.LastSFDTime = 0;
	CC2420Defs.Operation   = 0;
	CC2420Defs.NewTRXState = PHY_SUCCESS;
	CC2420Defs.LQValues    = 0;
	
	CC2420Defs.TxData = NULL;
	CC2420Defs.TxLen  = 0;
	CC2420Defs.RxLen  = 0;
	
	// init SPI interface
	CC2420Defs.SPI = SPI_Open(CC2420_SPI_CHANNEL,SPI_MODE_MASTER|SPI_TRANSMISSION_MODE_SYNC,NULL);
	if(IS_INVALID_HANDLE(CC2420Defs.SPI))
		return FAIL;
	
	// create timer
	CC2420Defs.Timer = Timer_Create(CC2420_TimerFired,NULL);
	if(IS_INVALID_HANDLE(CC2420Defs.Timer))
		return FAIL;
	
	// create thread
	CC2420Defs.Thread = Thread_Create(CC2420_ThreadProc,NULL);
	if(IS_INVALID_HANDLE(CC2420Defs.Thread))
		return FAIL;
	
	// start thread
	if(Thread_Start(CC2420Defs.Thread,THREAD_PROCESS_MODE)==FAIL)
		return FAIL;
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements PHYLayer_DATA_Request
 **********************************************************************************/
RESULT PHYLayer_DATA_Request(uint8_t Length,uint8_t *Data)
{
	if(CC2420Defs.State<CC2420_STATE_IDLE)
		return FAIL;
	
	if(Data==NULL||Length==0)
		return FAIL;
	
	CC2420Defs.TxData = Data;
	CC2420Defs.TxLen  = Length;
	CC2420Defs.Operation |= 1<<PHY_OPERATION_REQUEST_DATA;
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements PHYLayer_CCA_Request
 **********************************************************************************/
RESULT PHYLayer_CCA_Request(void)
{
	// check CC2420 state
	if(CC2420Defs.State<CC2420_STATE_IDLE)
		return FAIL;
	
	CC2420Defs.Operation |= 1<<PHY_OPERATION_CCA;
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements PHYLayer_ED_Request
 **********************************************************************************/
RESULT PHYLayer_ED_Request(void)
{
	// check CC2420 state
	if(CC2420Defs.State<CC2420_STATE_IDLE)
		return FAIL;
	
	CC2420Defs.Operation |= 1<<PHY_OPERATION_ED;
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements PHYLayer_GET_Request
 **********************************************************************************/
RESULT PHYLayer_GET_Request(PHY_PIB_ATTRIBUTE_PARAM PIBAttribute)
{
	// check CC2420 state
	if(CC2420Defs.State<CC2420_STATE_IDLE)
		return FAIL;
	
	CC2420Defs.GetPIBAttribute = PIBAttribute;
	CC2420Defs.Operation |= 1<<PHY_OPERATION_GET;
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements PHYLayer_SETTRXSTATE_Request
 **********************************************************************************/
RESULT PHYLayer_SETTRXSTATE_Request(PHY_ENUM State)
{
	// check CC2420 state
	if(CC2420Defs.State<CC2420_STATE_IDLE)
		return FAIL;
	
	CC2420Defs.NewTRXState = State;
	CC2420Defs.Operation |= 1<<PHY_OPERATION_SET_TRX_STATE;
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements PHYLayer_SET_Request
 **********************************************************************************/
RESULT PHYLayer_SET_Request(PHY_PIB_ATTRIBUTE_PARAM PIBAttribute,
                            uint32_t PIBAttributeValue)
{
	// check CC2420 state
	if(CC2420Defs.State<CC2420_STATE_IDLE)
		return FAIL;
	
	CC2420Defs.SetPIBAttribute = PIBAttribute;
	CC2420Defs.Operation |= (1<<PHY_OPERATION_SET);
	CC2420Defs.Operation |= (1<<PHY_OPERATION_ATTR_VALID);
	
	// set attribute value
	switch(CC2420Defs.SetPIBAttribute)
	{
		// channel
		case PHY_PIB_CURRENT_CHANNEL_ID:
			// check channel
			if(PIBAttributeValue<11||PIBAttributeValue>26)
				CC2420Defs.Operation &= ~(1<<PHY_OPERATION_ATTR_VALID);
			else if(!(CC2420Defs.SupportedChannels&(1<<PIBAttributeValue)))
				CC2420Defs.Operation &= ~(1<<PHY_OPERATION_ATTR_VALID);
			else
				CC2420Defs.Channel = (uint8_t)PIBAttributeValue;
			break;
		
		// supported channels
		case PHY_PIB_CHANNELS_SUPPORTED_ID:
			CC2420Defs.SupportedChannels = PIBAttributeValue;
			break;
		
		// tx power
		case PHY_PIB_TX_POWER_ID:
			CC2420Defs.TxPower = (uint8_t)PIBAttributeValue;
			break;
		
		// CCA mode
		case PHY_PIB_CCA_MODE_ID:
			// check CCA mode
			if(PIBAttributeValue==0||PIBAttributeValue>3)
				CC2420Defs.Operation &= ~(1<<PHY_OPERATION_ATTR_VALID);
			else
				CC2420Defs.CCAMode = (uint8_t)PIBAttributeValue;
			break;
		
	}
	
	// return success
	return SUCCESS;
}

/*******************************************************************************//**
 * @implements CC2420_SFDReceived
 **********************************************************************************/
uint64_t PHYLayer_GetLastSFDTime(void)
{
	return CC2420Defs.LastSFDTime;
}

/*******************************************************************************//**
 * @implements CC2420_SFDReceived
 **********************************************************************************/
EVENT CC2420_SFDReceived(void)
{
	if(CC2420Defs.State==CC2420_STATE_RX)
	{
		CC2420Defs.LastSFDTime = GetTime();
		CC2420Defs.State = CC2420_STATE_RX_GOT_SFD;
		
	}
	else if(CC2420Defs.State==CC2420_STATE_TX)
	{
		CC2420Defs.LastSFDTime = GetTime();
		CC2420Defs.State = CC2420_STATE_TX_GOT_SFD;
		
	}
	
}

/*******************************************************************************//**
 * @implements CC2420_FIFOPReceived
 **********************************************************************************/
EVENT CC2420_FIFOPReceived(void)
{
	// check current state
	if(CC2420Defs.State!=CC2420_STATE_RX_GOT_SFD)
		return;
	
	CC2420Defs.State = CC2420_STATE_RX_READING;
	
}
