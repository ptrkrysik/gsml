/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: pdch.c
*
* Description:
*   This file contains functions that process GPRS frames.
*
******************************************************************************/
#include "stdlib.h"
#include "gsmdata.h"
#include "bbdata.h"   
#include "rrmsg.h"
#include "dsprotyp.h"
#include "agcdata.h" 
#include "diagdata.h" 

Uint32 pdchReceived = 0; 
  
Uint8 dlPacketDummyFrame[23] = { 0x40, 0x94, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B,
   0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B};


/*****************************************************************************
*
* Function: ulChan2TbfMap
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/ 
tbfStruct * ulChan2TbfMap(Uint8 rfCh, Uint8 timeSlot)
{     
   Uint16         i;
   tbfStruct      *tbf;
   t_TNInfo       *ts; 
   Uint8          currentUSF; 
   Uint8          offSetFN;
   t_tagFillCause fillCause;
   tbfStruct      *tbfVal;
   
   
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
   

   switch (ts->groupNum){


   case GSM_GROUP_11:
          {
             currentUSF = ts->u.group11.currentUsf;
             fillCause = ts->u.group11.blkNumWatch[ts->u.group11.ulBlock].tagFillCause;
             tbfVal    = ts->u.group11.blkNumWatch[ts->u.group11.ulBlock].tbf;
             
             break;
          }

                           

   case GSM_GROUP_13: 
   {
           currentUSF = ts->u.group13.currentUsf;
  /*
   *  Check watch table to see whether this uplink RLC block is reserved for
   *  single blk assignment, DL assignment response or relative reserved blk
   */

           fillCause = ts->u.group13.blkNumWatch[ts->u.group13.ulBlock].tagFillCause;
           tbfVal    = ts->u.group13.blkNumWatch[ts->u.group13.ulBlock].tbf;
                   
           break;
   }

   default:
   {
           return(NULL);
           break;
   }
   }
   
           
  
                        
   
           
   
   if ( fillCause == SBA || fillCause == DLA )
   {
      return(NULL);
   }
   else if ( fillCause == RRB_AB || fillCause == RRB_NB )
   {
      return(tbfVal);
   }

   /*
   *  If this block has not been reserved, then search all TBFs to find a match
   */     
   for(i=0; i<TBF_NUMBER_MAX; i++)
   {  
      tbf = & g_tbfInfo[rfCh][i];     
      /*
      *  check TBF State, Time Slot, USF on the TimeSlot to find out right TBF
      */  
      if( tbf->ulTbf.state == CHAN_ACTIVE &&
          (((tbf->ulTbf.timeSlotAllocation) >> timeSlot) & 0x01) )
      {  
       if(g_loopBackMode == LOCAL_LOOPBACK)
          return(tbf);
       else
        {
         /*
         *  For now, handle dynamic mode only
         */
                  
         if( tbf->ulTbf.tbfMode == TBF_DYNAMIC && 
           ((tbf->ulTbf.opMode.dynamic.usfTsMap >> (timeSlot<<2)) & 0x07) == currentUSF )         
         {
            return(tbf);      
         }
        }
      }

   }
   return(NULL);
}
                               
/*****************************************************************************
*
* Function: dlChan2TbfMap
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/ 
tbfStruct * dlChan2TbfMap(Uint8 rfCh, Uint8 timeSlot, Uint8 TFI, Uint8 dir)
{     
   Uint16 i;
   tbfStruct *tbf; 

   /*
   *  search all TBFs  to find a match
   */     
   for(i=0; i<TBF_NUMBER_MAX; i++)
   {  
      tbf = & g_tbfInfo[rfCh][i];

      /*
      *  check TBF State, Time Slot Allocation, and TFI on the TimeSlot to find out right TBF
      *  If dir==0, the input TFI is an uplink TFI. If dir==1, it's a downlink TFI.
      */
      if( tbf->dlTbf.state == CHAN_ACTIVE &&
          (((tbf->dlTbf.timeSlotAllocation) >> timeSlot) & 0x01) )
      {
       if(g_loopBackMode == LOCAL_LOOPBACK)
         return(tbf);  
       else
       {    
        if( (dir==0 && tbf->ulTbf.tfi==TFI) || (dir==1 && tbf->dlTbf.tfi==TFI) ) 
        {            
            /***** >>>> temporary diag
            Uint8 tempBuf[20]; 
            tempBuf[0] = rfCh;
            tempBuf[1] = timeSlot;
            tempBuf[2] = TFI;
            tempBuf[3] = dir;
            tempBuf[4] = (Uint16)tbf >> 8;
            tempBuf[5] = (Uint16)tbf & 0xff;
            tempBuf[6] = (tbf->TLLI >> 24) & 0xff;
            tempBuf[7] = (tbf->TLLI >> 16) & 0xff;
            tempBuf[8] = (tbf->TLLI >>  8) & 0xff;
            tempBuf[9] = (tbf->TLLI      ) & 0xff;
            tempBuf[10] = tbf->ulTbf.state;
            tempBuf[11] = tbf->dlTbf.state;
            tempBuf[12] = tbf->ulTbf.tfi;
            tempBuf[13] = tbf->dlTbf.tfi;
            tempBuf[14] = tbf->ulTbf.timeSlotAllocation;
            tempBuf[15] = tbf->dlTbf.timeSlotAllocation;
            sendDiagMsg(37, rfCh, timeSlot, 16, tempBuf);
             */
          
           return(tbf); 
        }
       }  
      }  
 
   }
   return(NULL);
}

