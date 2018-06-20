/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_SMSCB_CPP__
#define __RM_SMSCB_CPP__

#include "rm\rm_head.h"

void rm_ProcSmscbcMsg(void)
{  
   u16 size;

   //Block CBCH broadcast if needed
   if (rm_pCbchChan->amState!=unlocked)
   {
       //Stop CBCH by sending a NULL
       IDEBUG__(("WARNING@rm_ProcSmscbcMsg: CBCH amState=%d\n",rm_pCbchChan->amState));
       rm_SmscbcMsg.morPage=RM_FALSE;
       rm_BroadcastNilCbMsg();
       return;
   }
 
   size = rm_pItcRxVbMsg->size;

   switch (rm_pItcRxVbMsg->message_type)
   {
   case INTER_SMSCBC_CB_START: //add message
	
        // If received CB proper or reject!
        if (size > INTER_CBC_CB_MAX_DAT_LEN)
        {  
             IDEBUG__(("WARNING@rm_ProcSmscbcMsg: invalid CB msg size: %d\n",
                        rm_pItcRxVbMsg->size));
             return;
        }
	// instant message,6/29
       // memcpy(&rm_SmscbcMsg.data, rm_pItcRxVbMsg->data, size); 
       //rm_SmscbcMsg.totPage = size/88; 
       // if (size%88) rm_SmscbcMsg.totPage++;
       // rm_SmscbcMsg.curPage = 1;    //Cur CB msg page to send

        //Broadcast the first page of a new cb msg
        //rm_BroadcastNewCbMsg(rm_SmscbcMsg.curPage);
        //if ((++rm_SmscbcMsg.curPage)>rm_SmscbcMsg.totPage)
            rm_SmscbcMsg.curPage = 1;        
        //if (rm_SmscbcMsg.totPage>1) 
        //    rm_SmscbcMsg.morPage = RM_TRUE;
        //else 
        //    rm_SmscbcMsg.morPage = RM_FALSE;
	
	//PR 1323,shedule message, add/delete message to table
	int match,i;
	match=0;
	i=0;
	while(i<SmsCbc_TableRow_MAX && rm_CbMsgTable.table[i].state != 0)
	{
		if(rm_pItcRxVbMsg->message_id == rm_CbMsgTable.table[i].msgId)//msg exist
		{
			match=1;
	  	      memcpy(rm_CbMsgTable.table[i].msg, rm_pItcRxVbMsg->data, size); 
			rm_CbMsgTable.table[i].state = 1;
			rm_CbMsgTable.table[i].size = size; 
			rm_CbMsgTable.table[i].totPage = rm_CbMsgTable.table[i].size/88; 
        		if (rm_CbMsgTable.table[i].size % 88) 
                      rm_CbMsgTable.table[i].totPage++;
				rm_CbMsgTable.table[i].curPageInCurMsg=1;

                  break;
            }
		++i;
	}

	if(match == 0 && i< SmsCbc_TableRow_MAX )//msg not exist or table empty
	{	
			
  	     rm_CbMsgTable.table[i].state = 1;
	     memcpy(rm_CbMsgTable.table[i].msg, rm_pItcRxVbMsg->data, size); 
	     rm_CbMsgTable.table[i].msgId = rm_pItcRxVbMsg->message_id;
	     rm_CbMsgTable.table[i].size = size; 
   	     rm_CbMsgTable.table[i].totPage = rm_CbMsgTable.table[i].size/88; 
           if (rm_CbMsgTable.table[i].size % 88) 
               rm_CbMsgTable.table[i].totPage++;
		   rm_CbMsgTable.table[i].curPageInCurMsg=1;

	}
	else if(match == 0 && i >= SmsCbc_TableRow_MAX )
	{
		IDEBUG__(("WARNING@rm_ProcSmscbcMsg: Table overflow"));              
	}
	break;
	
   case INTER_SMSCBC_CB_STOP://delete message
	for(i = 0; i< SmsCbc_TableRow_MAX && rm_CbMsgTable.table[i].state != 0; ++i)
	{
		if(rm_pItcRxVbMsg->message_id == rm_CbMsgTable.table[i].msgId)
		{	
		   	rm_CbMsgTable.table[i].state = -1;
                  break;
		}
	
	}
	if(i >= SmsCbc_TableRow_MAX || rm_CbMsgTable.table[i].state == 0)
	{
		IDEBUG__(("WARNING@rm_ProcSmscbcMsg: invalid msgType=%d\n",
                   rm_pItcRxVbMsg->message_type));              
	}
	break;
   //PR 1323 end 

   default:
        IDEBUG__(("WARNING@rm_ProcSmscbcMsg: invalid msgType=%d\n",
                   rm_pItcRxVbMsg->message_type));              
        break;
   }
}

void rm_BroadcastNewCbMsg(u8 page)
{
   rm_PassDownBlock(0, 0x20, page, RM_FALSE); //Pass down 1st block
   rm_PassDownBlock(1, 0x20, page, RM_FALSE); //Pass down 2nd block
   rm_PassDownBlock(2, 0x20, page, RM_FALSE); //Pass down 3rd block
   rm_PassDownBlock(3, 0x30, page, RM_FALSE); //Pass down 4th block
}

void rm_BroadcastNilCbMsg(void)
{
   rm_PassDownBlock(0, 0x20, 0, RM_TRUE); //Pass down 1st block
   rm_PassDownBlock(1, 0x20, 0, RM_TRUE); //Pass down 2nd block
   rm_PassDownBlock(2, 0x20, 0, RM_TRUE); //Pass down 3rd block
   rm_PassDownBlock(3, 0x30, 0, RM_TRUE); //Pass down 4th block
}

