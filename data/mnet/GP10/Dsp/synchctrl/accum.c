 /****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: accum.c
*
* Description:
*   This file contains functions that process accumulate power, BER and TOA.
*
* Public Functions:
*   accumPower, accumBer   
*
* Private Functions:
*   accumToa
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
#include "codec.h"

/*****************************************************************************
*
* Function: accumPower
*
* Description:
*   Accumulates MS power for agc and uplink dyn power ctrl. 
*   Increments dynamic power report counter. 
*   Intended for Burst 3 of 0,1,2,3.
*
* Input:
*   rfCh                   RF channel index
*   timeSlot               GSM time slot   
*   groupNum               GSM Group number          
*   chanSel                Logical channel type (i.e. SACCH, TCH, ...)
*   sacchInfo              ptr to updated SACCH Info structure 
*   TBF                    GPRS - pointer to temp block flow
*
* Output:
*   sacchInfo              ptr to updated SACCH Info structure 
*   .MSPowerAccumFull        MS power in all TCH frames of SACCH multiframe. 
*   .MSPowerAccumSub         MS power in DTX (SACCH) frames (dB relative to FS)           
*   .MSPowerAccum            MS power in all frames (SACCH + TCH)
*   .rcvPowerNormSmooth      Smoothed, normalized rcvPower value used for 
*                                 ul_agc(), dyn pwr ctrl.    
*
******************************************************************************/
ProcResult accumPower(Uint8 rfCh, Uint8 timeSlot, t_groupNum groupNum,
                      t_chanSel chanSel, t_SACCHInfo *sacchInfo, tbfStruct *TBF)
{   
   t_TNInfo    *ts;
   Int16       currentRxLev;     
   t_ULSignalStrength *sigS;

   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];   

   switch(ts->groupNum)
   { 
     case GSM_GROUP_1:      
       /*-------------------------------------------------------------------
       * If channel is active and DPC Hold is off, then accumulate power
       *-------------------------------------------------------------------*/
       if( sacchInfo->state == CHAN_ACTIVE && !sacchInfo->dpcHoldFlag )
       {       
         sigS = & sacchInfo->ulsigstren;
         currentRxLev = computeUplinkRxLev(sigS);
		 
         /*----------------------------------------------------------
         * For all burst phases except hoProc/idle, accumulate power 
         * Sub, Full and smooth received power for AGC.
         * Note: Accumulate Sub over SACCH and 8 potential SID frames
         * (total of 12 per SACCH interval. Accumulate Full over all
         * non-idle frames (total of 100 per SACCH interval).
         *----------------------------------------------------------*/
         if ( chanSel == SACCH || chanSel == SACCH4 )
         {
            sacchInfo->MSPowerAccumFull += currentRxLev;         
            sacchInfo->MSPowerAccumSub  += currentRxLev;          
            if ( g_AGC_ON ) { 
               sigS = & sacchInfo->ulsigstren;
               rcvPwrSmoothingFilter(sigS, g_ULBurstData.rcvPower);
            }   
         }
         else if( chanSel == TCHF )
         {
            sacchInfo->MSPowerAccumFull += currentRxLev;
            
            /*-------------------------------------------------------------------
            *  Also accumulate power of potential SID frames into Sub (as well as
            *  Full). Always include SID frames in AGC smoothing; include non-SID
            *  frames in AGC only if uplink DTX is not currently in use.
            *-------------------------------------------------------------------*/
            if( ts->u.group1.ulIndex==((12-timeSlot) & 0x7) && ts->ulTblIndex<8 )
            { 
               sacchInfo->MSPowerAccumSub += currentRxLev;
               if ( g_AGC_ON ) {
                  sigS = & sacchInfo->ulsigstren;
                  rcvPwrSmoothingFilter(sigS, g_ULBurstData.rcvPower);
               }     
            }
            else
            {
               if ( g_AGC_ON && !ts->u.group1.tch.ulDtxUsed ) {
                  sigS = & sacchInfo->ulsigstren;
                  rcvPwrSmoothingFilter(sigS, g_ULBurstData.rcvPower);
               }     
            }
            /*-------------------------------------------------------------------
            *  If uplink DTX is used at any time within SACCH interval, latch it
            *-------------------------------------------------------------------*/
            if ( ts->u.group1.tch.ulDtxUsed )
            {
               ts->u.group1.tch.ulDtxUsed104 = TRUE;
            }    
         }
         /*----------------------------------------------------------
         * In final burst (hoProc/idle), set final power 
         *----------------------------------------------------------*/ 
         else if ( chanSel == SACCH_OR_IDLE )
         {  
            /*------------------------------------------------------------------
            * Choose 100-burst Full average (four bursts of SACCH + 96 bursts of
            * TCH) if uplink DTX was never used in this SACCH interval; choose
            * 12-burst Sub average (4 bursts of SACCH plus 8 SID) if uplink DTX
            * was used at any time during the SACCH interval.
            *------------------------------------------------------------------*/
            if ( ts->u.group1.tch.ulDtxUsed104 )
            {
               sacchInfo->MSPowerAccum = sacchInfo->MSPowerAccumSub / 12;
            }
            else
            {
               sacchInfo->MSPowerAccum = sacchInfo->MSPowerAccumFull / 100;
            }      		
            /* Clear power accumulators and UL DTX latch for next SACCH interval */
            sacchInfo->MSPowerAccumSub  = 0;
            sacchInfo->MSPowerAccumFull = 0;
            ts->u.group1.tch.ulDtxUsed104 = FALSE;    

            /* Used for periodic interference reporting defined by Intave parameter */
            if( timeSlot == 7) {
               g_IntRprtCntr += 1;
            }
         }
       }

       /*-------------------------------------------------------------------
       * Calculate co-channel interference only for Group 1, DPC held or not
       *-------------------------------------------------------------------*/
	      else
       {
         /*----------------------------------------------------------
         * In final burst (hoProc/idle), set final Power for int. processing
         *----------------------------------------------------------*/ 
         if ( chanSel == SACCH_OR_IDLE )
         {   
           sacchInfo->MSPowerAccumFull += currentRxLev;
           sacchInfo->MSPowerAccum = sacchInfo->MSPowerAccumFull / 4;
               		
           /* clear sacch, tch power for next sacch multiframe */        
           sacchInfo->MSPowerAccumFull = 0;   
           sacchInfo->MSPowerAccumSub  = 0;

           /* Used for periodic interference reporting defined by Intave parameter */
           if( timeSlot == 7) {
             g_IntRprtCntr += 1;
           }
         }
       }
	   break;

     case GSM_GROUP_5: 
       if( sacchInfo->state == CHAN_ACTIVE && !sacchInfo->dpcHoldFlag )
       {
               sigS = & sacchInfo->ulsigstren;
         currentRxLev = computeUplinkRxLev(sigS);

         if ( chanSel == SDCCH || chanSel == SDCCH4 )
         {
            sacchInfo->MSPowerAccumSub += currentRxLev;
            if(g_AGC_ON) {
               rcvPwrSmoothingFilter(sigS, g_ULBurstData.rcvPower);  
            }
         }
         if ( chanSel == SACCH || chanSel == SACCH4 )
         {
            sacchInfo->MSPowerAccumSub += currentRxLev;
            if(g_AGC_ON) {
               sigS = & sacchInfo->ulsigstren;
               rcvPwrSmoothingFilter(sigS, g_ULBurstData.rcvPower);   
            }
            if ( chanSel == SACCH4 )
            {
               sacchInfo->MSPowerAccum = sacchInfo->MSPowerAccumSub / 12; 

               /* clear power accumulator on this subchan for next SACCH interval */
               sacchInfo->MSPowerAccumSub  = 0;
            }
         }
       }  
       break;



  case GSM_GROUP_7: 
       if( sacchInfo->state == CHAN_ACTIVE &&  !sacchInfo->dpcHoldFlag )
       {
               sigS = & sacchInfo->ulsigstren;
         currentRxLev = computeUplinkRxLev(sigS);

        if ( chanSel == SDCCH || chanSel == SDCCH4 )
         {
            if(g_AGC_ON) {
               sigS = & sacchInfo->ulsigstren;
               rcvPwrSmoothingFilter(sigS, g_ULBurstData.rcvPower);  
            }
         }
        if ( chanSel == SACCH || chanSel == SACCH4 )
         {
            sacchInfo->MSPowerAccumSub += currentRxLev;
            if(g_AGC_ON) {
               sigS = & sacchInfo->ulsigstren;
               rcvPwrSmoothingFilter(sigS, g_ULBurstData.rcvPower);   
            }
        if ( chanSel == SACCH4)
            {
               sacchInfo->MSPowerAccum = sacchInfo->MSPowerAccumSub / 12; 

               /* clear power accumulator on this subchan for next SACCH interval */
               sacchInfo->MSPowerAccumSub  = 0;
            }
         }
       }  



	  
       break;
     case GSM_GROUP_13:
       if ( TBF!=NULL && TBF->ulTbf.state==CHAN_ACTIVE)
       {
         /* GPRS */
         sigS = & TBF->ulTbf.sigstren;
         currentRxLev = computeUplinkRxLev(sigS);
               
         if ( chanSel == PDCH || chanSel == PDCH4 )
         {
           TBF->ulTbf.accum.MSPowerSub += currentRxLev;
           if(g_AGC_ON) {
             rcvPwrSmoothingFilter(sigS, g_ULBurstData.rcvPower);   
           } 
           if ( chanSel == PDCH4 )
           {
             TBF->ulTbf.accum.MSPower = TBF->ulTbf.accum.MSPowerSub / 4; 

             /* clear power accumulator on this subchan for next RLC block */
             TBF->ulTbf.accum.MSPowerSub  = 0;
           }
         }
       }
       break;

     default:
       break;
  }

  return SUCCESS;
}


