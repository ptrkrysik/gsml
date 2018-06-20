/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __RM_S2ACTIVATINGSIGTRAFFICCHAN_CPP__
#define __RM_S2ACTIVATINGSIGTRAFFICCHAN_CPP__

#include "rm\rm_head.h"

void rm_S2ActivatingSigTrafficChan( void )
{
   //Monitoring entrance to a function
   RDEBUG__(("ENTER-rm_S2ActivatingSigTrafficChan\n"));

   if (rm_pSmCtxt->smType != RM_MAIN_SM)
   {
       //Channel Activation always associated with main SM
       EDEBUG__(("ERROR-rm_S2ActivatingSigTrafficChan: not main SM\n"));
       return;
   }

   switch(rm_ItcRxOrigin)
   {
   case MODULE_L1:
	  //Get a MPH_INFO_INF message. Analyze and process it
        if ( rm_pItcRxL1Msg->l3_data.buffer[0]==RM_L1MG_DEDMGMT )
        {
	       //Check if having got Channel Act. Ack or Nack
             if ( rm_pItcRxL1Msg->l3_data.buffer[1]==RM_L1MT_CHANACTIVACK_MSB &&
                  rm_pItcRxL1Msg->l3_data.buffer[2]==RM_L1MT_CHANACTIVACK_LSB )
             {
                //Got a positive ack to Channel Activation

                if (rm_pItcRxL1Msg->l3_data.buffer[8] != 0)
                {
                    IDEBUG__(("WARNING@rm_S2Act..: received warning(%d) to chan act on entryId(%d)\n",
                               rm_pItcRxL1Msg->l3_data.buffer[8], rm_ItcRxEntryId));
                }
              
                //SMS-SAPI3-Begin
                //Call LAPDm local release to clean up the channel in case
                //rm_SendDlRelReq(rm_pSmCtxt->mOId,rm_pSmCtxt->sapi,RM_LOCAL_RELEASE);

                rm_SendDlRelReq(rm_pSmCtxt->mOId,RM_SAPI0,RM_LOCAL_RELEASE);
                rm_SendDlRelReq(rm_pSmCtxt->mOId,RM_SAPI3,RM_LOCAL_RELEASE);
                rm_SendDlRelReq(rm_pSmCtxt->aOId,RM_SAPI3,RM_LOCAL_RELEASE);

                //SMS-SAPI3-End

                //Further check if Channel Activation is caused by Channel Request
                //from Mobile or Handover Request from MM
                if (rm_pSmCtxt->isForHo==RM_FALSE)
                {
                    //Related to Channel Request from Mobile   
                    rm_SendImmAssign(rm_pSmCtxt->pPhyChan);
 		        rm_StartTimer(rm_ItcRxEntryId, RM_T3101);
		        rm_pSmCtxt->state = RM_S3SIGCHAN_ACTIVATED;
                } else
                {
                    //HOADDED
                    //Related to Handover Request from MM
 			  //Send back Handover Request Ack to MM
                    if (rm_SendHoReqAck())
                    {
                        rm_pSmCtxt->waitHoComplt = RM_FALSE;
 		            rm_StartTimer(rm_ItcRxEntryId, RM_T3101);
                        rm_pSmCtxt->state = RM_S4TRAFFICCHAN_ACTIVATED;
                    }
                    else
                    {
                        //Failed in sending back Handover Request Ack and release
                        //radio resource
                        EDEBUG__(("ERROR@rm_S2ActivatingSigTrafficChan: failed in rm_SendHoReqAck\n"));

                        rm_SendHoReqNAck(rm_pSmCtxt->pPhyChan->hoRef);

				//rm_RslRelease(); //This contains unecessary info, may mod.
                        rm_pSmCtxt->state = RM_S1RM_RUNNING;
                        rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_FREE;
                        rm_SendRfChanRelease(rm_pSmCtxt->pPhyChan);
                    }
                }
	     }
	     else if ( rm_pItcRxL1Msg->l3_data.buffer[1]==RM_L1MT_CHANACTIVNACK_MSB &&
	               rm_pItcRxL1Msg->l3_data.buffer[2]==RM_L1MT_CHANACTIVNACK_LSB )
	     {
		    //Got a negative Ack to Channel Activation
          	    rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
                rm_SendRfChanRelease(rm_pSmCtxt->pPhyChan);

		    IDEBUG__(("WARNING@rm_S2ActivatingSigTrafficChan: nack to act chan(%d,%d,%d)\n",
 	  	              rm_pItcRxL1Msg->l3_data.buffer[3],
 	  	              rm_pItcRxL1Msg->l3_data.buffer[4],
			        rm_pItcRxL1Msg->l3_data.buffer[5]));

                //Further check if Channel Activation is caused by Channel Request
                //from Mobile or Handover Request from MM
                if (rm_pSmCtxt->isForHo==RM_TRUE)
                {
                    //If it's Handover Request caused, send HoReqAck back to MM
                    rm_SendHoReqNAck(rm_pSmCtxt->pPhyChan->hoRef);
                } 

                //SMS-SAPI3-Begin

                //For benefit of LAPDm's internal cleanup 
                rm_SendDlRelReq(rm_pSmCtxt->mOId,RM_SAPI0,RM_LOCAL_RELEASE);
                rm_SendDlRelReq(rm_pSmCtxt->mOId,RM_SAPI3,RM_LOCAL_RELEASE);
                rm_SendDlRelReq(rm_pSmCtxt->aOId,RM_SAPI3,RM_LOCAL_RELEASE);

                //SMS-SAPI3-End

                //Reset relevant RM channel resources
		    rm_pSmCtxt->state = RM_S1RM_RUNNING;
		    rm_SmCtxt[rm_pSmCtxt->aEntryId].state = RM_S1RM_RUNNING;
		    rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_FREE;
                  
		    //Report this negative ack to Channel Activation
		    EDEBUG__(("ERROR-rm_S2ActivatingSigTrafficChan:NACK received\n"));
		    //EDUMP__(( MAREA__, rm_pItcRxL1Msg->l3_data.buffer,
		    //          rm_pItcRxL1Msg->l3_data.msgLength ));
	     }
	     else
		    EDEBUG__(("ERROR-rm_S2ActivatingSigTrafficChan:L1 MsgType:%d,%d\n",
		  	        rm_pItcRxL1Msg->l3_data.buffer[1],
			        rm_pItcRxL1Msg->l3_data.buffer[2] ));
	  } 
        else
           EDEBUG__(("ERROR@rm_S2ActivatingSigTrafficChan:L1 MsgGroup:%d\n",
		          rm_pItcRxL1Msg->l3_data.buffer[0]));
	  break;

   default:
	  //TBD: Unexpected message orgination
        UDEBUG__(("ERROR@rm_S2ActivatingSigTrafficChan:wrong msg origination:%d\n",
		  rm_ItcRxOrigin));
	  break;

   } //End of switch()
              		       
} //End of rm_S2ActivatingSigTrafficChan()

#endif //__RM_S2ACTIVATINGSIGTRAFFICCHAN_CPP__
