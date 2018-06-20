/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: pbcch.c              
*                       
* Description: Subroutines to Process downlink PBCCH Messages
*              in 4 bursts and CCCH process
*
*****************************************************************************/
#include "stdlib.h" 
#include "gsmdata.h" 
#include "bbdata.h"
#include "dsprotyp.h"



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














