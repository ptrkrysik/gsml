/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_GETIMSIMOD1000_CPP__
#define __RM_GETIMSIMOD1000_CPP__

#include "rm\rm_head.h"

u16 rm_GetImsiMod1000(rm_IeImsi_t *pImsi)
{

  RDEBUG__(("ENTER-rm_GetImsiMod1000\n"));

  return (
	  ( pImsi->digits[pImsi->numDigits-1] & 0x0f )      + 
	  ( pImsi->digits[pImsi->numDigits-2] & 0x0f ) *10  + 
	  ( pImsi->digits[pImsi->numDigits-3] & 0x0f ) *100 
         );

} /* End of rm_GetImsiMod1000() */

#endif /* __RM_GETIMSIMOD1000_CPP__ */
