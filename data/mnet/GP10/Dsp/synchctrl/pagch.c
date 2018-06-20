/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/******************************************************************
* file name: pagch.c             
*
* Description: The file contains Paging Access Grant channel
*              process routings and RACH processing routine
*                                                        
******************************************************************/
#include "stdlib.h"
#include "gsmdata.h" 
#include "bbdata.h"
#include "dsprotyp.h"
#include "intr.h"

/************************************************************
* Function Prototype:
*          dlPagchProc0(Uint8 rfCh, Uint8 timeSlot)
*
*
* Description: Process downlink PAGCH burst 0
*
* Cautions: rfCh is '0' for current situation
*           timeSlot must be from 0 to 7
*   
* Outputs: embedded in channel structure
*
* Output: N/A
*
************************************************************/       
void dlPagchProc0(Uint8 rfCh, Uint8 timeSlot)
{             
	t_DLPPCHBufs   *pagMsg;  
	t_TNInfo *ts;          
 Uint8  t1_prime, t2, t3, del_total, delay_offset;
 Uint32 FN_start_time; 
 msgStruc    respMsg;
 Uint8 PPCHBuffIdx;
 Uint8  usedBufs;
	
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
	
	switch(ts->groupNum)
	{
	case GSM_GROUP_5:
		         	         
  /*
  *  must search the PPCH buffers to find which index matches the paging number.
  */
  PPCHBuffIdx = getPPCHBuff(rfCh, timeSlot);
  pagMsg = &g_dlSigBufsPPCH[rfCh][PPCHBuffIdx];
		
		/*
		* check if there is message to send
		* do channel encode only if there is a valid Paging or
		*  Access Grant message, other build a dummy burst
		*/

  /*
  *  Send Access Grant message first, then Paging message.
  */
  if(	!pagMsg->valid && (pagMsg->msgReady) )
		{     
			pagMsg->valid = 1;
		}
							        
  if(pagMsg->valid)
  {             
			/*
			*  Send AG Message 1st only if one or less Paging Message in the queue
			*/
            if(pagMsg->msgReady && (pagMsg->logCh == AGCH) )
            {
               /*
               *  Look for IMMEDIATE ASSIGNMENT message types in TBF mode, as we need to 
               *  modify starting time IE (04.08) from zeroes (default) to:
               *
               *  FN_start_time = (51x((t3-t2) mod 26) + t3 + 51x25xt1') + delta, where
               *
               *  t1' = (FN % 1326) mod 32
               *  t2  =  FN % 26
               *  t3  =  FN % 51
               *  delta = 3,  number or radio blocks.
               */       
               if(((pagMsg->data[3] & 0x10) >> 4) == 1) {
                 /*
                 *  TBF mode
                 */

                 del_total = 0;
                 delay_offset = 13;  // 26 frames (6 blocks) in initial testing; need at least 3 for RRB case

                 if ( (sysFrameNum.t2 == 0)  || (sysFrameNum.t2 == 5)  ||
                      (sysFrameNum.t2 == 9)  || (sysFrameNum.t2 == 13) ||
                       (sysFrameNum.t2 == 18) || (sysFrameNum.t2 == 22) ) {
                     del_total = 8+delay_offset;
                 }
                 else if ( (sysFrameNum.t2 == 4)  || (sysFrameNum.t2 == 8)  || 
                           (sysFrameNum.t2 == 12) || (sysFrameNum.t2 == 17) ||
                           (sysFrameNum.t2 == 21) || (sysFrameNum.t2 == 25) ) {
                     del_total = 9+delay_offset;
                 }
                 else if ( (sysFrameNum.t2 == 3)  || (sysFrameNum.t2 == 7)  ||
                           (sysFrameNum.t2 == 11) || (sysFrameNum.t2 == 16) ||
                           (sysFrameNum.t2 == 20) || (sysFrameNum.t2 == 24) ) {
                     del_total = 10+delay_offset;
                 }
                 else if ( (sysFrameNum.t2 == 2)  || (sysFrameNum.t2 == 6)  ||
                           (sysFrameNum.t2 == 10) || (sysFrameNum.t2 == 15) ||
                           (sysFrameNum.t2 == 19) || (sysFrameNum.t2 == 23) ) {
                     del_total = 11+delay_offset;
                 }
                 else if ( (sysFrameNum.t2 == 1)  || (sysFrameNum.t2 == 14) ) {
                     del_total = 12+delay_offset;
                 }
                 FN_start_time = sysFrameNum.FN + del_total;
                                  
                 /* The starting frame is coded as T1', T2, and T3 as defined in 04.08 section
                 * 10.5.2.38.  It occupies 16 consecutive bits with T1' (5 bits) followed by T3
                 * (6 bits) followed by T2 (5 bits).  In the IMMEDIATE ASSIGNMENT it starts at 
                 * bit 14 of the 15th byte and continues for the next 16 bits (i.e. byte 15 =
                 * xttttttt, byte 16 = tttttttt, byte 17 = txxxxxxx where t=starting time bits
                 * and x=bits that you must not modify).
                 */
                 t1_prime = (FN_start_time / 1326) % 32;
                 t2 = FN_start_time % 26;
                 t3 = FN_start_time % 51;

                 if(((pagMsg->data[12] & 0xC0) >> 6) == 3) {
                   /*
                   *  IA Rest Octets;  first octet
                   */
                   if(((pagMsg->data[12] & 0x30) >> 4) == 0) {
                     /* 
                     *  Packet Uplink Assignment (usually a Single Block Assignment for control channel);
                     *  Send Alerting to the higher GPRS layers in the Host.
                     */
                                        
                     pagMsg->data[14] |= ( (t1_prime << 2) | ((t3 & 0x30) >> 4) );  
                     pagMsg->data[15] |= ( ((t3 & 0x0f) << 4) | ((t2 & 0x1e) >> 1) );  
                     pagMsg->data[16] |= ((t2 & 0x01) << 7 );  

                     respMsg.msgSize = 11;
                     respMsg.function = PDCH_MANAGE_MSG;
                     respMsg.typeHiByte = PDCH_SINGLE_BLOCK_ASSIGN_ALERT >> 8;
                     respMsg.typeLoByte = PDCH_SINGLE_BLOCK_ASSIGN_ALERT;
                     /* respMsg.trx = rfCh; */  

                     // send 3-bit TN
                     respMsg.params[0] = pagMsg->data[4] & 0x07;
                          
                     // send 32-bit calculated FN               
                     respMsg.params[1] = FN_start_time >> 24;
                     respMsg.params[2] = FN_start_time >> 16;
                     respMsg.params[3] = FN_start_time >> 8;
                     respMsg.params[4] = FN_start_time;
 
                     // send 10-bit ARFCN
                     respMsg.params[5] = pagMsg->data[5] & 0x03;
                     respMsg.params[6] = pagMsg->data[6];
                 
                     SendMsgHost_Que(&respMsg);              
                   }

                   else {
                     /* 
                     *  Packet Downlink Assignment
                     */
                     pagMsg->data[19] |= (t1_prime >> 2);  
                     pagMsg->data[20] |= ( (t1_prime << 6) | t3 );  
                     pagMsg->data[21] |= (t2  << 3 );  

                     respMsg.msgSize = 12;
                     respMsg.function = PDCH_MANAGE_MSG;
                     respMsg.typeHiByte = PDCH_DL_ASSIGN_ALERT >> 8;
                     respMsg.typeLoByte = PDCH_DL_ASSIGN_ALERT;
                     /* respMsg.trx = rfCh; */  

                     // send 3-bit TN
                     respMsg.params[0] = pagMsg->data[4] & 0x07;
                          
                     // send 32-bit calculated FN
                     respMsg.params[1] = FN_start_time >> 24;
                     respMsg.params[2] = FN_start_time >> 16;
                     respMsg.params[3] = FN_start_time >> 8;
                     respMsg.params[4] = FN_start_time;
 
                     // send 10-bit ARFCN
                     respMsg.params[5] = pagMsg->data[5] & 0x03;
                     respMsg.params[6] = pagMsg->data[6];
                 
                     // send 8-bit Tag is stored by Host in the first byte of the Reference Request IE.
                     respMsg.params[7] = pagMsg->data[7];

                     SendMsgHost_Que(&respMsg);

                   }     // UL/DL Assignment
                 }   // IA Rest Octets
               } // TBF/Dedicated Mode

               /*
               * A message is pending and is ready for transmission. Load the message
               * into the Channel CODEC input buffer
               */               
               unpackFromByteBuffer( &g_dlSigBufsPPCH[0][PPCHBuffIdx].data[0],
                                     (UChar *)&g_DLCCHData.data[0],
                                     NUM_BITS_CCH_FRAME);
 
               pagMsg->msgReady = 0;
               pagMsg->logCh = NULL;   /* serves similar function to the former pageRepeats[] array */
               usedBufs = availablePPCHBuff(rfCh, timeSlot);

               g_DLCCHData.bufferValid = TRUE;
               
			            rtsCheck(rfCh, timeSlot, AGCH,  28, (NUM_PPCH_SUBCHANS - usedBufs) );    
			         }
			         else
            {
				          /*
				          *  Send a Paging Message
				          */
              processPCCCHBuff(pagMsg);

              pagMsg->logCh = NULL;   /* serves similart function to the former pageRepeats[] array */
              usedBufs = availablePPCHBuff(rfCh, timeSlot);
              /*
              *  inform L3 that DSP has room for new paging message
              */                   
              rtsCheck(rfCh, timeSlot, PCH,  ts->u.group5.dlPpchSubCh, (NUM_PPCH_SUBCHANS - usedBufs) ); 
            }
       
            if(pagMsg->logCh == NULL)
            {
        	     pagMsg->valid = 0;
            }            	
         }

         else
         {
			        unpackFromByteBuffer(bcchFiller,
                              (UChar *)&g_DLCCHData.data[0], 
                              NUM_BITS_CCH_FRAME);        
                               
	    	     g_DLCCHData.bufferValid = TRUE;
         }                               

    	    if(++ts->u.group5.dlPpchSubCh >= g_BBInfo[rfCh].BSPaMfrms*PAGE_BLOCKS_G5) ts->u.group5.dlPpchSubCh = 0;
		
		/*
		  PAGING RTS will be installed later
		if(ts->u.group5.pagch.state[ts->u.group5.dlSdcchSubCh] == CHAN_ACTIVE)
			rtsCheck(rfCh, timeSlot, PCH,  ts->u.group5.dlPagchSubCh);
	    */
 		
		break;
		
	case GSM_GROUP_4:
		break;

 case GSM_GROUP_11:
		break;

 case GSM_GROUP_12:
		break;
	
 default:
		break;          
	}
 
 chanEncoder(PAGCH, 1, timeSlot,0);		
	buildGSMBurst(PAGCH,  g_BBInfo[rfCh].tseqNum);
	GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, PAGCH);
}