/*****************************************************************************
*
* Function: ulPdchProc0
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/ 
void ulPdchProc0(Uint8 rfCh, Uint8 timeSlot)
{   
   tbfStruct      *TBF;         
   t_TNInfo       *ts;
   Uint8          currentUSF;
   Uint8          subChan = 0;
   t_tagFillCause fillCause;
   
		          
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

   /*
   *  Search for 4 consecutive access bursts and send an acknowledgement to the Host.
   *  The acknowledgement can be for PACCH (ack or unack mode) or PDTCH (unack mode only).
   */

   if (ts->groupNum == GSM_GROUP_13){
           fillCause = ts->u.group13.blkNumWatch[ts->u.group13.ulBlock].tagFillCause ;
   } else {

           fillCause = ts->u.group11.blkNumWatch[ts->u.group11.ulBlock].tagFillCause;
           if ( ts->u.group11.currentUsf == USF_FREE){
                   ulPrachProc0(rfCh, timeSlot);
                   return;
           }
           
           
   }
   
   
           
   if((fillCause == RRB_AB) ||(fillCause == DLA))
   {
     ulRachProc(rfCh, timeSlot);    
   }

   else if (fillCause == GRP13_AB_FOUND) 
   { 
     return;
   }

   else  /* process as a normal burst */
   {
     
     if( !(TBF = ulChan2TbfMap(rfCh, timeSlot)) )
     {        
       g_ULCCHData.chCodec = GPRS_CS_1;
     }
     else
     {  
       g_ULCCHData.chCodec = TBF->ulTbf.channelCodec;       
     }
     
     DCOffset( g_BBInfo[rfCh].ulBBSmpl, ts->groupNum, rfCh, timeSlot );

     demodulate( g_BBInfo[rfCh].ulBBSmpl, 
               PDCH, 
               g_BBInfo[rfCh].tseqNum, 
               &g_ULBurstData,
               ts->groupNum,
               rfCh,
               timeSlot,
               subChan );              
                                  
     chanDecoder(PDCH, 0, timeSlot);
   
     if ( TBF != NULL )
     {
       accumPower(rfCh, timeSlot, ts->groupNum, PDCH, NULL, TBF);
       accumToa  (rfCh, timeSlot, ts->groupNum, 0, PDCH, TBF);
     }
  }
}


/*****************************************************************************
*
* Function: ulPdchProc3
*
* Description:
*   Processes uplink PDCH bursts for frame 3 of every 0-3    block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void ulPdchProc3(Uint8 rfCh, Uint8 timeSlot)
{   
   tbfStruct   *TBF;
   t_TNInfo    *ts;
   Uint8       currentUSF;
   Uint8       subChan = 0;	
   t_SACCHInfo *sacchInfo;
   t_ULSignalStrength *sigS;
   t_blkNumWatch *fillBlk;
   Uint8        *blkNum;
   Uint8        tmp[3];
   
   
   

   
    ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
  
   if (ts->groupNum == GSM_GROUP_13){
           fillBlk = &ts->u.group13.blkNumWatch[ts->u.group13.ulBlock];
           blkNum   = &(ts->u.group13.ulBlock);
   } else {
           fillBlk = &ts->u.group11.blkNumWatch[ts->u.group11.ulBlock];
           blkNum   = &(ts->u.group11.ulBlock);
           if ( ts->u.group11.currentUsf == USF_FREE){
                   ulPrachProc3(rfCh, timeSlot);
                   return;
           }
           
        
   }
  
    
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
   
   /*
   *  Search for 4 consecutive access bursts and send an acknowledgement to the Host.
   *  The acknowledgement can be for PACCH (ack or unack mode) or PDTCH (unack mode only).
   */
   if((fillBlk->tagFillCause== RRB_AB) ||(fillBlk->tagFillCause== DLA))
   {
     fillBlk->tagFillCause = LAST_CHANCE_4_AB_DET;
     ulRachProc(rfCh, timeSlot);
 
     /* clear this block from the watch table */  
     fillBlk->tbf = NULL;
     fillBlk->tagFillCause = NONE;
   }

   else if (fillBlk->tagFillCause == GRP13_AB_FOUND)
   { 
     /* clear this block from the watch table */  
     fillBlk->tbf = NULL;
     fillBlk->tagFillCause = NONE;
   }

   else  /* process as a normal burst */
   {

     if( !(TBF = ulChan2TbfMap(rfCh, timeSlot)) )
     {        
       g_ULCCHData.chCodec = GPRS_CS_1;
     }
     else
     {
       g_ULCCHData.chCodec = TBF->ulTbf.channelCodec;
     }
     
     DCOffset( g_BBInfo[rfCh].ulBBSmpl, ts->groupNum, rfCh, timeSlot );

     demodulate( g_BBInfo[rfCh].ulBBSmpl, 
               PDCH, 
               g_BBInfo[rfCh].tseqNum, 
               &g_ULBurstData,
               ts->groupNum,
               rfCh,
               timeSlot,
               subChan );              
                        
     /*
     *  frame boundary is 1
     */                	
     chanDecoder(PDCH, 1, timeSlot);
                                                                                          
     if(!g_ULCCHData.ulcchword0.bitval.fireStatus)
     {
       pdchReceived++;
     }
    
     /*
     *  Accumulate power, TOA, bit errors and RLC block errors into 
     *  UL TBF accum structure
     */
     if ( TBF != NULL )
     {
       accumPower(rfCh, timeSlot, ts->groupNum, PDCH4, NULL, TBF); 
       accumToa  (rfCh, timeSlot, ts->groupNum, 0, PDCH4, TBF);   		
       accumBitBlockErrs(rfCh, timeSlot, PDCH4, TBF); 
              
       /*-----------------------------------------------------------
       * Update AGC on final burst of RLC block
       *----------------------------------------------------------*/
       if(g_AGC_ON)
       {
         sigS = & TBF->ulTbf.sigstren;	    
         sigS->rxPwrNormSmooth = ul_agc(sigS);  
       }
     }
      
     ulSyncMsgProcess(PDCH, subChan, timeSlot, rfCh);          

     /*-------------------------------------------------------------
     * If we detect PDTCH (via g_rateChangeCheck), then check if code 
     *  rate selection needs to be done.  If so, notify RLC-MAC layer  
     *-------------------------------------------------------------*/
     if ( TBF != NULL )
     {
              if((g_RATE_TRAN_ON) && (g_loopBackMode != LOCAL_LOOPBACK) && (g_rateChangeCheck))
	      rateTransition(rfCh, TBF);

     }

     /* clear this block from the watch table */  
     fillBlk->tbf = NULL;
     fillBlk->tagFillCause = NONE;
   }  

   /*
   * updata GPRS Block Index
   */
   if(++(*blkNum) >= BLOCKS_PER_GPRS_MULTI_FRAMES) *blkNum = 0;
   

           
   
}

