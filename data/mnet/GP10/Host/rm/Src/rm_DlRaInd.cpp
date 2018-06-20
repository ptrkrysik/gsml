/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef __RM_DLRAIND_CPP__
#define __RM_DLRAIND_CPP__

#include "rm\rm_head.h"
#include "rlc_mac\RlcMacIntf.h"

void rm_DlRaInd( void )
{
   //rm_ChanReq_t		chanReq;
   T_CNI_RIL3RRM_MSG_CHANNEL_REQUEST chanReq;
   rm_EdRet_t		raEdRet;
   JC_STATUS		ret;

   //Monitoring entrance to a function
   RDEBUG__(("ENTER-rm_DlRaInd\n" ));

   //CT_CHN if ( rm_pItcRxMdMsg->channel_type != CT_RACH )
   if ( rm_pItcRxMdMsg->entry_id != rm_RachEntryId )
   {
	  //TBD: Received is not Channel Request msg
        EDEBUG__(( "ERROR-rm_DlRaInd: expected RA on RACH not on:\n",
		        rm_pItcRxMdMsg->entry_id ));
                    //CT_CHN rm_pItcRxMdMsg->channel_type ));
        return;
   }
        
   //Call RA decoder to decode channel request message
   chanReq.neci = CNI_RIL3_NECI_NOT_SUPPORTED;	//TBD:Sheausong
   raEdRet = CNI_RIL3RRM_DecodeChannelRequest (
                         rm_pItcRxMdMsg->l3_data.buffer,
                         &chanReq );

   PDEBUG__(("TRACK-rm_DlRaInd: neci=%d,cos=%d,ref=%d\n",
               chanReq.neci,chanReq.estCause,chanReq.randomReference));

   //T_CNI_RIL3_NECI				neci;
   //neci has to be passed in for correct decoding,
   //T_CNI_RIL3_ESTABLISH_CAUSE	      estCause;
   //char						randomReference;
	
   switch (raEdRet)
   {
   case RM_RAED_SUCCESS:
        //G2
        if (rm_IsGprsChanReq(chanReq.estCause))
        {
		if (JC_OK != (ret=RlcMacRmMdMsgInd(rm_pItcRxMdMsg)) )
            {
		    IDEBUG__(("WARNING@rm_DlRaInd: API RlcMacRmMdMsgInd failed ret(%d)\n",ret));
            }
        } else
            rm_ChanReq(&chanReq);
	  break;

   default:
        //RA-decoding failed
        EDEBUG__(("ERROR@rm_DlRaInd:RA decode failed\n"));
	  break;

   } //End of switch()

} //End of rm_DlRaInd()

//G2
bool rm_IsGprsChanReq(T_CNI_RIL3_ESTABLISH_CAUSE cause)
{
    if ( (cause == CNI_RIL3_EST_CALL_ONE_PHASE_PACKET_ACCESS)   ||
         (cause == CNI_RIL3_EST_CALL_SINGLE_BLOCK_PACKET_ACCESS) )
         return true;
    else
         return false;
}

#endif //__RM_DLRAIND_CPP__