/*****************************************************************************
*
* Function: computeUplinkRxLev
*
* Decription:
*      Compute uplink RXLEV of burst using rcvPower (dBr) and
*      current rxgain (0-0x3ff) as input. All other terms are
*      measured calibration constants specific to GP10 H/W.
*
* Input:
*   TBF->ulTbf.sigstren                   for GPRS
*      or
*   sacchInfo->ulsigstren                 for voice
*
* Output:
*   RxLev             current uplink value           
*
******************************************************************************/
Int16 computeUplinkRxLev(t_ULSignalStrength *sigstren)
{
  Int16 currentRxLev;

  currentRxLev = g_MeasAgcDpcDbm + DBM_2_RXLEV_CONV_FACTOR
                 + (((sigstren->rxgain - g_MeasAgcDpcRxgain)
                 * g_gainStepsPerDb.inverseSlope) >> DIVISION_FACTOR); 

  if(currentRxLev < 0) {   /* Limit to a positive range */
    currentRxLev = 0;
  }

  return (currentRxLev);
}

/*****************************************************************************
*
* Function: accumToa
*
* Decription:
*   Accumulates MS TOA.  
*   Intended for SACCH Burst 3 of 0,1,2,3 and 
*
* Input:
*   rfCh                   RF channel index 
*   timeSlot               GSM time slot    
*   groupNum               GSM Group number (I or V)
*   subChan                subchannel (if SDCCH)
*   chanSel                logical channel type   
*   sacchInfo              ptr to SACCH Info structure
*   TBF                    GPRS - pointer to temp block flow
*
* Output:
*   sacchInfo              ptr to updated SACCH Info structur
*   .toaAccumSub                TOA on DTX frames         
*   .toaAccum                   TOA on all frames   
*   .dynPwrRprtCntr             incremented dynamic pwr control count           
*
******************************************************************************/
ProcResult accumToa(Uint8 rfCh, Uint8 timeSlot, t_groupNum groupNum, 
                    Uint8 subChan, t_chanSel chanSel, tbfStruct *TBF)
{   
   t_diagMsReport     *msReport;
   t_diagGprsMsReport *gprsMsReport;
   t_DLSigBufs    *ptcchMsg;
   t_PTCCHInfo    *ptcch;
   t_TNInfo       *ts;
   t_SACCHInfo    *sacchInfo;
   Uint8          tfi;
   Uint8          currentTAI;
   

   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

   switch(groupNum)
   {
    case GSM_GROUP_11:
    case GSM_GROUP_13:
     if ( TBF )
     {  
         /*-------------------------------------------------------------------
         * For all burst phases, accumulate TOA
         *-------------------------------------------------------------------*/
         TBF->ulTbf.accum.toaSub += g_ULBurstData.toa;
         tfi = TBF->ulTbf.tfi;

         /*-------------------------------------------------------------------
         * If GPRS MS Report enabled for this TFI, save min/max TOA 
         *-------------------------------------------------------------------*/
         if ( g_diagData.gprsMsReportTfiMask & (1L<<tfi) )
         {         
          gprsMsReport = & g_diagData.gprsMsReport[rfCh][tfi]; 
          
          /*----------------------------------------------------------
          * Update min and max TOA. This runs over PDCH interval. 
          *----------------------------------------------------------*/
          if ( g_ULBurstData.toa < gprsMsReport->minToa )
            gprsMsReport->minToa = g_ULBurstData.toa;
          if ( g_ULBurstData.toa > gprsMsReport->maxToa )
            gprsMsReport->maxToa = g_ULBurstData.toa;
         }
         
         /*-----------------------------------------------------------------
         * In final burst phase, set final TOA, check whether to adjust TA
         *-----------------------------------------------------------------*/
         if ( chanSel == PDCH4 )
         {  
            /*----------------------------------------------------------
            * Average TOA over last RLC block, then clear accum value
            *----------------------------------------------------------*/
            TBF->ulTbf.accum.toa = (TBF->ulTbf.accum.toaSub+2)>>2;  
            TBF->ulTbf.accum.toaSub = 0;  
   
            /*----------------------------------------------------------------
            * If GPRS MS Report enabled for this TFI, accumulate TOA averages
            *----------------------------------------------------------------*/
            if ( g_diagData.gprsMsReportTfiMask & (1L<<tfi) )
            {
               gprsMsReport->cumToa += TBF->ulTbf.accum.toa;
            }
         
            /*----------------------------------------------------------
            * If SACCH passed parity, adjust TA and reset lostCount
            *----------------------------------------------------------*/
            if( ! g_ULCCHData.ulcchword0.bitval.fireStatus && TBF->ulTbf.accum.toa != 0 ) 
            {  
               /*-----------------------------------------------------------------------
               * Allow TA to adjust only if MS is already using most recent commanded TA,
               * FOR GPRS, NOT SURE WHERE toaLastGoodActual GETS POPULATED.  MAY DELETE 
               * THIS CHECK!!!!
               *-----------------------------------------------------------------------*/
               if(TBF->ulTbf.accum.toaLastGoodActual == TBF->ulTbf.timingAdvance)
               {
                  /*----------------------------------------------------------
                  * If TOA is within allowed range, update TA to be sent to MS
                  *----------------------------------------------------------*/
                  if( TBF->ulTbf.accum.toa>=NB_TOA_MIN && TBF->ulTbf.accum.toa<=NB_TOA_MAX )
                  {                
                     /*
                     *  adjust TA by 1 bit at a time
                     */     
                     if (TBF->ulTbf.accum.toa > 0)
                     {
                        TBF->ulTbf.timingAdvance += 1;              
                     }
                     else
                     {
                        TBF->ulTbf.timingAdvance -= 1;
                     }                                                   
                     sendDiagMsg(0x68, rfCh, timeSlot, 1, (Uint8*)(&TBF->ulTbf.accum.toa));
                                
                     /*
                     *  limit TA to 0-63
                     */
                     if (TBF->ulTbf.timingAdvance < 0)
                     {
                        TBF->ulTbf.timingAdvance = 0;                
                     }
                     else if (TBF->ulTbf.timingAdvance > 63)
                     {
                        TBF->ulTbf.timingAdvance = 63;
                     }

                   

                     switch(ts->groupNum){

                     case  GSM_GROUP_13:
                             currentTAI = ts->u.group13.currentTAI;
                             ptcch = &(ts->u.group13.ptcch);
                                                  
                             break;

                     case  GSM_GROUP_11:

                             currentTAI = ts->u.group11.currentTAI;
                             ptcch = &(ts->u.group11.ptcch);
                             break;
                             default:
                                     break;
                     }


                     ptcchMsg = ptcch->dlSigBuf;
                                     

                     if (currentTAI >= 0 && currentTAI <= 15){
  
                       ptcchMsg->numMsgsReady++;
                       ptcchMsg->data[ptcchMsg->readIndex][0] = currentTAI;
                       ptcchMsg->data[ptcchMsg->readIndex][1] = TBF->ulTbf.tai;
                       ptcchMsg->data[ptcchMsg->readIndex][2] = TBF->ulTbf.timingAdvance;
                       ptcchMsg->data[ptcchMsg->readIndex][3] = TBF->ulTbf.taiTimeSlot;
                     }        
                     else 
                     {
                       /* Invalid TAI */
                       sendDiagMsg ( 0x71, rfCh, timeSlot, 1, (Uint8*)(&TBF->ulTbf.tai)); 
                     }
                  }    
               }
            }
         }
     }
   
            
      break;
     
   case GSM_GROUP_1:   
   case GSM_GROUP_5: 
   case GSM_GROUP_7:	   

     if(groupNum == GSM_GROUP_1) {
       sacchInfo = & ts->u.group1.sacch; 
       msReport = & g_diagData.msReport[rfCh][timeSlot]; 
     }
     else if (groupNum == GSM_GROUP_5) {
       sacchInfo = & ts->u.group5.sacch[subChan]; 
       msReport = & g_diagData.msReport[rfCh][NUM_TN_PER_RF+subChan];
     } else if (groupNum == GSM_GROUP_7) {
		sacchInfo = & ts->u.group7.sacch[subChan]; 
       msReport = & g_diagData.sdcch8Report[rfCh][subChan];
		
	 }

     /*-------------------------------------------------------------------
     * For all burst phases, if DPC Hold is off, then accumulate TOA
     *-------------------------------------------------------------------*/
     if ( !sacchInfo->dpcHoldFlag )
     {
       sacchInfo->toaAccumSub += g_ULBurstData.toa;
     }
   
     /*-------------------------------------------------------------------
     * If MS Report enabled for this TS and DPC Hold off, save min/max TOA 
     *-------------------------------------------------------------------*/
     if ( g_diagData.msReportTsMask & (1<<timeSlot) && !sacchInfo->dpcHoldFlag )
     {
       /*----------------------------------------------------------
       * Update min and max TOA. This runs over SACCH interval. 
       *----------------------------------------------------------*/
       if ( g_ULBurstData.toa < msReport->minToa )
         msReport->minToa = g_ULBurstData.toa;
       if ( g_ULBurstData.toa > msReport->maxToa )
         msReport->maxToa = g_ULBurstData.toa;
     }
     /*-----------------------------------------------------------------
     * In final burst phase, set final TOA, check whether to adjust TA
     *-----------------------------------------------------------------*/
     if ( ((chanSel == SACCH4) || (chanSel == SACCH8)) && !sacchInfo->dpcHoldFlag )
     {  
       /*----------------------------------------------------------
       * Average TOA over 4 bursts of SACCH interval, then clear
       * accumulating value. Increment dyn report counter, which
       * is reset by UL dynamic power control (dynpwrctrl.c).
       *----------------------------------------------------------*/
       sacchInfo->toaAccum = (sacchInfo->toaAccumSub+2)>>2;  
       sacchInfo->toaAccumSub = 0;  

       if(sacchInfo->dynPwrRprtCntr < g_DynPwrOAMInfo.ulsigstren.numRxSigSamples)
       {
         sacchInfo->dynPwrRprtCntr += 1; 		
       }
 
       /*----------------------------------------------------------
       * If MS Report enabled for this TS, accumulate TOA averages
       *----------------------------------------------------------*/
       if ( g_diagData.msReportTsMask & (1<<timeSlot) )
       {
         msReport->cumToa += sacchInfo->toaAccum;
       }
     
       /*----------------------------------------------------------
       * If SACCH passed parity, adjust TA and reset lostCount
       *----------------------------------------------------------*/
       if( ! g_ULCCHData.ulcchword0.bitval.fireStatus && sacchInfo->toaAccum != 0 ) 
       {  
         /*-----------------------------------------------------------------------
         * Allow TA to adjust only if MS is already using most recent commanded TA
         *-----------------------------------------------------------------------*/
         if(sacchInfo->toaLastGoodActual == sacchInfo->timingAdvance)
         {
            /*----------------------------------------------------------
            * If TOA is within allowed range, update TA to be sent to MS
            *----------------------------------------------------------*/
            if( sacchInfo->toaAccum>=NB_TOA_MIN && sacchInfo->toaAccum<=NB_TOA_MAX )
            {                
               /*
               *  adjust TA by 1 bit at a time
               */     
               if (sacchInfo->toaAccum > 0)
               {
                  sacchInfo->timingAdvance += 1;              
               }
               else
               {
                  sacchInfo->timingAdvance -= 1;
               }                                                   
               sendDiagMsg(0x67, rfCh, timeSlot, 1, (Uint8*)(&sacchInfo->toaAccum));
                                
               /*
               *  limit TA to 0-63
               */
               if (sacchInfo->timingAdvance < 0)
               {
                   sacchInfo->timingAdvance = 0;                
               }
               else if (sacchInfo->timingAdvance > 63)
               {
                  sacchInfo->timingAdvance = 63;
               }
            }
            /*----------------------------------------------------------
            * If TOA is not within allowed range, send diag message
            *----------------------------------------------------------*/
            else
            {  
               sendDiagMsg ( INVALID_TIMING_ADV, rfCh, timeSlot, 1,
                             (Uint8*)(&sacchInfo->toaAccum));                     
            }
         }                
         sacchInfo->lostCount = 0;          
       }
     }
     break;

   default:
      break;
   }
   
   return SUCCESS;
}

