/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: dspmain.c
*
* Description:
*   This file contains the DSP main.
*
* Public Functions:
*   ulTchNetProc, dlTchNetProc
*
* Private Functions:
*   none
*
******************************************************************************/
#include "stdlib.h"
#include "gsmdata.h" 
#include "oamdata.h" 
#include "dsp6201.h"               
#include "dsp/dsphmsg.h"
#include "diagdata.h" 
#include "bbdata.h"
#include "dsprotyp.h"
#include "agcdata.h"

/*****************************************************************************
*
* Function: DSP main
*
* Description:
*   Initializes and drops into background event loop, which handles all HPI
*   messages.
*
* Inputs:
*   None  
*
* Outputs:
*   None
*
******************************************************************************/
void main(void)
{                     
   Uint8 on_off = 1;
   Uint32 index = 0;
   rspStruc msg;
   Int32 i,j;
   msgUnionStruc *dcchMsg;

   /*
   * set loopback mode as NO_LOOPBACK for normal operation
   g_loopBackMode = LOCAL_LOOPBACK;
   */
   g_loopBackMode = NO_LOOPBACK;

   // g_loopBackMode = LOCAL_LOOPBACK;
   dsp_init(); 
  
   board_init();            /* init EVM Board */
   /*
   * set external SBSRAM as half rate
   */
   //*(Uint32 *) 0x01800000 = 0x00003760;
     
   /* set to DSP control (=0 dsp, =1 fpga control). */
   /*
   * enable DSPs frame/bit clock synchronization 
   * Turn Both LED2 Bit 7 and 8   
   */
   AGC_SRC_SELECT_REG = 0x180;
   
   /*           
   g_BBInfo[0].TNInfo[4].u.group1.tch.state = CHAN_ACTIVE;    
   g_BBInfo[0].TNInfo[4].u.group1.sacch.dlSigBuf->valid = 1;
   g_BBInfo[0].TNInfo[1].u.group1.tch.state = CHAN_ACTIVE;    
   g_BBInfo[0].TNInfo[1].u.group1.sacch.state = CHAN_ACTIVE;     
   g_BBInfo[0].TNInfo[1].u.group1.sacch.dlSigBuf->valid = 1;
   g_BBInfo[0].TNInfo[1].u.group1.tch.speechDataInd = IND_SPEECH; 

   g_BBInfo[0].TNInfo[2].u.group1.tch.state = CHAN_ACTIVE;    
   g_BBInfo[0].TNInfo[2].u.group1.sacch.state = CHAN_ACTIVE;     
   g_BBInfo[0].TNInfo[2].u.group1.sacch.dlSigBuf->valid = 1;
   g_BBInfo[0].TNInfo[2].u.group1.tch.speechDataInd = IND_SPEECH; 
 
   send_test_msg(); 
   */        
   
	/*
	   dlGroupVTableLpbkInit();                       
	*  Local loopback test
 	*/ 
 	if(g_loopBackMode == LOCAL_LOOPBACK)
 	{ 
	  /*              
      tsConfigure(0, 0, GSM_GROUP_5); 
      tsConfigure(0, 1, GSM_GROUP_7);
      tsConfigure(0, 2, GSM_GROUP_1); 
      tsConfigure(0, 3, GSM_GROUP_1);
      tsConfigure(0, 4, GSM_GROUP_13);    
      tsConfigure(0, 5, GSM_GROUP_1);
      tsConfigure(0, 6, GSM_GROUP_1);      
      tsConfigure(0, 7, GSM_GROUP_1);
         	
      ulGroupVTableLpbkInit();
      ulGroupVIITableLpbkInit();
 
    
      for (i = 0; i < 4 ; i++){
      g_BBInfo[0].TNInfo[0].u.group5.sacch[i].state = CHAN_ACTIVE;
      g_BBInfo[0].TNInfo[0].u.group5.sdcch[i].state = CHAN_ACTIVE;
      }
    
    
      for (i = 0; i <8 ; i++){  
      g_BBInfo[0].TNInfo[1].u.group7.sacch[i].state = CHAN_ACTIVE;
      g_BBInfo[0].TNInfo[1].u.group7.sdcch[i].state = CHAN_ACTIVE;
      }	
      

      g_BBInfo[0].TNInfo[2].u.group1.tch.state = CHAN_ACTIVE;
      g_BBInfo[0].TNInfo[2].u.group1.sacch.state = CHAN_ACTIVE;          
      g_BBInfo[0].TNInfo[2].u.group1.sacch.dlSigBuf->valid = 1;          
      g_BBInfo[0].TNInfo[2].u.group1.tch.speechDataInd = IND_SPEECH;     
      g_BBInfo[0].TNInfo[2].u.group1.tch.vocAlgo = VOC_GSMF;    
      g_BBInfo[0].TNInfo[2].u.group1.tch.DLDtxMode = TRUE;
      g_BBInfo[0].TNInfo[2].u.group1.tch.ULDtxMode = TRUE;
	  
	  */
      /*---------------------------------------------------
      * Initialize TCH jitter buffer related parameters  			
      *---------------------------------------------------*/
      g_dlTchFrame[2].frameCount = 0;   
      g_dlTchFrame[2].readIndex = 0;      
      g_dlTchFrame[2].writeIndex = 0;			         
      g_dlTchFrame[2].frameStarted = 0;  


      /*---------------------------------------------------
      * Initialize SID frame count   			
      *---------------------------------------------------*/            
      g_dlSidFrame[2].frameCount = 0;     
		                                                       
      /*-------------------------------------------------------        
      * Initialize agc, dynamic pwr control    
      * This simulates a channel activation msg in dcchmsg.c
      *-------------------------------------------------------*/
      initPower(& dcchMsg->u.chActInfo, 
                & g_BBInfo[0].TNInfo[2].u.group1.sacch);

      /*
      *  init a TBF pair
      */                           
      g_tbfInfo[0][0].dlTbf.state = CHAN_ACTIVE;      
      g_tbfInfo[0][0].dlTbf.tfi = 0;
      g_tbfInfo[0][0].dlTbf.timeSlotAllocation = 0x80;
      g_tbfInfo[0][0].dlTbf.channelCodec = GPRS_CS_1;
            
      g_tbfInfo[0][0].ulTbf.state = CHAN_ACTIVE; 
      g_tbfInfo[0][0].ulTbf.tfi = 0;
      g_tbfInfo[0][0].ulTbf.tbfMode = TBF_DYNAMIC;
      g_tbfInfo[0][0].ulTbf.timeSlotAllocation = 0x80;
      g_tbfInfo[0][0].ulTbf.opMode.dynamic.usfTsMap = 0x00050000;     /* USF 5 on Time Slot 4 */
      g_tbfInfo[0][0].ulTbf.channelCodec = GPRS_CS_1;
      g_tbfInfo[0][0].ulTbf.tai = 0;
      g_tbfInfo[0][0].ulTbf.taiTimeSlot = 4;
      g_tbfInfo[0][0].ulTbf.timingAdvance = 0;
 
      sysSyncProc();
      
      g_frmSync.state = SET_SYNC;

      init_hw(); 
   }

    /* Bootstrap confirm message to the host */
   msg.msgSize = 5;
   msg.function =  0x10;
   msg.typeHiByte = 0x20;
   msg.typeLoByte = 0x00;
   msg.trx = 0;
   msg.params[0] = g_encryptionCapability;

   sendMsgHost(&msg);

   /* Request tables for downlink TX power characteristics */
   msg.msgSize = 4;      
   msg.function = TRX_MANAGE_MSG;
   msg.typeHiByte =  REQ_CONFIG_RADIO_TX_PWR >> 8;  
   msg.typeLoByte =  REQ_CONFIG_RADIO_TX_PWR;  
   msg.trx = 0;   
   sendMsgHost(&msg);

   while(1)
   {  
      hpiRcvMsg();
      dspMsgHandler();
      hpiXmtMsg();

      if(g_loopBackMode == LOCAL_LOOPBACK)
      {
         mod2DmodLpbk();
      }               
      /*------------------------------------------------------
      * Report diags and errors that accumulate for output
      * once per superframe (and might otherwise flood Host)
      *-----------------------------------------------------*/
      ReportDiagToHost();
      ReportErrorToHost();     
   }
}

/*****************************************************************************
*
* Function: ulTchNetProc
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void ulTchNetProc(void)
{

}

/*****************************************************************************
*
* Function: dlTchNetProc
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dlTchNetProc(void)
{

}
 

                                   









