/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: sacch.c
*
* Description:
*   This file contains functions that process SACCH frames.
*
* Public Functions:
*   ulSacchProc0, ulSacchProc3, dlSacchProc0, dlSacchProc1, dlSacchProc3,     
*
* Private Functions:
*   idleChanDiagReport
*
******************************************************************************/
#include "stdlib.h"
#include "gsmdata.h"
#include "oamdata.h"
#include "dsp/dsphmsg.h"
#include "bbdata.h"
#include "dsprotyp.h"
#include "l1config.h"
#include "diagdata.h" 
#include "agcdata.h"          
#include "dem_const.h"          





/*****************************************************************************
*
* Function: ulSacchProc0
*
* Description:
*   Processes uplink SACCH bursts for frames 0,1,2 of every 0-3 block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void ulSacchProc0(Uint8 rfCh, Uint8 timeSlot)
{
   t_TNInfo    *ts;          
   Uint8       subChan;	
   t_SACCHInfo *sacchInfo;
																							  
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

												
   switch(ts->groupNum)
   {

   case GSM_GROUP_7:
	  subChan = ts->u.group7.ulSacchSubCh;
      sacchInfo = & ts->u.group7.sacch[subChan]; 	
	 
     if(sacchInfo->state == CHAN_ACTIVE)
      {
         /*--- Update DC offset for SDCCH associated with this SACCH ---*/ 
         DCOffset( g_BBInfo[rfCh].ulBBSmpl, ts->groupNum,  rfCh, subChan );
         
         demodulate( g_BBInfo[rfCh].ulBBSmpl, 
                     SACCH, 
                     g_BBInfo[rfCh].tseqNum, 
                     &g_ULBurstData,
                     ts->groupNum,
                     rfCh,
                     timeSlot,
                     subChan );              
         decrypt(rfCh, timeSlot);      

         chanDecoder(SACCH, 0, timeSlot);

         /*-----------------------------------------------------------
         * Accumulate TOA and power Sub (indicated by Burst Phase 0) 
         *-----------------------------------------------------------*/
         accumPower(rfCh, timeSlot, ts->groupNum, SACCH, sacchInfo, NULL);   
         accumToa  (rfCh, timeSlot, ts->groupNum, subChan, SACCH, NULL);       
      }																

	   break;


   case GSM_GROUP_5:
      subChan = ts->u.group5.ulSacchSubCh;
      sacchInfo = & ts->u.group5.sacch[subChan]; 								  
      if(sacchInfo->state == CHAN_ACTIVE)
      {
         /*--- Update DC offset for SDCCH associated with this SACCH ---*/ 
         DCOffset( g_BBInfo[rfCh].ulBBSmpl, ts->groupNum, rfCh, NUM_TN_PER_RF + subChan );
         
         demodulate( g_BBInfo[rfCh].ulBBSmpl, 
                     SACCH, 
                     g_BBInfo[rfCh].tseqNum, 
                     &g_ULBurstData,
                     ts->groupNum,
                     rfCh,
                     timeSlot,
                     subChan );              
         decrypt(rfCh, timeSlot);      
         chanDecoder(SACCH, 0, timeSlot);

         /*-----------------------------------------------------------
         * Accumulate TOA and power Sub (indicated by Burst Phase 0) 
         *-----------------------------------------------------------*/
         accumPower(rfCh, timeSlot, ts->groupNum, SACCH, sacchInfo, NULL);   
         accumToa  (rfCh, timeSlot, ts->groupNum, subChan, SACCH, NULL);       
      }																
      break;
        
   case GSM_GROUP_1:
      sacchInfo = & ts->u.group1.sacch; 								  
      if(sacchInfo->state == CHAN_ACTIVE)
      {
         /*------------------------------------------------------
         * If handover, try to detect HO access burst
         *------------------------------------------------------*/
         if(ts->u.group1.tch.HOFlag)
         {
            ulRachProc(rfCh, timeSlot);		  
         }
         /*------------------------------------------------------
         * Else demodulate normal burst
         *------------------------------------------------------*/
         else
         {		    
            /*--- Update DC offset for TCH associated with this SACCH ---*/ 
            DCOffset( g_BBInfo[rfCh].ulBBSmpl, ts->groupNum, rfCh, timeSlot );
         
            demodulate( g_BBInfo[rfCh].ulBBSmpl, 
                        SACCH, 
                        g_BBInfo[rfCh].tseqNum, 
                        &g_ULBurstData,
                        ts->groupNum,
                        rfCh,
                        timeSlot,
                        0 );              
            decrypt(rfCh, timeSlot);      
            chanDecoder(SACCH, 0, timeSlot);

            /*-----------------------------------------------------------
            * Accumulate TOA and power Sub (indicated by SACCH argument) 
            *-----------------------------------------------------------*/
            accumPower(rfCh, timeSlot, ts->groupNum, SACCH, sacchInfo, NULL);
            accumToa  (rfCh, timeSlot, ts->groupNum, 0, SACCH, NULL);
         }  
      }
      break;
        
   default:
      break;
   }
}