/*****************************************************************************
*
* Function: dlPdchProc0
*
* Description:
*   Processes downlink PDCH frames and burst 0
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dlPdchProc0(Uint8 rfCh, Uint8 timeSlot)
{   
   t_TNInfo       *ts;
   tbfStruct      *TBF;
   t_dlPdtchBufs  *pdtchMsg;
   t_DLSigBufs    *pacchMsg;
   Uint8          *payload;
   Uint8          subChan = 0;
   Bool           msgReady = FALSE;
   Bool           incrementCount = FALSE;
   Uint8          tempBuf[64], i;
   Uint8          blockNumber;
   Uint8          dir;
   Uint8          tfi;
   Uint8          chCodec, tag, ackBurstType;
   Uint8          dlBlock;
   t_blkNumWatch *fillBlk;
   
			
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
    
   /*
   *  initialize Channel Codec with Default value CS_1
   */                           
   g_DLCCHData.chCodec = GPRS_CS_1;   


   /*
      *  PACCH message has higher priority
      */


   switch( ts->groupNum){
      case GSM_GROUP_13:    
        pacchMsg = ts->u.group13.pacch.dlSigBuf;
        pdtchMsg = ts->u.group13.pdtch.dlSigBuf;
        dlBlock  = ts->u.group13.dlBlock;
        
      break;

      case GSM_GROUP_11:
        pacchMsg = ts->u.group11.pacch.dlSigBuf;
        pdtchMsg = ts->u.group11.pdtch.dlSigBuf;
        dlBlock  = ts->u.group11.dlBlock;

        
      break;

      default:
           break;
   }
   
            
           

      
      tag          = pacchMsg->tag[pacchMsg->readIndex];
      ackBurstType = pacchMsg->burstType[pacchMsg->readIndex];
      payload      = pacchMsg->data[pacchMsg->readIndex];
       
      if ( pacchMsg->numMsgsReady )
      {
              /*
         *  Test to see if Packet Uplink Ack/Nack; "sniff" out its' coding rate to the ulTbf.channelCodec
         */
         if ( (payload[1]>>2) == 9)
         {
           if (TBF = ulChan2TbfMap(rfCh, timeSlot) )
           {
              TBF->ulTbf.channelCodec = payload[3]>>6;
/**** >>> temporary diag w*/             
             tempBuf[0] = payload[1]>>2;
             tempBuf[1] = payload[3]>>6;
             tempBuf[2] = TBF->TLLI >> 24;
             tempBuf[3] = TBF->TLLI >> 16;
             tempBuf[4] = TBF->TLLI >> 8; 
             tempBuf[5] = TBF->TLLI;
             sendDiagMsg(0x03, rfCh, timeSlot, 6, tempBuf);

           }
         }


        /*  If next Uplink Block is reserved, set the outgoing USF to the
         *  reserved (unused by MS) value so that no other MS will transmit
         */
         blockNumber = (dlBlock + 1) % BLOCKS_PER_GPRS_MULTI_FRAMES;
     
         /*
         *  assert USF used, need to check if  extra delay on uplink ????!!!!!
         *  for AGC and Power Control Purpose
         */
              switch (ts->groupNum) {
                case GSM_GROUP_11:
                        ts->u.group11.currentUsf = ts->u.group11.nextUsf;
                        if ( ts->u.group11.blkNumWatch[blockNumber].tagFillCause != NONE ){
                          ts->u.group11.nextUsf = USF_RESERVED;  
                          payload[0] &= 0xf8;
                          payload[0] |= USF_RESERVED;
                        }else{
                           ts->u.group11.nextUsf = payload[0] & 0x7;       
                         }
                 break;
                 default:
                        ts->u.group13.currentUsf = ts->u.group13.nextUsf;
                        if ( ts->u.group13.blkNumWatch[blockNumber].tagFillCause != NONE ){
                           ts->u.group13.nextUsf = USF_RESERVED;  
                           payload[0] &= 0xf8;
                           payload[0] |= USF_RESERVED;
                         }else{
                            ts->u.group13.nextUsf = payload[0] & 0x7;       
                          }
                  break;
              }
         
         

         /*
         *  If this DL block schedules a Relative Reserved Block, S/P bit (4th) will be set
         */
         if ( (payload[0]>>3) & 0x01 ){
             /*
             *  If the future block is not already reserved for SBA or DLA, then reserve it as RRB
             */
                 blockNumber = (dlBlock + 3 + ((payload[0]>>4) & 0x3)) % BLOCKS_PER_GPRS_MULTI_FRAMES;

                 switch(ts->groupNum){

                 case GSM_GROUP_13:
                         fillBlk =  &ts->u.group13.blkNumWatch[blockNumber];
                         break;

                 case GSM_GROUP_11:
                         fillBlk =  &ts->u.group11.blkNumWatch[blockNumber];
                         break;
                 }
                 
                if ( fillBlk->tagFillCause == NONE ){
                    if ( ackBurstType == ACCESS_BURST ){
                       fillBlk->tagFillCause = RRB_AB;
                     } else{
                      fillBlk->tagFillCause = RRB_NB;
                     }
                fillBlk->tag = tag;
                tfi = (payload[2]>>1) & 0x1f;
                dir = payload[2] & 0x1;
                fillBlk->tbf = dlChan2TbfMap(rfCh, timeSlot, tfi, dir);

                msgReady = TRUE;
                }
         }else
         {
             msgReady = TRUE;
         }
         /*
         *  If the PACCH section of code set msgReady, then send PACCH. The only negative
         *  case is to avoid an uplink collision, in which case a dummy block is sent below.
         */
         if ( msgReady ){
            processCCHBuff(pacchMsg) ;       
            rtsCheck(rfCh, timeSlot, PACCH, subChan, DL_SIG_Q_DEPTH - pacchMsg->numMsgsReady);
         }
      }else{      
         /*
         *  PDTCH message has lower priority than PACCH
         */       


              
         tag          = pdtchMsg->tag[pdtchMsg->readIndex];
         ackBurstType = pdtchMsg->burstType[pdtchMsg->readIndex];
         chCodec      = pdtchMsg->chCodec[pdtchMsg->readIndex];
         payload      = (Uint8*)pdtchMsg->buffer[pdtchMsg->readIndex];
         /* Note: payload now points to first byte of PDTCH message */  

/* simulate queued pdtch messages during loopback mode */
         if(g_loopBackMode == LOCAL_LOOPBACK)
            pdtchMsg->frameCount = 0xff;

         if ( pdtchMsg->frameCount )
         {  
             /*
             *  check if the message is associated with an active TBF(rfCh, TS, TFI)
             */                      
             tfi = (payload[1]>>1) & 0x1f;
             dir = 1;  // DL direction always

             if( (TBF = dlChan2TbfMap(rfCh, timeSlot, tfi, dir)) )
             {        
                /*
                *  use assigned channel codec for the active TBF (this one byte preceeds the data bytes)
                *  ...new...use DSPMAIN defined value if local loopback test
                */
                if(g_loopBackMode == LOCAL_LOOPBACK)
                 g_DLCCHData.chCodec =  TBF->dlTbf.channelCodec;             
                else
                 g_DLCCHData.chCodec =  chCodec;             



                switch (ts->groupNum){
                case GSM_GROUP_13:
                        
                  /*
                   *  assert USF used, need to check if extra delay on uplink ????!!!!!
                   *  for AGC and Power Control Purpose
                   */                  
                   ts->u.group13.currentUsf = ts->u.group13.nextUsf;

                   /*
                    *  If next Uplink Block is already reserved, set outgoing USF to the value that 
                    *  ensures no other MS will transmit. If not reserved, save USF from DL block.
                   */ 
                   blockNumber = (ts->u.group13.dlBlock + 1) % BLOCKS_PER_GPRS_MULTI_FRAMES;
                  if ( ts->u.group13.blkNumWatch[blockNumber].tagFillCause != NONE )
                   {
                     ts->u.group13.nextUsf = USF_RESERVED;  
                     payload[0] &= 0xf8;
                     payload[0] |= USF_RESERVED;
                   }
                  else
                  {
                    ts->u.group13.nextUsf = payload[0] & 0x7;       
                   } 

                  break;

                default:
                        
 /*
                   *  assert USF used, need to check if extra delay on uplink ????!!!!!
                   *  for AGC and Power Control Purpose
                   */                  
                   ts->u.group11.currentUsf = ts->u.group11.nextUsf;

                   /*
                    *  If next Uplink Block is already reserved, set outgoing USF to the value that 
                    *  ensures no other MS will transmit. If not reserved, save USF from DL block.
                   */ 
                   blockNumber = (ts->u.group11.dlBlock + 1) % BLOCKS_PER_GPRS_MULTI_FRAMES;
                  if ( ts->u.group11.blkNumWatch[blockNumber].tagFillCause != NONE )
                   {
                     ts->u.group11.nextUsf = USF_RESERVED;  
                     payload[0] &= 0xf8;
                     payload[0] |= USF_RESERVED;
                   }
                  else
                  {
                    ts->u.group11.nextUsf = payload[0] & 0x7;       
                   } 

                  break;
                  
                }
                
                /*
                *  If this DL block schedules a Relative Reserved Block, S/P bit (4th) will be set
                */
                if ( (payload[0]>>3) & 0x01 )  
                {
                 

                   switch(ts->groupNum){

                   case GSM_GROUP_13:
                             /*
                   *  If the future block is not already reserved for SBA or DLA, then reserve it as RRB
                   */
                         blockNumber = (ts->u.group13.dlBlock + 3 + ((payload[0]>>4) & 0x3)) % BLOCKS_PER_GPRS_MULTI_FRAMES;
                         fillBlk =  &ts->u.group13.blkNumWatch[blockNumber];
                         break;

                   case GSM_GROUP_11:
                            /*
                   *  If the future block is not already reserved for SBA or DLA, then reserve it as RRB
                   */
                         blockNumber = (ts->u.group11.dlBlock + 3 + ((payload[0]>>4) & 0x3)) % BLOCKS_PER_GPRS_MULTI_FRAMES; 
                         fillBlk =  &ts->u.group11.blkNumWatch[blockNumber];
                         break;
                  }

                   if ( fillBlk->tagFillCause == NONE )
                   {
                      if ( ackBurstType == ACCESS_BURST )
                      {
                        fillBlk->tagFillCause = RRB_AB;
                      }
                      else
                      {
                        fillBlk->tagFillCause = RRB_NB;
                      }
                      fillBlk->tag = tag;
                      fillBlk->tbf = TBF;

                      msgReady = TRUE;
                   }
                   
      
                }
                else
                {
                   msgReady = TRUE;
                }
                /*
                *  If the PDTCH section of code set msgReady, then continue to construct PDTCH msg.
                *  The only negative case is to avoid an uplink collision.
                */
                if ( msgReady )
                {
                   
                   /*
                   *  Added Channel Coding for USF 
                   */                                                               
                   switch(g_DLCCHData.chCodec)
                   {
                   case GPRS_CS_1:
                     /*
                     *  unpack a RLC Frame; channelCodec has to be CS_1
                     */     
                     unpackFromByteBuffer(payload,
                                   (UChar*)g_DLCCHData.data, 
                                   g_GPRSCodecBits[g_DLCCHData.chCodec]);
                     break;
                  
                   case GPRS_CS_2:
                   case GPRS_CS_3:                  
                     /*
                     *  unpack a RLC Frame; channelCodec has to be CS_2 or CS_4 
                     */     
                     unpackFromByteBuffer(payload,
                                   (UChar*)&g_DLCCHData.data[3], 
                                   g_GPRSCodecBits[g_DLCCHData.chCodec]);                   
                     /*
                     *  unpack 6-bit Pre-Codec USF 
                     
                     unpackFromByteBuffer((UChar*)USFCodingTableCS23[ts->u.group13.nextUsf],
                                   (UChar*)&g_DLCCHData.data[0], 
                                   6);                   
                     */
                     break;                                                 
                  
                   case GPRS_CS_4:                     
                                                                       
                     /*
                     *  unpack a RLC Frame; channelCodec has to be CS_4 
                      
                     unpackFromByteBuffer((UChar*)pdtchMsg->buffer[pdtchMsg->readIndex],
                                   (UChar*)&g_DLCCHData.data[9], 
                                   g_GPRSCodecBits[g_DLCCHData.chCodec]);                   
                     */             
                     unpackFromByteBuffer(payload,
                                   (UChar*)&g_DLCCHData.data[9], 
                                   g_GPRSCodecBits[g_DLCCHData.chCodec]); 
                     /*
                     *  unpack 12-bit Codec USF 
                      
                     unpackFromByteBuffer((UChar*)USFCodingTableCS4[ts->u.group13.nextUsf],
                                   (UChar*)&g_DLCCHData.data[0], 
                                   12);  
                     */              
                     break;
                  
                   default:
                     break;
                   }
                   /* Message will be sent. Set flag to increment read index and decrement frameCount. */
                   incrementCount = TRUE;
                }
            }
            else
            {
                /* TBF not found. Flush msg. Set flag to increment read index and decrement frameCount. */
                incrementCount = TRUE;
            }
            /* If msg will be sent or TBF was not found, increment read index and decrement frameCount */
            if ( incrementCount )
            {
                /*
                *  If msg is ready to send, update read index and frame count
                */
                if(++pdtchMsg->readIndex >= NUM_TCH_JBUF)  pdtchMsg->readIndex = 0;
                pdtchMsg->frameCount--;     
                /*
                *  Report available Buffers to host RLC/MAC layer for flow control
                */                                                                   
                rtsCheck(rfCh, timeSlot, PDTCH, subChan, (NUM_TCH_JBUF - pdtchMsg->frameCount));
            }                         
          }
          else
          {
             /*
             *  Send Packet PSI13 if it is available
             */
            if(g_dlSigBufsSYSINFO[0][PACKET_TYPE_13].valid)
            {
                g_dlSigBufsSYSINFO[0][PACKET_TYPE_13].data[g_dlSigBufsSYSINFO[0][PACKET_TYPE_13].state][0] |= USF_RESERVED;
                unpackFromByteBuffer
                (
                   (UChar*)&g_dlSigBufsSYSINFO[0][PACKET_TYPE_13].data[g_dlSigBufsSYSINFO[0][PACKET_TYPE_13].state],
                   (UChar*)g_DLCCHData.data, 
                   NUM_BITS_CCH_FRAME
                );	
                msgReady = TRUE;      
            }                               
         }
       }

   /*
   *  always send PDCH filler if no message to send
   */	        
   if(!msgReady)
   {
      /*
      *   use current USF for dummy frame
      */
      //dlPacketDummyFrame[0] |= ts->u.group13.currentUsf;
      dlPacketDummyFrame[0] |= USF_RESERVED;             /* USE TBF Free for PRACH */

      unpackFromByteBuffer(dlPacketDummyFrame,
                           (UChar *)&g_DLCCHData.data[0], 
                           NUM_BITS_CCH_FRAME);								 	
   }

   g_DLCCHData.bufferValid = TRUE;	                               		
   chanEncoder(PDCH, 1, timeSlot,0);              
   buildGSMBurst(SDCCH4,  g_BBInfo[rfCh].tseqNum);
   GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, SDCCH4);
   /*
   * update GPRS Block Index
   */
   if (ts->groupNum == GSM_GROUP_13){     
           if(++ts->u.group13.dlBlock >= BLOCKS_PER_GPRS_MULTI_FRAMES) ts->u.group13.dlBlock = 0;
   } else {
           if(++ts->u.group11.dlBlock >= BLOCKS_PER_GPRS_MULTI_FRAMES) ts->u.group11.dlBlock = 0;
   }
   
           
   
}