/*****************************************************************************
*
* Function: accumBer 
*
* Decription:
*   Accumulates bit errors and frame errors for diagnostic reports  
*
* Input:
*   rfCh                   RF channel index
*   timeSlot               GSM time slot
*   groupNum               GSM Group number (I or V)
*   chanSel                logical channel type (TCH or SACCH)
*   subChan                subchannel (if SDCCH)    
*   sacchInfo              ptr to SACCH Info structure
*
* Output:
*   sacchInfo              ptr to updated SACCH Info structure
*   .bitErrorAccum           updated number of bit errors          
*   .bitCountAccum           updated number of bits checked          
*
******************************************************************************/
ProcResult accumBer(Uint8 rfCh, Uint8 timeSlot, t_groupNum groupNum,
                    t_chanSel chanSel, Uint8 subChan, t_SACCHInfo *sacchInfo,
                    Bool dtxUsed, Bool dtxSid)
{ 
   t_diagMsReport *msReport;

   /*------------------------------------------------------------------------
   * Traffic channel, TCH/FS or FACCH. Accumulate only if DPC Hold is off.
   *------------------------------------------------------------------------*/
   if ( chanSel == TCHF && !sacchInfo->dpcHoldFlag )
   {
      /*------------------------------------------------------------------
      * If FACCH, accumulate CCHData's nErrs and use nbr FACCH bits/frame.
      * A lot of false FACCH detections are generated in DTX mode, so
      * accum only if ULDTX is off or if this is a SID frame.
      *-----------------------------------------------------------------*/
      if ( g_ULCCHData.ulcchword0.bitval.bufferValid )
      {
         if ( !dtxUsed || dtxSid )
         {
                 /* Limit one frame's BER contribution to 13% */
            if ( g_ULCCHData.nErrs > (NUM_BER_BITS_FACCH_FRAME*13/100) )
               g_ULCCHData.nErrs = (NUM_BER_BITS_FACCH_FRAME*13/100);

            /* Accumulate bit errors and total bit count */
            sacchInfo->bitErrorAccum += g_ULCCHData.nErrs;
            sacchInfo->bitCountAccum += NUM_BER_BITS_FACCH_FRAME;
         
            /* Accumulate frame errors and frame count */
            if ( g_ULCCHData.ulcchword0.bitval.fireStatus )
            {                                       
               sacchInfo->frameErrorAccum += 1;
            }                                  
            sacchInfo->frameCountAccum += 1;
         }
      }
      /*----------------------------------------------------------------
      * If TCH, accumulate TCHData's nErrs and use nbr TCH bits/frame.
      * If non-DTX, observe all frames. If DTX, observe only SID.
      * For both, do not accumulate TCH errors in DCCH Report Only,
      * which is a diagnostic test mode that will disrupt handover.
      *----------------------------------------------------------------*/
      else
      {
        if ( (!dtxUsed || dtxSid) && !g_diagData.dcchReportOnly )
         {
            /* Limit one frame's BER contribution to 13% */
            if ( g_ULTCHData.nErrs > (NUM_BER_BITS_TCH_FRAME*13/100) )
               g_ULTCHData.nErrs = (NUM_BER_BITS_TCH_FRAME*13/100);       

            /* Accumulate bit errors and total bit count */
            sacchInfo->bitErrorAccum += g_ULTCHData.nErrs;
            sacchInfo->bitCountAccum += NUM_BER_BITS_TCH_FRAME;
            
            /* Accumulate frame errors and frame count */
            if ( g_ULTCHData.ultchword0.bitval.bfi )
            {                                       
               sacchInfo->frameErrorAccum +=1;
            }                   
            sacchInfo->frameCountAccum += 1;
         }       
      }
   }

   /*------------------------------------------------------------------------
   * SDCCH, Group V or VII. Accumulate only if DPC Hold is off.
   *------------------------------------------------------------------------*/
   else if ((chanSel == SDCCH4 || chanSel == SDCCH || chanSel == SDCCH8) && !sacchInfo->dpcHoldFlag )
   {
      sacchInfo->bitErrorAccum += g_ULCCHData.nErrs;
      sacchInfo->bitCountAccum += NUM_BER_BITS_FACCH_FRAME;
      
      if ( g_ULCCHData.ulcchword0.bitval.fireStatus )
      {                                       
         sacchInfo->frameErrorAccum += 1;
      }                                  
      sacchInfo->frameCountAccum += 1 ; 
   }

   /*------------------------------------------------------------------------
   * SACCH, either Group 1,Group V, and Group VII  (don't accum bit errors on SACCH)
   *------------------------------------------------------------------------*/
   else if ( (chanSel == SACCH4 || chanSel == SACCH || chanSel == SACCH8) && !sacchInfo->dpcHoldFlag )
   {
            if ( !g_diagData.dcchReportOnly )
      {
         /* Limit one frame's BER contribution to 13% */
         if ( g_ULCCHData.nErrs > (NUM_BER_BITS_SACCH_FRAME*13/100) )
            g_ULCCHData.nErrs = (NUM_BER_BITS_SACCH_FRAME*13/100);       
      
         /* Accumulate bit errors and total bit count */
         sacchInfo->bitErrorAccum += g_ULCCHData.nErrs;
         sacchInfo->bitCountAccum += NUM_BER_BITS_SACCH_FRAME;
         /*
          * NOTE: Don't accumulate SACCH FER. That way FER will truly reflect the
          * performance of TCH or SDCCH only. We don't want to mix SACCH's FER
          * characteristic into the reported FER. SACCH's spread-out bursts make
          * its FER performance different, but BER is the same, so we include it.
          */
      }

      /*----------------------------------------------------------------
      * Compute and save BER in units of 0.01% (resolution=1/10000)
      *----------------------------------------------------------------*/      
      sacchInfo->ulsigqual.ber = (10000*(Uint32)sacchInfo->bitErrorAccum) /
                                       sacchInfo->bitCountAccum;

      /*----------------------------------------------------------
      * If MS Report enabled for this TS, accumulate bit errors
      *----------------------------------------------------------*/
      if ( g_diagData.msReportTsMask & (1<<timeSlot) )
      {
         /*---------------------------------------------------------------
         * If Group V and Group VII, it's SACCH associated with SDCCH. Use the
         * last 4 msReport structures.
         *---------------------------------------------------------------*/
         if ( groupNum == GSM_GROUP_5)
         {
            msReport = & g_diagData.msReport[rfCh][NUM_TN_PER_RF+subChan]; 
         } else if (groupNum == GSM_GROUP_7){
	    msReport = & g_diagData.sdcch8Report[rfCh][subChan];
	 } else
         {
            msReport = & g_diagData.msReport[rfCh][timeSlot]; 
         }
         /*----------------------------------------------------------
         * Accumulate bit and frame counts in msReport structure
         *----------------------------------------------------------*/
         msReport->sacchCount    += 1;
         msReport->cumErrs       += sacchInfo->bitErrorAccum;
         msReport->cumBits       += sacchInfo->bitCountAccum;
         msReport->cumFrameErrs  += sacchInfo->frameErrorAccum;
         msReport->cumFrames     += sacchInfo->frameCountAccum;
      }
      /*----------------------------------------------------------
      * Clear BER and FER counts for next SACCH interval
      *----------------------------------------------------------*/
      sacchInfo->bitErrorAccum = 0;
      sacchInfo->bitCountAccum = 0;   
      sacchInfo->frameErrorAccum = 0;
      sacchInfo->frameCountAccum = 0;
   }   
}