/*****************************************************************************
*
* Function: ulSacchProc3
*
* Description:
*   Processes uplink SACCH bursts for frame 3 of every 0-3 block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void ulSacchProc3(Uint8 rfCh, Uint8 timeSlot)
{               
   t_TNInfo       *ts; 
   Uint8          subChan;									 
   t_SACCHInfo    *sacchInfo;
   t_chanState    chState;
   t_diagMsReport *msReport;
   msgStruc       ackMsg;
   Int8           radioLinkLost=FALSE;
   Uint8          radioLinkLostThrsh;     
   t_ULSignalStrength *sigS;


   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];


    /*
   *  Set Radio Link Lost timeout threshold. Convert from MIB units to
   *  SACCH messages, as described in GSM 05.08.
   */
   radioLinkLostThrsh = 4 + (4 * g_HoOAMInfo.serveParms.radioLinkTimeout);

												
   switch(ts->groupNum)
   {


      case GSM_GROUP_7:
             /*----------------------------------------------------------
              * Get current SACCH subchannel, increment structure value
              * to next subchannel, and set SACCH info pointer and state 
              *----------------------------------------------------------*/

      subChan = ts->u.group7.ulSacchSubCh;
	 
	

      if ( ++ts->u.group7.ulSacchSubCh >= NUM_SACCH8_SUBCHANS )
      {
         ts->u.group7.ulSacchSubCh = 0;
      }      
      

     /* --------------------------------------------------------
      * Get SACCH info pointer and SDCCH state (not SACCH state).
      *----------------------------------------------------------*/
      sacchInfo = & ts->u.group7.sacch[subChan];                            
      chState = ts->u.group7.sdcch[subChan].state;
   
		   
      /*----------------------------------------------------------
       * Process the SACCH channel if it's active. NOTE: Check
       * SACCH state for this, not SDCCH state (chState).  
       *----------------------------------------------------------*/
     if(sacchInfo->state == CHAN_ACTIVE)
      {
      
		
	       

         /*--- Update DC offset for SDCCH associated with this SACCH ---*/ 
         DCOffset( g_BBInfo[rfCh].ulBBSmpl, ts->groupNum, rfCh, subChan );
         
         demodulate( g_BBInfo[rfCh].ulBBSmpl, 
                     SACCH, 
                     g_BBInfo[rfCh].tseqNum, 
                     &g_ULBurstData,
                     ts->groupNum,
                     rfCh,
                     timeSlot,
                     subChan );              
         decrypt(rfCh, timeSlot);      
         chanDecoder(SACCH, 1, timeSlot);
         
         /*-----------------------------------------------------------
         * Finish accumulating bit errors and frame errors
         *----------------------------------------------------------*/
         accumBer(rfCh, timeSlot, ts->groupNum, SACCH, subChan,
                  sacchInfo, FALSE, FALSE); 

         /*----------------------------------------------------------
         * Accumulate TOA and power. Indicate SACCH4 to set final 
         * TOA and power, increment dyn pwr rpt count and adjust TA. 
         *----------------------------------------------------------*/
         accumPower(rfCh, timeSlot, ts->groupNum, SACCH4, sacchInfo, NULL);      
         accumToa  (rfCh, timeSlot, ts->groupNum, subChan, SACCH4, NULL); 
  		  
         /*-----------------------------------------------------------
         * Update AGC on last sacch subchannel burst
         *------------------------------------------------------------*/
         if ( g_AGC_ON )
         {
            if ( sacchInfo->agcHoldFlag )
            {
               /*----------------------------------------------------------
               * If SACCH passed parity, reset and release hold on AGC
               *----------------------------------------------------------*/
               if ( ! g_ULCCHData.ulcchword0.bitval.fireStatus ) 
               {  
                  sacchInfo->agcHoldFlag = FALSE;
                  sacchInfo->ulsigstren.rxPwrNormSmooth = 0;
               }
            }
            else
            {
              sigS = & sacchInfo->ulsigstren;
              sigS->rxPwrNormSmooth = ul_agc(sigS);
            }
         }
         /*-----------------------------------------------------------
         * If SACCH passed parity, release hold on DPC
         *------------------------------------------------------------*/
         if ( ! g_ULCCHData.ulcchword0.bitval.fireStatus ) 
         {  
            sacchInfo->dpcHoldFlag = FALSE;
         }
         /*-----------------------------------------------------------
         * Accumulate average power and perform dynamic power control 
         *------------------------------------------------------------*/
         averageS(& sacchInfo->ulsigstren,
                  & sacchInfo->dlsigstrenSCell,
                  g_DynPwrOAMInfo.ulsigstren.numRxSigSamples, 
                  & sacchInfo->MSPowerAccum,
                  TRUE);

         if (g_DPC_ON)
         {
            uplink_dynamic_power_control(ts, subChan);  
         }
         /*-----------------------------------------------------------
         * Process SACCH message
         *----------------------------------------------------------*/
         ulSyncMsgProcess(SACCH, subChan, timeSlot, rfCh);
      }
	  else
	  {
		  /*
		  *  when SACCH is deactivated, allow Radio Link Lost counting continue
		  *  Set Fire Status for current SACCH frame
		  */
          g_ULCCHData.ulcchword0.bitval.fireStatus = 1;
	  }

	  /*
	  *  pre fill channel Number for Radio Link Lost Message to RRM
	  */
      ackMsg.params[0] = timeSlot | 0x40 | (subChan <<3);
      
      break;

   case GSM_GROUP_5:
      /*----------------------------------------------------------
      * Get current SACCH subchannel, increment structure value
      * to next subchannel, and set SACCH info pointer and state  
      *----------------------------------------------------------*/
      subChan = ts->u.group5.ulSacchSubCh;
      if ( ++ts->u.group5.ulSacchSubCh >= NUM_SACCH_SUBCHANS )
      {
         ts->u.group5.ulSacchSubCh = 0;
      }      
      /*----------------------------------------------------------
      * Check for cell broadcast. If so, no SACCH to monitor.
      *----------------------------------------------------------*/
      if(ts->u.group5.cbchUsed && (subChan == 2))
      {         
         return;
      }
      /*-----------------------------------------------------------
	   * Get SACCH info pointer and SDCCH state (not SACCH state).
      *----------------------------------------------------------*/
      sacchInfo = & ts->u.group5.sacch[subChan];                            
      chState = ts->u.group5.sdcch[subChan].state;     
      /*----------------------------------------------------------
	   * Process the SACCH channel if it's active. NOTE: Check
      * SACCH state for this, not SDCCH state (chState).  
      *----------------------------------------------------------*/
      if(sacchInfo->state == CHAN_ACTIVE)
      {
         /*--- Update DC offset for SDCCH associated with this SACCH ---*/ 
         DCOffset( g_BBInfo[rfCh].ulBBSmpl, ts->groupNum, rfCh, NUM_TN_PER_RF + subChan );
         
         demodulate( g_BBInfo[rfCh].ulBBSmpl, 
                     SACCH, 
                     g_BBInfo[rfCh].tseqNum, 
                     &g_ULBurstData,
                     ts->groupNum,
                     rfCh,
                     timeSlot,
                     subChan );              
         decrypt(rfCh, timeSlot);      
         chanDecoder(SACCH, 1, timeSlot);
         
         /*-----------------------------------------------------------
         * Finish accumulating bit errors and frame errors
         *----------------------------------------------------------*/
         accumBer(rfCh, timeSlot, ts->groupNum, SACCH, subChan,
                  sacchInfo, FALSE, FALSE); 

         /*----------------------------------------------------------
         * Accumulate TOA and power. Indicate SACCH4 to set final 
         * TOA and power, increment dyn pwr rpt count and adjust TA. 
         *----------------------------------------------------------*/
         accumPower(rfCh, timeSlot, ts->groupNum, SACCH4, sacchInfo, NULL);      
         accumToa  (rfCh, timeSlot, ts->groupNum, subChan, SACCH4, NULL); 
  		  
         /*-----------------------------------------------------------
         * Update AGC on last sacch subchannel burst
         *------------------------------------------------------------*/
         if ( g_AGC_ON )
         {
            if ( sacchInfo->agcHoldFlag )
            {
               /*----------------------------------------------------------
               * If SACCH passed parity, reset and release hold on AGC
               *----------------------------------------------------------*/
               if ( ! g_ULCCHData.ulcchword0.bitval.fireStatus ) 
               {  
                  sacchInfo->agcHoldFlag = FALSE;
                  sacchInfo->ulsigstren.rxPwrNormSmooth = 0;
               }
            }
            else
            {
               sigS = & sacchInfo->ulsigstren;
               sigS->rxPwrNormSmooth = ul_agc(sigS);
            }
         }
         /*-----------------------------------------------------------
         * If SACCH passed parity, release hold on DPC
         *------------------------------------------------------------*/
         if ( ! g_ULCCHData.ulcchword0.bitval.fireStatus ) 
         {  
            sacchInfo->dpcHoldFlag = FALSE;
         }
         /*-----------------------------------------------------------
         * Accumulate average power and perform dynamic power control 
         *------------------------------------------------------------*/
         averageS(& sacchInfo->ulsigstren,
                  & sacchInfo->dlsigstrenSCell,
                  g_DynPwrOAMInfo.ulsigstren.numRxSigSamples, 
                  & sacchInfo->MSPowerAccum,
                  TRUE);

         if (g_DPC_ON)
         {
            uplink_dynamic_power_control(ts, subChan);  
         }
         /*-----------------------------------------------------------
         * Process SACCH message
         *----------------------------------------------------------*/
         ulSyncMsgProcess(SACCH, subChan, timeSlot, rfCh);
      }
	  else
	  {
		  /*
		  *  when SACCH is deactivated, allow Radio Link Lost counting continue
		  *  Set Fire Status for current SACCH frame
		  */
          g_ULCCHData.ulcchword0.bitval.fireStatus = 1;
	  }

	  /*
	  *  pre fill channel Number for Radio Link Lost Message to RRM
	  */
      ackMsg.params[0] = timeSlot | 0x20 | (subChan <<3);
      
      break;
		
   case GSM_GROUP_1:
      /*-----------------------------------------------------------
	   * Get SACCH info pointer and TCH state (not SACCH state).
      * TCH/F has only one SACCH channel, so subChan is zero.  
      *----------------------------------------------------------*/
      sacchInfo = & ts->u.group1.sacch;                            
      chState  = ts->u.group1.tch.state;
      subChan = 0;
      /*-----------------------------------------------------------
	   * Process the SACCH channel if it's active. NOTE: Check
      * SACCH state for this, not TCH state (chState).  
      *----------------------------------------------------------*/
      if(sacchInfo->state == CHAN_ACTIVE)
      {
         /*------------------------------------------------------
         * If handover, try to detect HO access burst. If found,
         * set Fire Status for current SACCH frame. NOTE: HO
         * Access uses the Radio Link Timeout from OAM.
         *------------------------------------------------------*/
         if(ts->u.group1.tch.HOFlag)
         {
            ulRachProc(rfCh, timeSlot);
            g_ULCCHData.ulcchword0.bitval.fireStatus = 1;
         }
         /*------------------------------------------------------
         * Else demodulate normal burst
         *------------------------------------------------------*/
         else
         {		    
            /*--- Update DC offset for TCH associated with this SACCH ---*/ 
            DCOffset( g_BBInfo[rfCh].ulBBSmpl, ts->groupNum, rfCh, timeSlot );
         
            demodulate( g_BBInfo[rfCh].ulBBSmpl, 
                        SACCH, 
                        g_BBInfo[rfCh].tseqNum, 
                        &g_ULBurstData,
                        ts->groupNum,
                        rfCh,
                        timeSlot,
                        0 );              
            decrypt(rfCh, timeSlot);      
            chanDecoder(SACCH, 1, timeSlot);
         
            /*-----------------------------------------------------------
            * Finish accumulating bit errors and frame errors
            *----------------------------------------------------------*/
            accumBer(rfCh, timeSlot, ts->groupNum, SACCH, 0, sacchInfo,
                     ts->u.group1.tch.ULDtxMode, FALSE); 

            /*----------------------------------------------------------
            * Accumulate TOA and power. Indicate Burst 3 to set final 
            * TOA and power, increment dyn pwr rpt count and adjust TA. 
            *----------------------------------------------------------*/
            accumPower(rfCh, timeSlot, ts->groupNum, SACCH4, sacchInfo, NULL);      
            accumToa  (rfCh, timeSlot, ts->groupNum, 0, SACCH4, NULL);
               
            /*-----------------------------------------------------------
            * Update AGC on last sacch subchannel burst
            *------------------------------------------------------------*/
            if(g_AGC_ON)
            {
               if ( sacchInfo->agcHoldFlag )
               {
                  /*----------------------------------------------------------
                  * If SACCH passed parity, reset and release hold on AGC
                  *----------------------------------------------------------*/
                  if ( ! g_ULCCHData.ulcchword0.bitval.fireStatus ) 
                  {  
                     sacchInfo->agcHoldFlag = FALSE;
                     sacchInfo->ulsigstren.rxPwrNormSmooth = 0;
                  }
               }
               else
               {
                  sigS = & sacchInfo->ulsigstren;
                  sigS->rxPwrNormSmooth = ul_agc(sigS);
               }
            }
            /*-----------------------------------------------------------
            * If SACCH passed parity, release hold on DPC
            *------------------------------------------------------------*/
            if ( ! g_ULCCHData.ulcchword0.bitval.fireStatus ) 
            {  
               sacchInfo->dpcHoldFlag = FALSE;
            }
            /*-----------------------------------------------------------
            * Process SACCH message
            *----------------------------------------------------------*/
            ulSyncMsgProcess(SACCH, subChan, timeSlot, rfCh);
		 }
	  }
      else
	  {
		  /*
		  *  when SACCH is deactivated, allow Radio Link Lost counting continue
		  *  Set Fire Status for current SACCH frame
		  */
          g_ULCCHData.ulcchword0.bitval.fireStatus = 1;
	  } 
	  
	  /*
	  *  pre fill channel Number for Radio Link Lost Message to RRM
	  */
      ackMsg.params[0] = timeSlot | 0x8;
                  				
      break;
      
   default:       /* Non supported GSM Group */
	  chState = CHAN_INACTIVE;
	  break;  
   } 

   /*----------------------------------------------------------------------------------
   * If TCH or SDCCH is active and this SACCH failed parity, check for RF link lost
   *----------------------------------------------------------------------------------*/
   if ( (chState == CHAN_ACTIVE) && g_ULCCHData.ulcchword0.bitval.fireStatus )
   {           
      /*-----------------------------------------------------------
      * If lostCount at threshold and RLL enabled, send message
      *----------------------------------------------------------*/
      if( ++sacchInfo->lostCount >= radioLinkLostThrsh && g_diagData.sendRadioLinkLost )
      {
		 /*
		 *  allow send additional RLL on next Period, if the channel is not released
		 */
		 sacchInfo->lostCount = 0;
         /*
         * send RF link loss indication to RRM to release RF resource
         */
         ackMsg.msgSize = 9;
         ackMsg.function =  DCCH_MANAGE_MSG;
         ackMsg.trx =  rfCh;
         ackMsg.typeLoByte = DCH_RF_LINK_LOST_IND;
         ackMsg.typeHiByte = DCH_RF_LINK_LOST_IND>>8;    
         /* ackMsg.params[0] is set above */
         ackMsg.params[1] = 0; 
         ackMsg.params[2] = 0;                    
         ackMsg.params[3] = sacchInfo->lostCount;
         ackMsg.params[4] = g_idleFrameInfo[rfCh][timeSlot].rcvPower;  
         SendMsgHost_Que(&ackMsg);

	
	 
	

         /*
         *   If enabled, send idle frame I/Q samples to host
         */
         if ( g_diagData.sendIdleFrameCapture )
         { 
            idleChanDiagReport(rfCh, timeSlot);
         }     
      }                   
   } 
}