/*****************************************************************************
*
* Function: dlPdchProc3
*
* Description:
*   Processes downlink PDCH frames and burst 1-3
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dlPdchProc3(Uint8 rfCh, Uint8 timeSlot)
{                    
   t_TNInfo    *ts;          
   t_DLSigBufs *sdcchMsg;

   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
			
	  chanEncoder(PDCH, 0, timeSlot,0);              
	  buildGSMBurst(SDCCH4,  g_BBInfo[rfCh].tseqNum);
	  GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, SDCCH4);	
}

/*****************************************************************************
*
* Function: dlPtcchProc0
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/ 
void dlPtcchProc0(Uint8 rfCh, Uint8 timeSlot)
{   
   t_TNInfo         *ts;
   t_DLSigBufs      *ptcchMsg;
   Uint8            subChan = 0;
   Uint8            temp[10];
         
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

   /*
   *  initialize Channel Codec with Default value CS_1
   */                           
   g_DLCCHData.chCodec = GPRS_CS_1;   
      
   switch(ts->groupNum)
   {
   case GSM_GROUP_13:
      
      if (ts->u.group13.currentTAI >= 0 && ts->u.group13.currentTAI <= 15) {
        ptcchMsg = ts->u.group13.ptcch.dlSigBuf;
      }	  

      break;
      
     case GSM_GROUP_11:
      
      if (ts->u.group11.currentTAI >= 0 && ts->u.group11.currentTAI <= 15) {
        ptcchMsg = ts->u.group11.ptcch.dlSigBuf;
      }	  
   
   break;
 default:         
   break;
}

      
      /*
      *  check if Timing Advance / Power Control Message is required to send
      */
      if(ptcchMsg->numMsgsReady)
      {
         /*
         *  assert USF used, need to check if  extra delay on uplink ????!!!!!
         *  for AGC and Power Control Purpose
         */

              switch (ts->groupNum){
              case GSM_GROUP_13:       
                ts->u.group13.currentUsf = ts->u.group13.nextUsf;
                ts->u.group13.nextUsf = ptcchMsg->data[ptcchMsg->readIndex][0] & 0x7;
              break;

              case GSM_GROUP_11:
                ts->u.group11.currentUsf = ts->u.group11.nextUsf;
                ts->u.group11.nextUsf = ptcchMsg->data[ptcchMsg->readIndex][0] & 0x7;

                break;

              default:
              break;
              }
              
         processCCHBuff(ptcchMsg);       
         rtsCheck(rfCh, timeSlot, PTCCH, subChan, DL_SIG_Q_DEPTH-ptcchMsg->numMsgsReady);         
      }  
      else if (g_dlSigBufsSYSINFO[0][PACKET_TYPE_13].valid)
      {    
         /*
         *  Otherwise, Send Packet System Info 13
         */ 
         unpackFromByteBuffer
         (
            g_dlSigBufsSYSINFO[0][PACKET_TYPE_13].data[g_dlSigBufsSYSINFO[0][PACKET_TYPE_13].state],
            (UChar*)g_DLCCHData.data, 
            NUM_BITS_CCH_FRAME
         );		
      }
      else
      {
         /*
         *   use current USF for dummy frame
         */
         dlPacketDummyFrame[0] |= ts->u.group13.currentUsf; // is this needed?

         unpackFromByteBuffer( dlPacketDummyFrame,
                              (UChar *)&g_DLCCHData.data[0], 
                              NUM_BITS_CCH_FRAME);			 		   
      }
      


	                         
   g_DLCCHData.bufferValid = TRUE;	                               		
   chanEncoder(PTCCH, 1, timeSlot,0);              
   buildGSMBurst(PTCCH,  g_BBInfo[rfCh].tseqNum);
   GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, SDCCH4);
}

