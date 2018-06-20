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
// File        : MMBssgpMsgProc.cpp
// Author(s)   : 
// Create Date : 12-18-2000 
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "taskLib.h"
#include "stdLib.h"
#include "string.h"
#include "tickLib.h"
#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"
#include "oam_api.h"
#include "JCC/JCCUtil.h"
#include "ril3/ril3_common.h"
#include "JCC/LUDBapi.h"
// included MD and IRT headers for messages from MS
#if defined(_NO_ED) || defined(_SOFT_MOB)
	#include "JCC/JCC_ED.h"
#else
	#define JCC_RIL3MM_Decode CNI_RIL3MM_Decode
#endif

// MM Message Constants
#include "MMTypes.h"
#include "MM/MMInt.h"
#include "MMUtil.h"
#include "MMMdMsgProc.h"
#include "MMSendMsg.h"
#include "MMPmProc.h"
#include "MMLudbMsgProc.h"
#include "MMRmMsgProc.h"

// Global Variable for current MM Connections
extern MMEntry_t mmEntries[];
extern T_CNI_RIL3_IE_LOCATION_AREA_ID	mmCfgLocId;


char bssgpOutMsg[2048];


// ****************************************************************************
void MM_Bssgp_LocUpdateReqProc(BSSGP_LOCATION_UPDATE_MSG *locUpdReq)
{	
    DBG_FUNC("MM_Bssgp_LocUpdateReqProc", MM_LAYER);
	DBG_ENTER();

	short	mmId;

    mmIncIMSIAttach();			// PM report
    if (locUpdReq->imsi.mobileIdType != CNI_RIL3_IMSI)
    {		
        DBG_ERROR("MM_Bssgp_LocUpdateReqProc: invalid  mobileIdType %d\n ",
            locUpdReq->imsi.mobileIdType);

		DBG_LEAVE();
		return;
    }

	if ( (mmId = entryAlloc()) == JCC_ERROR )
	{
		DBG_ERROR("MM_Bssgp_LocUpdateReqProc: entryAlloc Failed\n ");

		DBG_LEAVE();
		return;
	}

	mmEntries[mmId].procData.procType = MM_PROC_LUDB;
    mmEntries[mmId].gsInitiated = TRUE;
    mmEntries[mmId].bvci = locUpdReq->bvci.bvci;
  	mmEntries[mmId].mobileId = locUpdReq ->imsi;

    mmIncTrnSubIdIMSI();		// PM report
    sendMobileRegister(mmId);

    DBG_LEAVE();
}

// ****************************************************************************
void MM_Bssgp_MsgProc(BSSGP_API_MSG *msg)
{	
    DBG_FUNC("MM_Bssgp_MsgProc", MM_LAYER);
	DBG_ENTER();

    switch(msg->msg_type)
    {
    case BSSGP_API_MSG_LOCATION_UPDATE:
        MM_Bssgp_LocUpdateReqProc(&msg->msg.location_update);
        break;

    default:
		DBG_ERROR("Only BSSGP_API_MSG_LOCATION_UPDATE expected. Bad msg type: %d\n",
			msg->msg_type);
    }


    DBG_LEAVE();
}

void MM_Bssgp_SendLuAccept(short mmId)
{
    BSSGP_API_MSG msg;
    BSSGP_LOCATION_UPDATE_ACK_MSG &locationUpdateAck = msg.msg.location_update_ack;
    locationUpdateAck.imsi = mmEntries[mmId].mobileId;
    locationUpdateAck.location_area.ie_present = true;
    locationUpdateAck.location_area.mcc[0] = mmCfgLocId.mcc[0];
	locationUpdateAck.location_area.mcc[1] = mmCfgLocId.mcc[1];  
	locationUpdateAck.location_area.mcc[2] = mmCfgLocId.mcc[2];
	locationUpdateAck.location_area.mnc[0] = mmCfgLocId.mnc[0];
	locationUpdateAck.location_area.mnc[1] = mmCfgLocId.mnc[1];
	locationUpdateAck.location_area.mnc[2] = mmCfgLocId.mnc[2];

	locationUpdateAck.location_area.lac = mmCfgLocId.lac;
    
#ifdef NOT_YET
    bssgp_api_send_msg(msg);
#endif
}
          
// ****************************************************************************
int gmm_bssgp_msg_callback(BSSGP_API_MSG bssgpMsg)
{
	DBG_FUNC("gmm_bssgp_msg_callback", MM_LAYER);
	DBG_ENTER();

	if (bssgpMsg.msg_type != BSSGP_API_MSG_LOCATION_UPDATE)
	{
		DBG_ERROR("Only BSSGP_API_MSG_LOCATION_UPDATE expected. Bad msg type: %d\n",
			bssgpMsg.msg_type);
		DBG_LEAVE();
		return 0;
	}

    bssgpOutMsg[0] = MODULE_BSSGP;
    memcpy(&bssgpOutMsg[1], &bssgpMsg, sizeof(bssgpMsg));

    // forward it to mm msg queue
	if (ERROR == msgQSend(mmMsgQId, 
						(char *) bssgpOutMsg, 
						sizeof(BSSGP_API_MSG), 
						NO_WAIT, MSG_PRI_NORMAL)
						)
	{
		DBG_ERROR("gmm_bssgp_msg_callback msgQSend (QID=%d) error\n", (int)mmMsgQId);
        return 0;
	}
  
	
	DBG_LEAVE();
    return TRUE;
}



