/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
** Filename: syncmsg.c
**
** Description:   
**   This file contains routines in support of the synchronous message
**   handling.  Specifically, the routines in this file are responsible
**   for interfacin the Channel CODEC input and output buffers with the
**   asynchronous control code.
**
*****************************************************************************/
#include "gsmdata.h" 
#include "oamdata.h" 
#include "rrmsg.h" 
#include "bbdata.h" 
#include "dsprotyp.h" 
#include "diagdata.h" 
#include "codec.h"

extern Uint8 USFCodingTableCS23[8];
extern Uint16 USFCodingTableCS4[8];

static DataBlkSizes[] = {23, 34, 39, 53, 0, 0, 0, 0};



               
/*****************************************************************************
** ROUTINE NAME: populateNCellInfo
**
** Description:      
**
** Method:
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**    None
**
** Data Store  I/O: 
**
*****************************************************************************/
Uint8 populateNCellInfo(Uint8 TN, Uint8 rfCh, Uint8 *outbuf)
{       
   t_TNInfo  *ts;
   Uint8     tempBSIC[6], tempRXLEV[6], noMeasPerSacch, i, j;
   Uint8     tempARFCN_IDX[6]; 
   Uint8     tempbuf[10];     /* !! */
       
   ts = & g_BBInfo[rfCh].TNInfo[TN];  
               
   /* NO-NCELL-M is the number of neighbouring cell measurments (octets 4,5) */
   noMeasPerSacch = ((*outbuf & 0x01) << 2) | (((*(outbuf+1)) & 0xC0) >> 6); 
    
   if(noMeasPerSacch == 7) {
     noMeasPerSacch = 0;
   }
         
   /* global storage area for handover alg */
   ts->u.group1.sacch.noNCellM = noMeasPerSacch;
      
   /* each neighbouring cell has 3 values associated with it:
   *   1) RXLEV-NCELL#     (6 bits) ... copy to global data if 2) and 3) are ok.
   *   2) BCCH-FREQ-NCELL# (5 bits) ... find first
   *   3) BSIC-NCELL#      (6 bits) ... check second
   */
   
   if(noMeasPerSacch != 0) {
     /* brute force copy to 3 temp buffers for easier manipulation. No need to 
      * optimize the hardcoded masks, unless the gsm spec changes */
     tempRXLEV[0] = *(outbuf+1) & 0x3F;
     tempARFCN_IDX[0] = (*(outbuf+2) & 0xF8) >> 3; 
     tempBSIC[0] = ((*(outbuf+2) & 0x07) << 3) | ((*(outbuf+3) & 0xE0) >> 5);
     
     tempRXLEV[1] = ((*(outbuf+3) & 0x1F) << 1) | ((*(outbuf+4) & 0x80) >> 7);  
     tempARFCN_IDX[1] = (*(outbuf+4) & 0x7C) >> 2; 
     tempBSIC[1] = ((*(outbuf+4) & 0x03) << 4) | ((*(outbuf+5) & 0xF0) >> 4);

     tempRXLEV[2] = ((*(outbuf+5) & 0x0F) << 2) | ((*(outbuf+6) & 0xC0) >> 6);
     tempARFCN_IDX[2] = (*(outbuf+6) & 0x3E) >> 1; 
     tempBSIC[2] = ((*(outbuf+6) & 0x01) << 5) | ((*(outbuf+7) & 0xF8) >> 3);

     tempRXLEV[3] = ((*(outbuf+7) & 0x07) << 3) | ((*(outbuf+8) & 0xE0) >> 5);
     tempARFCN_IDX[3] = (*(outbuf+8) & 0x1F) >> 0; 
     tempBSIC[3] =  (*(outbuf+9) & 0xFC) >> 2;

     tempRXLEV[4] = ((*(outbuf+9) & 0x03) << 4) | ((*(outbuf+10) & 0xF0) >> 4);
     tempARFCN_IDX[4] = ((*(outbuf+10) & 0x0F) << 1) | (*(outbuf+11) & 0x80) >> 7; 
     tempBSIC[4] =  (*(outbuf+11) & 0x7E) >> 1;

     tempRXLEV[5] = ((*(outbuf+11) & 0x01) << 5) | ((*(outbuf+12) & 0xF8) >> 3);
     tempARFCN_IDX[5] = ((*(outbuf+12) & 0x07) << 2) | ((*(outbuf+13) & 0xC0) >> 6); 
     tempBSIC[5] =  (*(outbuf+13) & 0x3F) >> 0;    
   }
       
   /* refresh always */
   for(i=0; i < MAX_NUM_NCELL_CANDIDATES; i++) {
     ts->u.group1.tch.NCell[i].rfCh = 0;
     ts->u.group1.tch.NCell[i].bsic = 0;
     ts->u.group1.tch.NCell[i].rxLev = 0;
     ts->u.group1.tch.NCell[i].oamIdx = 0;  
   }
 
   for(i=0; i < noMeasPerSacch; i++) {
     /* search into global struct to copy
      *  1) oam index mapped to arfcn, 2) bsic, both to NCellInfo struct in tchInfo
      */       
      if(g_HoOAMInfo.uniqueParms[tempARFCN_IDX[i]].bsic == tempBSIC[i]) {    
        ts->u.group1.tch.NCell[i].rfCh = g_HoOAMInfo.uniqueParms[tempARFCN_IDX[i]].rfCh;
        ts->u.group1.tch.NCell[i].bsic = tempBSIC[i];
        ts->u.group1.tch.NCell[i].rxLev = tempRXLEV[i];
        ts->u.group1.tch.NCell[i].oamIdx = tempARFCN_IDX[i];                             
      }      
   }      
                                                                            
   return(noMeasPerSacch);                                                                        
}


