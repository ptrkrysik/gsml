/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef  __RM_SENDSACCHFILLINGSI5_CPP__
#define  __RM_SENDSACCHFILLINGSI5_CPP__

#include "rm\rm_head.h"

extern AdjCell_HandoverEntry adjCell_HandoverTable[HandoverTableLimit];
extern AdjCell_HandoverEntry adjCell_HandoverTable_t[HandoverTableLimit];

void rm_SendSacchFillingSI5(u8 trx, u8 SI)
{
   int				i,j;
   u16		        length;	 // Length of a msg sent to L1
   rm_EdRet_t       rrEdRet; // Return code from RR decode
   rm_L3Data_t      l3_data; // Encoded SI Type 5        
   rm_ItcTxMsg_t    msgToL1; // Send BCCH INFORMATION msg to L1
  
   //Monitoring entrance to a function 
   RDEBUG__(("ENTER-rm_SendSacchFillingToTrx\n"));

   //Zap rm_UmMsg 0-clean first
   RM_MEMSET( &rm_UmMsg, sizeof(rm_UmMsg_t) );
        
   //Populate SI2 for use by RRM encoder
   rm_PopulateSI5();
   RM_MEMCPY( &rm_UmMsg, &rm_UmSI5, sizeof(rm_UmSI5_t));

   //Call RR message encoding functionality

   
   rrEdRet = CNI_RIL3RRM_Encode( &rm_UmMsg, &l3_data);
   
     
   //PR1223 Begin

   //RIL3 will reoder the frequencies during encoding. Get the resulted f.list
   RM_MEMCPY(&rm_UmSI5, &rm_UmMsg, sizeof(rm_UmSI5_t));

   //Construct internal adjacent ho cell tables in terms of the resulted order
   for (i=0;i<rm_UmSI5.bcchFreqList.numRFfreq;i++)
   {
	   memset(&adjCell_HandoverTable_t[i],0,
		      sizeof(AdjCell_HandoverEntry));

	   for (j=0;j<rm_UmSI5.bcchFreqList.numRFfreq;j++)
	   {

		   if (rm_UmSI5.bcchFreqList.arfcn[i]==OAMrm_HO_ADJCELL_BCCH_ARFCN(j))
			   break;
	   }

	   //printf("RM@===f(i#=%d, o#=%d, total=%d), freq=%d\n", i,j,
	   //      rm_UmSI5.bcchFreqList.numRFfreq,
	   //       rm_UmSI5.bcchFreqList.arfcn[i]);
	   
	   if (j<rm_UmSI5.bcchFreqList.numRFfreq)
	   {
		   //Put this adjacent-cell configuration in its right place
		   memcpy(&adjCell_HandoverTable_t[i],
			      &adjCell_HandoverTable[j],
                  sizeof(AdjCell_HandoverEntry));            

  	    //printf("RM@===f(#=%d, total=%d), freq=%d, id=%d\n", i,
		//       rm_UmSI5.bcchFreqList.numRFfreq,
		//       OAMrm_HO_ADJCELL_BCCH_ARFCN_t(i),
		//	     OAMrm_HO_ADJCELL_ID_t(i));
	   
       } else
	   {
		   EDEBUG__(("ERROR@rm_PopulateSI5:alien freq after RIL3 encoding\n"));
	   }

   }   

   //PR1223 End

   //check the encoding result before sending the message
   switch (rrEdRet)
   {
   case RM_RRED_SUCCESS:
	
	    //Wrap the encoded SI in SACCH FILLING sent to L1
        length = 0;
        msgToL1.buffer[length++] = RM_L1MG_TRXMGMT;
        msgToL1.buffer[length++] = RM_L1MT_SACCHFILL_MSB;
        msgToL1.buffer[length++] = RM_L1MT_SACCHFILL_LSB;
	    msgToL1.buffer[length++] = trx;
	    msgToL1.buffer[length++] = SI;
        msgToL1.buffer[length++] = l3_data.msgLength;
	    RM_MEMCPY(&msgToL1.buffer[length], l3_data.buffer, l3_data.msgLength);
        length = length + l3_data.msgLength;
        msgToL1.buffer[length++] = 1; //Start time==immediate
        msgToL1.buffer[length++] = 0;
        msgToL1.buffer[length++] = 0;
        msgToL1.msgLength        = length;

	    //Send SACCH FILLING TO L1
        api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
       
	    break;

   default:
	    //Encoding failed
        EDEBUG__(("ERROR-rm_SendSacchFillingSI5: RIL3-RR encode error:%d\n",
		           rrEdRet ));
   	    break;

   } //End of switch(rrEdRet)

} //End of rm_SendSacchFillingSI5()

#endif //__RM_SENDSACCHFILLINGSI5_CPP__