/*****************************************************************************
*
* Function: accumBitBlockErrs
*
* Decription:
*   Used by GPRS only, so far.  Smoothes bit error rates (BER) and RLC block 
*    error rates (BLER) for diagnostic reports.  
*
* Input:
*   chanSel                logical channel type (PDCH4)
*   TBF                    GPRS - pointer to temp block flow
*
* Output:
*   .bitErrors           updated number of bit errors 
*                            (number of bits checked is constant)         
*
******************************************************************************/
ProcResult accumBitBlockErrs(Uint8 rfCh, Uint8 timeSlot, t_chanSel chanSel, tbfStruct *TBF)
{  
   t_diagMsReport       *msReport;
   t_diagGprsMsReport   *gprsMsReport;
   GPRSCodecType        currentULCodeRate, currentDLCodeRate;
   msgStruc             ulPhDataMsg;
   Uint8                i;
   
   gprsMsReport = & g_diagData.gprsMsReport[rfCh][TBF->ulTbf.tfi];
   msReport =     & g_diagData.msReport[rfCh][timeSlot];
   
   TBF->ulTbf.channelCodec = g_ULCCHData.chCodec;
   
   /*------------------------------------------------------------------------
   * PDCH4, Group XIII, GPRS
   *------------------------------------------------------------------------*/
   if ( chanSel == PDCH4 ) 
   {  
     /*
	    *  CS-4 is uncoded, thus not able to calculate BER.
	    */
     if ( TBF->ulTbf.channelCodec != GPRS_CS_4 && g_ULCCHData.ulcchword0.bitval.bufferValid ) 
     {                 
       /*----------------------------------------------------------------
       * Compute and save BER in units of 0.01% (resolution=1/10,000)
       *----------------------------------------------------------------*/ 
        
       TBF->ulTbf.accum.bitErrors = g_ULCCHData.nErrs;
       TBF->ulTbf.accum.bitCount = NUM_CCH_CNV_ENC_OUT_BITS;
       TBF->ulTbf.sigqual.ber = (10000*(Uint32)TBF->ulTbf.accum.bitErrors) /
                                      TBF->ulTbf.accum.bitCount;
	      /*
	      * Smooth BER, alpha = 1/8
	      */
	      TBF->ulTbf.sigqual.berAve = expAverage(TBF->ulTbf.sigqual.ber, 
		                                        TBF->ulTbf.sigqual.berAve, 7, 3);

       /*
	      * Smooth BER, alpha = 1/4
	      */
	      TBF->dlTbf.pktDlAckNack.berAve = expAverage(TBF->dlTbf.pktDlAckNack.ber, 
		                                                 TBF->dlTbf.pktDlAckNack.berAve, 3, 2);
     }

     if ( g_ULCCHData.ulcchword0.bitval.bufferValid )
     {
         if ( g_ULCCHData.ulcchword0.bitval.fireStatus )
         {                                       
            TBF->ulTbf.accum.blockErrors += 1;
         }                                  
         /*
         * Smooth BLER, input is always either zero for a good RLC block, 10,000 for a bad one.
         *  alpha = 1/8
         */
         TBF->ulTbf.sigqual.blerAve = expAverage(10000*TBF->ulTbf.accum.blockErrors, 
		                                                 TBF->ulTbf.sigqual.blerAve, 7, 3);                    
     }

     /*-------------------------------------------------------------------
     * If MS Report enabled for this TS and logCh = PDCH4, send report for 
     *  Type Approval 
     *-------------------------------------------------------------------*/
     if ( g_diagData.msReportTsMask & (1<<timeSlot) )
     {
       for (i=0; i<TBF_NUMBER_MAX; i++)
       {
         if ( (g_tbfInfo[rfCh][i].ulTbf.state == CHAN_ACTIVE) && 
             ((g_tbfInfo[rfCh][i].ulTbf.timeSlotAllocation>>timeSlot) & 0x1) )
         {
           if ( g_ULCCHData.ulcchword0.bitval.bufferValid )
           {
             if ( g_ULCCHData.ulcchword0.bitval.fireStatus )
             {                                       
               msReport->cumFrameErrs += 1;
             }
           }
           msReport->cumFrames += 1;
         }
       }
     }
       
     /*----------------------------------------------------------
     * If GPRS MS Report enabled for TS, accumulate bit/block errors
     *----------------------------------------------------------*/
     if ( g_diagData.gprsMsReportTfiMask & (1L<<TBF->ulTbf.tfi) )
     {       
         /*----------------------------------------------------------
         * Accumulate bit and frame counts in gprsMsReport structure
         *----------------------------------------------------------*/
         gprsMsReport->rlcBlkCount    += 1;
         gprsMsReport->cumErrs        += (Uint32) (TBF->ulTbf.accum.bitErrors);
         gprsMsReport->cumBits        += (Uint32) (TBF->ulTbf.accum.bitCount);
         gprsMsReport->cumFrameErrs   += (Uint32) (TBF->ulTbf.accum.blockErrors);
         gprsMsReport->cumFrames      += 1;

         if ( TBF->ulTbf.channelCodec == GPRS_CS_4 ) 
         {
            gprsMsReport->cumErrs = 0;
         } 
     }

     /*----------------------------------------------------------
     * Clear BER and BLER errors, counts for next RLC block interval
     *----------------------------------------------------------*/
     TBF->ulTbf.accum.bitErrors = 0;   
     TBF->ulTbf.accum.blockErrors = 0;
     TBF->ulTbf.accum.bitCount = 0; 
   }     
}