/*****************************************************************************
*
* Function: dlPtcchProc3
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/ 
void dlPtcchProc3(Uint8 rfCh, Uint8 timeSlot)
{  

   t_TNInfo    *ts;            
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
			
	  chanEncoder(PTCCH, 0, timeSlot,0);              
	  buildGSMBurst(PTCCH,  g_BBInfo[rfCh].tseqNum);
	  GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, SDCCH4);	

}
/*****************************************************************************
*
* Function: dlPtcchIdleProc
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/ 
void dlPtcchIdleProc(Uint8 rfCh, Uint8 timeSlot)
{
   Void (** funcTbl ) ();
   t_TNInfo *ts;          
	          
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

   switch (ts->groupNum){
 
   case GSM_GROUP_13:
           
   funcTbl = (void *) & dlPtcchTable[ts->u.group13.dlPtcchIdleIndex];
   (* funcTbl)(rfCh, timeSlot);
   
   /*
   *  circular function table size is 8
   */                                          
   ts->u.group13.dlPtcchIdleIndex++;
   ts->u.group13.dlPtcchIdleIndex &= 0x07;
   break;



   case GSM_GROUP_11:
           
   funcTbl = (void *) & dlPtcchTable[ts->u.group11.dlPtcchIdleIndex];
   (* funcTbl)(rfCh, timeSlot);
   
   
   /*
   *  circular function table size is 8
   */                                          
   ts->u.group11.dlPtcchIdleIndex++;
   ts->u.group11.dlPtcchIdleIndex &= 0x07;
   break;
  default:
          break;
   }
   




   
   
}

