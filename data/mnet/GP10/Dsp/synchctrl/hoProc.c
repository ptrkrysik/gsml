/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: hoProc.c
*
* Description:
*   This file contains functions that support handover 
*
* Public Functions:
*
* Private Functions:
*
******************************************************************************/ 
#include "stdlib.h" 
#include "gsmdata.h"
#include "oamdata.h"
#include "dsp/dsphmsg.h"
#include "dsprotyp.h"
#include "bbdata.h" 
#include "l1config.h" 
#include "diagdata.h"   
#include "agcdata.h"  

/*****************************************************************************
*
* Function: oamInit
*
* Decription:
*   Initializes all data structures related to handover processing. To be
*   called by dspinit().  
*
* Input:
*   None
*
* Output:
*   g_DynPwrOAMInfo     initialized structure
*   g_HoOAMInfo         initialized structure
*
******************************************************************************/
void oamInit(void)  
{
     Uint8 i, j, k;

     /* used for frequency of reporting power control */
     g_DynPwrOAMInfo.dynPwrSacchFrmCnt = 8;

     /* used for frequency of reporting co-channel interference */
     g_DynPwrOAMInfo.ulsigstren.Intave = 8;

     /* serving cell init */

     /* renamed 'hreqave' in gsm 05.08 */
     g_DynPwrOAMInfo.ulsigstren.numRxSigSamples = NUM_RX_SIGNAL_SAMPLES;
     g_DynPwrOAMInfo.ulsigqual.numRxSigSamples = NUM_RX_SIGNAL_SAMPLES;
      
     /* used for frequency of reporting power control */
     g_DynPwrOAMInfo.dynPwrSacchFrmCnt = NUM_RX_SIGNAL_SAMPLES; 
     
     g_DynPwrOAMInfo.ulsigstren.LRxPwrULH = UL_DL_HO_THRESH_DEF; /* cell threshold for handover to commence */
   
     g_DynPwrOAMInfo.ulsigqual.LRxQualULH = 4;    /* cell threshold for handover to commence */ 
  
     g_DynPwrOAMInfo.dlsigstrenSCell.LRxLevDLH = UL_DL_HO_THRESH_DEF; /* RXLEV units */
     g_DynPwrOAMInfo.dlsigstrenSCell.hoAveWindow = 8;

     g_DynPwrOAMInfo.dlsigqualSCell.LRxQualDLH = 4;    /* cell threshold for handover to commence */
     g_DynPwrOAMInfo.dlsigqualSCell.hoAveWindow = 8;

     g_DynPwrOAMInfo.sigstren.pxH = 2;  /* used to measure px out of nx failures on the threshold */
     g_DynPwrOAMInfo.sigstren.nxH = 4;  /*   before handover commences (P5,N5) */
     g_DynPwrOAMInfo.sigqual.pxH = 2;  /* used to measure px out of nx failures on the threshold */
     g_DynPwrOAMInfo.sigqual.nxH = 4;  /*   before handover commences  (P6,N6) */
    
     /* co-channel interference parameters */
     g_DynPwrOAMInfo.intsigstren.pxH = 2;  /* used to measure px out of nx failures on the threshold */
     g_DynPwrOAMInfo.intsigstren.nxH = 2;  /*   before handover commences (P7,N7) */
     g_DynPwrOAMInfo.ulsigstren.RxLevUL_IH = 45; /* RXLEV thresholds for intracell (interference) handover */
     g_DynPwrOAMInfo.dlsigstrenSCell.RxLevDL_IH = 45;  /* typical range is -85 to -40 dBm */

     /* neighbour cells init */
     for(i=0; i < NUM_NCELLS; i++){  
       g_HoOAMInfo.uniqueParms[i].rfCh = 0; 
       g_HoOAMInfo.uniqueParms[i].bsic = 0;
       g_HoOAMInfo.uniqueParms[i].hoMargin = HO_MARGIN_DEF;
       g_HoOAMInfo.uniqueParms[i].msTxPwrMax = MS_TXPWR_MAX_DEF;
       g_HoOAMInfo.uniqueParms[i].rxLevMin = RXLEV_MIN_DEF;
     }                        
     
    /* defaults if serving or neighbour cell's values are undefined */
       g_HoOAMInfo.commonParms.hoMarginDef   = HO_MARGIN_DEF;
       g_HoOAMInfo.commonParms.msTxPwrMaxDef = MS_TXPWR_MAX_DEF;
       g_HoOAMInfo.commonParms.rxLevMinDef   = RXLEV_MIN_DEF;      
       
    /* defaults if serving or neighbour cell's values are undefined */
       g_HoOAMInfo.serveParms.hoMargin   = HO_MARGIN_DEF;
       g_HoOAMInfo.serveParms.msTxPwrMax = MS_TXPWR_MAX_DEF;
       g_HoOAMInfo.serveParms.rxLevMin   = RXLEV_MIN_DEF;         

    /* for unit test */ 
     if(g_loopBackMode == LOCAL_LOOPBACK) {
       g_BBInfo[0].TNInfo[2].u.group1.tch.NCell[0].rxLev = 1;
       //g_BBInfo[0].TNInfo[2].u.group1.tch.NCell[1].rxLev = 2;
       //g_BBInfo[0].TNInfo[2].u.group1.tch.NCell[2].rxLev = -3;
       //g_BBInfo[0].TNInfo[2].u.group1.tch.NCell[3].rxLev = 4;
       //g_BBInfo[0].TNInfo[2].u.group1.tch.NCell[4].rxLev = 1;
       //g_BBInfo[0].TNInfo[2].u.group1.tch.NCell[5].rxLev = -6;
       //g_BBInfo[0].TNInfo[2].u.group1.sacch.dlsigstrenSCell.rxLevNew = 0;
     
       //g_HoOAMInfo.uniqueParms[0].rfCh = 20; 
       //g_HoOAMInfo.uniqueParms[0].bsic = 61;  
       //g_HoOAMInfo.uniqueParms[21].rfCh = 9; 
       //g_HoOAMInfo.uniqueParms[21].bsic = 17;
       //g_HoOAMInfo.uniqueParms[31].rfCh = 0; 
       //g_HoOAMInfo.uniqueParms[31].bsic = 4;  
       //g_HoOAMInfo.uniqueParms[4].rfCh = 24; 
       //g_HoOAMInfo.uniqueParms[4].bsic = 57; 
       g_HoOAMInfo.uniqueParms[4].rfCh = 0; 
       g_HoOAMInfo.uniqueParms[4].bsic = 55;
     }

   /* other misc. initializations related to gsm05.08 */

   /*-----------------------------------------------------------
   * Initialize parameters for co-channel interference processing
   *----------------------------------------------------------*/
   g_IntRprtCntr = 0;


   /*
   *  Set Defauld radio Link Lost threshold
   */
   g_HoOAMInfo.serveParms.radioLinkTimeout = RF_LINK_LOST_THRESHOLD;

  /*
  *   Conversion look up table for translating rxQual [0..7] to bit error rate (ber).
  *   The assumed values in the table are taken from gsm05.08 section 8.2.4.
  *
  * Index value is:
  *   rxQual        from sacchInfo.sigqual or dlsigqualSCell
  *        or 
  *   rxQualAve     averaged over OAM defined period (hreqave)
  *
  * Output: 
  *   ber           in units of 0.01% (resolution = 1/10000)
  *                  (e.g. 1810 is 18.10%).  This was deliberately done
  *                   for the sake of readibility.
  */
  g_rxQual2ber_LookUpTbl[0] = 14;
  g_rxQual2ber_LookUpTbl[1] = 28;
  g_rxQual2ber_LookUpTbl[2] = 57;
  g_rxQual2ber_LookUpTbl[3] = 113; 
  g_rxQual2ber_LookUpTbl[4] = 226; 
  g_rxQual2ber_LookUpTbl[5] = 453; 
  g_rxQual2ber_LookUpTbl[6] = 905; 
  g_rxQual2ber_LookUpTbl[7] = 1810;

  return;  
}  
     