/*****************************************************************************
*
* Function: expAverage()
*
* Description:
*   Generic IIR Smoothing Filter/Exponential averager
*
* Input: 
*   input_x, the new input value of type void to preserve generic nature of the 
*            function for global uses.
*   output_y_current, type void also, this is the current output which will be fed back to calculate 
*                     the new output.
*   smoothMult, numerator of the 1-alpha fraction.
*   smoothDiv,  denominator of the 1-alpha fraction. 
*
* Output:
*   (void) output_y, the new calculated output. 
*
*        this input value gets updated periodically via a closed-loop smoothing filter 
*        that is IIR with a difference equation of y[n] = (1-alpha)*y[n-1] + (alpha)*x[n].
*
*
                       alpha
             x[n]  o----->-----o----->-----o----->-----o  y[n]
                               |           |
                               |           |  -1
                               |           | z
                               |           |
                               o-----<-----o

                                 (1-alpha)
*                                            
*
******************************************************************************/
Uint16 expAverage(Uint16 input_x, Uint16 output_y_current, Uint8 smoothMult, Uint8 smoothDiv)
{
  Uint32 average;
  Uint16 output_y;
 									  
  average = ((smoothMult * output_y_current) + input_x) * 128;  // scale values to get around truncation problems.
  average >>= smoothDiv;                                     
  output_y = (( (Uint16) average + (64-1)) / 128);

  return(output_y);
}

 
/*****************************************************************************
*
* Function: rateTransition()
*
* Description:
*   Checks if a codec rate transition is needed to boost TBF throughput.
*
* Input: 
*   tbfStruct *TBF  - pointer to values for 
*                                           TBF->ulTbf.channelCodec    
*                                           TBF->ulTbf.sigqual.berAve  
*                                           TBF->ulTbf.sigqual.blerAve 
* Output:
*                                          
******************************************************************************/
void rateTransition(Uint8 rfCh, tbfStruct *TBF)
{
  Uint8 tsAllocCount, i, mask;
  GPRSCodecType  Cmd_UL_Rate, Cmd_DL_Rate;
  msgStruc       ulPhDataMsg;
  Uint8      dir, cval, iLevelTn, tempBuf[10];
  Uint16     berAve;

  switch ( TBF->ulTbf.channelCodec )
  {
    /* if construct order is important in each case due to hysteresis (upward or downward directions) 
	   *  pattern for transitions on the GSM 05.50 BLER vs. C/I curves.  
	   */
    case GPRS_CS_1:

      /* upward hysteresis */
      if (( TBF->ulTbf.sigqual.berAve < 500) && ( TBF->ulTbf.sigqual.blerAve < 600)) {
        Cmd_UL_Rate = GPRS_CS_2;
      }

	     /*
	     *  According to GSM 05.50, CS-2 and CS-3 exhibit very similar BLER characteristics vs. C/I.
	     *  Hence, transitions from CS-1 to CS-3, and vice versa are possible.
	     */
      if (( TBF->ulTbf.sigqual.berAve < 400) && ( TBF->ulTbf.sigqual.blerAve < 1000)) {
        Cmd_UL_Rate = GPRS_CS_3;
      }

      break;


    case GPRS_CS_2:

	     /* downward hysteresis */
      if (( TBF->ulTbf.sigqual.berAve > 1000) && ( TBF->ulTbf.sigqual.blerAve > 2000)) {
        Cmd_UL_Rate = GPRS_CS_1;
      }

      /* upward hysteresis */
      if (( TBF->ulTbf.sigqual.berAve < 400) && ( TBF->ulTbf.sigqual.blerAve < 1000)) {
        Cmd_UL_Rate = GPRS_CS_3;
      }

      break;


    case GPRS_CS_3:

      /* downward hysteresis */
      if (( TBF->ulTbf.sigqual.berAve > 800) && ( TBF->ulTbf.sigqual.blerAve > 2600)) {
        Cmd_UL_Rate = GPRS_CS_2;
      }

      /*
	     *  According to GSM 05.50, CS-2 and CS-3 exhibit very similar BLER characteristics vs. C/I.
	     *  Hence, transitions from CS-1 to CS-3, and vice versa are possible.
	     */
      if (( TBF->ulTbf.sigqual.berAve > 1000) && ( TBF->ulTbf.sigqual.blerAve > 2000)) {
        Cmd_UL_Rate = GPRS_CS_1;
      }

      /* upward hysteresis */
      if (( TBF->ulTbf.sigqual.berAve < 0.5) && ( TBF->ulTbf.sigqual.blerAve < 200)) {
        Cmd_UL_Rate = GPRS_CS_4;
      }
      break;


    case GPRS_CS_4:

      /* downward hysteresis */
      if ( TBF->ulTbf.sigqual.blerAve > 900 ) {
        Cmd_UL_Rate = GPRS_CS_3;
      }
      break;


    default:
      break;
  }

  

  /*  See if downlink needs a rate change as well */

  /*
  *  At least one timeslot must be assigned; maximum 4 TS per TBF, but only 1 for now.
  */
  mask = 0x01;  /* walking ones to isolate which timeslots are used by this TBF */

  
  /* MSbit represents timeslot 0, next MSbit is timeslot 1, etc... */
  for (i=0; i<NUM_TS_PER_RF; i++) {
    /*
    *  units relative to C value, each decrement/increment is 2dB
    */
    if (TBF->dlTbf.timeSlotAllocation & mask) {
      
      /* locals */
      cval     = TBF->dlTbf.pktDlAckNack.C_VALUE;
      iLevelTn = cval - 2*(TBF->dlTbf.pktDlAckNack.iLevelTN[i]);
      berAve = TBF->dlTbf.pktDlAckNack.berAve;

      switch ( TBF->dlTbf.channelCodec ) {
        /* if construct order is important in each case due to hysteresis (upward or downward directions) 
	       *  pattern for transitions on the GSM 05.50 BLER vs. C/I curves.  Threshold constants in dB.
	       */
        case GPRS_CS_1:
          if ( ( (cval - iLevelTn) > 13 ) && ( berAve < 500) )
          {
            Cmd_DL_Rate = GPRS_CS_2;
          }
          break;

        case GPRS_CS_2:
          if ( ( (cval - iLevelTn) > 13 ) && ( berAve < 400) )
          {
            Cmd_DL_Rate = GPRS_CS_3;
          }

          if ( ( (cval - iLevelTn) > 10 ) && ( berAve > 1000) )
          {
            Cmd_DL_Rate = GPRS_CS_1;
          }
          break;

        case GPRS_CS_3:
          if ( ( (cval - iLevelTn) > 10 ) && ( berAve > 800) )
          {
            Cmd_DL_Rate = GPRS_CS_2;
          }

          if ( ( (cval - iLevelTn) > 20 ) && ( berAve < 1) )
          {
            Cmd_DL_Rate = GPRS_CS_4;      
          } 
          break;

       case GPRS_CS_4:
          if ( (cval - iLevelTn) > 17 ) 
          {
            Cmd_DL_Rate = GPRS_CS_3;
          }
          break;
      }
    }
    mask = mask<<1;
  }


  if (Cmd_UL_Rate != TBF->ulTbf.channelCodec) 
  {
     dir = 0;

     /*
     *  load PDCH_CODE_RATE_REQ message onto outgoing queue.
     */
     ulPhDataMsg.msgSize = 4+5;   /* header + payload */
     ulPhDataMsg.function = PDCH_MANAGE_MSG;
     ulPhDataMsg.typeHiByte = PDCH_CODE_RATE_REQ >> 8;
     ulPhDataMsg.typeLoByte = PDCH_CODE_RATE_REQ;
     ulPhDataMsg.trx = rfCh; /* for now */
     ulPhDataMsg.params[0] = dir;
     ulPhDataMsg.params[1] = TBF->ulTbf.tfi; /* Uplink PDCH Group and TFI */
     ulPhDataMsg.params[2] = Cmd_UL_Rate;    /* New Commanded Uplink Channel Coding  */  
     // !! DEBUG.  RLC_MAC doesn't use this
     ulPhDataMsg.params[3] = (TBF->ulTbf.sigqual.blerAve >> 8) & 0xff;
     ulPhDataMsg.params[4] =  TBF->ulTbf.sigqual.blerAve;
     
     SendMsgHost_Que(& ulPhDataMsg); 
  }

  if (Cmd_DL_Rate != TBF->dlTbf.channelCodec) 
  {
    dir = 1;

     /*
     *  load PDCH_CODE_RATE_REQ message onto outgoing queue.
     */
     ulPhDataMsg.msgSize = 4+14;
     ulPhDataMsg.function = PDCH_MANAGE_MSG;
     ulPhDataMsg.typeHiByte = PDCH_CODE_RATE_REQ >> 8;
     ulPhDataMsg.typeLoByte = PDCH_CODE_RATE_REQ;
     ulPhDataMsg.trx = rfCh; /* for now */
     ulPhDataMsg.params[0] = dir;
     ulPhDataMsg.params[1] = TBF->dlTbf.tfi; /* Downlink PDCH Group and TFI */
     ulPhDataMsg.params[2] = Cmd_DL_Rate;   /* New Commanded Downlink Channel Coding  */  
     // !! DEBUG.  RLC_MAC doesn't use this
     ulPhDataMsg.params[3] = TBF->dlTbf.pktDlAckNack.iLevelTN[0];
     ulPhDataMsg.params[4] = TBF->dlTbf.pktDlAckNack.iLevelTN[1];
     ulPhDataMsg.params[5] = TBF->dlTbf.pktDlAckNack.iLevelTN[2];
     ulPhDataMsg.params[6] = TBF->dlTbf.pktDlAckNack.iLevelTN[3];
     ulPhDataMsg.params[7] = TBF->dlTbf.pktDlAckNack.iLevelTN[4];
     ulPhDataMsg.params[8] = TBF->dlTbf.pktDlAckNack.iLevelTN[5];
     ulPhDataMsg.params[9] = TBF->dlTbf.pktDlAckNack.iLevelTN[6];
     ulPhDataMsg.params[10] = TBF->dlTbf.pktDlAckNack.iLevelTN[7];

     ulPhDataMsg.params[11] = TBF->dlTbf.pktDlAckNack.C_VALUE;
     ulPhDataMsg.params[12] = TBF->dlTbf.pktDlAckNack.berAve >> 8; 
     ulPhDataMsg.params[13] = TBF->dlTbf.pktDlAckNack.berAve;

     SendMsgHost_Que(& ulPhDataMsg); 
  }

  g_rateChangeCheck = 0;
}













