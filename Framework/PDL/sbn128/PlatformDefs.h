/**
 * @file PlatformDefs.h
 * Constants and data types definition header.
 * @author Nezametdinov I.E.
 */

#ifndef __PLATFORM_DEFS_H__
#define __PLATFORM_DEFS_H__

#include <inttypes.h>

/// TWI defs
#define TWI_PORT PORTD
#define TWI_PIN  PIND
#define TWI_DDR  DDRD
#define TWI_SCL 1
#define TWI_SDA 2

/// OWI defs
#define OWI_PORT    PORTF
#define OWI_PIN     PINF
#define OWI_DDR     DDRF
#define OWI_PIN_NUM 3

/// time to wait before vreg is ready
#ifndef CC2420_WAIT_TIME
#define CC2420_WAIT_TIME 20
#endif

/// PHY layer shall handle CRC
#define PHY_LAYER_HANDLE_CHECKSUM

/// spi channel for CC2420
#ifndef CC2420_SPI_CHANNEL
#define CC2420_SPI_CHANNEL 0
#endif

#define SHT11_TWI_CHANNEL 0

#define NUM_SENSORS 2
/// sensors
enum
{
	/// temperature sensor
	TEMPERATURE_SENSOR = 0,
	/// humidity sensor
	HUMIDITY_SENSOR    = 1
};

#endif
