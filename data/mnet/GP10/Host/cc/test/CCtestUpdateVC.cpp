// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : CCtestUpdateVC.cpp
// Author(s)   : Igal Gutkin
// Create Date : 11-01-98
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included VxWorks headers
#include <vxWorks.h>

#include "jcc/JCCVcMsg.h"
#include "voip/vblink.h"

#include "CC/CCTypes.h"

bool sendVcFailNotification (unsigned short);

bool sendVcFailNotification (unsigned short cellId)
{
	BOOL             retVal ;
	VOIP_API_MESSAGE voipMsg;
	char             vcName [256];

	sprintf ((char *)vcName, "%d", cellId);

	if ((retVal = CcVBLinkCellStatusUpdate (vcName, VC_STATUS_DOWN)) == TRUE)
	{
		printf ("DBG: Using test function for sending ViperCell Num %s Failure Msg.\n",
		        vcName                                                                );
	}
	else
	{
		printf ("DBG: Failed to send ViperCell Num %s Failure Msg to CC using test function.\n",
		        vcName                                                                         );
	}

	return retVal;
}

