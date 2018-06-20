// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : JCCUtil.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include <stdio.h>

// included MD and IRT headers
#include "JCC/JCCUtil.h"
#include "JCC/JCCEvent.h"
#include "JCC/JCCState.h"

// Utility functions

int
operator==(const T_CNI_L3_ID& lhs,  const T_CNI_L3_ID& rhs) {

  return ( (lhs.msgq_id == rhs.msgq_id) &&
	   (lhs.sub_id == lhs.sub_id) );
}



// Mobile Id comparison. -gz
int
operator==(const T_CNI_RIL3_IE_MOBILE_ID& lhs,  
	   const T_CNI_RIL3_IE_MOBILE_ID& rhs)
{

  if ( ( lhs.ie_present && rhs.ie_present) &&
       lhs.mobileIdType  == rhs.mobileIdType )
  {
    if (lhs.mobileIdType == CNI_RIL3_TMSI)
    {
      return (lhs.tmsi == rhs.tmsi);
    }
    else
    {
      if (lhs.numDigits == rhs.numDigits)
      {
	 for (short i = 0; i< lhs.numDigits; i++)
	 {
	    if(lhs.digits[i] != rhs.digits[i])
		return false;
	 }
	 return true;
      }
    }
  }


  return false;
}


// add print mobile Id. -gz

void printMobileId(const T_CNI_RIL3_IE_MOBILE_ID& mobileId)
{
	if (mobileId.mobileIdType == CNI_RIL3_TMSI)
	{
		printf("TMSI ");
	}
	else if (mobileId.mobileIdType == CNI_RIL3_IMSI)
	{
		for (short i = 0; i< mobileId.numDigits; i++)
		{
			printf("%d",  mobileId.digits[i]);
		}
	}
	else
		printf("Unknown ID Type  %d",mobileId.mobileIdType);
}


T_CNI_RIL3_PROTOCOL_DISCRIMINATOR extractPd (const T_CNI_LAPDM_L3MessageUnit *msInMsg)
{
    return ((T_CNI_RIL3_PROTOCOL_DISCRIMINATOR)(msInMsg->buffer[0] & 0x0f));
}

