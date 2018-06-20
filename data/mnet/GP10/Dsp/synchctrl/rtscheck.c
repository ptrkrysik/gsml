/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* file name: rtsCheck.c              
*                       
* Description: check RTS for channels
*
*****************************************************************************/
#include "stdlib.h"
#include "gsmdata.h"
#include "dsp/dsphmsg.h"
#include "rrmsg.h"
#include "dsprotyp.h"
  
/*****************************************************************************
*
* Function: rtsCheck
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void rtsCheck(Uint8 rfCh, Uint8 timeSlot, Uint8 logCh,  Uint8 subCh, Int8 availableBuf)
{   
   msgStruc phRTSIndBuf;
  
	t_TNInfo *ts;          
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

	   if(logCh == PACCH || logCh == PDTCH)
    {
	     phRTSIndBuf.function = PDCH_MANAGE_MSG;	
    }
    else if( logCh == CBCH || logCh == CBCH_EXT || logCh == AGCH || logCh == PCH)
    {
       phRTSIndBuf.function = DCCH_MANAGE_MSG;
       if( logCh == CBCH || logCh == CBCH_EXT)
       {
          /*
          *  USE SDCCH4 for CHCH and CBCH_EXT until RRM can handle
          *  CHCH and CBCH_EXT !!!
          */
          logCh = SDCCH4;
       }    
    }
    else
    {    
       phRTSIndBuf.function = RR_MANAGE_MSG;
    }
    phRTSIndBuf.typeHiByte = PH_READY_TO_SEND_IND_HI;
    phRTSIndBuf.typeLoByte = PH_READY_TO_SEND_IND_LO;
    phRTSIndBuf.trx = rfCh;  /* for now */

    chXlate(ts->groupNum,
            logCh,
            subCh,
            &phRTSIndBuf.params[0],
            &phRTSIndBuf.params[2]);
   
    phRTSIndBuf.params[0] |= timeSlot;  
    phRTSIndBuf.params[1] = 0;
         
    phRTSIndBuf.params[3] =  availableBuf; 

    phRTSIndBuf.params[4] =  (sysFrameNum.t3 >> 3) | (sysFrameNum.t1 << 3);
    phRTSIndBuf.params[5] =  sysFrameNum.t2  | (sysFrameNum.t3 << 5);      

    phRTSIndBuf.msgSize= MSG_HEAD_SIZE+6;
             
    SendMsgHost_Que(&phRTSIndBuf);	
}

#ifdef RTS_MSG_SEND_INDIRECT

/*****************************************************************************
*
* Function: rtsSend
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void rtsSend(Uint8 rfCh, Uint8 timeSlot)
{
   msgStruc phRTSIndBuf;
  
	t_TNInfo *ts;          
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

	if(ts->rtsInfo->numMsgsReady > 0)
	{	
      phRTSIndBuf.function = RR_MANAGE_MSG;
      phRTSIndBuf.typeHiByte = PH_READY_TO_SEND_IND_HI;
      phRTSIndBuf.typeLoByte = PH_READY_TO_SEND_IND_LO;
      phRTSIndBuf.trx = 0;  /* for now */

      chXlate(	ts->groupNum,
      			ts->rtsInfo->data[ts->rtsInfo->bufRdIndex].chanSel,
      			ts->rtsInfo->data[ts->rtsInfo->bufRdIndex++].subCh,
      			&phRTSIndBuf.params[0],
      			&phRTSIndBuf.params[2]);
     
      phRTSIndBuf.params[3] =  (ts->rtsInfo->frameNum.t3 >> 3) |  (ts->rtsInfo->frameNum.t1 << 3);
      phRTSIndBuf.params[4] =  ts->rtsInfo->frameNum.t2  | (ts->rtsInfo->frameNum.t3 << 5);      
        
      ts->rtsInfo->bufRdIndex &= NUM_RTS_BUFS>>1;
      			
      ts->rtsInfo->numMsgsReady--;
        			
      phRTSIndBuf.params[0] |= timeSlot;  
      phRTSIndBuf.params[1] = 0;
      
/*
      phRTSIndBuf.params[0] = timeSlot;
      phRTSIndBuf.params[0] |= logCh<<3;
      phRTSIndBuf.params[1] = subCh | 0x08;
      phRTSIndBuf.params[2] = 0;
*/          
      phRTSIndBuf.msgSize= MSG_HEAD_SIZE+5;
             
      SendMsgHost_Que(&phRTSIndBuf);	
   }
}  

#endif    