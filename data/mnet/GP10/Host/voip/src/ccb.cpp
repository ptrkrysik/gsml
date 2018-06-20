
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: ccb.cpp														*
 *																						*
 *	Description			: The Call control block manipulation functions					*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |03/24/99 | DSN    | File created												*
 *		 |		   |		|															*
 *		 |		   |	    |															*
 ****************************************************************************************
 */
#include <stdlib.h>
#include <string.h>
#include <cm.h>
#include <stkutils.h>

#include <tickLib.h>
#include <drv\timer\timerDev.h>
#include <taskLib.h>
#include <seli.h>

#include "logging\vcmodules.h"
#include "logging\vclogging.h"
#include "vipermem.h"
#include "h323task.h"
#include "ccb.h"

extern PH323TASK_CONFIG	pVoipTaskConfig;
extern CALL_CONTROL_BLOCK	CCBArray[MAX_ACTIVE_CALLS];

static int CCBCount=0;

/*
 ************ Call Control Block Management functions *************************
 */


void
InitializeCallControlBlocks()
{
	DBG_FUNC("InitializeCallControlBlocks",H323_LAYER);
	DBG_ENTER();

	PCALL_CONTROL_BLOCK NewCCB;
	int Count;

	for(Count=0;Count<MAX_ACTIVE_CALLS;Count++)
	{
		NewCCB = &CCBArray[Count];
		InitializeCCB(NewCCB,Count);
	}
	DBG_LEAVE();
	return;
}



/*
 * GetCCBFromPool
 */
PCALL_CONTROL_BLOCK GetCCBFromPool()
{
	DBG_FUNC("GetCCBFromPool",H323_LAYER);
	DBG_ENTER();
	PCALL_CONTROL_BLOCK pCCB=NULL;
	int Count;
	HJCRTPSESSION	RtpHandle;

	LOCK_CCB_LIST();
	for(Count=0;Count<MAX_ACTIVE_CALLS;Count++)
	{
		pCCB = &CCBArray[Count];
		if (pCCB->Used == FALSE) break;
		else continue;
	}
	if (pCCB->Used == TRUE) 
	{
		DBG_ERROR("All CCBs exhausted!! total %d\n",Count);
		pCCB = NULL;
	}
	else 
	{
		RtpHandle = pCCB->RtpHandle;
		InitializeCCB(pCCB,pCCB->Index);
		pCCB->RtpHandle = RtpHandle;
		pCCB->Used = TRUE;
	}

	UNLOCK_CCB_LIST();

	DBG_LEAVE();
	return pCCB;
}




/*
 * ReturnCCBToPool
 */
void ReturnCCBToPool(PCALL_CONTROL_BLOCK pCCB)
{
	DBG_FUNC("ReturnCCBToPool",H323_LAYER);
	DBG_ENTER();
	HJCRTPSESSION	RtpHandle;

	LOCK_CCB_LIST();

	RtpHandle = pCCB->RtpHandle;

	InitializeCCB(pCCB,pCCB->Index);
    pCCB->LastTxnId     = VOIP_API_NULL_TXN_ID;
	pCCB->Used          = FALSE;
	pCCB->RtpHandle     = RtpHandle;
    pCCB->CanDropCall   = FALSE;
	UNLOCK_CCB_LIST();

	DBG_LEAVE();
	return;
}






/*
 * InitializeCCB
 */
void
InitializeCCB(PCALL_CONTROL_BLOCK pCCB,short Index)
{
	STATE_TRANSITION_HISTORY	HistoryBackup;
    
    /*
     * Stash the history for now; remember, we always need it !!!
     */
    memcpy(&HistoryBackup,&pCCB->StateTransitionHistory,sizeof(STATE_TRANSITION_HISTORY));

	memset(pCCB,0,sizeof(CALL_CONTROL_BLOCK));
	pCCB->H323CallState     = H323_CALL_STATE_IDLE;
	pCCB->LudbId            = VOIP_NULL_MOBILE_HANDLE;
	pCCB->Magic             = H323TASK_MAGIC_VALUE;
	pCCB->H323CRV			= 0;
	pCCB->Used              = FALSE;
    pCCB->CanDropCall       = FALSE;
    pCCB->Index             = Index;
	pCCB->Q931Connected		= FALSE;
	pCCB->CodecInfoSent		= FALSE;
	pCCB->OutChannelConnected = FALSE;
	pCCB->IsSuppCall		= FALSE;
	pCCB->CallTransferred	= FALSE;
    /*
     * Now copy back the history 
     */
    memcpy(&pCCB->StateTransitionHistory,&HistoryBackup,sizeof(STATE_TRANSITION_HISTORY));

}



/*
 * GetCCBByTpktHandle
 */
PCALL_CONTROL_BLOCK 
GetCCBByTpktHandle(HPROTCONN TpktHandle)
{
	DBG_FUNC("GetCCBByTpktHandle",H323_LAYER);
	DBG_ENTER();
	PCALL_CONTROL_BLOCK pCCB=NULL;
	int Count;

	LOCK_CCB_LIST();
	for(Count=0;Count<MAX_ACTIVE_CALLS;Count++)
	{
		pCCB = &CCBArray[Count];
		if (pCCB->Used == TRUE) {
            if (pCCB->TpktHandle == TpktHandle) {
                DBG_LEAVE();
                return (pCCB);
            }
        }
		else continue;
	}

	UNLOCK_CCB_LIST();

	DBG_LEAVE();
	return pCCB;
}