/*****************************************************************************
** ROUTINE NAME: processCCHBuff
**
** Description:      
**
** Method:
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**    None
**
** Data Store  I/O: 
**
*****************************************************************************/
void processCCHBuff(t_DLSigBufs* sigBufPtr)
{
   UShort index;

   if(sigBufPtr->numMsgsReady)
   {
      /*
      * A message is pending and is ready for transmission. Load the message
      * into the Channel CODEC input buffer
      */
      index = sigBufPtr->readIndex;

      unpackFromByteBuffer(&sigBufPtr->data[index][0],
                           (UChar *)&g_DLCCHData.data[0],
                           NUM_BITS_CCH_FRAME);

      g_DLCCHData.bufferValid = TRUE;

      sigBufPtr->numMsgsReady--;

      sigBufPtr->readIndex = (sigBufPtr->readIndex + 1) & (DL_SIG_Q_DEPTH-1);
   }
}


/*****************************************************************************
** ROUTINE NAME: processPCCCHBuff
**
** Description:      
**
** Method:
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**    None
**
** Data Store  I/O: 
**
*****************************************************************************/
void processPCCCHBuff(t_DLPPCHBufs* sigBufPtr)
{
   if(sigBufPtr->msgReady)
   {
      /*
      * A message is pending and is ready for transmission. Load the message
      * into the Channel CODEC input buffer
      */
      unpackFromByteBuffer(&sigBufPtr->data[0],
                           (UChar *)&g_DLCCHData.data[0],
                           NUM_BITS_CCH_FRAME);

      g_DLCCHData.bufferValid = TRUE;
      sigBufPtr->msgReady = 0;
   }
}

/*****************************************************************************
** ROUTINE NAME: ulSyncMsgProcess
**
** Description:      
**   This routine is responsible for interfacing the Channel CODEC with the
**   aynchronous control code. Specifically, TN, this routine processes recieved
**   frame (if a frame is ready) and loads the uplink signaling buffer 
**   (g_BBInfo[0].TNInfo[TN].ulSigBuf) with the frame if a frame has been
**   received.
**
** Method:
**   This routine assummes uses the data store in the g_BBInfo data structure as 
**   defined in the design document section 5.3.1, and 5.4.1.
**
**   The design and coding of this routine is based on the description in
**   the design document section 5.4.1, and 4.6.8.1.
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**   TN - Timeslot number
**
** Returns: 
**    None
**
** Data Store  I/O: 
**   The format of the data and the bit order is consistent with GSM 04.04
**   sections 7.1, 7.3, and 7.5, and GSM 04.06, section 2.8, where the lowest 
**   order bit in an octet is assummed to be the LSB of that octet. The first
**   octet is reserved for the channel type to make the frame identification
**   easier. This is shown in the illustration below:
**
**
**          MSB                              LSB
**           8    7    6    5    4    3   2   1
**
**                       chanSel                       octet 1
**           d7   d6   d5   d4   d3   d2  d1  d0       octet 2
**          d15  d14  d13  d12  d11  d10  d9  d8       octet 3
**
**                ....(etc)....
**
**                         dM ..............dM-4       octet N+1
**
**   Note that for CCH channels, N = 23 and M = 183.
**   For RACH, N = 1 and M = 7.
**
*****************************************************************************/
Int32 rachReceived = 0;

Int32 sacchReceived = 0;
Int32 sdcchReceived = 0; 
Int32 facchReceived = 0;

Int32 sacch4Received[4] = {0, 0, 0, 0};
Int32 sdcch4Received[4] = {0, 0, 0, 0};