/*****************************************************************************
*
* Function: ulPtcchIdleProc
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/ 
void ulPtcchIdleProc(Uint8 rfCh, Uint8 timeSlot)
{                          
   Void (** funcTbl ) ();
   t_TNInfo *ts;          
       
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

   switch (ts->groupNum){
   case GSM_GROUP_13:     
      funcTbl = (void *) & ulPtcchTable[ts->u.group13.ulPtcchIdleIndex];
      (* funcTbl)(rfCh, timeSlot);

      ts->u.group13.ulPtcchIdleIndex++;
      ts->u.group13.ulPtcchIdleIndex &= 0x01;
   break;

   case GSM_GROUP_11:     
      funcTbl = (void *) & ulPtcchTable[ts->u.group11.ulPtcchIdleIndex];
      (* funcTbl)(rfCh, timeSlot);

      ts->u.group11.ulPtcchIdleIndex++;
      ts->u.group11.ulPtcchIdleIndex &= 0x01;
   break;
   default:

           break;
           
   
   }

}  

/*****************************************************************************
*
* Function: ulPtcchIdleProc0
*
* Description:
*   Packet Timing Control Channel Processing
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/ 
void ulPtcchIdleProc0(Uint8 rfCh, Uint8 timeSlot)
{

}


/*****************************************************************************
*
* Function: dlPtcchIdleProc0
*
* Description:
*   Packet Timing Control Channel Processing
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/ 
void dlPtcchIdleProc0(Uint8 rfCh, Uint8 timeSlot)
{

}

/*****************************************************************************
*
* Function: dlPtcchIdleProc3
*
* Description:
*   Packet Timing Control Channel Processing
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/ 
void dlPtcchIdleProc3(Uint8 rfCh, Uint8 timeSlot)
{

}

                           


/*****************************************************************************
*
* Function: ulPtcchProc
*
* Description:
*   Packet Timing Control Channel Processing
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/ 
void ulPtcchProc(Uint8 rfCh, Uint8 timeSlot)
{         
   t_TNInfo *ts;          
   Uint16 ptcchReg;

   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
		
   /*
   *  Demodulate a PRACH burst
   */
   demodulate( g_BBInfo[rfCh].ulBBSmpl, 
               RACH, 
               g_BBInfo[rfCh].tseqNum, 
               &g_ULBurstData,
               ts->groupNum,
               rfCh,
               timeSlot,
               0 );              
   /*
   *  Decode a PRACH frame
   */                  
   chanDecoder(PTCCH, 1, timeSlot);	
       
  
	                                
  
   if (ts->groupNum == GSM_GROUP_13){
  /*
   *  Process a PTCCH message
   */        
         ulSyncMsgProcess(PTCCH, ts->u.group13.currentTAI, timeSlot, rfCh);   
   
   /*
   *  TAI circulation through 0-15
   */
           
           if(++ts->u.group13.currentTAI >= 15) ts->u.group13.currentTAI = 0;
   } else {

	    /*
         *  Process a PTCCH message
         */        
         ulSyncMsgProcess(PTCCH, ts->u.group11.currentTAI, timeSlot, rfCh);   

           if(++ts->u.group11.currentTAI >= 15) ts->u.group11.currentTAI = 0;
   }
   
   
}             
                                              







   
   