/*****************************************************************************
*
* Function: dlSacchProc0
*
* Description:
*   Processes downlink SACCH bursts for frame 0 of every 0-3 block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dlSacchProc0(Uint8 rfCh, Uint8 timeSlot)
{
	t_chanState sacchState;
	t_DLSigBufs *sacchMsg;
	t_sysInfoQueue *sysInfoMsg;	
	Uint8 subCh;
	Uint8 sysInfo, validSacch; 
	Uint8 msTimingAdvance;
	Uint8 msPowerLevel;      
	Uint8 sendSysInfo = 1;
	
	t_TNInfo *ts; 	
	
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];    
	
	validSacch = 0;	
	
	switch(ts->groupNum)
	{
	case GSM_GROUP_1:	
		sacchState  = ts->u.group1.sacch.state;	
		sacchMsg = ts->u.group1.sacch.dlSigBuf;
		msPowerLevel = ts->u.group1.sacch.powerLevel;
		msTimingAdvance = ts->u.group1.sacch.timingAdvance;
	    
		/*
		*  Check if L2/L3 has SACCH message, otherwise send SYS INFO
		*/		    
        if(sacchMsg->numMsgsReady)
        {           
            sendSysInfo = 0;
		    rtsCheck(rfCh, timeSlot, SACCH, 0, 0);
			sacchMsg->numMsgsReady--;
        }                             
        else
        {
			sysInfo = ts->u.group1.sacch.sysInfo++;
				                                                
			/*
			*  send only sysinfo TYPE_5 , TYPE_5 and TYPE_6
			*/		                                                
			if(sysInfo >= 2)
			{
                sysInfoMsg = & g_dlSigBufsSYSINFO[rfCh][TYPE_6]; 
                ts->u.group1.sacch.sysInfo = 0;
			}						
			else
            {
                sysInfoMsg = & g_dlSigBufsSYSINFO[rfCh][TYPE_5];
            }
		}             
		break;

	case GSM_GROUP_5: 
		subCh = ts->u.group5.dlSacchSubCh;
		sacchState  = ts->u.group5.sacch[subCh].state;
	        sacchMsg = ts->u.group5.sacch[subCh].dlSigBuf;			
		msPowerLevel = ts->u.group5.sacch[subCh].powerLevel;
		msTimingAdvance = ts->u.group5.sacch[subCh].timingAdvance;
	
		/*
		*  Check if L2/L3 has SACCH message, otherwise send SYS INFO
		*/		                                                   
		if(sacchMsg->numMsgsReady)                                       
		{
            sendSysInfo = 0;
		    rtsCheck(rfCh, timeSlot, SACCH, subCh, 0);            
			sacchMsg->numMsgsReady--;
		}
		else
		{
			/*
			*  get sysinfo to be sent
			*/
			sysInfo = ts->u.group5.sacch[subCh].sysInfo++;
		                                                
			/*
			*  send only sysinfo TYPE_5 , TYPE_5 and TYPE_6
			*/		                                                
			if(sysInfo >= 2)
			{
               sysInfoMsg = & g_dlSigBufsSYSINFO[rfCh][TYPE_6]; 
               ts->u.group5.sacch[subCh].sysInfo = 0;
			}						
			else
            {
               sysInfoMsg = & g_dlSigBufsSYSINFO[rfCh][TYPE_5];
            }
		}   
	
		break;


		
	case GSM_GROUP_7: 
		subCh = ts->u.group7.dlSacchSubCh;
		sacchState  = ts->u.group7.sacch[subCh].state;
		sacchMsg = ts->u.group7.sacch[subCh].dlSigBuf;			
		msPowerLevel = ts->u.group7.sacch[subCh].powerLevel;
		msTimingAdvance = ts->u.group7.sacch[subCh].timingAdvance;
	
		/*
		*  Check if L2/L3 has SACCH message, otherwise send SYS INFO
		*/		                                                   
		if(sacchMsg->numMsgsReady)                                       
		{   
                    sendSysInfo = 0;
		    rtsCheck(rfCh, timeSlot, SACCH, subCh, 0);            
		    sacchMsg->numMsgsReady--;
		}
		else
		{
			/*
			*  get sysinfo to be sent
			*/
			sysInfo = ts->u.group7.sacch[subCh].sysInfo++;
		                                                
			/*
			*  send only sysinfo TYPE_5, TYPE_5 and TYPE_6
			*/		                                                
			if(sysInfo >= 2)
			{
                             sysInfoMsg = & g_dlSigBufsSYSINFO[rfCh][TYPE_6]; 
                            ts->u.group7.sacch[subCh].sysInfo = 0;
			} else {
                           sysInfoMsg = & g_dlSigBufsSYSINFO[rfCh][TYPE_5];
                          }
		}   
	
		break;
		
		
	default: 
               sacchState = CHAN_INACTIVE;
      break;  
	}           
		
   if(sacchState == CHAN_ACTIVE)
   {	             
       if(sendSysInfo)
       {
          /*
          *  send SYSTEM_INFO
          */
         if(sysInfoMsg->frameNumUpdate == sysFrameNum.FN % (32*1326))
	   {
            sysInfoMsg->state ^= 0x01;
            sysInfoMsg->frameNumUpdate = 0xffffffff; 
            sysInfoMsg->valid = 1;
		  }   
		  /*
		  *  add L1 headers
		  */
         sysInfoMsg->data[sysInfoMsg->state][0] = msPowerLevel;
         sysInfoMsg->data[sysInfoMsg->state][1] = msTimingAdvance;	
		 // This seems to be a bug that has remained.

         if (ts->groupNum == GSM_GROUP_1){
                 sysInfoMsg->data[sysInfoMsg->state][4] |= ts->u.group1.tch.dlDtxUsed<<7;
         }
         
	
	
         unpackFromByteBuffer(&sysInfoMsg->data[sysInfoMsg->state][0],
                              (UChar *)&g_DLCCHData.data[0], 
                              NUM_BITS_CCH_FRAME);
	  	      
       }
       else
       {     
          /*
          *  send SACCH MSG, e.g SMS-PP
          */       
         sacchMsg->data[sacchMsg->readIndex][0] = msPowerLevel;
         sacchMsg->data[sacchMsg->readIndex][1] = msTimingAdvance;
         sacchMsg->data[sacchMsg->readIndex][4] |= ts->u.group1.tch.dlDtxUsed<<7;
                
         unpackFromByteBuffer(&sacchMsg->data[sacchMsg->readIndex++][0],
                              (UChar *)&g_DLCCHData.data[0], 
                              NUM_BITS_CCH_FRAME);		         
                                                             
		  sacchMsg->readIndex &= (DL_SIG_Q_DEPTH-1);

       }

       g_DLCCHData.bufferValid = TRUE;	              
       chanEncoder(SACCH, 1, timeSlot,0);	         
       encrypt(rfCh, timeSlot, 0);      
       buildGSMBurst(SACCH,  g_BBInfo[rfCh].tseqNum);
       GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, SACCH);	              
	}    
	else 
	{     
      /*
      * send a filler frame
      unpackFromByteBuffer(& dcchFiller,
                           (UChar *)&g_DLCCHData.data[0], 
                           NUM_BITS_CCH_FRAME);	
      */                                    
        
      /*
      *  send a dummy burst if channel is not activated
      */     
      dlIdleProc(rfCh, timeSlot);
	}	
}