Int32 sacch8Received[8] = {0, 0, 0, 0, 0, 0 , 0 , 0};
Int32 sdcch8Received[8] = {0, 0, 0, 0, 0, 0 , 0 , 0};

void ulSyncMsgProcess(t_chanSel chanSel, Uint8 subchan, Uint8 TN, Uint8 rfCh)
{   
   Uchar             frameDetected;
   UInt              i;
   UChar             index;
   msgStruc          ulPhDataMsg;
   t_encryptState*   encryptState; 
   t_TNInfo          *ts;    
   Uint8             chanNum;
   Uint8             subCh;          
   Uint8             linkId;
   Bool              flag = 0;
   Uint8             tmp[10];
   Uint8             noSacchMeas = 0;  
   Uint8             j, tempbuf[15];      
   Uint16            tempUsfCode;
   Uint8             *dataBuf;
   static Uint8      diagRptCount=0;   
   chActMsgStruc     hostChActInfo;
   Uint8             rxLevFull;
   Uint8             rxLevSub;
   Uint8             rxLev;
   Uint8             rxQualFull;
   Uint8             rxQualSub;
   Uint8             rxQual;
   Uint8 dataBytes;
   tbfStruct         *tbf;
   Uint8             ulBlock, dlBlock;
   t_blkNumWatch     *blkNumWatch;
   
   
   
   

   ts = & g_BBInfo[rfCh].TNInfo[TN];


   switch (ts->groupNum){
         case GSM_GROUP_13:
                 blkNumWatch = ts->u.group13.blkNumWatch;
                 ulBlock     = ts->u.group13.ulBlock;
                 dlBlock     = ts->u.group13.dlBlock;
                 break;
         case GSM_GROUP_11:
                 blkNumWatch = ts->u.group11.blkNumWatch;
                 ulBlock     = ts->u.group11.ulBlock;
                 dlBlock     = ts->u.group11.dlBlock;
                 break;
         default:
                 break; 
         }

   
   switch(chanSel)
   { 
   case RACH:

      if(g_ULRACHData.ulRACHword0.bitval.bufferValid && 
         g_ULRACHData.ulRACHword0.bitval.rachDet)
      {
         /*
         * A RACH frame has been detected, Obtain a pointer to 
         * the destination uplink signaling buffer. The RACH channel
         * uses the same uplink signaling buffer.
         */

         if (sacchReceived+sdcchReceived + rachReceived <156)
         {             
            g_UlBurstLog[sacchReceived+sdcchReceived+rachReceived] =
               g_ULBurstData.toa & 0x00ff | g_ULBurstData.rcvPower<<16 | sysFrameNum.t3 <<8;
         }
         rachReceived++; 
                            
         /*
         * Copy the unpacked decoded data into the packed buffer
         * indexed by the local state variable.
         */
         packToByteBuffer( & g_ULRACHData.data[0], 
                           & ulPhDataMsg.params[3],
                           NUM_BITS_RACH_FRAME );
                               
         /*
         *  Check if it is a HO Access burst
         */                               
         if((ts->groupNum == GSM_GROUP_1) && ts->u.group1.tch.HOFlag)  
         {
            if(ulPhDataMsg.params[3] == ts->u.group1.tch.HOReference)
            {
               /* Settings for MS going to the target cell */
               hostChActInfo.timeAdv = g_ULBurstData.toa;  
               hostChActInfo.msPowerInit = ts->u.group1.sacch.powerLevelInit;
               hostChActInfo.msPower = g_ULBurstData.rcvPower; 
               initPower(&hostChActInfo, &ts->u.group1.sacch);
               ts->u.group1.tch.HOFlag = FALSE; 
            }
            else
            {   
               /*
               * Access Burst does not match HO Reference. Don't send it
               * to host. Indicate that the UL buffer has been consumed
               */
               g_ULRACHData.ulRACHword0.bitval.bufferValid = FALSE;
               return;
            }
         }
  
         chXlate(ts->groupNum, chanSel, subchan, &chanNum, &linkId);

         
         if ((ts->groupNum == GSM_GROUP_13) || (ts->groupNum == GSM_GROUP_11))
         {                 
           /*
           *  Put tag in UL PACCH and clear tag cause.
           */
           if((blkNumWatch[ulBlock].tagFillCause == RRB_AB) || 
              (blkNumWatch[ulBlock].tagFillCause == DLA)    ||
              (blkNumWatch[ulBlock].tagFillCause == LAST_CHANCE_4_AB_DET))
           {           
             /* build PH_PACCH_AB_INDication message */
             ulPhDataMsg.msgSize = 10;
             ulPhDataMsg.function = PDCH_MANAGE_MSG;
             ulPhDataMsg.typeHiByte = PH_PACCH_AB_IND>>8;
             ulPhDataMsg.typeLoByte = PH_PACCH_AB_IND;
             ulPhDataMsg.trx = rfCh; /* for now */  
             ulPhDataMsg.params[0] = chanNum | TN; /* channel number */
             ulPhDataMsg.params[1] = 0;  /* channel number */

             ulPhDataMsg.params[2] = blkNumWatch[ulBlock].tag;
             /* ulPhDataMsg.params[3] = 0; /* Access burst info 8 msbs, for 8 or 11 bits */
             /* ulPhDataMsg.params[4] = 0; /* Access burst info 3 lsbs, if 11 bits */
             ulPhDataMsg.params[5] = g_ULBurstData.toa;  /* same as timing advance for now */  
    
             blkNumWatch[ulBlock].tagFillCause = GRP13_AB_FOUND;
             
             /**** >>> temporary diag when receiving UL PACCH */             
             tempbuf[0] = dlBlock;
             tempbuf[1] = ulBlock;    
             tempbuf[2] = blkNumWatch[ulBlock].tagFillCause;
             tempbuf[3] = blkNumWatch[ulBlock].tag;
             sendDiagMsg(0x31, rfCh, TN, 4, tempbuf);
           }
         }

         else
         { 
           /* build PH_RANDOM_ACCESS_INDication message */
           ulPhDataMsg.msgSize = 13;
           ulPhDataMsg.function = RR_MANAGE_MSG;
           ulPhDataMsg.typeHiByte = PH_RANDOM_ACCESS_IND_HI;
           ulPhDataMsg.typeLoByte = PH_RANDOM_ACCESS_IND_LO;
           ulPhDataMsg.trx = rfCh; /* for now */  
           ulPhDataMsg.params[0] = chanNum | TN; /* channel number */
           ulPhDataMsg.params[1] = 0;  /* channel number */
           ulPhDataMsg.params[2] = linkId;  /* link identifier */
           /* ulPhDataMsg.params[3] = sigBufPtr->msg[index].data[0]; */
           ulPhDataMsg.params[4] = (sysFrameNum.t3 >> 3) | (sysFrameNum.t1 << 3);  /* Request Referanse */
           ulPhDataMsg.params[5] = sysFrameNum.t2 | (sysFrameNum.t3 << 5);  /* Request Referanse */
           ulPhDataMsg.params[6] = g_ULBurstData.toa;       /* Acsess Delay */
           ulPhDataMsg.params[7] = g_ULBurstData.rcvPower;  /* Access Delay */
           ulPhDataMsg.params[8] = g_ULBurstData.snr;       /* SINR */
         }


         
         
         SendMsgHost_Que(& ulPhDataMsg);
         
      }
      
      
      
      else if ((ts->groupNum == GSM_GROUP_13 || ts->groupNum == GSM_GROUP_11) &&
               blkNumWatch[ulBlock].tagFillCause == LAST_CHANCE_4_AB_DET )
      {  
        /*
        * When no access burst message is received and if (1) Reserved Block is expected from PACCH
        * or (2) DL Assignment (both set on the calling function ulPdchProc3, then send 
        * PACCH negative acknowledgement.
        */
        ulPhDataMsg.msgSize = 7;                   /* always CS-1 */
        ulPhDataMsg.function = PDCH_MANAGE_MSG;
        ulPhDataMsg.typeHiByte = PH_PACCH_NACK >> 8;
        ulPhDataMsg.typeLoByte = PH_PACCH_NACK;
        ulPhDataMsg.trx = rfCh; 
        ulPhDataMsg.params[0] = chanNum | TN;     /* channel number */
        ulPhDataMsg.params[1] = 0;                
        ulPhDataMsg.params[2] = blkNumWatch[ulBlock].tag;           
         
        SendMsgHost_Que(& ulPhDataMsg);              
      }

      /*
      * Indicate that the UL buffer has been consumed
      */
      g_ULRACHData.ulRACHword0.bitval.bufferValid = FALSE;
      break;


    case PDCH:
      /*
      * Other uplink signaling (PACCH) or data (PDTCH) channels
      * use the same buffers.  Format is based on GSM 04.60 (section 10 RLC/MAC messages)
      */
      if ( g_ULCCHData.ulcchword0.bitval.bufferValid &&
           ! g_ULCCHData.ulcchword0.bitval.fireStatus )
      { 

         chXlate(ts->groupNum, chanSel, subchan, &chanNum, &linkId);  

         dataBuf = g_ULCCHData.data;

         /*
         * Obtain a pointer to the destination uplink signaling buffer. 
         * Copy the unpacked decoded data into the packed buffer
         * indexed by the local state variable.
         */                                         
         packToByteBuffer( dataBuf, 
                          & ulPhDataMsg.params[3],
                            NUM_BITS_GPRS_CS4 ); 

         /* Check out MAC header to see if control or data block */    
         switch ( ((ulPhDataMsg.params[3] & 0xC0) >> 6) )
         {
         case 0:
          /*
          *  Data block, GROUP_13 PDTCH message
          */
          dataBytes = DataBlkSizes[g_ULCCHData.chCodec];
         
   
          ulPhDataMsg.msgSize = dataBytes+7;     /* Up to 53 octets for CS-4 */
          ulPhDataMsg.function = PDCH_MANAGE_MSG;
          ulPhDataMsg.typeHiByte = PH_PDTCH_IND >> 8;
          ulPhDataMsg.typeLoByte = PH_PDTCH_IND;
          ulPhDataMsg.trx = rfCh; 
          ulPhDataMsg.params[0] = chanNum | TN; /* channel number */
          ulPhDataMsg.params[1] = chanSel;      /* MAC header  */
          ulPhDataMsg.params[2] = g_ULCCHData.chCodec;
                  
          SendMsgHost_Que(& ulPhDataMsg);   

          g_rateChangeCheck= 1;

          /*
          *  L1 proxy debug
          */

          //ulPhDataMsg.params[0] = g_ULCCHData.chCodec;
          //ulPhDataMsg.params[1] = tempUsfCode;
          //ulPhDataMsg.params[2] = tempUsfCode>>8;

          //sendDiagMsg(0x22,0,ts->tsNumber,3, & ulPhDataMsg.params[0]); 

          break;

        case 1:
          /*
          *  Control block, GROUP_13 PACCH message
          */

          switch ((ulPhDataMsg.params[4] & 0xFC) >> 2)  /* RLC control message type */
          {
           /*
           *  Decode according to GSM 04.60, section 11.2.0.2, to get uplink RLC/MAC messages.
           */

          case 0:
            /*
            *  Packet Cell Change Failure
            */
            break;

          case 1:
            /*
            *  Packet Control Acklowledgement
            */
            break;

          case 2:
                  
            break;

          case 3:
            /*
            *  Uplink Dummy Control blocks (supress for now)
            */
            return;

          case 4:
            /*
            *  Packet Measurement Report
            */
            break;
          }

          ulPhDataMsg.msgSize = 23+7;           /* always uses CS-1 coding */
          ulPhDataMsg.function = PDCH_MANAGE_MSG;
          ulPhDataMsg.typeHiByte = PH_PACCH_IND >> 8;
          ulPhDataMsg.typeLoByte = PH_PACCH_IND;
          ulPhDataMsg.trx = rfCh;
          ulPhDataMsg.params[0] = chanNum | TN; /* channel number */
          ulPhDataMsg.params[1] = 0; 
  
          /* temporary diag msg *** try if(1) here as a test for blockNum */
          if(blkNumWatch[ulBlock].tagFillCause == RRB_NB)  
          {
            tempbuf[0] = blkNumWatch[ulBlock].tag;
            tempbuf[1] = dlBlock;
            tempbuf[2] = ulBlock;
            sendDiagMsg(19, rfCh, TN, 3, tempbuf);            
          }

          /*
          *  Put tag in UL PACCH and clear tag cause.
          */
          if((blkNumWatch[ulBlock].tagFillCause == RRB_NB) || 
             (blkNumWatch[ulBlock].tagFillCause == DLA))
          {
             ulPhDataMsg.params[2] = blkNumWatch[ulBlock].tag;
          }
          else
          {
             ulPhDataMsg.params[2] = SBA_TAG;       /* can be used for NACK also */              
          }
          
          SendMsgHost_Que(& ulPhDataMsg);   

          break;

        default:
          /*
          *  Reserved for uplink; ignore.
          */
          break;
        }       
      } 

      else
      { 
        /*
        * When no message is received and if (1) Reserved Block is expected from PACCH
        * or (2) DL Assignment, then send PACCH negative acknowledgement
        */
        if( blkNumWatch[ulBlock].tagFillCause != NONE  &&
            blkNumWatch[ulBlock].tag != SBA_TAG )
        {             
            ulPhDataMsg.msgSize = 7;                   /* always CS-1 */
            ulPhDataMsg.function = PDCH_MANAGE_MSG;
            ulPhDataMsg.typeHiByte = PH_PACCH_NACK >> 8;
            ulPhDataMsg.typeLoByte = PH_PACCH_NACK;
            ulPhDataMsg.trx = rfCh; 
            ulPhDataMsg.params[0] = chanNum | TN;     /* channel number */
            ulPhDataMsg.params[1] = 0;                
            ulPhDataMsg.params[2] = blkNumWatch[ulBlock].tag;

            SendMsgHost_Que(& ulPhDataMsg);
        }
      } 

      /*
      * Indicate that the UL buffer has been consume
      */
      g_ULCCHData.ulcchword0.bitval.bufferValid = FALSE;
      break;
	  

    case PTCCH:

      if(g_ULRACHData.ulRACHword0.bitval.bufferValid && 
         g_ULRACHData.ulRACHword0.bitval.rachDet)
      {
         /*
         * A PTCCH (same decode as PRACH) frame has been detected, Obtain a pointer to 
         * the destination uplink signaling buffer. The PTCCH channel
         * uses the same uplink signaling buffer as PRACH/RACH.
         * Copy the unpacked decoded data into the packed buffer
         * indexed by the local state variable.
         */
         packToByteBuffer( & g_ULRACHData.data[0], 
                           & ulPhDataMsg.params[3],
                           NUM_BITS_PRACH_FRAME );
                               
         chXlate(ts->groupNum, chanSel, subchan, &chanNum, &linkId);                                                                             
      }

      /*
      * Indicate that the UL buffer has been consumed
      */
      g_ULRACHData.ulRACHword0.bitval.bufferValid = FALSE;
      break;

    case PRACH:

      if(g_ULRACHData.ulRACHword0.bitval.bufferValid && 
         g_ULRACHData.ulRACHword0.bitval.rachDet)
      {
         /*
         * A PRACH frame has been detected, Obtain a pointer to 
         * the destination uplink signaling buffer. The PRACH channel
         * uses the same uplink signaling buffer as RACH.
         * Copy the unpacked decoded data into the packed buffer
         * indexed by the local state variable.
         */
         packToByteBuffer( & g_ULRACHData.data[0], 
                           & ulPhDataMsg.params[4],
                           NUM_BITS_PRACH_FRAME );
                               

         chXlate(ts->groupNum, chanSel, subchan, &chanNum, &linkId);                  

         /* build Packet Random Access - PRACH message */
         ulPhDataMsg.msgSize = 12;
         ulPhDataMsg.function = PDCH_MANAGE_MSG;
         ulPhDataMsg.typeHiByte = PH_PRACH_IND >> 8;
         ulPhDataMsg.typeLoByte = PH_PRACH_IND;
         ulPhDataMsg.trx = rfCh; /* for now */  
         ulPhDataMsg.params[0] = chanNum | TN; /* channel number */
         ulPhDataMsg.params[1] = 0;  /* channel number */
         ulPhDataMsg.params[2] = (sysFrameNum.t3 >> 3) | (sysFrameNum.t1 << 3);  /* Frame Number */
         ulPhDataMsg.params[3] = sysFrameNum.t2 | (sysFrameNum.t3 << 5);  /* Frame Number */

         if ( g_BBInfo[0].frame.offset <= TN){
                 if  (ulPhDataMsg.params[3] == 0){                
                  ulPhDataMsg.params[2] -=1;
                }
                 
                 ulPhDataMsg.params[3] -= 1;
         }
         
      
         //  ulPhDataMsg.params[4] = sigBufPtr->msg[index].data[0];
          // Packet Access Reference (3 MSbits of PRACH) 
         //  ulPhDataMsg.params[5] = sigBufPtr->msg[index].data[3];
         // Packet Access Reference (8 LSbits of PRACH) */
         ulPhDataMsg.params[6] = g_ULBurstData.toa;       /* Access Delay */
         ulPhDataMsg.params[7] = g_ULBurstData.rcvPower;  /* Access Delay */
               
         SendMsgHost_Que(& ulPhDataMsg);               
      }

      /*
      * Indicate that the UL buffer has been consumed
      */
      g_ULRACHData.ulRACHword0.bitval.bufferValid = FALSE;
      break;
	  
   default:
      /*
      * All other uplink signaling channels (SACCH, SDCCH, FACCH) 
      * use the same buffers and format
      */
      if ( g_ULCCHData.ulcchword0.bitval.bufferValid &&
           ! g_ULCCHData.ulcchword0.bitval.fireStatus )
      {    
         /*-------------------------------------------------------------
         * Log received frames and set pointer to encryption mode 
         *-------------------------------------------------------------*/                                    
         if ( chanSel == SDCCH4 ){
	   if (ts->groupNum == GSM_GROUP_5){
            sdcchReceived++;
            sdcch4Received[subchan]++;
            subCh = ts->u.group5.ulSdcchSubCh;
            encryptState = & ts->u.group5.sdcch[subCh].encryptState;
            }
           if (ts->groupNum == GSM_GROUP_7){
            sdcchReceived++;
            sdcch8Received[subchan]++;
            subCh = ts->u.group7.ulSdcchSubCh;
            encryptState = & ts->u.group7.sdcch[subCh].encryptState;
	   }
	 }
         else if ( chanSel == SACCH )
         {
	   if ( ts->groupNum == GSM_GROUP_5 ){
               sacch4Received[subchan]++;
	   }
	   else if (ts->groupNum == GSM_GROUP_7){
	        sacch8Received[subchan]++;
	    }else
            sacchReceived++;          
         }                               
         else if ( chanSel == FACCH )
         {
            facchReceived++;
            encryptState = & ts->u.group1.tch.encryptState;
         }   
         /*-------------------------------------------------------------
         * If encryption just changed, switch to Normal encryption
         * mode using the newly commanded algorithm from now on 
         *-------------------------------------------------------------*/                                    
         if ( (chanSel == SDCCH4 || chanSel == FACCH) &&
               encryptState->encryptMode == ENCR_CHANGE )
         {
            encryptState->encryptMode = ENCR_NORMAL;
            encryptState->algorithm = encryptState->newAlgorithm;
         }

         /*
         * for debug only to find where is UL burst positioned on rcv buffer  
         if(sacchReceived+sdcchReceived + rachReceived <156)
         { 
            g_UlBurstLog[sacchReceived+sdcchReceived] =
               g_ULBurstData.toa & 0x00ff | g_ULBurstData.rcvPower<<16;
         }
         */                                       
         /*
         * A fill frame has a information field octet length of 0.
         * Therefore, check to see if the Length Indicator Field is 0.
         * Ignore the Length Indicator Extension Bit and the More Data
         * Bit.
         *
         * If so, frame is a fill frame - don't send up.
         send back fill frames any way for monitoring channel quality
         frameDetected = 0;  
         for(i=FILLFRAME_LEN_START; i<FILLFRAME_LEN_END; i++)
         {
            frameDetected |= (g_ULCCHData.data[i]);
         }
         if(frameDetected)              
         * Send All recovered frames to Host, L1 Agent on Host is filtering out
         * unnecessary messages to L2/L3           
         */
             
         {        
            /*
            * Obtain a pointer to the destination uplink signaling buffer
            */

            {
               /*
               * Copy the unpacked decoded data into the packed buffer
               * indexed by the local state variable.
               */
               if(chanSel == SACCH) /* Take out L1 Headers */
               {
                  /* populate serving cell (t_SACCHInfo) struct with 
                  *  octets 2 and 3 of measurement results 
                  * information element (gsm04.08)
                  */
                  packToByteBuffer( & g_ULCCHData.data[0], 
                                    & ulPhDataMsg.params[3],
                                    NUM_BITS_CCH_FRAME );
                
                  switch (ts->groupNum)
                  {
                  case GSM_GROUP_1:
                     /* L1 headers from SACCH downlink block format
                        *  (gsm 04.04)
                        */
                     ts->u.group1.sacch.dlsigstrenSCell.MSPwrSetting 
                        = ulPhDataMsg.params[3] & 0x1f;
                     /* populate actual timing advance used*/                           
                     if( (ulPhDataMsg.params[4] & 0x7F) != 0x7F)  /* check if TA is valid */
                        ts->u.group1.sacch.toaLastGoodActual = ulPhDataMsg.params[4] & 0x7F;
                                         
                     /* now L3 payload (gsm 04.08) */
                           
                     /* If RXLEV Full changed, set flag for diagnostic */
                     if ( (ts->u.group1.sacch.dlsigstrenSCell.rxLevNew !=
                           ulPhDataMsg.params[10] & 0x3f) )
                     {   
                           flag = 1;     
                     }
                      
                     /*
                     *  Extract RXLEV and RQUAL full and sub to local variables
                     */
                     rxLevFull = ulPhDataMsg.params[10] & 0x3f; 
                     rxLevSub  = ulPhDataMsg.params[11] & 0x3f; 
                     rxQualFull = (ulPhDataMsg.params[12] >> 4) & 0x7; 
                     rxQualSub  = (ulPhDataMsg.params[12] >> 1) & 0x7; 
                     /*
                     *  Use mobile's Full or Sub measurements, depending on whether we're in
                     *  DL DTX Mode. Don't use dlDtxUsed unless we add code to handle the delay
                     *  between our DLDTX usage and MS measurement reports. Also, DL RXQUAL
                     *  seems to be untrustworthy with DL DTX on; hard code here so it doesn't
                     *  cause unnecessary handovers.
                     */
                     if ( ts->u.group1.tch.DLDtxMode )
                     {  
                        ts->u.group1.sacch.dlsigstrenSCell.rxLevNew = rxLevSub;     
                        ts->u.group1.sacch.dlsigqualSCell.rxQualNew = 1;      // see note above     
                     }
                     else
                     { 
                        ts->u.group1.sacch.dlsigstrenSCell.rxLevNew = rxLevFull;    
                        ts->u.group1.sacch.dlsigqualSCell.rxQualNew = rxQualFull;     
                     }                     
                     /*
                     *  If measurements are not valid, set mid-range values which should be
                     *  neutral to our handover decision.
                     */
                     if ( (ulPhDataMsg.params[11] & 0x40) )
                     {
                        ts->u.group1.sacch.dlsigstrenSCell.rxLevNew = 40;     
                        ts->u.group1.sacch.dlsigqualSCell.rxQualNew = 1;    
                     }
                     break;

                  case GSM_GROUP_5:
                     /* do nothing for OAM data population */
                     break; 
                  }
                    
                  /* now populate t_TCHInfo (t_NCellInfo) with 
                  * global neighbour cell info starting
                  * at octet 4 of measurement results 
                  * information element (gsm04.08)
                  */
                  if(ts->groupNum == GSM_GROUP_1)
                  {
                     noSacchMeas = populateNCellInfo(TN, rfCh, &ulPhDataMsg.params[12]);
                     diagRptCount += 1;

                     if((flag == 1) || (diagRptCount == 16))
                     {
                        /*-----------------------------------------------------------
                        * If MS Report enabled for this TS, accuumulate TOA averages
                        *----------------------------------------------------------*/
                        if ( g_diagData.dlMeasReportTsMask & (1<<TN) )
                        {
                           tempbuf[0] = rxLevFull;
                           tempbuf[1] = rxLevSub;
                           tempbuf[2] = rxQualFull; 
                           tempbuf[3] = rxQualSub;
                           tempbuf[4] = noSacchMeas;    

                           for(i=0; i < noSacchMeas; i++) 
                           {
                             tempbuf[5+i*5] = ts->u.group1.tch.NCell[i].rfCh>>8;
                             tempbuf[6+i*5] = ts->u.group1.tch.NCell[i].rfCh;        
                             tempbuf[7+i*5] = ts->u.group1.tch.NCell[i].bsic; 
                             tempbuf[8+i*5] = ts->u.group1.tch.NCell[i].oamIdx;                  
                             tempbuf[9+i*5] = ts->u.group1.tch.NCell[i].rxLev;         
                           }
                          
                           sendDiagMsg(DL_MEAS_REPORT,0,ts->tsNumber,(5+noSacchMeas*5),tempbuf);
          
                        }
                        if(flag== 1)
                        {
                           flag = 0;
                        }
                        if(diagRptCount == 16)
                        {
                           diagRptCount = 0;
                        }
                     }
                  }                       
               }                                                    
               else
               {                                      
                  packToByteBuffer( & g_ULCCHData.data[0], 
                                    & ulPhDataMsg.params[5],
                                       NUM_BITS_CCH_FRAME );
               }      
                  
               ulPhDataMsg.msgSize = 32;
               ulPhDataMsg.function = RR_MANAGE_MSG;
               ulPhDataMsg.typeHiByte = PH_DATA_IND_HI;
               ulPhDataMsg.typeLoByte = PH_DATA_IND_LO;
               ulPhDataMsg.trx = rfCh; /* for now */

               chXlate(ts->groupNum, chanSel, subchan, &chanNum, &linkId); 
               ulPhDataMsg.params[0] = chanNum | TN; /* channel number */
               ulPhDataMsg.params[1] = 0;  /* channel number */
               ulPhDataMsg.params[2] = linkId;  /* link identifier */
               /*
               ulPhDataMsg.params[3] = ((sysFrameNum.t3 >> 3) & 0x7) |  (sysFrameNum.t1 << 3);
               ulPhDataMsg.params[4] = (sysFrameNum.t2 & 0x1f) | (sysFrameNum.t3 << 5);
               */
                  
               ulPhDataMsg.params[3] = g_ULBurstData.toa;       /* Access Delay */
               ulPhDataMsg.params[4] = g_ULBurstData.rcvPower;  /* Rx Power in dBr */
                  
               SendMsgHost_Que(& ulPhDataMsg);                      
            }
         }
      }   

      /*
      * Indicate that the UL buffer has been consumed
      */
      g_ULCCHData.ulcchword0.bitval.bufferValid = FALSE;
      break;
   }
}