/*****************************************************************************
*
* Function: handOverProc
*
* Decription:
*   Averages uplink power over mulitple SACCH intervals and computes
*   MS commanded power (dynamic power control) for Group 1 and Group 5.
*   Checks whether handover is required for Group 1 only. To be called
*   from grouptbl.c for Group 1. Currently called from ulSacchProc3 for
*   Group 5.  
*
* Input:
*   rfCh                TRX number (starting with zero)
*   timeSlot            GSM time slot
*
* Output:
*   g_DynPwrOAMInfo     initialized structure
*   g_HoOAMInfo         initialized structure
*
******************************************************************************/
void handOverProc(Uint8 rfCh, Uint8 timeSlot)
{
  /* to be called from grouptbl.c ...
     rfCh is NOT the ARFCN value, but rather the trx number 
     with range of 0..1. Use 0 for now.
  */
  
  t_TNInfo     *ts;
  t_TCHInfo    *tch;
  t_PNbufs     *PNbufs;
  Uint8        i, j, P_ulStren, P_dlStren, P_ulQual, P_dlQual, P_ulStrenInt, P_dlStrenInt;
  Int16        *pHead;
  Bool         hoThreshTestPassed; 
  t_chanState  chState; 
  msgStruc     hoMsg;  /* to assist DEBUG reports */
  Uint8        tempBuf[32];  /* !! DEBUG */            
  t_SACCHInfo  *sacchInfo;
  Int8         subChan; /* must be signed */
  Uint8        hoCause;
  Uint16       temp;                          

  ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
         
  switch(ts->groupNum)
  {
    case GSM_GROUP_1:   
            
      chState  = ts->u.group1.sacch.state;
      sacchInfo = & ts->u.group1.sacch;

      if(chState == CHAN_ACTIVE) {  

        tch = & g_BBInfo[rfCh].TNInfo[timeSlot].u.group1.tch; 
        pHead = & g_BBInfo[rfCh].TNInfo[timeSlot].u.group1.tch.hoCandidateBuf.rxLev[0];
        PNbufs = & g_PNbufs[timeSlot];   /* P out of N testing is timeslot dependent */

        /*-----------------------------------------------------------------
        * Save uplink RXQUAL and recompute average RXQUAL for HO algorithm
        *-----------------------------------------------------------------*/  
            averageQ(&sacchInfo->ulsigqual, 
                     &sacchInfo->dlsigqualSCell,
                     g_DynPwrOAMInfo.ulsigqual.numRxSigSamples, 
                     &sacchInfo->ulsigqual.ber,
                     TRUE);
        /*
        tempBuf[0]  =  sacchInfo->ulsigqual.rxQual[0]; 
        tempBuf[1]  =  sacchInfo->ulsigqual.rxQual[1];
        tempBuf[2]  =  sacchInfo->ulsigqual.rxQual[2];
        tempBuf[3]  =  sacchInfo->ulsigqual.rxQual[3];
        tempBuf[4]  =  sacchInfo->ulsigqual.rxQual[4];
        tempBuf[5]  =  sacchInfo->ulsigqual.rxQual[5];
        tempBuf[6]  =  sacchInfo->ulsigqual.rxQual[6];
        tempBuf[7]  =  sacchInfo->ulsigqual.rxQual[7];
        tempBuf[8]  =  sacchInfo->ulsigqual.rxQualAve;
        tempBuf[9]  =  sacchInfo->ulsigqual.berAve >> 8;
        tempBuf[10]  =  sacchInfo->ulsigqual.berAve;
        tempBuf[11]  =  sacchInfo->ulsigqual.count;
        
        sendDiagMsg(0x95,0,ts->tsNumber,12,tempBuf);
        */

        /*-----------------------------------------------------------
        * do power computations for uplink automatic gain control 
        *-----------------------------------------------------------*/
			accumPower(rfCh, timeSlot, ts->groupNum, SACCH_OR_IDLE, sacchInfo, NULL);

           
        /*-----------------------------------------------------------------
        * Check for ul gain diagnostic.  
        *----------------------------------------------------------------*/ 
        //tempBuf[0] = (ts->u.group1.sacch.ulsigstren.rxgain >> 8) & 0xff;
        //tempBuf[1] =  ts->u.group1.sacch.ulsigstren.rxgain & 0xff;      
        //sendDiagMsg(0x98, 0, ts->tsNumber, 2, tempBuf);

        /*-----------------------------------------------------------------
        * Save uplink RXLEV and recompute average RXLEV for HO algorithm
        *-----------------------------------------------------------------*/  
              averageS(&ts->u.group1.sacch.ulsigstren, 
                       &ts->u.group1.sacch.dlsigstrenSCell,
                       g_DynPwrOAMInfo.ulsigstren.numRxSigSamples, 
                       &ts->u.group1.sacch.MSPowerAccum,
                       TRUE);  
        /*  
        tempBuf[0]  =  sacchInfo->ulsigstrenSCell.rxLev[0] >> 8; 
        tempBuf[1]  =  sacchInfo->ulsigstrenSCell.rxLev[0];
        tempBuf[2]  =  sacchInfo->ulsigstrenSCell.rxLev[1] >> 8;
        tempBuf[3]  =  sacchInfo->ulsigstrenSCell.rxLev[1];
        tempBuf[4]  =  sacchInfo->ulsigstrenSCell.rxLev[2] >> 8;
        tempBuf[5]  =  sacchInfo->ulsigstrenSCell.rxLev[2];
        tempBuf[6]  =  sacchInfo->ulsigstrenSCell.rxLev[3] >> 8;
        tempBuf[7]  =  sacchInfo->ulsigstrenSCell.rxLev[3];
        tempBuf[8]  =  sacchInfo->ulsigstrenSCell.rxLev[4] >> 8; 
        tempBuf[9]  =  sacchInfo->ulsigstrenSCell.rxLev[4];
        tempBuf[10]  =  sacchInfo->ulsigstrenSCell.rxLev[5] >> 8;
        tempBuf[11]  =  sacchInfo->ulsigstrenSCell.rxLev[5]; 
        tempBuf[12]  =  sacchInfo->ulsigstrenSCell.rxLev[6] >> 8;
        tempBuf[13]  =  sacchInfo->ulsigstrenSCell.rxLev[6];
        tempBuf[14]  =  sacchInfo->ulsigstrenSCell.rxLev[7] >> 8;
        tempBuf[15]  =  sacchInfo->ulsigstrenSCell.rxLev[7];
        tempBuf[16]  =  sacchInfo->ulsigstrenSCell.rxLevAve >> 8;
        tempBuf[17]  =  sacchInfo->ulsigstrenSCell.rxLevAve;
        tempBuf[18]  =  sacchInfo->ulsigstrenSCell.count;

        sendDiagMsg(0x95,0,ts->tsNumber,19,tempBuf);
        */

        /*------------------------------------------------------------------
        * Save downlink RXQUAL and recompute average RXQUAL for HO algorithm
        *------------------------------------------------------------------*/
        temp = sacchInfo->dlsigqualSCell.rxQualNew;       /* cast to Uint16 */   
        averageQ(&sacchInfo->ulsigqual, 
                 &sacchInfo->dlsigqualSCell, 
                 g_DynPwrOAMInfo.dlsigqualSCell.hoAveWindow, 
                 &temp,
                 FALSE);
        sacchInfo->dlsigqualSCell.rxQualNew = (Uint8)temp; /* copy to Uint8 */   
        /*  
        tempBuf[0]  =  sacchInfo->dlsigqualSCell.rxQual[0]; 
        tempBuf[1]  =  sacchInfo->dlsigqualSCell.rxQual[1];
        tempBuf[2]  =  sacchInfo->dlsigqualSCell.rxQual[2];
        tempBuf[3]  =  sacchInfo->dlsigqualSCell.rxQual[3];
        tempBuf[4]  =  sacchInfo->dlsigqualSCell.rxQual[4];
        tempBuf[5]  =  sacchInfo->dlsigqualSCell.rxQual[5];
        tempBuf[6]  =  sacchInfo->dlsigqualSCell.rxQual[6];
        tempBuf[7]  =  sacchInfo->dlsigqualSCell.rxQual[7];
        tempBuf[8]  =  sacchInfo->dlsigqualSCell.rxQualAve;
        tempBuf[9]  =  sacchInfo->dlsigqualSCell.berAve >> 8;
        tempBuf[10]  =  sacchInfo->dlsigqualSCell.berAve;
        tempBuf[11]  =  sacchInfo->dlsigqualSCell.count;
        tempBuf[12]  =  sacchInfo->dlsigqualSCell.rxQualNew;

        sendDiagMsg(0x95,0,ts->tsNumber,13,tempBuf);  
        */


        /*-----------------------------------------------------------------
        * Save downlink RXLEV and recompute average RXLEV for HO algorithm
        *-----------------------------------------------------------------*/  
        averageS(&ts->u.group1.sacch.ulsigstren, 
                 &ts->u.group1.sacch.dlsigstrenSCell,
                 g_DynPwrOAMInfo.dlsigstrenSCell.hoAveWindow, 
                 &ts->u.group1.sacch.dlsigstrenSCell.rxLevNew,
                 FALSE); 
        /*
        tempBuf[0]  =  sacchInfo->dlsigstrenSCell.rxLev[0] >> 8; 
        tempBuf[1]  =  sacchInfo->dlsigstrenSCell.rxLev[0];
        tempBuf[2]  =  sacchInfo->dlsigstrenSCell.rxLev[1] >> 8;
        tempBuf[3]  =  sacchInfo->dlsigstrenSCell.rxLev[1];
        tempBuf[4]  =  sacchInfo->dlsigstrenSCell.rxLev[2] >> 8;
        tempBuf[5]  =  sacchInfo->dlsigstrenSCell.rxLev[2];
        tempBuf[6]  =  sacchInfo->dlsigstrenSCell.rxLev[3] >> 8;
        tempBuf[7]  =  sacchInfo->dlsigstrenSCell.rxLev[3];
        tempBuf[8]  =  sacchInfo->dlsigstrenSCell.rxLev[4] >> 8; 
        tempBuf[9]  =  sacchInfo->dlsigstrenSCell.rxLev[4];
        tempBuf[10]  =  sacchInfo->dlsigstrenSCell.rxLev[5] >> 8;
        tempBuf[11]  =  sacchInfo->dlsigstrenSCell.rxLev[5]; 
        tempBuf[12]  =  sacchInfo->dlsigstrenSCell.rxLev[6] >> 8;
        tempBuf[13]  =  sacchInfo->dlsigstrenSCell.rxLev[6];
        tempBuf[14]  =  sacchInfo->dlsigstrenSCell.rxLev[7] >> 8;
        tempBuf[15]  =  sacchInfo->dlsigstrenSCell.rxLev[7];
        tempBuf[16]  =  sacchInfo->dlsigstrenSCell.rxLevAve >> 8;
        tempBuf[17]  =  sacchInfo->dlsigstrenSCell.rxLevAve;
        tempBuf[18]  =  sacchInfo->dlsigstrenSCell.count;

        sendDiagMsg(0x95,0,ts->tsNumber,19,tempBuf);
        */

        if ( g_DPC_ON && !ts->u.group1.sacch.dpcHoldFlag )
        {
          if ( ts->u.group1.sacch.averagingHoldCounter > g_DynPwrOAMInfo.ulsigstren.numRxSigSamples )
          {
            ts->u.group1.sacch.averagingHoldCounter = g_DynPwrOAMInfo.ulsigstren.numRxSigSamples;
 
            /* increase/decrease MS power */
            uplink_dynamic_power_control(& g_BBInfo[rfCh].TNInfo[timeSlot], 0);   
          }
          else {
            ts->u.group1.sacch.averagingHoldCounter += 1;
          }
        }

         /*-----------------------------------------------------------
         *         HANDOVER PROCESSING
         *-----------------------------------------------------------*/
        if( (++tch->hoCandidateBuf.hoReportCnt > HO_REPORT_COUNTER)  && 
            ( ts->u.group1.sacch.averagingHoldCounter > g_DynPwrOAMInfo.ulsigstren.numRxSigSamples ) ) {    
            /*
            *  Do not reset guard period for Ho request
            */
            tch->hoCandidateBuf.hoReportCnt == HO_REPORT_COUNTER;
           
            /* ---------------------------------------------------------------- */
            /* The following 4 tests are to determine if intercell HO is needed */  
            /* ---------------------------------------------------------------- */
            /*
            * Test UL power (from MS) thresholds for possible handover.
            * Also require MS to be transmitting the max power we allow. 
            */
            if (  ts->u.group1.sacch.ulsigstren.rxLevAve < 
                     g_DynPwrOAMInfo.ulsigstren.LRxPwrULH  &&
                  ts->u.group1.sacch.dlsigstrenSCell.MSPwrSetting <= 
                     g_HoOAMInfo.serveParms.msTxPwrMax &&
                  ts->u.group1.sacch.powerLevel <= 
                     g_HoOAMInfo.serveParms.msTxPwrMax ) {
               hoThreshTestPassed = TRUE;
            }
            else { 
               hoThreshTestPassed = FALSE;
            }
            
            P_ulStren = hoPNtest(&g_PNbufs[timeSlot].hoUL.sigstren, 
                        g_DynPwrOAMInfo.sigstren.nxH, 
                        hoThreshTestPassed);

            /*
            * Test DL power (from Vipercell) thresholds for possible handover
            */
            if((Int16) ts->u.group1.sacch.dlsigstrenSCell.rxLevAve < 
               g_DynPwrOAMInfo.dlsigstrenSCell.LRxLevDLH) {
               hoThreshTestPassed = TRUE;
            }
            else { 
               hoThreshTestPassed = FALSE;
            }

            P_dlStren = hoPNtest(&g_PNbufs[timeSlot].hoDL.sigstrenSCell, 
                        g_DynPwrOAMInfo.sigstren.nxH, 
                        hoThreshTestPassed); 

            /*
            * Test UL quality (from MS) thresholds for possible handover
            */
            if(ts->u.group1.sacch.ulsigqual.rxQualAve > 
               g_DynPwrOAMInfo.ulsigqual.LRxQualULH) {
               hoThreshTestPassed = TRUE;
            }
            else { 
               hoThreshTestPassed = FALSE;
            }

            P_ulQual = hoPNtest(&g_PNbufs[timeSlot].hoUL.sigqual, 
                        g_DynPwrOAMInfo.sigqual.nxH, 
                        hoThreshTestPassed);

            /*
            * Test DL quality (from Vipercell) thresholds for possible handover 
            */
            if((Int16) ts->u.group1.sacch.dlsigqualSCell.rxQualAve > 
               g_DynPwrOAMInfo.dlsigqualSCell.LRxQualDLH) {
               hoThreshTestPassed = TRUE;
            }
            else { 
               hoThreshTestPassed = FALSE;
            }

            P_dlQual = hoPNtest(&g_PNbufs[timeSlot].hoDL.sigqualSCell, 
                        g_DynPwrOAMInfo.sigqual.nxH, 
                        hoThreshTestPassed);

            /* ---------------------------------------------------------------- */
            /* The following 2 tests are to determine if intracell HO is needed */  
            /* ---------------------------------------------------------------- */
         
            /* test UL power (from MS) interference thresholds for possible handover */
            if(ts->u.group1.sacch.ulsigstren.rxLevAve < 
               g_DynPwrOAMInfo.ulsigstren.RxLevUL_IH) {
              hoThreshTestPassed = TRUE;
            }
            else { 
              hoThreshTestPassed = FALSE;
            }

            P_ulStrenInt = hoPNtest(&g_PNbufs[timeSlot].hoUL.intsigstren, 
                       g_DynPwrOAMInfo.intsigstren.nxH, 
                       hoThreshTestPassed);

            /* test DL power (from Vipercell) interference thresholds for possible handover */
            if((Int16) ts->u.group1.sacch.dlsigstrenSCell.rxLevAve < 
              g_DynPwrOAMInfo.dlsigstrenSCell.RxLevDL_IH) {
              hoThreshTestPassed = TRUE;
            }
            else { 
              hoThreshTestPassed = FALSE;
            }

            P_dlStrenInt = hoPNtest(&g_PNbufs[timeSlot].hoDL.intsigstrenSCell, 
                        g_DynPwrOAMInfo.intsigstren.nxH, 
                        hoThreshTestPassed); 

            /*-------------------------------------------------------------------
            *  Set Bits 0-5 of the Handover Cause bitmap. Handover is considered
            *  IMPERATIVE if one or more of the six causes is met.
            *
            *   a) RXLEV 
            *      The power level (UL and/or DL) is below the threshold 
            *      despite power control (MS or/and BSS have reached the
            *      maximum allowed power).
            *
            *   b) RXQUAL 
            *      The quality of the link (UL and/or DL) is below the
            *      quality threshold and the power level (UL and/or DL) 
            *      is below the intracell power threshold.
            *
            *   c) RXQUAL_IH 
            *      The quality of the link (UL and/or DL) is below the
            *      quality threshold while the power level (UL and/or DL)
            *      is above the intracell power threshold. These conditions,
            *      if verified at the same time, indicate a high probability  
            *      of co-channel interference.
            *-------------------------------------------------------------------*/  
            hoCause =   ((P_dlStren >= g_DynPwrOAMInfo.sigstren.pxH) * DL_RXLEV)  | 
                        ((P_ulStren >= g_DynPwrOAMInfo.sigstren.pxH) * UL_RXLEV)  |
                        ((P_dlQual  >= g_DynPwrOAMInfo.sigqual.pxH)  * DL_RXQUAL) | 
                        ((P_ulQual  >= g_DynPwrOAMInfo.sigqual.pxH)  * UL_RXQUAL) |
                        ((P_dlStrenInt < g_DynPwrOAMInfo.intsigstren.pxH &&
                          P_dlQual >= g_DynPwrOAMInfo.sigqual.pxH) * DL_RXQUAL_IH)|
                        ((P_ulStrenInt < g_DynPwrOAMInfo.intsigstren.pxH &&
                          P_ulQual >= g_DynPwrOAMInfo.sigqual.pxH) * UL_RXQUAL_IH);
                                
            /*-------------------------------------------------------------------
            * If one or more HO cause triggered, send HO Report diagnostic
            *-------------------------------------------------------------------*/  
            if ( (g_diagData.hoReportTsMask & (1<<timeSlot)) && hoCause )
            {
               tempBuf[0] = P_ulStren; 
               tempBuf[1] = g_DynPwrOAMInfo.sigstren.pxH; 
               tempBuf[2] =  ts->u.group1.sacch.ulsigstren.rxLevAve;
               tempBuf[3] = P_dlStren; 
               tempBuf[4] = g_DynPwrOAMInfo.sigstren.pxH; 
               tempBuf[5] =  ts->u.group1.sacch.dlsigstrenSCell.rxLevAve; 

               tempBuf[6] = P_ulQual; 
               tempBuf[7] = g_DynPwrOAMInfo.sigqual.pxH; 
               tempBuf[8] =  ts->u.group1.sacch.ulsigqual.rxQualAve;
               tempBuf[9] = P_dlQual; 
               tempBuf[10] = g_DynPwrOAMInfo.sigqual.pxH; 
               tempBuf[11] =  ts->u.group1.sacch.dlsigqualSCell.rxQualAve;       
            
               tempBuf[12] = P_ulStrenInt; 
               tempBuf[13] = g_DynPwrOAMInfo.intsigstren.pxH; 
               tempBuf[14] = P_dlStrenInt; 
               tempBuf[15] = g_DynPwrOAMInfo.intsigstren.pxH; 

               sendDiagMsg(HO_PN_REPORT,0,ts->tsNumber,16,tempBuf);
            }

            /*-------------------------------------------------------------------
            * If one or more HO causes triggered, look for a handover candidate.
            * If multiple causes triggered, then the order of priority from 
            * highest to lowest shall be: RXQUAL (Intracell), RXLEV (Intercell).
            *-------------------------------------------------------------------*/  
            if ( hoCause )
            {
              /* compare with up to 6 neighbor cells sent by MS in
               * order to find suitable candidates for handover.
               */ 
              hoCompare2NCells(rfCh, timeSlot, hoCause);                
                 
              /* sort candidates (if any) from best to worst */
              if(tch->hoCandidateBuf.size != 0) {      
                handOverProcSort(rfCh, timeSlot);
  
               /* call the signalling function which sends candidates 
                * (in report buffer) back to layer 3 RRM.      
                */ 
                sendAsyncHoMsg(rfCh, timeSlot, hoCause);   /* DCCH message */

               /*
                *  Reset guard period to not repeat the Ho request
                */
                tch->hoCandidateBuf.hoReportCnt = 0;
              }
              /* else no report is sent if there are no candidates */
            } /* test for P */
            else {
              /* else do nothing except update report buffer (without 
                 sorting,reporting) by copying the current (upto 6) values from
                 the uplink MS measurement report.
              */   
              j=0;
              for(i=0; i < MAX_NUM_NCELL_CANDIDATES; i++) {                                                                  
                if(tch->NCell[i].candidate == TRUE) {
                  *(pHead+j) = tch->NCell[i].rxLev;
                  j += 1;
                }  
              }

              /* function does not send report to layer 3 code */
              UCopy2SendBuf(rfCh, timeSlot);
            
            }  /* test for P */       
        }      /* test for counter */        
      }        /* test for channel active */

      else  /* non active channel;
               Collect power measurements for interference calculations */
      {
        accumPower(rfCh, timeSlot, ts->groupNum, SACCH_OR_IDLE, sacchInfo, NULL);
        
        /*-----------------------------------------------------------
        * average RXLEV for interference reporting
        * -----------------------------------------------------------*/
        averageS(&ts->u.group1.sacch.ulsigstren, 
                 &ts->u.group1.sacch.dlsigstrenSCell,
                 g_DynPwrOAMInfo.ulsigstren.Intave, 
                 &ts->u.group1.sacch.MSPowerAccum,
                 TRUE);  
      }   
        
      /*-------------------------------------------------------------
      * Send interference report to host every Intave SACCH multiframes 
      *------------------------------------------------------------*/
      if (g_IntRprtCntr >= g_DynPwrOAMInfo.ulsigstren.Intave)
      {   
        /* reset counter */
        g_IntRprtCntr = 0;

        for (i=0; i<NUM_RFS_PER_DSP; i++) {
          for (j=0; j < NUM_TS_PER_RF; j++) {
            chState  = g_BBInfo[i].TNInfo[j].u.group1.sacch.state;

            if(chState == CHAN_ACTIVE) {
              tempBuf[j] = 255;  /* Set to the max. 8-bit value to signify active chnl. */
            }
            else {
              tempBuf[j] = g_BBInfo[i].TNInfo[j].u.group1.sacch.ulsigstren.rxLevAve;
            }
          }
        }
        
        /* MCH CAN TURN THIS REPORT ON/OFF */
        sendDiagMsg(IF_REPORT, rfCh, 0, 8, tempBuf);      
      }

      break;

    case GSM_GROUP_5: /* Do nothing for Group 5. No handover yet. */ 
    
      break;
      
  }  /* switch construct */
 
 return;
}
     
