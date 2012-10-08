/**
 * @file Utils.h
 * Utils header.
 * @author Nezametdinov I.E.
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include "../PIL/Defs.h"

/*******************************************************************************//**
 * inits utils
 * @return SUCCESS if utils successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT Utils_Init(void);

/*******************************************************************************//**
 * sets seed for random value
 * @param[in] Seed seed
 **********************************************************************************/
void Utils_Seed(uint8_t Seed);

/*******************************************************************************//**
 * generates random value
 * @param[in] MaxValue max value wich may be generated
 * @return random value
 **********************************************************************************/
uint8_t Utils_Rand(uint8_t MaxValue);

/*******************************************************************************//**
 * computes ITU-T CRC16
 * @param[in] Length data length
 * @param[in] Data   data
 * @return CRC16
 **********************************************************************************/
uint16_t Utils_ITUTCRC16(uint8_t Length,uint8_t *Data);

/*******************************************************************************//**
 * computes 8 bit checksum
 * @param[in] Length data length
 * @param[in] Data   data
 * @return checksum
 **********************************************************************************/
uint8_t Utils_CKSUM(uint8_t Length,uint8_t *Data);

#endif