/************************************************************
* Function Prototype:
*          dlPagchProc1(Uint8 rfCh, Uint8 timeSlot)
*
* Description: Process downlink PAGCH burst 0
*
* Cautions: rfCh is '0' for current situation
*           timeSlot must be from 0 to 7
*   
* Outputs: embedded in channel structure
*
* Output: N/A
*
************************************************************/      
void dlPagchProc1(Uint8 rfCh, Uint8 timeSlot)
{           
           
	t_DLSigBufs *pagMsg;
	t_TNInfo *ts;          
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
        
 chanEncoder(PAGCH, 0, timeSlot,0);			
	buildGSMBurst(PAGCH,  g_BBInfo[rfCh].tseqNum);
	GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, PAGCH);			   		   
}

/************************************************************
* Function Prototype:
*          dlPagchProc2(Uint8 rfCh, Uint8 timeSlot)
*
* Description: Process downlink PAGCH burst 0
*
* Cautions: rfCh is '0' for current situation
*           timeSlot must be from 0 to 7
*   
* Outputs: embedded in channel structure
*
* Output: N/A
*
************************************************************/     
void dlPagchProc2(Uint8 rfCh, Uint8 timeSlot)
{
	t_DLSigBufs *pagMsg;
	t_TNInfo *ts;          
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

 chanEncoder(PAGCH, 0, timeSlot,0);			
	buildGSMBurst(PAGCH,  g_BBInfo[rfCh].tseqNum);
	GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, PAGCH);			   		   
}