/*****************************************************************************
*
* Function: hoPNtest
*
* Decription:
*   Maintains most recent N pass/fail outcomes  
*
* Input:
*   nxBuffer         ptr to length-N array of outcomes
*   nx               number of outcomes to maintain, N
*   pass             indicates whether newest outcome passes (1) or not (0)
*
* Output:
*   return value     number of passing values in most recent N outcomes
*
******************************************************************************/
char hoPNtest(t_nxBuf *nxBuffer, Uint8 nx, Uint8 pass)
{
  /* This function was deliberately made to be generic 
     in order for all px-nx buffers to use.
  */
  /* pass = 1, fail = 0 */
  
  Uint8     *pi, *pfl, *pflsum; 
  
  pi     = & nxBuffer->index;
  pfl    = & nxBuffer->flag[*pi];
  pflsum = & nxBuffer->flagsum;          
   
  /* add newest sample, subtract oldest from power */
  *pflsum += pass - *pfl;
  *pfl = pass;
        
  if (*pi == nx-1) { 
        /* circular buffer's last address..rollover */
    *pi = 0;
      }                       
      else {
            /* point to the next entry in the flag buffer */
            *pi += 1;                        
      }
  
  return(*pflsum);
}

/*****************************************************************************
*
* Function: hoCompare2NCells
*
* Decription:
*
* Input:
*
* Output:
*
******************************************************************************/
void hoCompare2NCells(Uint8 rfCh, Uint8 timeSlot, Uint8 hoCause)
{
  t_TNInfo *ts;
  t_TCHInfo *tch;
  Uint8 i, j;       
  Uint8 matchNCell, matchNCellNum, msTxPwrMax;
  Int16 rxLvlMin, hoMargin;
  Uint8 tempBuf[20];
  
  ts = & g_BBInfo[rfCh].TNInfo[timeSlot]; 


  switch(ts->groupNum)
  {
    case GSM_GROUP_1:
      tch = & g_BBInfo[rfCh].TNInfo[timeSlot].u.group1.tch;

      /* avoid accumulating older values */      
      tch->hoCandidateBuf.size = 0;
                        
      /* Check the neighbor cells reported this sacch period */ 
      for(i=0; i<ts->u.group1.sacch.noNCellM; i++) {
        
        matchNCell = FALSE;
        
        for(j=0; j<NUM_NCELLS; j++)
        {
           /*
           * search an N Cell matches with candidate
           */
           if(tch->NCell[i].rfCh == g_HoOAMInfo.uniqueParms[j].rfCh && 
              tch->NCell[i].bsic == g_HoOAMInfo.uniqueParms[j].bsic)
           {
              /*
              * found a matching neighbor Cell set
              */ 
              matchNCell = TRUE;
              matchNCellNum = j;
              break; 
           }
        }                   
              
        if(!matchNCell)
        {     
           /*
           *  compare with a default value if no match N Cell is found
           */        
           hoMargin = g_HoOAMInfo.commonParms.hoMarginDef;
           rxLvlMin = g_HoOAMInfo.commonParms.rxLevMinDef; 
           msTxPwrMax = g_HoOAMInfo.commonParms.msTxPwrMaxDef;
           
           tempBuf[0] =  tch->NCell[i].bsic;
           tempBuf[1] =  tch->NCell[i].rfCh >> 8;
           tempBuf[2] =  tch->NCell[i].rfCh;           
           /*
           *  The wanted Cell is not in N Cell table
           */           
           ReportError(0x99,rfCh,timeSlot,3,tempBuf);

        }
        else   
        {     
           /*
           *  compare with values from NCell Table if matched
           */
           hoMargin = g_HoOAMInfo.uniqueParms[matchNCellNum].hoMargin;
           rxLvlMin = g_HoOAMInfo.uniqueParms[matchNCellNum].rxLevMin;
           msTxPwrMax = g_HoOAMInfo.uniqueParms[matchNCellNum].msTxPwrMax;
        }                                                            
        
        /* 
        *  Always require a matching neighbor cell and require its DL power to
        *  meet a minimum level. If HO reason is strictly downlink power, also
        *  require the neighbor's DL power to meet the HO power margin.
        */
        if ( matchNCell &&
             tch->NCell[i].rxLev > rxLvlMin &&
             ( hoCause != DL_RXLEV || 
               tch->NCell[i].rxLev > (ts->u.group1.sacch.dlsigstrenSCell.rxLevNew + 
                                      hoMargin + 
                                      msTxPwrMax - 
                                      g_HoOAMInfo.serveParms.msTxPwrMax) ) ) {
           tch->NCell[i].candidate = TRUE;
           tch->hoCandidateBuf.size += 1; /* one buffer per timeslot */
        }
        else {
           tch->NCell[i].candidate = FALSE;
        }               
        
        tempBuf[0] =  ts->u.group1.sacch.dlsigstrenSCell.rxLevNew;
        tempBuf[1] =  tch->NCell[i].rxLev;
        tempBuf[2] =  hoMargin;
        tempBuf[3] =  rxLvlMin; 
        tempBuf[4] =  ts->u.group1.sacch.noNCellM; 
        tempBuf[5] =  i;                        
        
        sendDiagMsg(0x99,rfCh,timeSlot,6,tempBuf);
        
      }
      break;

    case GSM_GROUP_5:
      break;
  }
  return;
}

