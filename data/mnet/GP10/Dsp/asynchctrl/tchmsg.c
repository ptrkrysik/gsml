/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: tchmsg.c
*
* Description:
*   This file contains functions that process TCH messages from the host.
*
* Public Functions:
*   tchFrameInit, tchMsgProc, sidMsgProc
*
* Private Functions:
*   none
*
******************************************************************************/
#include "stdlib.h"
#include "gsmdata.h"
#include "dsp/dsphmsg.h"
#include "dsp6201.h"                 
#include "intr.h"
#include "bbdata.h"
#include "diagdata.h" 
#include "dsprotyp.h"
            
/*****************************************************************************
*
* Function: tchFrameInit
*
* Description:
*   Initializes downlink TCH parameters
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void tchFrameInit(void)
{
  Int16 timeSlot;
  for( timeSlot=0; timeSlot<8; timeSlot++)
  {
      g_dlTchFrame[timeSlot].frameCount = 0;    
      g_dlTchFrame[timeSlot].readIndex = 0;      
      g_dlTchFrame[timeSlot].writeIndex = 0;       
  }
}

/*****************************************************************************
*
* Function: tchMsgProc
*
* Description:
*   Process TCH messages from host
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
ProcResult tchMsgProc(msgUnionStruc *tchMsg)
{  
   Uint8    rfCh;   
   Uint8    timeSlot;
   Int16    i;
   Uint32   *rdPtr;
   t_TNInfo *ts;          
   
   rfCh = tchMsg->trx & 0x1; 
   timeSlot = tchMsg->u.params[0] & 0x7;
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

   if ( g_loopBackMode == DSP_TCH_LOOPBACK )
   {
      /*
      *  downlink TCH frame comes from Uplink TCH frame
      *  don't take TCH frame from host
      */
      return(SUCCESS);
   }
   else if ( g_loopBackMode == DSP_TCH_TO_HOST_LOOPBACK )
   {                           
      /*
      *  Send downlink TCH frames back to host directly
      */
      tchMsg->typeHiByte =  UP_LINK_TCH >> 8;  
      tchMsg->typeLoByte =  UP_LINK_TCH;
      SendMsgHost_Que((msgStruc*)tchMsg);                        
   }

   rdPtr = (Uint32 *) & tchMsg->u.tchInfo.tchFrame[0];
   g_dlTchFrame[timeSlot].frameStarted = TRUE;
   
   /*
   *  Check for properly incrementing RTP sequence number. Save for next time.
   *  If we're not in DL DTX mode, track out-of-sequence errors.
   */                 
   if ( tchMsg->u.tchInfo.rtpSeqNum != (g_dlTchFrame[timeSlot].rtpSeqNum+1) &&
        !ts->u.group1.tch.DLDtxMode &&
        !ts->u.group1.tch.ULDtxMode )
   {   
      g_DlOutOfSequenceCount[rfCh][timeSlot] += 1; 
   }      
   g_dlTchFrame[timeSlot].rtpSeqNum = tchMsg->u.tchInfo.rtpSeqNum; 
   
   /*
   *  Copy GSM TCH payload to downlink TCH buffer, only if traffic frame type matches
   *  with current channel speech mode
   */                                          
   if ( (ts->u.group1.tch.vocAlgo == VOC_EFR &&
        (tchMsg->u.tchInfo.tchFrame[0] & 0xF0) == 0xC0) || 
        (ts->u.group1.tch.vocAlgo == VOC_GSMF &&
        (tchMsg->u.tchInfo.tchFrame[0] & 0xF0) == 0xD0))
   {
      INTR_DISABLE(CPU_INT8); /* disable burst interrupt */    

      for (i=0; i<10; i++)
      {
         g_dlTchFrame[timeSlot].buffer[g_dlTchFrame[timeSlot].writeIndex][i] = rdPtr[i];
      }
      /*
      *  Update buffer index (circular)
      */       
      if ( ++g_dlTchFrame[timeSlot].writeIndex >= NUM_TCH_JBUF)
      {
         g_dlTchFrame[timeSlot].writeIndex = 0;
      }
      /*
      *  Increase frame count and check for overflow
      */   
      if ( ++g_dlTchFrame[timeSlot].frameCount > NUM_TCH_JBUF )
      {
         g_DlOverflowCount[rfCh][timeSlot] += 1;
         g_dlTchFrame[timeSlot].frameCount = NUM_TCH_JBUF;
         /*
         *  New frame overwrote next buffer to be read. Set readIndex to oldest frame.
         */
         g_dlTchFrame[timeSlot].readIndex = g_dlTchFrame[timeSlot].writeIndex;
      }  
      INTR_ENABLE(CPU_INT8); /* disable burst interrupt */                          
  }
  return(SUCCESS);
}   
 
/*****************************************************************************
*
* Function: sidMsgProc
*
* Description:
*   Process SID messages from host
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
ProcResult sidMsgProc(msgUnionStruc *sidMsg)
{  
   Uint8 timeSlot;
   Int16 i;
   Uint32 *rdPtr;
   Uint32 *wrPtr;
   
   timeSlot = sidMsg->u.params[0] & 0x7;
   
   wrPtr =  g_dlSidFrame[timeSlot].buffer;
   /*
   *  copy GSM SID payload to downlink SID frame buffer
   */ 
   for(i=0; i<10; i++)
      wrPtr[i] = rdPtr[i]; 
   
   g_dlSidFrame[timeSlot].frameCount++;          

}