/*****************************************************************************
*
* Function: dlSacchProc1
*
* Description:
*   Processes downlink SACCH bursts for frames 1 and 2 of every 0-3 block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dlSacchProc1(Uint8 rfCh, Uint8 timeSlot)
{                      
	t_TNInfo *ts; 	
	t_chanState sacchState;
	Uint8 subCh;
			
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];    
	
	switch(ts->groupNum)
	{
	case GSM_GROUP_1:
	   sacchState  = ts->u.group1.sacch.state;
	   break;
	   
	case GSM_GROUP_5:
	   subCh = ts->u.group5.dlSacchSubCh;
 	   sacchState  = ts->u.group5.sacch[subCh].state;
 	   break;

	case GSM_GROUP_7:
	   subCh = ts->u.group7.dlSacchSubCh;
 	   sacchState  = ts->u.group7.sacch[subCh].state;
    break;
 	 
 	 
	default:
	   sacchState = CHAN_INACTIVE;
	   break;                      
	}
	
	if(sacchState == CHAN_ACTIVE)
	{
      chanEncoder(SACCH, 0, timeSlot,0);    
      encrypt(rfCh, timeSlot, 1);      
      buildGSMBurst(SACCH,  g_BBInfo[rfCh].tseqNum);
      GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, SACCH);			
   }
   else
   {
      /*
      *  send a dummy burst if channel is not activated
      */     
      dlIdleProc(rfCh, timeSlot);
    }    
}