/*****************************************************************************
*
* Function: handOverProcSort
*
* Decription:
*
* Input:
*
* Output:
*
******************************************************************************/
void handOverProcSort(Uint8 rfCh, Uint8 timeSlot)
{

 /* sort candidates (given by compare function) from
   most(head) to least(tail) favorable.  
 */ 
  t_TNInfo *ts;
  t_TCHInfo *tch;
  Uint8 i, j, k;
  Uint8 count;
  Int16 *pTail, *pHead, *pTop;           
  ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
  
  
  switch(ts->groupNum)
  {
    case GSM_GROUP_1:
      /* (re)set pointer values for convenience */
      tch = & g_BBInfo[rfCh].TNInfo[timeSlot].u.group1.tch;
      pHead = & g_BBInfo[rfCh].TNInfo[timeSlot].u.group1.tch.hoCandidateBuf.rxLev[0]; 
                     
      count = 0;

      for(i=0; i<MAX_NUM_NCELL_CANDIDATES; i++) {

        if(tch->NCell[i].candidate == TRUE) {
                                
          if((count > 0) && (count <= MAX_NUM_NCELL_CANDIDATES)) {
            for(j=0; j<count; j++) {                   

              if(tch->NCell[i].rxLev  < *pHead) {               
                if(pHead == pTail) {
                  /* candidate is least favorable for 
                     handover.
                   */               
                  pTail += 1;
                  *pTail = tch->NCell[i].rxLev;
                  pHead = pTop; /* reset head back to top 
                                     of report buffer. */
                }
                else {
                   /* use head as pointer to the next array 
                      element.
                    */                  
                    pHead += 1;
                }
              }
              else {
                /* insert candidate at head or somewhere 
                   between head and tail. push all lower values down.
                 */                                
                for(k = count; k > j; k--) {
                  *(pTop+k*sizeof(char)) = *(pTop+k-1*sizeof(char));
                } 
                
                pTail = pTop+count*sizeof(char);                                                    
                *pHead = tch->NCell[i].rxLev;
                pHead = pTop;   /* reset head back to top
                                        of report buffer. */
                j = count;  /* to break out of for loop  */
              } 

            }  /* for loop (count) */
          } /* non-zero count */
                 
          /* first candidate is always the head. */
          else if(count == 0) {                       
            *pHead = tch->NCell[i].rxLev;
            pTop  = pHead;  /* Always points to buffer top */
            pTail = pHead;  /* Initially only. */
          }

          count += 1;        

        } /* test fails, if no candidates exist. */
      }   /* End for loop.  */     
      break;
     
    case GSM_GROUP_5:
      /* for future usage possibly. */
      break;
  }  /* switch */
  return;
} 

