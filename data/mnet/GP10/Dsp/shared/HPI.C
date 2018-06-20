/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
*
* File: hpi.c              
*                          
*****************************************************************************/
#include "stdlib.h"
#include "gsmdata.h"
#include "dsp/dsphmsg.h"
#include "dsp6201.h"                 
#include "comdata.h"
#include "hpi.h"
#include "dsprotyp.h"

/*
 ************************************************************
 * Routine Name: SendMsgHost(Void)
 *
 * Description: Send message to Host, the message is queued for 
 *          transmitting 
 *
 ************************************************************
*|*/     
Void SendMsgHost_Que(msgStruc *DspMsg) 
{
  Uint8 dummyArg;
                                    
  if(!DspMsg)
  {      
    /* 
    *  Unassigned Message pointer
    */
    ReportError(NULL_MSG, 0, 0xFF, 0, &dummyArg);
  }                            
  else if(DspMsg->msgSize > (MAX_PARAMS + MSG_HEAD_SIZE)|| 
          DspMsg->msgSize < 0 )
  { 
    /*
    * Invalid parameters
    */
    ReportError(BAD_NAWC, DspMsg->trx, 0xff, 4, (Uint8*)&DspMsg->msgSize);
  }
  else
  {
    qPut(DspMsg, RspQue);
  }       
}
/*|*
 ************************************************************
 * Routine Name: sendMsgHost(Void)
 *
 * Description: Send message to Host immediately if HPI is free
 *
 * Warnning: This routine should not be calle on ANY ISR !!!
 *
 *
 ************************************************************
*|*/     
Void sendMsgHost(msgStruc *DspFastMsg)
{   
  Int32 *rdPtr;
  Int32 *wrPtr;
  Int16 i;
  Uint8 dummyArg;

  if(!DspFastMsg)
  { 
    /*
    * Unassigned message pointer
    */
    ReportError(NULL_MSG, 0, 0xFF, 0, &dummyArg);
    return;
  }                            
  else if(DspFastMsg->msgSize > (MAX_PARAMS + MSG_HEAD_SIZE)|| 
          DspFastMsg->msgSize < 0 )
  { 
    /*
    * Invalid parameters
    */
    ReportError(BAD_NAWC, DspFastMsg->trx, 0xFF, 4, (Uint8*)&DspFastMsg->msgSize);
    return;
  }
  /*
  * check if last message has been sent
  */
  if(DspMsgNumWord == 0)
  {                   
     
    /*  
    * copy msg from source to HPI Tx buffer
    *
    * Assumption Source and destination address in 32 bit word alignment
    */     
    rdPtr = (Int32 *)DspFastMsg;
    wrPtr = (Int32 *)hpiTxMsgPtr;
                 
    /*
    *  check if last 4 bytes of the message are used
    */    
    if(rdPtr[0] > MAX_PARAMS)
    {
      for(i=0; i<sizeof(msgStruc)/4; i++) wrPtr[i] = rdPtr[i];
    }
    else
    {
      for(i=0; i<sizeof(msgStruc)/4; i++) wrPtr[i] = rdPtr[i];
    
      /*
      *  DEBUG - sendback GSM Frame Number
      */ 
      wrPtr[i-1] =  sysFrameNum.FN;      
    }
       
    DspMsgNumWord++;  
    
   /*  
    * Interrupt host to indicate there is a message
    */   
    * (Uint32 *) HPIC_ADDR |= 0x04;
  }
  else
  {            
    /*
    *  If HPI Txmemory is not ready, put message into fast response queue
    */                                    
    qPut(DspFastMsg, RspQue);
  }
}   
/*|*
 ************************************************************
 * Routine Name: hpiRcvMsg(Void)
 *
 * Description: A backgroud task polls HPI receive buffer and
 *              move message to queue
 *
 ************************************************************
*|*/     
Void hpiRcvMsg(Void)
{                
  Int16 MsgLength;
  msgStruc *hostMsg;
  Int8 *Ptr;
  ProcResult Result;
                      
  hostMsg = (msgStruc *) hpiRxMsgPtr;
  while(HostMsgNumWord > 0)
  {
      if(hostMsg->msgSize > (MAX_PARAMS + MSG_HEAD_SIZE) || 
        hostMsg->msgSize < 0 )
      { 
        /*
        * Invalid parameters
        */
        ReportError(UNAUTHORIZED_MSG_SIZE, hostMsg->trx, 0xFF, MAX_PARAMS-2, (Uint8 *) hostMsg-4);
      }
      else 
      {      
        /*    
        * if there is a new host message, process the message in place.
        * clear host port message flag to allow new message
        * MMR_HPIC->DSPINT = 0;        
        */        
         Result = ProcessHostMsg(hostMsg);
                                  
         /*
         * if message can not be processed right way, put it into queue
         */                                      
       if(Result == UN_PROCESSED) 
         {                          
             qPut(hostMsg, HmsgQue);
         }
      }
      HostMsgNumWord--;      
      /*
      * point to next message
      hostMsg = (msgStruc *) ( (Int8 *) hostMsg + sizeof(msgStruc));
      */
      hostMsg++;
        
  }
}
/*|*
 ************************************************************
 * Routine Name: hpiXmtMsg(Void)
 *
 * Description: A backgroud task polls DSP message queue to host and
 *              send message to HPI transmit buffer.
 *
 ************************************************************
*|*/     
Void hpiXmtMsg(Void)
{   
  Int16 WordCount;  
/*
 * check if last message has been sent 
 * if(MMR_HPIC->HINT == 0)               
 */                              
  if(DspMsgNumWord == 0)  
  {                              
    if(DspMsgNumWord = qGetAll((Int32 *) hpiTxMsgPtr, FastRspQue, HPI_TX_MAX_MSGS))
    {
      /*
      * Interrupt host if there is a message in the queue
      */          
      /*
     *   MMR_HPIC->HINT = 1;     
     */
      * (Uint32 *) HPIC_ADDR |= 0x04;    
    }
    else if(DspMsgNumWord = qGetAll((Int32 *) hpiTxMsgPtr, RspQue, HPI_TX_MAX_MSGS))
    { 
      /*
      * Interrupt host if there is a message in the queue
      */          
      /*
     *   MMR_HPIC->HINT = 1;     
     */                             
      * (Uint32 *) HPIC_ADDR |= 0x04;    
    }
  }
}         
  