void UpdateGroupXITableUL(Uint8 rfCh, Uint8 timeSlot);
void UpdateGroupXITableDL(Uint8 rfCh, Uint8 timeSlot);



/**********************************************************
 *	Routine: void dlBcchProc0(Uint8 rfCh, Uint8 timeSlot)              
 *
 *	Description:
 *   Synchronousely process 1st of 4 PBCCH bursts,
 *   Channel encode is required
 *
 *	History: Change Mapping from GSM 05.02-820 Pagaraph 
 * 		
 ***********************************************************/



/*  PBCCH is used to send the PSI messages in for Group 11

    PSI1 is a mandatory message that must be sent:
    - must be sent with TC = 0
         TC = (FN div 52) mod PSI1_REPEAT

    - Make the assumption that PSI1_REPEAT is equal to
      at least 7 (1,2,3,3bis,4,5, 13). 
         

   Other mandatory messages that are sent 2, 3, and 3bis


   The assumption is that the number of instances of each
   message is equal 1
     - the spec allows for more but this number is fixed
       for ease of implementation.

   Assume that PSI_COUNT_HR = 0  - can't find how to define
   which PSI are high repitition messages

       

*/   
void dlPbcchProc0(Uint8 rfCh, Uint8 timeSlot)
{
   t_sysInfoQueue *pBcchInfo;
   t_TNInfo *ts;
   

   /* get pointer to the timeslot */
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];


   

         /* See whether it is time to send PSI 1 */
 
   if ((sysFrameNum.FN % (g11Params.psi1_cycle)) == 0){
           if ((g11Params.newAssignment == TRUE) && \
                     (ts->u.group11.dlBlock ==0)){
                     UpdateGroupXITableDL(rfCh, timeSlot);
                     ulGroupXITableInit();
                     UpdateGroupXITableUL(rfCh, timeSlot);
  	     	     g11Params.newAssignment = FALSE;   // assignment complete
           }
           
           ts->u.group11.pbcchBufIndex = PACKET_TYPE_1;
   }

   
   /* Point to the correct PSI buffer */

   pBcchInfo = ts->u.group11.pbcch.dlSigBuf + ts->u.group11.pbcchBufIndex ;


   /*  See if this a valid message to send */



   while (( pBcchInfo->valid == 0 ) && (ts->u.group11.pbcchBufIndex != PACKET_TYPE_1)){


           ++pBcchInfo;
           
           if (++ts->u.group11.pbcchBufIndex >= NUM_SYSINFO_BUFS){
                   ts->u.group11.pbcchBufIndex = PACKET_TYPE_1;
                   pBcchInfo = ts->u.group11.pbcch.dlSigBuf + PACKET_TYPE_1;
           }

   }

   /*  Modify the USF value with the previous blocks USF value */
   
   pBcchInfo->data[0][0] |=  (ts->u.group11.currentUsf &0x7);
   
   

   unpackFromByteBuffer(&pBcchInfo->data[0][0],
                           (UChar *)&g_DLCCHData.data[0], 
                           NUM_BITS_CCH_FRAME);
   

   g_DLCCHData.bufferValid = TRUE;
     		            
   chanEncoder(BCCH, 1, timeSlot,0);
   buildGSMBurst(BCCH, g_BBInfo[rfCh].tseqNum);
   GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, BCCH);
   
}



