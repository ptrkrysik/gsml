/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef __RM_S3SIGCHANACTIVATED_CPP__
#define __RM_S3SIGCHANACTIVATED_CPP__

#include "rm\rm_head.h"

void rm_S3SigChanActivated(void)
{
   //Monitoring entrance to a function
   RDEBUG__(("ENTER-rm_S3SigChanActivated\n"));
 
   if (rm_pSmCtxt->smType != RM_MAIN_SM)
   {
       //Channel Activation always associated with main SM
       EDEBUG__(("ERROR-rm_S3SigChanActivated: not main SM\n"));
       return;
   }
 
   switch(rm_ItcRxOrigin)
   {
   case MODULE_MD:
        //Check if received DL_REL_CNF sent before ImmAssignmmt
        if (rm_pItcRxMdMsg->primitive_type == L23PT_DL_REL_CONF)
            return;
        if (rm_pItcRxMdMsg->primitive_type == DL_EST_IND)
        {
	     //Received DL_EST_IND:Stop T3101 and go new ST
           rm_StopTimer( rm_ItcRxEntryId, RM_T3101 );
	     rm_pSmCtxt->sapi  = rm_pItcRxMdMsg->sapi;	
 	     rm_pSmCtxt->state = RM_S5SIGMODE_ESTABLISHED;  

           //rm_ProcClassMarkIE();

	     switch(rm_pSmCtxt->callType)
           {
           case RM_CHANREQ_COS_LUP:   //RM_CALLTYPE_LUP:

                //PMADDED:meanNbrOfBusySDCCHs,maxNbrOfBusySDCCHs

                rm_pSmCtxt->pPhyChan->chanBusy=RM_TRUE;          

                rm_NoOfBusySdcch4++;
                rm_maxNbrOfBusySDCCHs++;
                PM_CellMeasurement.meanNbrOfBusySDCCHs.addRecord(rm_NoOfBusySdcch4);
                PM_CellMeasurement.maxNbrOfBusySDCCHs.setValue(rm_maxNbrOfBusySDCCHs); //increment();
                //PM_CellMeasurement.maxNbrOfBusySDCCHs.addRecord(rm_maxNbrOfBusySDCCHs); //increment();

                break;
           case RM_CHANREQ_COS_EMC:   //RM_CALLTYPE_EMC:
	     case RM_CHANREQ_COS_MOC:   //RM_CALLTYPE_MOC:
	     case RM_CHANREQ_COS_MTC:   //RM_CALLTYPE_MTC:
     
                if (rm_pSmCtxt->callType==RM_CHANREQ_COS_MTC)
                    rm_SendPagingRsp();

                //PMADDED:meanNbrOfBusyTCHs,maxNbrOfBusyTCHs,meanTCHBusyTime,
                //        succTCHSeizures

                rm_pSmCtxt->pPhyChan->chanBusy=RM_TRUE;          

                //rm_NoOfBusyTchf++;
                //PM_CellMeasurement.meanNbrOfBusyTCHs.addRecord(rm_NoOfBusyTchf);
                //PM_CellMeasurement.maxNbrOfBusyTCHs.increment();
                //PM_CellMeasurement.succTCHSeizures.increment();
                //rm_pSmCtxt->pPhyChan->startTime=tickGet();
                break;

	      default:
		    UDEBUG__(("UNKNOWN-rm_S3SigChanActivated:wrong call type:%d\n",
		               rm_pSmCtxt->callType));
		    break;
	      }
	  }
        else
	      EDEBUG__(("ERROR-rm_S3SigChanActivated: wrong LAPDm message:%d\n",
		           rm_pItcRxMdMsg->primitive_type));
	
	  break;

   case MODULE_RM: 
        PDEBUG__(("rm_S3SigChanActivated: Timer Expiry for entryID=%d\n", rm_ItcRxEntryId));

        if ( rm_pItcRxRmMsg->primitive_type != RM_TIMER_IND  ||
             rm_pSmCtxt->pPhyChan->T3101.state == RM_FALSE   ||
             rm_pItcRxRmMsg->timerId != RM_T3101 )
        {
   	       EDEBUG__(("ERROR-rm_S3ReleasingSigTrafficChan: prim:%d,timer:%d, sta:%d\n",
		            rm_pItcRxRmMsg->primitive_type,
                        rm_pItcRxRmMsg->timerId,
                        rm_pSmCtxt->pPhyChan->T3101.state ));
             return;
        }


	 // No rsp to Imm Assignment. Release resources
    	 switch(rm_pSmCtxt->callType)
	 {
  	 //FFS: 
	 //RM goes ahead to release RF channel. But before a release
       //command reaching L1 and put into effect, a DL_EST_IND may
       //come to RR if T3101 time is not properly set.
   	 case RM_CALLTYPE_LUP:
       case RM_CALLTYPE_MOC:
       case RM_CALLTYPE_MTC:
	      //Release the RF channel as well as associated SMs
	      rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
            rm_SendRfChanRelease(rm_pSmCtxt->pPhyChan);
 	      rm_pSmCtxt->pPhyChan->state           = RM_PHYCHAN_RELEASING;
	      rm_pSmCtxt->pPhyChan->T3101.state     = RM_FALSE;
	      rm_pSmCtxt->state                     = RM_S9DEACTIVATING_SIGTRAFFICCHAN;
  	      rm_SmCtxt[rm_pSmCtxt->aEntryId].state = RM_S1RM_RUNNING;    
	      break;
		     	     	
        default:
	      UDEBUG__(("UNKNOWN-rm_S3SigChanActivated:wrong call type:%d\n",
	  		     rm_pSmCtxt->callType));
	      break;
	
	  } //End of switch()
		
        break; 
	
   default:
	  //Unexpected message origin at this SM state
	  EDEBUG__(("ERROR-rm_S3SigChanActivated: unexpected message origin:%d\n",
		       rm_ItcRxOrigin));
	  break;

   } //End of switch()

} //End of rm_S3SigChanActivated()

#endif //__RM_S3SIGCHANACTIVATED_CPP__

