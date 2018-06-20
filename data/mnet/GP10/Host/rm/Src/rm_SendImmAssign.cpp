/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __RM_SENDIMMASSIGN_CPP__
#define __RM_SENDIMMASSIGN_CPP__

#include "rm\rm_head.h"

void rm_SendImmAssign(rm_PhyChanDesc_t *pPhyChan )
{
   static u8	    agch_block=0;    //Paging group got from IMSI
   u16		        length;	         //Length of a msg sent to L1
   rm_EdRet_t       rrEdRet;	     //Return code from RR decode
   rm_L3Data_t      l3_data; 	     //Encoded Paging Req Type 1
   rm_ItcTxMsg_t    msgToL1;	     //Send PH_DATA_REQ msg to L1
   rm_UmImmAssign_t *pUmImmAssign;   //Data for encoding PageReq1

   //Monitoring entrance to a function
   RDEBUG__(("ENTER-rm_SendImmAssign\n"));

   //Choose Imm Assign message structure
   pUmImmAssign = (rm_UmImmAssign_t *) &(rm_UmMsg.immediateAssignment);
 
   
   //Populate the contents so that encoder knows what to encode but
   //before we start populating the message content, zap it clean
   //first.
   
   RM_MEMSET( &rm_UmMsg, sizeof(rm_UmMsg_t) );

   //Header--PD,MT,SI
   pUmImmAssign->header.protocol_descriminator = RM_PD;
   pUmImmAssign->header.si_ti		           = RM_SI;
   pUmImmAssign->header.message_type	       = RM_IMMEDIATE_ASSIGNMENT;

   //IE--Page Mode
   pUmImmAssign->pageMode.ie_present	       = RM_TRUE;
   
   //PR1261 Begin
   pUmImmAssign->pageMode.pm	 	           = CNI_RIL3_PAGING_SAME_AS_BEFORE;
   //pUmImmAssign->pageMode.pm                 = CNI_RIL3_PAGING_REORGANIZATION;
   //PR1261 End

   //IE--Channel Description
   pUmImmAssign->channelDescription.ie_present       = RM_TRUE; 
   pUmImmAssign->channelDescription.hopping          = RM_FALSE; 
   pUmImmAssign->channelDescription.channelType	     = pPhyChan->chanType;
   pUmImmAssign->channelDescription.subChannelNumber = pPhyChan->subChanNumber;
   pUmImmAssign->channelDescription.TN               = (pPhyChan->chanNumberMSB)&RM_U8_0X07;
   pUmImmAssign->channelDescription.TSC              = OAMrm_BTS_TSC; //pPhyChan->TSC;
   pUmImmAssign->channelDescription.arfcn            = OAMrm_ARFCN(pPhyChan->trxNumber);

   //IE--Request Reference
   pUmImmAssign->reqReference.ie_present       = RM_TRUE;
   pUmImmAssign->reqReference.randomAccessInfo = pPhyChan->savChanReq[0]; 
   pUmImmAssign->reqReference.T1               = (pPhyChan->savChanReq[1]>>3) & 0x1F;
   pUmImmAssign->reqReference.T2               = (pPhyChan->savChanReq[2]   ) & 0x1F;
   pUmImmAssign->reqReference.T3               = (((pPhyChan->savChanReq[1]<<3) & 0x38) |
				                                 ((pPhyChan->savChanReq[2]>>5) & 0x07)  );
   //IE--TA
   pUmImmAssign->timingAdvance.ie_present = RM_TRUE;
   pUmImmAssign->timingAdvance.value      = pPhyChan->savChanReq[3];

   //IE--Mobile Allocation
   pUmImmAssign->mobileAllocation.ie_present = RM_TRUE;
   pUmImmAssign->mobileAllocation.numRFfreq  = 0;

   //IE--Starting Time
   pUmImmAssign->startingTime.ie_present = RM_FALSE; //StartingTime_TBD;

   //IE--IA Rest Octets
   pUmImmAssign->iaRestOctets.ie_present = RM_TRUE;
   pUmImmAssign->iaRestOctets.p	         = RM_P_NOT_PRESENT;

    
   //Call RR message encoding functionality
   
   rrEdRet = CNI_RIL3RRM_Encode( &rm_UmMsg, &l3_data);

   //check the encoding result before sending the message
   switch (rrEdRet)
   {
   case RM_RRED_SUCCESS:
	
	    //Wrap the encoded Paging message in PH_DATA_REQ
        length = 0;
        msgToL1.buffer[length++] = RM_L1MG_LNKMGMT;
        msgToL1.buffer[length++] = RM_L1MT_PHDATAREQ_MSB;
        msgToL1.buffer[length++] = RM_L1MT_PHDATAREQ_LSB;
 	    msgToL1.buffer[length++] = OAMrm_BCCH_TRX;
        msgToL1.buffer[length++] = RM_L1AGCH_CHANNUMBER_MSB;

        //PR1261 Begin
		//msgToL1.buffer[length++] = 0x3F & agch_block++;
        msgToL1.buffer[length++] = 28;
		//PR1261 End

	    msgToL1.buffer[length++] = RM_L1LINK_VOID;

        RM_MEMCPY(&msgToL1.buffer[length], l3_data.buffer, l3_data.msgLength);

        msgToL1.msgLength   = length + l3_data.msgLength;

	    //Send PH_DATA_REQ to L1
        PostL3SendMsLog( &l3_data );
        api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
        PDEBUG__(("========rm_SendImmAssign: TA=%d\n", pPhyChan->savChanReq[3]));

		//PR1261 Begin
        //Adjust agch_block index
        //switch (OAMrm_BSCCCHSDCCH_COMB)
        //{
        //case RM_TRUE:
	    //     if ( agch_block>8 )  agch_block = 0;
	    //     break;

        //case RM_FALSE:
	    //     if ( agch_block>8 )  agch_block = 0;
        //     break;
        //}
		//PR1261 End

        //PMADDED:succImmediateAssingProcs,succImmediateAssingProcsPerCauseTable
        PM_CellMeasurement.succImmediateAssingProcs.increment();
        PM_CellMeasurement.succImmediateAssingProcsPerCauseTable.increment(pPhyChan->estCause);

	    break;

   default:
	    //Um message encoding failed
        EDEBUG__(("ERROR-rm_SendImmAssign: Um encode error:%d\n", rrEdRet ));
   	    break;

   } //End of switch(rrEdRet)

} //End of rm_SendImmAssign()

#endif //__RM_SENDIMMASSIGN_CPP__
