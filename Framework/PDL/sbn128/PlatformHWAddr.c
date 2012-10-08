/**
 * @file PlatformHWAddr.c
 * Hardware address implementation source file.
 * @author Nezametdinov I.E.
 */

#include "../PIL/NWK/MAC/MACLayer.h"
#include "../DRIVERS/DS2401/DS2401.h"
#include "../PIL/Hardware.h"

/*******************************************************************************//**
 * @implements MACLayer_GetHWAddr
 **********************************************************************************/

  MAC_EXTENDED_ADDR MACLayer_GetHWAddr(void)
{
	MAC_EXTENDED_ADDR HWAddr;
	int i;
	uint8_t *c;
	
	for(i=0;i<8;i++)
	{
	c=((uint8_t*)&HWAddr)+i;
	*c=MAC[i];
	}
	return HWAddr;
}