void rm_PassDownBlock(u8 blockSeq, u8 blockType, u8 curMsg, u8 nullFlag)
{

   int  length,i,j,k;
   u8	  buffer[60];

   curMsg = rm_CbMsgTable.curMsg;
   PDEBUG__(("INFO@rm_PassDownBlock: CB block (seq=%d,type=%d,curMsg=%d,curPage=%d,flg=%d)\n",
              blockSeq, blockType, rm_CbMsgTable.curMsg,
              rm_CbMsgTable.table[curMsg].curPageInCurMsg,
              nullFlag));

   //Pass down one SMS-CB block to L1Proxy
   length = 4;
   buffer[length++] = RM_L1MG_LNKMGMT;
   buffer[length++] = RM_L1MT_PHDATAREQ_MSB;
   buffer[length++] = RM_L1MT_PHDATAREQ_LSB;
   buffer[length++] = 0; //alwasys to trx 0
   buffer[length++] = RM_L1CBCH_CHANNUMBER_MSB; //CBCH:0xa0
   buffer[length++] = 0;
   buffer[length++] = blockSeq; //block sequence 
   //memset(&buffer[length], 0, 23);
   if (nullFlag)
   {
       buffer[length++] = blockType|0x0F;
       memset(&buffer[length], 0x2b, 22);
   } else
   { 
       buffer[length++] = blockType|blockSeq;
       //PR 1323
       memcpy(&buffer[length], &(rm_CbMsgTable.table[curMsg].msg[
              (rm_CbMsgTable.table[curMsg].curPageInCurMsg-1)*88 + blockSeq*22]), 22);

       //memcpy(&buffer[length], &(rm_SmscbcMsg.data[(page-1)*88+blockSeq*22]), 22);
   }

   //Fill length to the 1st four bytes in little endian format
   length = length + 22;
   buffer[0] = (unsigned char)length;
   buffer[1] = (unsigned char)(length>>8);
   buffer[2] = (unsigned char)(length>>16);
   buffer[3] = (unsigned char)(length>>24);
    
   //calling l1proxy func
   sendDsp(buffer, length);
}

void rm_ActivateCbchChan(void)
{
   rm_ItcTxMsg_t 	    msgToL1;
   u16			    length;

   PDEBUG__(("INFO@rm_ActivateCbchChan: activate CBCH channel\n"));

   //Reset rm_SmscbcMsg ctrl struct
   rm_SmscbcMsg.totPage = 0;
   rm_SmscbcMsg.curPage = 0;
   rm_SmscbcMsg.morPage = RM_FALSE;

   length = 0;
   msgToL1.buffer[length++] = RM_L1MG_DEDMGMT;
   msgToL1.buffer[length++] = RM_L1MT_CHANACTIV_MSB;
   msgToL1.buffer[length++] = RM_L1MT_CHANACTIV_LSB;
   msgToL1.buffer[length++] = rm_pCbchChan->trxNumber;
   msgToL1.buffer[length++] = rm_pCbchChan->chanNumberMSB;
   msgToL1.buffer[length++] = rm_pCbchChan->chanNumberLSB;
   msgToL1.buffer[length++] = RM_L1ACTTYPE_CBCH;         //actType
   msgToL1.buffer[length++] = RM_L1DTXCTRL_D0U0;         //dtxCtrl
   msgToL1.buffer[length++] = RM_L1DATATTR_SIGNALING;    //datAttr
   msgToL1.buffer[length++] = RM_L1RATTYPE_SDCCH;        //ratType
   msgToL1.buffer[length++] = RM_L1CODALGO_NORES;        //no algo
   msgToL1.buffer[length++] = RM_L1CIPHTYPE_NCIPH;       //ciphSet
   RM_MEMSET(&msgToL1.buffer[length], RM_L1CIPHKEY_LEN); //ciphKey
   length = length + RM_L1CIPHKEY_LEN - 1; //TEMP Adjustment
   msgToL1.buffer[length++] = 0; 
   msgToL1.buffer[length++] = 0; 
   msgToL1.buffer[length++] = 0; 
   msgToL1.buffer[length++] = 0; 
   msgToL1.buffer[length++] = 0; 
   msgToL1.msgLength = length;

   api_SendMsgFromRmToL1(msgToL1.msgLength, msgToL1.buffer);
}

//PR 1323
intg_ShowCbTable(void)
{
   printf ("========= Listing Items in CB Table (Current Msg %d)=========\n",
           rm_CbMsgTable.curMsg);
   
   for (int i=0;i<SmsCbc_TableRow_MAX;i++)
   {
        if (rm_CbMsgTable.table[i].state==1)
            printf("%d-Active:       msgId(%d,%d), msgSize=%d, totPage=%d, curPage=%d\n",
                     rm_CbMsgTable.table[i].state,   
                     rm_CbMsgTable.table[i].msgId,
                     rm_CbMsgTable.table[i].msg[3],
                     rm_CbMsgTable.table[i].size,   
                     rm_CbMsgTable.table[i].totPage,   
                     rm_CbMsgTable.table[i].curPageInCurMsg);
        else if (rm_CbMsgTable.table[i].state== -1 )
            printf("%d-InActive:     msgId(%d,%d), msgSize=%d, totPage=%d, curPage=%d\n",
                     rm_CbMsgTable.table[i].state,   
                     rm_CbMsgTable.table[i].msgId,
                     rm_CbMsgTable.table[i].msg[3],
                     rm_CbMsgTable.table[i].size,   
                     rm_CbMsgTable.table[i].totPage,   
                     rm_CbMsgTable.table[i].curPageInCurMsg);
    }
}

#endif //__RM_SMSCB_CPP__