/*****************************************************************************
*
* Function: dlSacchProc3
*
* Description:
*   Processes downlink SACCH bursts for frame 3 of every 0-3 block
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dlSacchProc3(Uint8 rfCh, Uint8 timeSlot)
{
 	t_TNInfo *ts; 	
	t_chanState sacchState;
	Uint8 subCh;
			
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];    
	
	switch(ts->groupNum)
	{
	case GSM_GROUP_1:
	   sacchState  = ts->u.group1.sacch.state;
	   break;
	   
	case GSM_GROUP_5:
      /*----------------------------------------------------------
      * Get current SACCH subchannel, increment structure value
      * to next subchannel, and set SACCH info pointer and state  
      *----------------------------------------------------------*/
      subCh = ts->u.group5.dlSacchSubCh;
      if ( ++ts->u.group5.dlSacchSubCh >= NUM_SACCH_SUBCHANS )
      {
         ts->u.group5.dlSacchSubCh = 0;
      }               
 	   sacchState  = ts->u.group5.sacch[subCh].state;
 	   break;
 	 
	case GSM_GROUP_7:
      /*----------------------------------------------------------
      * Get current SACCH subchannel, increment structure value
      * to next subchannel, and set SACCH info pointer and state  
      *----------------------------------------------------------*/
      subCh = ts->u.group7.dlSacchSubCh;
      if ( ++ts->u.group7.dlSacchSubCh >= NUM_SACCH8_SUBCHANS )
      {
         ts->u.group7.dlSacchSubCh = 0;
      }               
 	   sacchState  = ts->u.group7.sacch[subCh].state;
 	   break;

	default:
	   sacchState = CHAN_INACTIVE;
	   break;                      
	}
	
	if(sacchState == CHAN_ACTIVE)
	{
      chanEncoder(SACCH, 0, timeSlot,0);    
      encrypt(rfCh, timeSlot, 3);      
      buildGSMBurst(SACCH,  g_BBInfo[rfCh].tseqNum);
      GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, SACCH);			
   }
   else
   {
      /*
      *  send a dummy burst if channel is not activated
      */     
      dlIdleProc(rfCh, timeSlot);
   } 
}
     