/************************************************************
* Function Prototype:
*          dlPagchProc3(Uint8 rfCh, Uint8 timeSlot)
*
* Description: Process downlink PAGCH burst 0
*
* Cautions: rfCh is '0' for current situation
*           timeSlot must be from 0 to 7
*   
* Outputs: embedded in channel structure
*
* Output: N/A
*
************************************************************/      
void dlPagchProc3(Uint8 rfCh, Uint8 timeSlot)
{       
   chanEncoder(PAGCH, 0, timeSlot,0);		
   buildGSMBurst(PAGCH,  g_BBInfo[rfCh].tseqNum);
   GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, PAGCH);			   		   
}

/************************************************************
* Function Prototype:
*          dlPagchProc3(Uint8 rfCh, Uint8 timeSlot)
*
* Description: Process downlink PAGCH burst 0
*
* Cautions: rfCh is '0' for current situation
*           timeSlot must be from 0 to 7
*   
* Outputs: embedded in channel structure
*
* Output: N/A
*
************************************************************/
void ulRachProc(Uint8 rfCh, Uint8 timeSlot)
{   
	t_TNInfo *ts;
        t_chanSel decodeMsg;
        
	          
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

    /*
    *  Input samples offset by -1 to get a head room for case TOA becomes marginal to -1
    */	        
	demodulate( g_BBInfo[rfCh].ulBBSmpl-1, 
               RACH, 
               g_BBInfo[rfCh].tseqNum, 
               &g_ULBurstData, 
               ts->groupNum,
               rfCh,
               timeSlot,
               0 );              
   /*
   *  Correct effect of sample offset, but not less than zero
   */
   if ( g_ULBurstData.toa > 0 )
   {                
      g_ULBurstData.toa -= 1;
   }
                   
	/*
	*  channel decoding(Viterbi + CRC)
	*/
   switch (ts->groupNum){
   case GSM_GROUP_13:
   case GSM_GROUP_11:
           if (g_PrachBits == 11){
                   decodeMsg  = PRACH;
           } else {
                   decodeMsg  =  RACH;
           }
    break;
    default:
           decodeMsg = RACH;
     break;
   }
   
	chanDecoder(decodeMsg, 1, timeSlot);
	ulSyncMsgProcess(RACH, 0, timeSlot, rfCh);
                			
}

