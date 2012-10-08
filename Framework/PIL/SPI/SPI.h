/**
 * @file SPI.h
 * SPI implementation header.
 * @author Nezametdinov I.E.
 */

#ifndef __SPI_H__
#define __SPI_H__

#include "../../API/SPIAPI.h"

/*******************************************************************************//**
 * inits SPI
 * @return SUCCESS if SPI successfully initialised
 * @return FAIL    otherwise
 **********************************************************************************/
RESULT SPI_Init(void);

/*******************************************************************************//**
 * forces SPI to enter power save mode
 **********************************************************************************/
void SPI_PowerSave(void);

/*******************************************************************************//**
 * restores SPI after leaving power save mode
 **********************************************************************************/
void SPI_Restore(void);

#endif
