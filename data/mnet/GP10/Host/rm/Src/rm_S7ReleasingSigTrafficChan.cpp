/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __RM_S7RELEASINGSIGTRAFFICCHAN_CPP__
#define __RM_S7RELEASINGSIGTRAFFICCHAN_CPP__

#include "rm\rm_head.h"

void rm_S7ReleasingSigTrafficChan(void)
{
   //Monitoring entrance to a function 
   RDEBUG__(("ENTER@rm_S7ReleasingSigTrafficChan\n"));
 
   if ( rm_pSmCtxt->smType   != RM_MAIN_SM || 
        rm_pItcRxMdMsg->sapi == RM_SAPI3   )
   {
       //At state S7: Only main signaling i.e. main SM will be attended
       IDEBUG__(( "WARNING@rm_S7ReleasingSigTrafficChan: smType %d, sapi %d\n",
                    rm_pSmCtxt->smType, rm_pItcRxMdMsg->sapi ));
       return;
   }
 
   switch(rm_ItcRxOrigin)
   {
   case MODULE_MD:
	  //Expecting DL_REL_IND in normal signaling procedure but abnormal
        //situations are handled too.
        if (rm_pItcRxMdMsg->primitive_type == DL_REL_IND)
        {
	      //May carry up l3 data but make nonsense 
            rm_StopTimer( rm_ItcRxEntryId, RM_T3109 );
            rm_StartTimer(rm_ItcRxEntryId, RM_T3L02);

            //SMS-SAPI3-Begin

            //rm_pSmCtxt->relCause = CNI_RIL3_CAUSE_TEMPORARY_FAILURE;
            rm_SendRrRelInd(RM_MDLERR, rm_pSmCtxt->mEntryId);

            rm_SendDlRelReq(rm_pSmCtxt->mOId, RM_SAPI0, RM_LOCAL_RELEASE);
            rm_SendDlRelReq(rm_pSmCtxt->mOId, RM_SAPI3, RM_LOCAL_RELEASE);
            rm_SendDlRelReq(rm_pSmCtxt->aOId, RM_SAPI3, RM_LOCAL_RELEASE);

            rm_SmCtxt_t *pASmCtxt;
            pASmCtxt = rm_pSmCtxt->pPhyChan->pASmCtxt;
            pASmCtxt->state = RM_S1RM_RUNNING;

            //SMS-SAPI3-End
        }
        //    
        //else 
	  //    IDEBUG__(("WARNING@rm_S7ReleasingSigTrafficChan: LAPDm message:%d\n",
	  //	           rm_pItcRxMdMsg->primitive_type));

	  break;

   case MODULE_RM: 
	  // Standard T3109 and internal T3L02 will be watched
	  if ( rm_pItcRxRmMsg->primitive_type != RM_TIMER_IND )
        {
   	       IDEBUG__(("WARNING@rm_S7ReleasingSigTrafficChan: prim:%d,timerId=%d\n",
		            rm_pItcRxRmMsg->primitive_type,rm_pItcRxRmMsg->timerId ));
             return;
        }
        //PDEBUG__(("rm_S7ReleasingSigTrafficChan: Timer Handling, timerId=%d\n",
        //           rm_pItcRxRmMsg->timerId));
        switch (rm_pItcRxRmMsg->timerId)
        {
        case RM_T3L02:
             rm_StartTimer( rm_ItcRxEntryId, RM_T3111 );
             rm_SendRfChanRelease(rm_pSmCtxt->pPhyChan);

             //SMS-SAPI3-Begin
             //rm_SendRrRelCnf();
             //SMS-SAPI3-End

             rm_pSmCtxt->state = RM_S9DEACTIVATING_SIGTRAFFICCHAN;
             break;

        case RM_T3109:

             //SMS-SAPI3-Begin

             rm_SendRrRelInd(RM_MDLERR, rm_pSmCtxt->mEntryId);

             rm_SendDlRelReq(rm_pSmCtxt->mOId, RM_SAPI0, RM_LOCAL_RELEASE);
             rm_SendDlRelReq(rm_pSmCtxt->mOId, RM_SAPI3, RM_LOCAL_RELEASE);
             rm_SendDlRelReq(rm_pSmCtxt->aOId, RM_SAPI3, RM_LOCAL_RELEASE);

             rm_SmCtxt_t *pASmCtxt;
             pASmCtxt = rm_pSmCtxt->pPhyChan->pASmCtxt;
             pASmCtxt->state = RM_S1RM_RUNNING;

		 //Release RF channel and SM goes to a new state S9
             //rm_SendDlRelReq(rm_pSmCtxt->mOId,RM_SAPI0,RM_LOCAL_RELEASE);

	       //if (rm_pSmCtxt->sapi != RM_SAPI0)
             //    rm_SendDlRelReq(rm_pSmCtxt->mOId, rm_pSmCtxt->sapi,RM_LOCAL_RELEASE);

             //SMS-SAPI3-End

             rm_SendRfChanRelease(rm_pSmCtxt->pPhyChan);
             rm_StartTimer( rm_ItcRxEntryId, RM_T3111 );

             //SMS-SAPI3-Begin
	       //rm_SendRrRelCnf();
             //SMS-SAPI3-End

	       rm_pSmCtxt->state = RM_S9DEACTIVATING_SIGTRAFFICCHAN;
             break; 

        default:
   	       IDEBUG__(("WARNING@rm_S7ReleasingSigTrafficChan: wrong timer expiry:%d\n",
		           rm_pItcRxRmMsg->timerId));
		 break;
        }
	  break;

   default:
	  //Unexpected message origin at this SM state
	  IDEBUG__(("WARNING@rm_S7ReleasingSigTrafficChan: unexpected message origin:%d\n",
		       rm_ItcRxOrigin));
	  break;

   } //End of switch()

} //End of rm_S7ReleasingSigTrafficChan()

#endif //__RM_S7RELEASINGSIGTRAFFICCHAN_CPP__

