/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* file name: dspmh.c              
*                       
* Description: Search message from message queue and 
*              interperate the message
*
*****************************************************************************/
#include "stdlib.h"
#include "gsmdata.h"                
#include "dsp/dsphmsg.h"
#include "dsp6201.h"                 
#include "comdata.h"
#include "intr.h"
#include "dsprotyp.h"

ProcResult pdchMsgProc(msgStruc* msg);

/*****************************************************************************
*
* Function: dspMsgHandler
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
Void dspMsgHandler(Void)
{                 
  cmdStruc hmsg;
  ProcResult Result;
  static Uint32 lastFrameNumber = 0;
  Uint8 i;

  if(lastFrameNumber != sysFrameNum.FN)
  {
      /*
      *  Do not process the same queue until Frame number has changed
      */
      //return;
 
      lastFrameNumber = sysFrameNum.FN;


      if(qGet(& hmsg, FastHmsgQue) == SUCCESS ||
         qGet(& hmsg, HmsgQue) == SUCCESS)
      { 
    
         /*
         * process a message got from one of the queues
         */
         Result = ProcessHostMsg(& hmsg);

         /*
         * if message can not be processed right way, put it pack into queue
         */                                      
         if(Result == UN_PROCESSED) 
         {                          
            qPut(& hmsg, HmsgQue);
         }
         else
         {  
            /*
            *  a diag Msg to host tells how many host messages have been stored in
            *  this message queue, because they can't be processed on spot.
            */
            sendDiagMsg(13, hmsg.trx, hmsg.params[0] & 0x07, 2, (Uint8*)&HmsgQue->msgCount);
         }
      }

  }
}                  

/*****************************************************************************
*
* Function: ProcessHostMsg
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
ProcResult ProcessHostMsg(cmdStruc * hmsg)
{
  ProcResult Result;
      
  if(hmsg->trx > (NUM_RFS_PER_DSP-1)) 
  {       
     /*
     *  illegal TRX number
     */
     ReportError(0x20, hmsg->trx, 0, 1, & (hmsg->trx));
     return;
  }

  switch(hmsg->function & 0xfe)
  {    
    case TRAFFIC_MSG:
      Result = tchMsgProc((msgUnionStruc*)hmsg);
      break;
         
    case RR_MANAGE_MSG:
      Result = rrMsgProc(hmsg);
      break;
      
    case DCCH_MANAGE_MSG:
      Result = dcchMsgProc((msgUnionStruc*)hmsg);
      break;
      
    case CCCH_MANAGE_MSG:
      Result = ccchMsgProc((msgUnionStruc*)hmsg);
      break;
      
    case TRX_MANAGE_MSG:
      Result = trxMsgProc(hmsg);
      break;
      
    case OAM_MANAGE_MSG:
      Result = oamMsgProc(hmsg);
      break;

    case DIAGNOSTIC_MSG:
      Result = diagMsgProc(hmsg);
      break;
                             
    case PDCH_MANAGE_MSG:
      Result = pdchMsgProc(hmsg);
      break;

    default:
      ReportError(UNKNOWN_MSG_DISCRIMINATOR, hmsg->trx, 0, MAX_PARAMS-2, (Uint8 *) hmsg);
      break;
  }
  return(Result);
}


