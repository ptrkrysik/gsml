/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: groupini.c
*
* Description:
*   This file contains functions that initialize GSM Group and TS structures
*
* Public Functions:
*   tsGroupInit, tsConfigure  
*
* Private Functions:
*   None
*
******************************************************************************/
#include "stdlib.h" 
#include "string.h" 
#include "gsmdata.h"   
#include "bbdata.h"  
#include "diagdata.h" 
#include "dsprotyp.h"
#include "agcdata.h"
#include "intr.h"

extern t_DLBBData g_DLBBData;
                                        
/*****************************************************************************
*
* Function: tsGroupInit
*
* Description:
*   Initializes demod and BB structure variables
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/            
ProcResult tsGroupInit()
{
   Int16 timeSlot;
   Int16 rfCh;
   Int16 i, k;

   // g_dyn_prach_blk = 20;
   
   g_frameCounts.ulFNOffset = 0;
   /*
   * When in Normal mode,   TNNumInit = 5;   
   * When in baseband loopback mode,   TNNumInit = 2; 
   * When in local loopback mode,   TNNumInit = 0; 
   */                        
   if (g_loopBackMode == BASEBAND_LOOPBACK)
      g_frameCounts.TNNumInit = 2;
   else if(g_loopBackMode == LOCAL_LOOPBACK)      
      g_frameCounts.TNNumInit = 0;
    else /*(g_loopBackMode == NO_LOOPBACK)*/
      g_frameCounts.TNNumInit = 6;     
      
   g_burstProc.dlTN = 0;
   g_burstProc.ulTN = (g_burstProc.dlTN + 8-g_frameCounts.TNNumInit) & 0x07 ;    

   chanCodecInit();
    
    /* Init the demod constants      */    
    /*initDemodConsts();*/
                   
   group0TableInit();    
   group4TableInit();                           
   dlGroupVTableInit();
   ulGroupVTableInit();
   dlGroupVIITableInit();
   ulGroupVIITableInit();
   sacchIdleTableInit();
   ulGroupITableInit();
   dlGroupITableInit();
   ulGroupXITableInit();
   dlGroupXITableInit();
   ulGroupXIIITableInit();
   dlGroupXIIITableInit();
   dlPtcchTableInit();
   ulPtcchTableInit();
   dlPbcchTableInit();
   dlPpchTableInit();

   /* Initialize the group 11 radio block defs */

   for ( i = 0; i < MAX_RADIO_BLOCKS; i++){
           G11_dlRadioBlkType[i] = D_PDCH_T;
           G11_ulRadioBlkType[i] = U_PDCH_T;
   }

   /*  Initialize the parameter in the group 11 structure */

   g11Params.bsPBcchBlks   = 1;
   g11Params.bsPagBlksRes  = 0;  
   g11Params.bsPrachBlks   = 12;
   g_PrachBits   = 8;
   g11Params.newAssignment = TRUE;  // testing purposes
   g11Params.psi1_repeat   = 7;
   g11Params.psi1_cycle    = 7*52;   
   
   
   for(rfCh=0; rfCh < NUM_RFS_PER_DSP; rfCh++)
   {   
      /*
      * clear BB process structure
      */                   
      memset (& g_BBInfo[rfCh], 0, sizeof(t_BBInfo)); 
      
      g_BBInfo[rfCh].frame.offset = 0;              
                
      ulBurstBufPtr[0] =  (t_ULComplexSamp *) & g_ULBBData.burst04;     
      ulBurstBufPtr[1] =  (t_ULComplexSamp *) & g_ULBBData.burst15;     
      ulBurstBufPtr[2] =  (t_ULComplexSamp *) & g_ULBBData.burst26;
      ulBurstBufPtr[3] =  (t_ULComplexSamp *) & g_ULBBData.burst37;
      
      g_BBInfo[rfCh].ulBurst = 0;        
      g_BBInfo[rfCh].ulBBSmpl =  (t_ULComplexSamp *) & g_ULBBData.burst04;
       
      dlBurstBufPtr[0] = (t_DLComplexSamp *) & g_DLBBData.burst0246;
      dlBurstBufPtr[1] = (t_DLComplexSamp *) & g_DLBBData.burst1357;
      g_BBInfo[rfCh].dlBurst = 0;     
      g_BBInfo[rfCh].dlBBSmpl =  (t_DLComplexSamp *) & g_DLBBData.burst0246;
                   
      g_BBInfo[rfCh].bsic = 0x0;
      g_BBInfo[rfCh].tseqNum = g_BBInfo[rfCh].bsic & 0x07;
                    
                                  
      /*
      * Initialize SYS INFO Buffers
      */                                  
      for(i=0; i<NUM_SYSINFO_BUFS; i++)
      {
         g_dlSigBufsSYSINFO[rfCh][i].valid = 0;
         g_dlSigBufsSYSINFO[rfCh][i].state = 0;
         g_dlSigBufsSYSINFO[rfCh][i].update = 0;         
         g_dlSigBufsSYSINFO[rfCh][i].frameNumUpdate = 0xffffffff;
      }


	  /*  for test purposes make these PSI messages valid */
/*

      
	  for (i = PACKET_TYPE_1; i < PACKET_TYPE_13; i++){
		  
            g_dlSigBufsSYSINFO[rfCh][i].valid = 1;
            for (k = 0; k < SYS_INFO_MSG_SIZE; k++){
                    g_dlSigBufsSYSINFO[rfCh][i].data[0][k] = i;
                    
            }
            
		 
	  }
*/    
	  

      /*
      * SACCH buffer is part of SYS INFO buff
      g_dlSigBufsSACCH = & g_dlSigBufsSYSINFO[rfCh][TYPE_5];
      */       

                         
      for(timeSlot=0; timeSlot<NUM_TS_PER_RF; timeSlot++)      
      {              
         /*
         * initialize a Time Slot as No Group
         */
         tsConfigure(rfCh, timeSlot, GROUP_NULL);
         
         g_BBInfo[rfCh].TNInfo[timeSlot].tsNumber = timeSlot;
      }
      /*
      *  set default BS_PA_MFEMS
      */
      g_BBInfo[rfCh].BSPaMfrms = 3;

      for(i=0; i<TBF_NUMBER_MAX; i++)
						{
						   g_tbfInfo[rfCh][i].ulTbf.state = CHAN_INACTIVE;
									g_tbfInfo[rfCh][i].dlTbf.state = CHAN_INACTIVE;
						}
   }


   g_frmSync.state = OUT_OF_SYNC;
   g_frmSync.newPageState = FALSE;
    
   /*
   * initialize frame number counters
   */
   setFrameNum.FN = 0;
   
   sysFrameNum.FN = 0;
   sysFrameNum.t1 = 0;
   sysFrameNum.t2 = 0;
   sysFrameNum.t3 = 0;
   

   return(SUCCESS);
}

