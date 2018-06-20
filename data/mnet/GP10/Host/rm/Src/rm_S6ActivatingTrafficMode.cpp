/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __RM_S6ACTIVATINGTRAFFICMODE_CPP__
#define __RM_S6ACTIVATINGTRAFFICMODE_CPP__


#include "rm\rm_head.h"

void rm_S6ActivatingTrafficMode(void)
{
   //Monitoring entrance to a function 
   RDEBUG__(("ENTER@rm_S6ActivatingTrafficMode\n"));

   if (rm_pSmCtxt->smType != RM_MAIN_SM)
   {
       //Mode Modify always associated with main SM
       EDEBUG__(("ERROR@rm_S6ActivatingTrafficMode: not main SM\n"));
       return;
   }

   switch(rm_ItcRxOrigin)
   {
   case MODULE_L1:
        //Handle Mode Modify Ack or NAck
        rm_ProcL1ModeAckNAck();
        break;
   default:
	  //Unexpected message orgination
        IDEBUG__(("WARNING@rm_S9ActivatingTrafficMode:wrong msg origination:%d\n",
		       rm_ItcRxOrigin));
	  break;

   } //End of switch()
              		       
} //End of rm_S6ActivatingTrafficMode()


void rm_ProcL1ModeAckNAck(void)
{
    PDEBUG__(("INFO@rm_ProcL1ModeAckNAck:smType=%d, smState=%d\n",
               rm_pSmCtxt->smType, rm_pSmCtxt->state));
    
    if ( rm_pItcRxL1Msg->l3_data.buffer[0]==RM_L1MG_DEDMGMT )
    {
	   //Check if having got Mode Modify Ack or NAck message from L1
         if ( rm_pItcRxL1Msg->l3_data.buffer[1]==RM_L1MT_MODEMODIFYACK_MSB &&
	        rm_pItcRxL1Msg->l3_data.buffer[2]==RM_L1MT_MODEMODIFYACK_LSB )
         {
		  //Get a positive ack from DSP	
              rm_SendChanModeModify(rm_pSmCtxt);
		  rm_StartTimer(rm_ItcRxEntryId, RM_T3L01 );
		  rm_pSmCtxt->state = RM_S8TRAFFICMODE_ACTIVATED;
	   }
	   else if ( rm_pItcRxL1Msg->l3_data.buffer[1]==RM_L1MT_MODEMODIFYNACK_MSB &&
	             rm_pItcRxL1Msg->l3_data.buffer[2]==RM_L1MT_MODEMODIFYNACK_LSB )
	   {
		  //Get a negative ack from DSP
		  EDEBUG__(("ERROR@rm_ProcL1ModeAckNAck:NACK received\n"));
		  EDUMP__((MAREA__, rm_pItcRxL1Msg->l3_data.buffer,
			      rm_pItcRxL1Msg->l3_data.msgLength ));
		  //Release the call since this DSP ts is not functioning right now
		  rm_SendChanRelease(rm_ItcRxEntryId);
	        rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
	        rm_StartTimer(rm_ItcRxEntryId, RM_T3109);
	    
	        //Adjust SM state to S7
              rm_pSmCtxt->relCause = CNI_RIL3_CAUSE_REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE;
              rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;
	        rm_pSmCtxt->state = RM_S7RELEASING_SIGTRAFFICCHAN;
	   } else
         {
		  IDEBUG__(("WARNING@rm_ProcL1ModeAckNAck: sm(%d,%d), l1 msgType:%d,%d\n",
                         rm_pSmCtxt->smType, 
                         rm_pSmCtxt->state,
			       rm_pItcRxL1Msg->l3_data.buffer[1],
			       rm_pItcRxL1Msg->l3_data.buffer[2] ));
         }
    } else
    {
	   //Got an unexpected message from L1 
         IDEBUG__(("WARNING@rm_ProcL1ModeAckNAck: sm(%d,%d), l1 msgGroup:%d\n",
                    rm_pSmCtxt->smType, 
                    rm_pSmCtxt->state,
	              rm_pItcRxL1Msg->l3_data.buffer[0]));
    }
}

#endif /* __RM_S6ACTIVATINGTRAFFICMODE_CPP__ */