/*****************************************************************************
*
* Function: sendAsyncHoMsg
*
* Decription:
*
* Input:
*
* Output:
*
******************************************************************************/
void sendAsyncHoMsg(Uint8 rfCh, Uint8 timeSlot, Uint8 hoCause)
{
  /* copy oam indices (representing arfcns) and bsics
   * to report buffer in sendAsyncHoMsg() and send it!
   */
  t_TNInfo *ts;
  t_TCHInfo *tch;
  msgStruc hoMsg;
  Uint8 i,j,k;     
  Bool skip[MAX_NUM_NCELL_CANDIDATES];   /* flag to skip candidate, since it has already been chosen. */    
   
  ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
  
  switch(ts->groupNum)
  {
    case GSM_GROUP_1:
      /* (re)set pointer values for convenience */
      tch = & g_BBInfo[rfCh].TNInfo[timeSlot].u.group1.tch;

      /*
       *  HO request in preference of ordering of candidate
       */
      hoMsg.msgSize = 19;
      hoMsg.function =  DCCH_MANAGE_MSG;
      hoMsg.trx = rfCh;
   
      // !! a one octet "cause" field wil be appended immediately after the message type.
      //  when this is added, remove 0x0038 ID representing INTRACELL HO
      if ( (hoCause & DL_RXLEV)  || (hoCause & UL_RXLEV) ||
           (hoCause & DL_RXQUAL) || (hoCause & UL_RXQUAL) ) {
        hoMsg.typeLoByte = DCH_ASYNC_HO_REQUIRED_IND;
        hoMsg.typeHiByte = DCH_ASYNC_HO_REQUIRED_IND>>8;            
      }
      else {
        hoMsg.typeLoByte = 0x3800;
        hoMsg.typeHiByte = 0x3800>>8;            
      }
          
      hoMsg.params[0] = timeSlot| 0x8;  
      hoMsg.params[1] = 0;
      hoMsg.params[2] = 0; 
      hoMsg.params[3] = tch->hoCandidateBuf.size;  /* number of candidates */

      /* refresh report buffer */
	  for(j=0; j < MAX_NUM_NCELL_CANDIDATES; j++) {
        hoMsg.params[4+(j*2)] = 0;
        hoMsg.params[5+(j*2)] = 0;
        skip[j] = 0;
      }

      /* copy to send buffer */ 
      for(i=0; i < tch->hoCandidateBuf.size; i++) {
        for(j=0; j < MAX_NUM_NCELL_CANDIDATES; j++) {
          for(k=0; k < MAX_NUM_NCELL_CANDIDATES; k++) {	
		    if((skip[k] == 1) || (ts->u.group1.tch.NCell[k].rfCh == 0) 
				              || ((ts->u.group1.tch.NCell[k].oamIdx == 0) 
							  &&  (ts->u.group1.tch.NCell[k].bsic == 0)) ) {
			  j += 1;  /* either the neighbor is chosen already, or disabled */
			}
			else
              break;
		  }
         
          if(tch->hoCandidateBuf.rxLev[i] == tch->NCell[j].rxLev) {
            hoMsg.params[4+(i*2)] = ts->u.group1.tch.NCell[j].oamIdx;  /* Frequency of candidate i,  5 bits */
            hoMsg.params[5+(i*2)] = ts->u.group1.tch.NCell[j].bsic;    /* BSIC of candidate i, 6 bits */
            skip[j] = 1;
	        break;
		  }       
        }
      }

      
      /* if HO Report enabled for this TS, send diagnostic msg */
      if ( g_diagData.hoReportTsMask & (1<<timeSlot) )
      {
         sendDiagMsg(HO_CAND_REPORT,0,ts->tsNumber,(hoMsg.params[3]*2)+1,&hoMsg.params[3]);
      }

      SendMsgHost_Que(& hoMsg);

      break;  

    case GSM_GROUP_5:
      break;
  }
  return;
}

