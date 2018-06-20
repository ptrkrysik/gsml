/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: sdcch.c
*
* Description:
*   This file contains functions that process SDCCH frames.
*
* Public Functions:
*   ulSdcchProc0, ulSdcchProc3, dlSdcchProc0, dlSdcchProc3
*
* Private Functions:
*   None
*
******************************************************************************/
#include "stdlib.h"
#include "gsmdata.h"
#include "bbdata.h"   
#include "rrmsg.h"
#include "dsprotyp.h"
#include "agcdata.h"          

/*****************************************************************************
*
* Function: ulSdcchProc0
*
* Description:
*   Processes uplink SDCCH bursts for frames 0,1,2 of every 0-3 block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void ulSdcchProc0(Uint8 rfCh, Uint8 timeSlot)
{         
   t_TNInfo    *ts;
   Uint8       chType;
   Uint8       subChan;	
   Uint8       tmpChan;
   t_chanState chState;
   t_SACCHInfo *sacchInfo;
	          
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
   tmpChan = 0;
	    
   if (ts->groupNum == GSM_GROUP_5)
	{
      chType = SDCCH4;
      subChan = ts->u.group5.ulSdcchSubCh;		
      chState = ts->u.group5.sdcch[subChan].state;
      sacchInfo = & ts->u.group5.sacch[subChan];
      tmpChan = subChan +NUM_TN_PER_RF;
	}
   else
   {
      chState  = CHAN_INACTIVE;
   }

     if (ts->groupNum == GSM_GROUP_7){
        chType = SDCCH4;
        subChan = ts->u.group7.ulSdcchSubCh;		
        chState = ts->u.group7.sdcch[subChan].state;
        sacchInfo = & ts->u.group7.sacch[subChan];
        tmpChan   = subChan;
      }
  

     if(chState == CHAN_ACTIVE){

      /*--- Update the DC offset for this SDCCH ---*/ 
      DCOffset( g_BBInfo[rfCh].ulBBSmpl, ts->groupNum, rfCh, tmpChan );
      
      demodulate( g_BBInfo[rfCh].ulBBSmpl, 
                  chType, 
                  g_BBInfo[rfCh].tseqNum, 
                  &g_ULBurstData,
                  ts->groupNum,
                  rfCh,
                  timeSlot,
                  subChan );
              
      chanDecoder(chType, 0, timeSlot);
      accumPower(rfCh, timeSlot, ts->groupNum, SDCCH, sacchInfo, NULL);
   }   
}

/*****************************************************************************
*
* Function: ulSdcchProc3
*
* Description:
*   Processes uplink SDCCH bursts for frame 3 of every 0-3 block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void ulSdcchProc3(Uint8 rfCh, Uint8 timeSlot)
{   
   t_TNInfo    *ts;
   Uint8       chType;
   Uint8       subChan;	
   Uint8       tmpChan;
   t_chanState chState;
   t_SACCHInfo *sacchInfo;

   t_ULSignalStrength *sigS;
	          
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
        tmpChan = 0;
	    
	if(ts->groupNum == GSM_GROUP_5)
	{
      /*----------------------------------------------------------
      * Get current SDCCH subchannel, increment structure value
      * to next subchannel, and set its SACCH info pointer  
      *----------------------------------------------------------*/
      chType = SDCCH4;
      subChan = ts->u.group5.ulSdcchSubCh;
      if ( ++ts->u.group5.ulSdcchSubCh >= NUM_SDCCH_SUBCHANS )
      {
         ts->u.group5.ulSdcchSubCh = 0;
      }      		
      chState  = ts->u.group5.sdcch[subChan].state;		
      sacchInfo = & ts->u.group5.sacch[subChan]; 
      tmpChan   = subChan + NUM_TN_PER_RF;
	}
   else
   {
      chState  = CHAN_INACTIVE;
   }

   
   	if(ts->groupNum == GSM_GROUP_7)
	{
      /*----------------------------------------------------------
      * Get current SDCCH subchannel, increment structure value
      * to next subchannel, and set its SACCH info pointer  
      *----------------------------------------------------------*/
      chType = SDCCH4;
      subChan = ts->u.group7.ulSdcchSubCh;
      if ( ++ts->u.group7.ulSdcchSubCh >= NUM_SDCCH8_SUBCHANS )
      {
         ts->u.group7.ulSdcchSubCh = 0;
      }      		
      chState   = ts->u.group7.sdcch[subChan].state;		
      sacchInfo = & ts->u.group7.sacch[subChan];
      tmpChan   = subChan;  
	}
   
	if(chState == CHAN_ACTIVE)
	{
      /*--- Update the DC offset for this SDCCH ---*/ 
      DCOffset( g_BBInfo[rfCh].ulBBSmpl, ts->groupNum, rfCh, tmpChan );
      
      demodulate( g_BBInfo[rfCh].ulBBSmpl, 
                  chType, 
                  g_BBInfo[rfCh].tseqNum, 
                  &g_ULBurstData,
                  ts->groupNum,
                  rfCh,
                  timeSlot,
                  subChan );              
      /*
      *  frame boundary is 1
      */                	
      chanDecoder(chType, 1, timeSlot);
      accumPower(rfCh, timeSlot, ts->groupNum, SDCCH4, sacchInfo, NULL);
				
      /*-----------------------------------------------------------
      * Accumlate bit errors and frame errors into SACCH structure
      *-----------------------------------------------------------*/
      accumBer(rfCh, timeSlot, ts->groupNum, SDCCH4, subChan,
               sacchInfo, FALSE, FALSE); 

      /*-----------------------------------------------------------
      * Update AGC on final burst of SDCCH frame
      *----------------------------------------------------------*/
      if(g_AGC_ON)
      {
         sigS = & sacchInfo->ulsigstren;
         sigS->rxPwrNormSmooth = ul_agc(sigS);
      }
      
	ulSyncMsgProcess(chType, subChan, timeSlot, rfCh);             
   }
}