/************************************************************
* Function Prototype:
*          putPPCHBuff(Uint8 rfCh, Uint8 timeSlot)
*
*
* Description: put paging group into PCCH global buffer for (P)CCCH resources.
*
* Cautions: rfCh is '0' for current situation
*           timeSlot must be from 0 to 7
*   
* Outputs: overflow or assignment to a PPCH buffer index. 
*
************************************************************/       
void putPPCHBuff(Uint8 rfCh, Uint8 timeslot, Uint8 ccch, Uint8 msgDiscrim, msgStruc *p_msg)
{    
   Uint8 i, temp[5];
   t_TNInfo *ts;
   t_DLPPCHBufs  *p_dlSigBuf;
   Uint8        *wrPtr, *rdPtr;

   /*
   * find right time slot Info to be configured
   */
   ts = & g_BBInfo[rfCh].TNInfo[timeslot];

   /* brute force linear search into PCH/PPCH for now since it is less than 64 entries */
   for (i=0; i<NUM_PPCH_SUBCHANS; i++)
   {
     p_dlSigBuf = &g_dlSigBufsPPCH[0][i];

     if ( p_dlSigBuf->logCh == NULL )
     {
       p_dlSigBuf->timeslot = timeslot;

       if (msgDiscrim == RR_MANAGE_MSG)
       {
         if(ccch < NUM_PAGCH_SUBCHANS)
         {   
           p_dlSigBuf->logCh = PCH; 
           p_dlSigBuf->pageNum = ccch;  
         }
	        else if(ccch == (NUM_PAGCH_SUBCHANS+1))
         {
		         /*
		         *   AGCH message put into same queue now
		         *
	          */ 
           p_dlSigBuf->logCh = AGCH; 
           p_dlSigBuf->pageNum = ccch; 
         }
       }

       else if (msgDiscrim == PDCH_MANAGE_MSG) /* for GROUP_11, GROUP_12, PPCH and PPAGCH */
       {
          
       } 
       
       /*
       *  Copy data to global paging buffer; Reserve 2 bytes L1 Header for SACCH
       */
       wrPtr =  & (p_dlSigBuf->data[0]);
       rdPtr =  &p_msg->params[3];  
    
       INTR_DISABLE(CPU_INT8);        
       for(i=0; i<DL_SIG_MSG_SIZE; i++) wrPtr[i] = rdPtr[i];
       INTR_ENABLE(CPU_INT8);     

       p_dlSigBuf = &g_dlSigBufsPPCH[0][i];

       return;
     }     
   }

   /* if you reach this point, the buffer is full */
   ts->u.group5.ppchOverflow = 1;

   p_dlSigBuf =  &g_dlSigBufsPPCH[0][NUM_PPCH_SUBCHANS-1];
   temp[0] = p_dlSigBuf->timeslot;
   temp[1] = p_dlSigBuf->logCh;
   temp[2] = p_dlSigBuf->pageNum;
   sendDiagMsg(PPCH_BUF_OVERFLOW, rfCh, timeslot, 3, &temp);
}