/*****************************************************************************
*
* Function: UCopy2SendBuf
*
* Decription:
*
* Input:
*
* Output:
*
******************************************************************************/
void UCopy2SendBuf(Uint8 rfCh, Uint8 timeSlot)
{
  /* copy oam indices (representing arfcns) and bsics
   * to report buffer in sendAsyncHoMsg() BUT do not send. 
   */
  t_TNInfo *ts;
  t_TCHInfo *tch;
  msgStruc hoMsg;
  Uint8 i,j;     
  ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
  
  switch(ts->groupNum)
  {
    case GSM_GROUP_1:
      /* (re)set pointer values for convenience */
      tch = & g_BBInfo[rfCh].TNInfo[timeSlot].u.group1.tch;

      hoMsg.params[3] = tch->hoCandidateBuf.size;  /* number of candidates */

      /* copy to send buffer with no sorting */ 
      for(i=0; i < tch->hoCandidateBuf.size; i++) { 
        hoMsg.params[4+(i*2)] = ts->u.group1.tch.NCell[i].oamIdx;  /* Frequency of candidate i,  5 bits */
        hoMsg.params[5+(i*2)] = ts->u.group1.tch.NCell[i].bsic;    /* BSIC of candidate i, 6 bits */       
      }
      break;  

    case GSM_GROUP_5:
      break;
  }
  return;
}