void dlPpchProc0(Uint8 rfCh, Uint8 timeSlot)
{ return; }

void dlPpchProc1(Uint8 rfCh, Uint8 timeSlot)
{return;}
void dlPpchProc2(Uint8 rfCh, Uint8 timeSlot)
{return;}
void dlPpchProc3(Uint8 rfCh, Uint8 timeSlot)
{
           t_TNInfo *ts;
   

   /* get pointer to the timeslot */
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

               if (++ts->u.group11.dlBlock >= MAX_RADIO_BLOCKS){
                        ts->u.group11.dlBlock = 0;
                }

        return;}







Uint8  RadioBlkPos[12]    = {0,4,8,13,17,21,26,30,34,39,43,47};
Uint8  OrderedBlks[12]    = {0,6,3,9,1,7,4,10,2,8,5,11};


void UpdateGroupXITableDL(Uint8 rfCh, Uint8 timeSlot){

        Uint8       index;
        Uint8       offset;
        t_TNInfo    *ts;
        Uint8       frameNum;
        Uint8       val;
        Uint8       numPaging;
        

        
        /* get the struture for the timeslot */
        
        ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

        /* Determine which frames are PBCCH frames */

        for ( index = 0; index < g11Params.bsPBcchBlks; index++){
                
                frameNum = RadioBlkPos[OrderedBlks[index]];
                G11_dlRadioBlkType[OrderedBlks[index]] = D_PBCCH_T;
                

                for (offset = 0 ; offset  < 4; offset++){
                        ts->groupDLTable[offset+frameNum] = (UINT32) dlPbcchTable[offset];
                }
        }
        
        /*  Determine the PPCH channels */

        numPaging = MAX_GPRS_PAGING +1 - g11Params.bsPBcchBlks;
        numPaging -= g11Params.bsPagBlksRes;
        
        
        /* Max sure of the limits */
        
        if (numPaging >MAX_GPRS_PAGING)
                numPaging = MAX_GPRS_PAGING;

        if (numPaging <0) numPaging = 0;

        /* Update the scheduling table */

        if (numPaging > 0){
               for (index = 0; index < numPaging; index++){    
				  val      =  MAX_GPRS_PAGING - index; 
                  frameNum = RadioBlkPos[OrderedBlks[val]];
                  G11_dlRadioBlkType[OrderedBlks[val]] = D_PPCH_T;
                  for (offset = 0 ; offset  < 4; offset++){
                        ts->groupDLTable[offset+frameNum] = (UINT32) dlPpchTable[offset];
                  }
               }
        }        
}



void UpdateGroupXITableUL(Uint8 rfCh, Uint8 timeSlot){
/*   Determine the position of the PRACH   */


        Uint8       index;
        Uint8       offset;
        t_TNInfo    *ts;
        Uint8       frameNum;
        Uint8       val,i;

        
        /* get the struture for the timeslot */
        
        ts = & g_BBInfo[rfCh].TNInfo[timeSlot];


           /* Initialize the group 11 radio block defs */

        for ( i = 0; i < MAX_RADIO_BLOCKS; i++){
           G11_ulRadioBlkType[i] = U_PDCH_T;
        }

        /*  Set up the blocks for PRACH */

        
        for (index = 0; index < g11Params.bsPrachBlks; index++){
                frameNum = RadioBlkPos[OrderedBlks[index]];
                G11_ulRadioBlkType[OrderedBlks[index]] = U_PRACH_T;

                for (offset = 0 ; offset  < 3; offset++){
                        
                 ts->groupULTable[offset+frameNum] = (Uint32) ulPrachProc0;
                }
                 ts->groupULTable[frameNum+3] = (Uint32) ulPrachProc3;
                
        }
}



void ulPrachProc0(Uint8 rfCh, Uint8 timeSlot)
{   
	t_TNInfo *ts;
        t_chanSel syncMsg, decodeMsg;

	          
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
                   syncMsg   = PRACH;
                   decodeMsg = PRACH;
           } else {
                   syncMsg  =  PRACH;
                   decodeMsg = RACH;
           }

     break;
     default:
           syncMsg = RACH;
           decodeMsg = RACH;
     break;
   }
   
	chanDecoder(decodeMsg, 1, timeSlot);
	ulSyncMsgProcess(syncMsg, 0, timeSlot, rfCh);

                			

}


void ulPrachProc3(Uint8 rfCh, Uint8 timeSlot)
{   
	t_TNInfo *ts;
        t_chanSel syncMsg, decodeMsg;

	          
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
                   syncMsg   = PRACH;
                   decodeMsg = PRACH;
           } else {
                   syncMsg  =  PRACH;
                   decodeMsg = RACH;
           }

     break;
     default:
           syncMsg = RACH;
           decodeMsg = RACH;
     break;
   }
   
	chanDecoder(decodeMsg, 1, timeSlot);
	ulSyncMsgProcess(syncMsg, 0, timeSlot, rfCh);

                			
       if (++ts->u.group11.ulBlock >= MAX_RADIO_BLOCKS){
                        ts->u.group11.ulBlock = 0;
                }

}