/*****************************************************************************
*
* Function: idleChanDiagReport
*
* Description:
*   Sends idle frame I/Q back to host for Diagnostic purpose
*   to check is Any interferences on idle channel or Abnormal signal
*   on I/Q samples
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void idleChanDiagReport(Uint8 rfCh, Uint8 timeSlot)
{
   msgStruc idleDiagMsg;
   Int16 i, j;
   Uint32 *outBuf;

   idleDiagMsg.msgSize = 60;
   idleDiagMsg.function =  DIAGNOSTIC_MSG;
   idleDiagMsg.trx =  rfCh;
   idleDiagMsg.typeLoByte = MPH_DSP_DIAG_IDLE_FRAME_LOG_RSP;
   idleDiagMsg.typeHiByte = MPH_DSP_DIAG_IDLE_FRAME_LOG_RSP>>8;    
   idleDiagMsg.params[0] = timeSlot;
   idleDiagMsg.params[1] = 0;
   idleDiagMsg.params[2] = 0;
   
   /*
   * Ready pointer to copy longwords
   */
   outBuf = (Uint32*)(&idleDiagMsg.params[4]);
   
   for(j=0; j<13; j++)
   {
      idleDiagMsg.params[3] = j;      /* block Number of an idle Frame */
      for(i=0; i<(156/13); i++)
         outBuf[i] = g_idleFrameInfo[rfCh][timeSlot].data[i+j*(156/13)];     		            
      SendMsgHost_Que( & idleDiagMsg);
   }
}
   		            
