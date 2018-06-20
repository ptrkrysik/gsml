/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: dspmaint.c
*
* Description:
*   This file contains functions for memory and queue maintenance and
*   diagnostic and error reporting.
*
******************************************************************************/
#include "stdlib.h"
#include "dsp/dsptypes.h"                
#include "dsp/dsphmsg.h"
#include "comdata.h"
#include "dsp6201.h"                 
#include "dsprotyp.h"                 
  
/************************************************************
* Routine Name: MemoryDump
*
* Description: Send DSP memory contents to Host
*
*************************************************************/
Void MemoryDump (cmdStruc *diagCmd)
{
  rspStruc  DumpMsg;
  Int16     DumpSize;
  Int16     i;                  
  Int8      *memAddr;
  Uint8     dummyArg;
     
  DumpSize = diagCmd->params[1];
               
  if(DumpSize < 0)
  {
    ReportError(BAD_NAWC, 0, 0, 0, &dummyArg);
    return;
  }              
  else if(DumpSize > MAX_PARAMS)
  {          
    DumpSize = MAX_PARAMS;                   
  }      
        
  DumpMsg.function = MPH_DSP_DIAG_READ_MEM_RSP,
  DumpMsg.msgSize = DumpSize + MSG_HEAD_SIZE;     
  
  memAddr = (Int8 *) diagCmd->params[2];
  
  for(i = 0; i<DumpSize; i++)
  {
    DumpMsg.params[i] = *memAddr++;
  } 
  sendMsgHost(&DumpMsg);
}  

/************************************************************
* Routine Name: getRspQueStatus
*
* Description: Send Rsp Queue status parameters to Host
*
*************************************************************/
Void getRspQueStatus (cmdStruc *diagCmd)
{
  rspStruc  rspMsg;
  Int16     DumpSize;
  Int8      *memAddr;
  
  rspMsg.msgSize = MSG_HEAD_SIZE + 4;  
  rspMsg.function =  DIAGNOSTIC_MSG;              
  rspMsg.typeLoByte =  MPH_DSP_DIAG_GET_DSP_QUE_STATUS_RSP;  
  rspMsg.typeHiByte =  MPH_DSP_DIAG_GET_DSP_QUE_STATUS_RSP>>8;    
  rspMsg.trx =  diagCmd->trx;  
  rspMsg.params[0] =  RspQue->msgCount;   
  rspMsg.params[1] =  RspQue->msgInIndex;  
  rspMsg.params[2] =  RspQue->msgOutIndex; 
  rspMsg.params[3] =  DspMsgNumWord;   
  sendMsgHost(&rspMsg);
}  

/************************************************************
* Routine Name: ReportError
*
* Description: Report an error to Host
*
*************************************************************/
Void ReportError (ErrorType Error, Uint8 trx, Uint8 timeSlot,
                  Uint8 numParams, Uint8 *params)
{
  rspStruc  ErrorMsg;
  Uint8     i;
  
  if(numParams > MAX_PARAMS-2) numParams = MAX_PARAMS-2;
              
  ErrorMsg.msgSize = 2+MSG_HEAD_SIZE+numParams; 
  ErrorMsg.function =  DIAGNOSTIC_MSG;
  ErrorMsg.trx =  trx;
  ErrorMsg.typeLoByte = MPH_DSP_DIAG_REPORT_ERR_RSP;
  ErrorMsg.typeHiByte = MPH_DSP_DIAG_REPORT_ERR_RSP>>8;    
  ErrorMsg.params[0] = timeSlot;
  ErrorMsg.params[1] = Error;
  for(i=0; i<numParams; i++)
    ErrorMsg.params[2+i] = *params++;
  
  SendMsgHost_Que(&ErrorMsg);
}
                                  
/************************************************************
* Routine Name: sendDiagMsg
*
* Description: Send diagnostic message Host
*
*************************************************************/
Void sendDiagMsg (Uint8 diagMsgType, Uint8 trx, Uint8 timeSlot,
                  Uint8 numParams, Uint8 *params)
{
  rspStruc DiagMsg;
  Uint8 i;
  
  if(numParams > MAX_PARAMS-2) numParams = MAX_PARAMS-2;
              
  DiagMsg.msgSize = 2+MSG_HEAD_SIZE+numParams; 
  DiagMsg.function =  DIAGNOSTIC_MSG;
  DiagMsg.trx =  trx;
  DiagMsg.typeLoByte = MPH_DSP_DIAG_REPORT_RSP;
  DiagMsg.typeHiByte = MPH_DSP_DIAG_REPORT_RSP>>8;    
  DiagMsg.params[0] = timeSlot;
  DiagMsg.params[1] = diagMsgType;
  for(i=0; i<numParams; i++)
    DiagMsg.params[2+i] = *params++;
  
  SendMsgHost_Que(&DiagMsg);
}                                  
       
                           
                  