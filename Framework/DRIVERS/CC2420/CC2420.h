/**
 * @file CC2420.h
 * CC2420 header.
 * @author Nezametdinov I.E.
 */

#ifndef __CC2420_H__
#define __CC2420_H__

#include "../../PIL/Defs.h"

/// registers and memory access constants
enum
{
	CC2420_RAM_REG_BIT    = 7,
	CC2420_READ_WRITE_BIT = 6
};

/// status bits
enum
{
	CC2420_STATUS_RSSI_VALID     = 1,
	CC2420_STATUS_LOCK           = 2,
	CC2420_STATUS_TX_ACTIVE      = 3,
	CC2420_STATUS_ENC_BUSY       = 4,
	CC2420_STATUS_TX_UNDERFLOW   = 5,
	CC2420_STATUS_XOSC16M_STABLE = 6
};

/// CC2420 strobe registers
typedef enum
{
	CC2420_SNOP     = 0x00,
	CC2420_SXOSCON  = 0x01,
	CC2420_SRXON    = 0x03,
	CC2420_STXON    = 0x04,
	CC2420_STXONCCA = 0x05,
	CC2420_SRFOFF   = 0x06,
	CC2420_SXOSCOFF = 0x07,
	CC2420_SFLUSHRX = 0x08,
	CC2420_SFLUSHTX = 0x09
}CC2420_COMMAND_STROBE;

/// CC2420 registers
typedef enum
{
	CC2420_MAIN     = 0x10,
	CC2420_MDMCTRL0 = 0x11,
	CC2420_RSSI     = 0x13,
	CC2420_TXCTRL   = 0x15,
	CC2420_FSCTRL   = 0x18,
	CC2420_SECCTRL0 = 0x19,
	CC2420_IOCFG0   = 0x1C,
	CC2420_TXFIFO   = 0x3E,
	CC2420_RXFIFO   = 0x3F
}CC2420_REGISTER;

/// CC2420 MDMCTRL0 register bits
enum
{
	CC2420_MDMCTRL0_RESERVED_FRAME_MODE = 13,
	CC2420_MDMCTRL0_PAN_COORDINATOR     = 12,
	CC2420_MDMCTRL0_ADR_DECODE          = 11,
	CC2420_MDMCTRL0_CCA_MODE            = 6,
	CC2420_MDMCTRL0_AUTOCRC             = 5,
	CC2420_MDMCTRL0_AUTOACK             = 4
};

/// CC2420 SECCTRL0 register bits
enum
{
	CC2420_SECCTRL0_RXFIFO_PROTECTION = 9,
	CC2420_SECCTRL0_SEC_MODE_1        = 1,
	CC2420_SECCTRL0_SEC_MODE_0        = 0,
};

/*******************************************************************************//**
 * sends command strobe
 * @param[in] CommandStrobe CC2420 command strobe
 * @return status byte
 **********************************************************************************/
uint8_t CC2420_SendCommandStrobe(CC2420_COMMAND_STROBE CommandStrobe);

/*******************************************************************************//**
 * writes register value
 * @param[in] Register CC2420 register
 * @param[in] Value    register value
 **********************************************************************************/
void CC2420_WriteRegister(CC2420_REGISTER Register,uint16_t Value);

/*******************************************************************************//**
 * reads register value
 * @param[in] Register CC2420 register
 * @return register value
 **********************************************************************************/
uint16_t CC2420_ReadRegister(CC2420_REGISTER Register);

/*******************************************************************************//**
 * inits CC2420
 * @return SUCCESS if CC2420 successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT CC2420_Init(void);

/*******************************************************************************//**
 * forces CC2420 to enter power save mode
 **********************************************************************************/
void CC2420_PowerSave(void);

/*******************************************************************************//**
 * restores CC2420 after leaving power save mode
 **********************************************************************************/
void CC2420_Restore(void);

/*******************************************************************************//**
 * switches on the voltage regulator
 **********************************************************************************/
void CC2420_VRegSwitchOn(void);

/*******************************************************************************//**
 * switches off the voltage regulator
 **********************************************************************************/
void CC2420_VRegSwitchOff(void);

/*******************************************************************************//**
 * returns state of rx fifo
 * @return TRUE  if fifo overflow happened
 * @return FALSE otherwise
 **********************************************************************************/
BOOL CC2420_GetRxFIFOOverflow(void);

/*******************************************************************************//**
 * returns state of FIFO pin
 * @return state of FIFO pin
 **********************************************************************************/
uint8_t CC2420_GetFIFO(void);

/*******************************************************************************//**
 * returns state of CCA pin
 * @return state of CCA pin
 **********************************************************************************/
uint8_t CC2420_GetCCA(void);

/*******************************************************************************//**
 * returns state of SFD pin
 * @return state of SFD pin
 **********************************************************************************/
uint8_t CC2420_GetSFD(void);

/*******************************************************************************//**
 * CC2420 "SFD" event
 **********************************************************************************/
EVENT CC2420_SFDReceived(void);

/*******************************************************************************//**
 * CC2420 "FIFOP" event
 **********************************************************************************/
EVENT CC2420_FIFOPReceived(void);

#endif