/*****************************************************************************
*
* Function: tsConfigure
*
* Description:
*   Initializes structure variables for one TS
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/            
ProcResult tsConfigure (Int16 rfCh, Int16 timeSlot, t_groupNum groupNum)
{                    
   Int16 i;
   t_TNInfo *ts;
   t_PNbufs *PNbufs;

   /*
   * find right time slot Info to be configured
   */
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
   PNbufs = & g_PNbufs[timeSlot];

   ts->tsNumber = timeSlot;
   /*
   ts->ulSigBuf = & g_ulSigBufs[rfCh][timeSlot];
   ts->ulSigBuf->readIndex = 0;
   ts->ulSigBuf->writeIndex = 0;
   ts->ulSigBuf->numMsgsReady = 0;
   ts->ulSigBuf->overflow = 0;      
                       

   *  initialize RTS buffer parameters
   ts->rtsInfo = & g_rtsInfo[rfCh][timeSlot];
   ts->rtsInfo->bufWrIndex = 0;  
   ts->rtsInfo->bufRdIndex = 0;
   ts->rtsInfo->numMsgsReady = 0;
   */       
   /*
   * always set channel combination as GROUP_NULL at beginning
   */
   ts->groupNum = GROUP_NULL;

   ts->ulTblIndex = 0;
   ts->dlTblIndex = 0;
   
   g_rxGainMinAct  = RXGAIN_MIN_ACTIV;

   INTR_GLOBAL_DISABLE();

   switch(groupNum)
   {
   case GROUP_NULL:
      ts->groupULTable = ulGroupTable0;
      ts->groupDLTable = dlGroupTable0;
      ts->tblLength = 1;
      break;

   case GSM_GROUP_1:          

        if(g_AGC_ON) {
           ts->u.group1.sacch.ulsigstren.rxgain = g_rxGainMinIdle;
        }
 
        ts->u.group1.tch.state = CHAN_INACTIVE;
        ts->u.group1.sacch.state = CHAN_INACTIVE;

        /* clear all px-nx buffers for this timeslot */
        PNbufs->dpcUL.sigstrenHi.index = 0;
        PNbufs->dpcUL.sigstrenHi.flagsum = 0;
        PNbufs->dpcUL.sigstrenLo.index = 0;
        PNbufs->dpcUL.sigstrenLo.flagsum = 0;

        PNbufs->dpcUL.sigqualHi.index = 0;
        PNbufs->dpcUL.sigqualHi.flagsum = 0;
        PNbufs->dpcUL.sigqualLo.index = 0;
        PNbufs->dpcUL.sigqualLo.flagsum = 0;

        PNbufs->dpcDL.sigstrenSCellHi.index = 0;
        PNbufs->dpcDL.sigstrenSCellHi.flagsum = 0;
        PNbufs->dpcDL.sigstrenSCellLo.index = 0;
        PNbufs->dpcDL.sigstrenSCellLo.flagsum = 0;

        PNbufs->dpcDL.sigqualSCellHi.index = 0;
        PNbufs->dpcDL.sigqualSCellHi.flagsum = 0;
        PNbufs->dpcDL.sigqualSCellLo.index = 0;
        PNbufs->dpcDL.sigqualSCellLo.flagsum = 0;

        PNbufs->hoUL.sigstren.index = 0;
        PNbufs->hoUL.sigstren.flagsum = 0;
        PNbufs->hoDL.sigstrenSCell.index = 0;
        PNbufs->hoDL.sigstrenSCell.flagsum = 0;

        PNbufs->hoUL.sigqual.index = 0;
        PNbufs->hoUL.sigqual.flagsum = 0;
        PNbufs->hoDL.sigqualSCell.index = 0;
        PNbufs->hoDL.sigqualSCell.flagsum = 0;

        PNbufs->hoUL.intsigstren.index = 0;
        PNbufs->hoUL.intsigstren.flagsum = 0;
        PNbufs->hoDL.intsigstrenSCell.index = 0;
        PNbufs->hoDL.intsigstrenSCell.flagsum = 0;

        for(i=0; i < NXBUF_SIZ_MAX; i++)
        {        
           PNbufs->dpcUL.sigstrenHi.flag[i] = 0;
           PNbufs->dpcUL.sigstrenLo.flag[i] = 0;
           PNbufs->dpcUL.sigqualHi.flag[i] = 0;
           PNbufs->dpcUL.sigqualLo.flag[i] = 0;
           PNbufs->dpcDL.sigstrenSCellHi.flag[i] = 0;
           PNbufs->dpcDL.sigstrenSCellLo.flag[i] = 0;
           PNbufs->dpcDL.sigqualSCellHi.flag[i] = 0;
           PNbufs->dpcDL.sigqualSCellLo.flag[i] = 0;
           PNbufs->hoUL.sigstren.flag[i] = 0;
           PNbufs->hoUL.sigqual.flag[i] = 0;
           PNbufs->hoDL.sigstrenSCell.flag[i] = 0;
           PNbufs->hoDL.sigqualSCell.flag[i] = 0;
           PNbufs->hoUL.intsigstren.flag[i] = 0;
           PNbufs->hoDL.intsigstrenSCell.flag[i] = 0;
        }

      /* end P and N buffer init */
      ts->u.group1.tch.hoCandidateBuf.hoReportCnt = 0;

      ts->groupULTable = ulGroupTableI;
      ts->groupDLTable = dlGroupTableI;
        
      ts->tblLength = T2_FRAMES;     
      
      ts->u.group1.dlSachIdleFunc = dlSacchIdleTbl; 

      /*
      * uplink and downlink function table Index
      * SPECIAL CASE: will be put back when GSM Frame Clock interrupt is used
      *      
      */           
      
      /*
      * SACCH_IDLE function table Index
      * SPECIAL CASE: will be put back when GSM Frame Clock interrupt is used
      *      
      */        
      ts->u.group1.dlIndex = 0;
      ts->u.group1.ulIndex = 0;     


      ts->u.group1.ulSachIdleFunc = ulSacchIdleTbl;      

      
      ts->u.group1.sacch.dlSigBuf = & g_dlSigBufsTchSACCH[rfCh][timeSlot];
      ts->u.group1.sacch.dlSigBuf->numMsgsReady = 0;
      ts->u.group1.sacch.dlSigBuf->valid = 0;      
      ts->u.group1.sacch.dlSigBuf->readIndex = 0;
      ts->u.group1.sacch.dlSigBuf->writeIndex = 0;    
      ts->u.group1.sacch.dlSigBuf->overflow = 0;
        
      /*
      * init FACCH buffers
      */                                                                     
      ts->u.group1.facch.dlSigBuf = & g_dlSigBufsFACCH[rfCh][timeSlot];
      ts->u.group1.facch.dlSigBuf->numMsgsReady = 0;
      ts->u.group1.facch.dlSigBuf->valid = 0;      
      ts->u.group1.facch.dlSigBuf->readIndex = 0;
      ts->u.group1.facch.dlSigBuf->writeIndex = 0;    
      ts->u.group1.facch.dlSigBuf->overflow = 0;
                  
      break;
                    
   case GSM_GROUP_4:          
      ts->groupULTable = ulGroupTableIV;

      ts->groupDLTable = dlGroupTableIV;


      ts->tblLength = 1;     
      
      break;
                          
   case GSM_GROUP_5:
      /*
      * When in baseband loopback mode,   channel state set as CHAN_ACTIVE;
      */    

      if(g_AGC_ON) {
         ts->u.group5.sacch[0].ulsigstren.rxgain = g_rxGainMinIdle;
         ts->u.group5.sacch[1].ulsigstren.rxgain = g_rxGainMinIdle;
         ts->u.group5.sacch[2].ulsigstren.rxgain = g_rxGainMinIdle;
         ts->u.group5.sacch[3].ulsigstren.rxgain = g_rxGainMinIdle;
      }

      ts->u.group5.sdcch[0].state = CHAN_INACTIVE;
      ts->u.group5.sdcch[1].state = CHAN_INACTIVE;
      ts->u.group5.sdcch[2].state = CHAN_INACTIVE;
      ts->u.group5.sdcch[3].state = CHAN_INACTIVE;          
      
      ts->groupULTable =  ulGroupTableV;

      ts->groupDLTable =  dlGroupTableV;

      ts->tblLength = T3_FRAMES;
      
      /*                        
      * SPECIAL CASE: will be put back when GSM Frame Clock interrupt is used
      */        
 
      ts->u.group5.bcchBufIndex = 0;
      ts->u.group5.dlSdcchSubCh = 0;
      ts->u.group5.dlSacchSubCh = 0;

      ts->u.group5.dlPpchSubCh  = 0;

      ts->u.group5.ulSdcchSubCh = 0;
      ts->u.group5.ulSacchSubCh = 0;

      /*
      * BCCH and SACCH buffers are part of SYS INFO buff
      */
      ts->u.group5.bcch.dlSigBuf = & g_dlSigBufsSYSINFO[rfCh][0]; 
        
      /*
      *
      */ 
      for(i=0; i<NUM_SDCCH_SUBCHANS; i++)
      {            
         ts->u.group5.sacch[i].dlSigBuf = & g_dlSigBufsSACCH[rfCh][timeSlot][i];
         ts->u.group5.sacch[i].state = CHAN_INACTIVE;
                  
         ts->u.group5.sacch[i].dlSigBuf->valid = 0;
         ts->u.group5.sacch[i].dlSigBuf->writeIndex = 0;       
         ts->u.group5.sacch[i].dlSigBuf->readIndex = 0;
         ts->u.group5.sacch[i].dlSigBuf->numMsgsReady = 0;


         ts->u.group5.sdcch[i].dlSigBuf = & g_dlSigBufsSDCCH[rfCh][timeSlot][i];
         ts->u.group5.sdcch[i].state = CHAN_INACTIVE;
         
         g_dlSigBufsSDCCH[rfCh][timeSlot][i].valid = 0;
         g_dlSigBufsSDCCH[rfCh][timeSlot][i].readIndex = 0;
         g_dlSigBufsSDCCH[rfCh][timeSlot][i].writeIndex = 0;         
         g_dlSigBufsSDCCH[rfCh][timeSlot][i].numMsgsReady = 0;
      }

      /* Paging scheme with resource allocated buffering for AGCH, PAGCH, PCH and PPCH */
      for(i=0; i<NUM_PPCH_SUBCHANS; i++)
      {
         g_dlSigBufsPPCH[rfCh][i].msgReady = 0;
         g_dlSigBufsPPCH[rfCh][i].timeslot = 0xff;
         g_dlSigBufsPPCH[rfCh][i].logCh = NULL;
         g_dlSigBufsPPCH[rfCh][i].pageNum = 0xff;  
         g_dlSigBufsPPCH[rfCh][i].valid = 0;        
						}  
      ts->u.group5.ppchOverflow = 0;

      g_dlSigBufsSCH[rfCh][0].state = 0;
      g_dlSigBufsSCH[rfCh][0].valid = 1;
      g_dlSigBufsSCH[rfCh][0].update = 0;
      g_dlSigBufsSCH[rfCh][0].frameNumUpdate = 0xffffffff;     
      ts->u.group5.sch.dlSigBuf =  & g_dlSigBufsSCH[rfCh][0];
      ts->u.group5.sch.dlSigBuf->data[0][0] = g_BBInfo[rfCh].bsic<<2;
      ts->u.group5.sch.dlSigBuf->data[1][0] = g_BBInfo[rfCh].bsic<<2;      
            
      ts->u.group5.cbch[0].dlSigBuf =  & g_dlSigBufsCBCH[0];
      ts->u.group5.cbch[0].dlSigBuf->bufValid = 0;
      ts->u.group5.cbch[1].dlSigBuf =  & g_dlSigBufsCBCH[1];
      ts->u.group5.cbch[1].dlSigBuf->bufValid = 0;
                      
      break;




   case GSM_GROUP_7:
      /*
      * When in baseband loopback mode,   channel state set as CHAN_ACTIVE;
      */    

	   for (i = 0; i < NUM_SACCH8_SUBCHANS ; i++){
	       if(g_AGC_ON) {
                 ts->u.group7.sacch[i].ulsigstren.rxgain = g_rxGainMinIdle;}

			ts->u.group7.sdcch[i].state = CHAN_INACTIVE;
	   }  /* for i */
     
      ts->groupULTable	=  ulGroupTableVII;
      ts->groupDLTable	=  dlGroupTableVII;
      ts->tblLength	=  T3_FRAMES;
      
      /*                        
      * SPECIAL CASE: will be put back when GSM Frame Clock interrupt is used
      */        
 
      ts->u.group7.dlSdcchSubCh = 0;
      ts->u.group7.dlSacchSubCh = 0;

      ts->u.group7.ulSdcchSubCh = 0;
      ts->u.group7.ulSacchSubCh = 0;

       
      for(i=0; i<NUM_SDCCH8_SUBCHANS; i++)
      {            
         ts->u.group7.sacch[i].dlSigBuf = & g_dlSigBufsSACCH8[rfCh][0][i];
         ts->u.group7.sacch[i].state = CHAN_INACTIVE;
                  
         ts->u.group7.sacch[i].dlSigBuf->valid = 0;
         ts->u.group7.sacch[i].dlSigBuf->writeIndex = 0;       
         ts->u.group7.sacch[i].dlSigBuf->readIndex = 0;
         ts->u.group7.sacch[i].dlSigBuf->numMsgsReady = 0;


         ts->u.group7.sdcch[i].dlSigBuf = & g_dlSigBufsSDCCH8[rfCh][0][i];
         ts->u.group7.sdcch[i].state = CHAN_INACTIVE;
         
         ts->u.group7.sdcch[i].dlSigBuf->valid = 0;
         ts->u.group7.sdcch[i].dlSigBuf->readIndex = 0;
         ts->u.group7.sdcch[i].dlSigBuf->writeIndex = 0;         
         ts->u.group7.sdcch[i].dlSigBuf->numMsgsReady = 0;
      }

     
      
    
      break;

   case GSM_GROUP_11:
           
      ts->tblLength    = GPRS_MULTI_FRAMES;
      ts->groupULTable = ulGroupTableXI;
      ts->groupDLTable = dlGroupTableXI;
      

      ts->u.group11.dlPtcchIdleIndex = 0;
      ts->u.group11.ulPtcchIdleIndex = 0;
      ts->u.group11.currentTAI = 0;              
      ts->u.group11.ulBlock = 0; 
      ts->u.group11.dlBlock = 0;
 
	  
	  ts->u.group11.pbcchBufIndex = PACKET_TYPE_1;
      ts->u.group11.pbcch.dlSigBuf = & g_dlSigBufsSYSINFO[rfCh][0]; 



      /* Unsure what this is for */
      
      for(i=0; i<MAX_RADIO_BLOCKS; i++)
      {
        ts->u.group11.blkNumWatch[i].tagFillCause = NONE;
      }
      

             g_RATE_TRAN_ON = FALSE;

      /*
      *  Use FACCH Buffer for PACCH
      */
      ts->u.group11.pacch.dlSigBuf = & g_dlSigBufsFACCH[rfCh][timeSlot];
      ts->u.group11.pacch.dlSigBuf->numMsgsReady = 0;
      ts->u.group11.pacch.dlSigBuf->valid = 0;     
      ts->u.group11.pacch.dlSigBuf->readIndex = 0;
      ts->u.group11.pacch.dlSigBuf->writeIndex = 0;      
      ts->u.group11.pacch.dlSigBuf->overflow = 0;

        for(i=0; i<DL_SIG_Q_DEPTH; i++)
      {
        ts->u.group11.pacch.dlSigBuf->tag[i] = 0;
        ts->u.group11.pacch.dlSigBuf->burstType[i] = NORMAL_BURST;
      }
      
      /*
      *  Cannot use SACCH Buffers for PTCCH
      */     

         ts->u.group11.ptcch.dlSigBuf =  & g_dlSigBufsPTCCH[rfCh][timeSlot];
         ts->u.group11.ptcch.dlSigBuf->numMsgsReady = 0;
         ts->u.group11.ptcch.dlSigBuf->valid = 0;     
         ts->u.group11.ptcch.dlSigBuf->readIndex = 0;
         ts->u.group11.ptcch.dlSigBuf->writeIndex = 0;      
         ts->u.group11.ptcch.dlSigBuf->overflow = 0;    

						
      ts->u.group11.pdtch.dlSigBuf = & g_dlTchFrame[timeSlot];       
      ts->u.group11.pdtch.dlSigBuf->frameCount = 0;
      ts->u.group11.pdtch.dlSigBuf->readIndex = 0;
      ts->u.group11.pdtch.dlSigBuf->writeIndex = 0;


      /* unsure what this is about */
      
      for(i=0; i<NUM_TCH_JBUF; i++)
      {
         ts->u.group11.pdtch.dlSigBuf->tag[i] = 0;
         ts->u.group11.pdtch.dlSigBuf->burstType[i] = 0;
         ts->u.group11.pdtch.dlSigBuf->chCodec[i] = 0;
      }


   
      break;
      
    case GSM_GROUP_12:
      /*
      * When in baseband loopback mode,   channel state set as CHAN_ACTIVE;
      */
       ts->tblLength = (GPRS_MULTI_FRAMES/4);   
       
       g_RATE_TRAN_ON = FALSE;

      break;      

    case GSM_GROUP_13:
      /*
      * When in baseband loopback mode,   channel state set as CHAN_ACTIVE;
      */                                                                
      ts->tblLength = (GPRS_MULTI_FRAMES/4);
            
      ts->groupULTable =  ulGroupTableXIII;

      ts->groupDLTable =  dlGroupTableXIII;

      ts->u.group13.dlPtcchIdleIndex = 0;
      ts->u.group13.ulPtcchIdleIndex = 0;
      ts->u.group13.currentTAI = 0;              
      ts->u.group13.ulBlock = 0; 
      ts->u.group13.dlBlock = 0; 

      for(i=0; i<MAX_RADIO_BLOCKS; i++)
      {
        ts->u.group13.blkNumWatch[i].tagFillCause = NONE;
      }

      /*
      *  Use FACCH Buffer for PACCH
      */
      ts->u.group13.pacch.dlSigBuf = & g_dlSigBufsFACCH[rfCh][timeSlot];
      ts->u.group13.pacch.dlSigBuf->numMsgsReady = 0;
      ts->u.group13.pacch.dlSigBuf->valid = 0;     
      ts->u.group13.pacch.dlSigBuf->readIndex = 0;
      ts->u.group13.pacch.dlSigBuf->writeIndex = 0;      
      ts->u.group13.pacch.dlSigBuf->overflow = 0;

        for(i=0; i<DL_SIG_Q_DEPTH; i++)
      {
        ts->u.group13.pacch.dlSigBuf->tag[i] = 0;
        ts->u.group13.pacch.dlSigBuf->burstType[i] = NORMAL_BURST;
      }
      
      g_RATE_TRAN_ON = FALSE;

      /*
      *  Cannot use SACCH Buffers for PTCCH
      */     
      
      ts->u.group13.ptcch.dlSigBuf = & g_dlSigBufsPTCCH[rfCh][timeSlot];
      ts->u.group13.ptcch.dlSigBuf->numMsgsReady = 0;
      ts->u.group13.ptcch.dlSigBuf->valid = 0;     
      ts->u.group13.ptcch.dlSigBuf->readIndex = 0;
      ts->u.group13.ptcch.dlSigBuf->writeIndex = 0;      
      ts->u.group13.ptcch.dlSigBuf->overflow = 0;
      				
      ts->u.group13.pdtch.dlSigBuf = & g_dlTchFrame[timeSlot];       
      ts->u.group13.pdtch.dlSigBuf->frameCount = 0;
      ts->u.group13.pdtch.dlSigBuf->readIndex = 0;
      ts->u.group13.pdtch.dlSigBuf->writeIndex = 0;
      for(i=0; i<NUM_TCH_JBUF; i++)
      {
         ts->u.group13.pdtch.dlSigBuf->tag[i] = 0;
         ts->u.group13.pdtch.dlSigBuf->burstType[i] = 0;
         ts->u.group13.pdtch.dlSigBuf->chCodec[i] = 0;
      }
	   sendDiagMsg(10, rfCh, timeSlot, 1, (Uint8 *) & groupNum); 			   
      break;

	  default:
		break;
   }     
   /*
   * Set Group Number to this time slot at last
   */ 
   *(volatile *) & (ts->groupNum) = groupNum;


	  ts->tsConfig = TRUE;
 
   INTR_GLOBAL_ENABLE();

   return(SUCCESS);
}