/************************************************************
* Function Name:
*          getPPCHBuff
*
*
* Description: get location of one paging buffer for (P)CCCH resources.
*
* Cautions: rfCh is '0' for current situation
*           timeSlot must be from 0 to 7
*   
* Outputs: NULL or a PPCH buffer index. 
*
************************************************************/       
Uint8 getPPCHBuff(Uint8 rfCh, Uint8 timeSlot)
{    
   Uint8 i, temp[5];
   t_TNInfo *ts;
   t_DLPPCHBufs  *p_dlSigBuf;

   /*
   * find right time slot Info to be configured
   */
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
    
   /* brute force linear search into PCH/PPCH for now since it is less than 64 entries */
   for (i=0; i<NUM_PPCH_SUBCHANS; i++)
   {
     p_dlSigBuf = &g_dlSigBufsPPCH[rfCh][i];

     if ((p_dlSigBuf->logCh == PCH) && (timeSlot == p_dlSigBuf->timeslot))
     {
       if ( ts->u.group5.dlPpchSubCh == p_dlSigBuf->pageNum)
       {
         /* DEBUG!!
         //temp[0] = i;
         //temp[1] = p_dlSigBuf->timeslot;
         //temp[2] = p_dlSigBuf->logCh;
         //temp[3] = p_dlSigBuf->pageNum;
         //temp[4] = ts->u.group5.dlPpchSubCh;
         //sendDiagMsg(0x9d, rfCh, timeSlot, 5, &temp);
         */

         p_dlSigBuf->msgReady++;
         return (i);
       }
     }  
     
     else if ((p_dlSigBuf->logCh == AGCH) && (timeSlot == p_dlSigBuf->timeslot))
     {
       p_dlSigBuf->msgReady++;
       return (i);   
     }
   }
}

/************************************************************
* Function Name:
*          availablePPCHBuff
*
*
* Description: find remaining number of open paging buffers for (P)CCCH resources.
*
* Cautions: rfCh is '0' for current situation
*           timeSlot must be from 0 to 7
*   
* Outputs: NULL or a PPCH buffer index. 
*
************************************************************/       
Uint8 availablePPCHBuff(Uint8 rfCh, Uint8 timeSlot)
{    

   Uint8 i, usedBufs;
   t_TNInfo *ts;
   t_DLPPCHBufs  *p_dlSigBuf;
 
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

   /* brute force linear search into PCH/PPCH for now since it is less than 64 entries */
   for (i=0; i<NUM_PPCH_SUBCHANS; i++)
   {
     p_dlSigBuf = &g_dlSigBufsPPCH[0][i];

     if (p_dlSigBuf->logCh != NULL ) usedBufs++;
   }

   return(usedBufs);
}
