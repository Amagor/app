/**
 * @file PlatformComponents.c
 * Other OS components init source file.
 * @author Nezametdinov I.E.
 */

#include "../../PIL/Components.h"
#include "../../DRIVERS/DS2401/DS2401.h"

/*******************************************************************************//**
 * @implements InitOther
 **********************************************************************************/
RESULT InitOther(void)
{
	// init DS2401
	#ifdef USE_NWK
	#ifdef USE_HWADDR
//	if(DS2401_Init()==FAIL)
	//	return FAIL;
	#endif
	#endif
	
	return SUCCESS;
}