/*****************************************************************************
*
* Function: dlSdcchProc0
*
* Description:
*   Processes downlink SDCCH bursts for frame 0 of every 0-3 block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dlSdcchProc0(Uint8 rfCh, Uint8 timeSlot)
{   
   t_TNInfo    *ts;
   t_DLSigBufs *sdcchMsg;
   t_CBCHInfo  smscbMsg;
   Uint8       subCh;
   Bool        msgReady = 0;
             
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

	switch(ts->groupNum)
	{
	case GSM_GROUP_5:
		subCh = ts->u.group5.dlSdcchSubCh++;
		/*if(ts->u.group5.sdcch.state[] == CHAN_ACTIVE)*/
		sdcchMsg = ts->u.group5.sdcch[subCh].dlSigBuf;
		ts->u.group5.dlSdcchSubCh &= 0x3;


      if(sdcchMsg->numMsgsReady)
      {	
         processCCHBuff(sdcchMsg) ;       
         rtsCheck(rfCh, timeSlot, SDCCH4, subCh, 0);
         msgReady = 1;
      }  
      else if(ts->u.group5.cbchUsed && (subCh == 2))
      {
           /*
           *  check if SMSCB normal or SMSCB ext
           */		
		   smscbMsg.dlSigBuf = ts->u.group5.cbch[(ts->u.group5.dlSmscbTB/4)].dlSigBuf;         
		   
         if(smscbMsg.dlSigBuf->bufValid == 1)
         {
	          unpackFromByteBuffer( & smscbMsg.dlSigBuf->data[ts->u.group5.dlSmscbTB & 0x03][0],
                             (UChar *)&g_DLCCHData.data[0], 
                             NUM_BITS_CCH_FRAME);           
              msgReady = 1;

              if(ts->u.group5.dlSmscbTB == 3)
              {
                 rtsCheck(rfCh, timeSlot, CBCH, subCh, 0);
              }
              else if(ts->u.group5.dlSmscbTB == 7)
              {
                 rtsCheck(rfCh, timeSlot, CBCH_EXT, subCh, 0);              
              }
           }

        }
         
        /*
        *   CBCH phase updates
        */           
        if(subCh == 2 )
        {
           ts->u.group5.dlSmscbTB++;
           ts->u.group5.dlSmscbTB &= 7;        
        }
        
    	break;


       case GSM_GROUP_7:
	    subCh = ts->u.group7.dlSdcchSubCh++;
	    sdcchMsg = ts->u.group7.sdcch[subCh].dlSigBuf;
	    ts->u.group7.dlSdcchSubCh &= 0x7;


	    if(sdcchMsg->numMsgsReady)
	      {	
		  processCCHBuff(sdcchMsg) ;       
		  rtsCheck(rfCh, timeSlot, SDCCH8, subCh, 0);
		  msgReady = 1;
	      }  
				
    	break;
		
	default:
		break;
	}

    /*
    *  always send DCCH filler if no message to send
    */	        
	if(!msgReady)
	{
		unpackFromByteBuffer(dcchFiller,
                           (UChar *)&g_DLCCHData.data[0], 
                           NUM_BITS_CCH_FRAME);
	}
	                         
	g_DLCCHData.bufferValid = TRUE;	                               		
	chanEncoder(SDCCH4, 1, timeSlot,0);              
	buildGSMBurst(SDCCH4,  g_BBInfo[rfCh].tseqNum);
	GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, SDCCH4);
}


/*****************************************************************************
*
* Function: dlSdcchProc0
*
* Description:
*   Processes downlink SDCCH bursts for frames 1,2,3 of every 0-3 block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dlSdcchProc3(Uint8 rfCh, Uint8 timeSlot)
{                    
   t_TNInfo    *ts;          
   t_DLSigBufs *sdcchMsg;
   
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
	
	chanEncoder(SDCCH4, 0, timeSlot,0);    
	buildGSMBurst(SDCCH4,  g_BBInfo[rfCh].tseqNum);
	GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, SDCCH4);			
}


















