/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: queue.c
*
* Description:
*   This file contains functions that create the message queues, put messages
*   into the queues and get messages from the queues.
*
* Public Functions:
*   qCreate, qPut, qGet, qGetAll
*
* Private Functions:
*   None
*
******************************************************************************/
#include "stdlib.h"
#include "gsmdata.h"               
#include "dsp/dsphmsg.h"
#include "dsp6201.h"                 
#include "comdata.h"                                      
#include "intr.h"               
#include "dsprotyp.h"               
  
/************************************************************
* Function Name: qCreate
* Description: create a message queue
* 
*
************************************************************/                       
qStruc *qCreate(Int16 NumberQ, Int16 Priority, qStruc *qName)
{
   Int16 qMemSize;
   Int8  *memName;
   Uint8 dummyArg;
   qMemSize = NumberQ * sizeof(msgStruc) + QUE_HEAD_SIZE;
   memName = (Int8 *) malloc(qMemSize);
 
   if(memName == NULL)
   { 
      ReportError(MALLOC, 0xFF, 0xFF, 0, &dummyArg);
   } 
   else
   {  
      qName = (qStruc *)memName;                     
      qName->msgInIndex = 0;
      qName->msgOutIndex = 0;
      qName->qSize = NumberQ;
      qName->qAttrb = Priority;
      qName->msgCount = 0;      
      qName->msgOverflow = 0;
   }
   return(qName);
}
                                                     
/************************************************************
* Function Name: qPut
* Description: put a message to a queue
* 
*
************************************************************/                                          
Bool qPut(msgStruc *msg, qStruc *qName)
{   
   Bool  result;
   Int32 *wrPtr;
   Int32 *rdPtr;            
   Int16 i;
   Int16 reenable=FALSE;                   
   Uint8 dummyArg;
  
   if(!msg)
   {  
      ReportError(NULL_MSG, 0xFF, 0xFF, 0, &dummyArg);
      return(FAILURE);
   }
                    
   if(qName->msgCount < qName->qSize)
   { 
      /*
      * queue is not full
      */                
      wrPtr = (Int32 *) & (qName->qMsg);
      wrPtr += qName->msgInIndex * sizeof(msgStruc)/4;
      rdPtr = (Int32 *)msg;
      /*
      * copy message to queue
      */
      for(i=0; i<sizeof(msgStruc)/4; i++)
      {
         wrPtr[i] = rdPtr[i];
      } 

      /*
	  *  disable Serving Burst Interrupt only, Don't Disable GLOBAL INTERRUPT which inteferes
	  *  DMA-BSP operation
	  */
      INTR_DISABLE(CPU_INT8);

      /*
      * increment message count and input index
      */
      qName->msgCount++;  
      if ( ++qName->msgInIndex >= qName->qSize )
      { 
         qName->msgInIndex = 0;
      }
          
      INTR_ENABLE(CPU_INT8);

      result = SUCCESS;
   }
   else
   { 
      /*
      * Increment dropped message count. Note: This count is not 
      * yet reported to host. This reporting could be added later.
      */                           
      if ( qName->msgOverflow < MSG_OVERFLOW_MAX_COUNT )
         qName->msgOverflow ++;
         
      result = FAILURE;
   }
   
   /*
   * If we disabled interrupts, re-enable them now
   */                        
   return(result);              
}
                                                       
/************************************************************
* Function Name: qGet
* Description: get a message from a queue
* 
*
************************************************************/                                                              
Bool qGet(msgStruc *msg, qStruc *qName)
{              
   Bool result;
   Int32 *wrPtr;
   Int32 *rdPtr;             
   Int16 i;
   Uint8 dummyArg;
                   
   if(!msg)
   {
      ReportError(NULL_POINT, 0xFF, 0xFF, 0, &dummyArg);
      return(FAILURE);
   }
                    
   if(qName->msgCount > 0)            
   {                             
      rdPtr = (Int32 *) & (qName->qMsg);
      rdPtr += qName->msgOutIndex * sizeof(msgStruc)/4;
      /*
      * copy message from the queue
      */
      wrPtr = (Int32 *)msg;  
      for(i=0; i<sizeof(msgStruc)/4; i++)            
      {
         wrPtr[i] = rdPtr[i];
      } 
      /*
      *  Disable interrupts while decrementing message count
      *  and advancing msgOutIndex, then re-enable
      */ 
      INTR_DISABLE(CPU_INT8);
      qName->msgCount--;                                      
      if ( ++qName->msgOutIndex >= qName->qSize )
      {
         qName->msgOutIndex = 0;
      }
      INTR_ENABLE(CPU_INT8);
    
      result = SUCCESS;
   }
   else
   {
      result = FAILURE;
   }
                           
   return(result);
}                                    
                 
/************************************************************
* Function Name: qGetAll
* Description: Gets all messages or up to given no. of messages
* from a queue. If maxMsgs is 0, gets all messages else gets up to
* maxMsgs.
*
************************************************************/                                                              
Int16 qGetAll(Int32 *wrPtr, qStruc *qName, Uint8 maxMsgs)
{              
   Int32  *rdPtr;             
   Int16  j, k;
   Int8   wordCount = 0;   
   Int8   msgCount = 0;  
   Uint8 dummyArg;

   if(!wrPtr)
   {
      ReportError(NULL_MSG, 0xFF, 0xFF, 0, &dummyArg);
      return(msgCount);
   }
              
   k = 0;              
   while ((qName->msgCount > 0) && ((!maxMsgs) || (msgCount < maxMsgs)))       
   {                             
      rdPtr = (Int32 *) & (qName->qMsg);
      rdPtr += qName->msgOutIndex * sizeof(msgStruc)/4;
      msgCount += 1; 
      /*
      *  Copy one message from the queue. If the last 4 bytes of the
      *  message are unused, use them to send the GSM frame number
      */    
      if(rdPtr[0] > MAX_PARAMS)
      {
         for(j=0; j<sizeof(msgStruc)/4; j++) wrPtr[k++] = rdPtr[j];
      }
      else
      {
         for(j=0; j<sizeof(msgStruc)/4; j++) wrPtr[k++] = rdPtr[j];     
         wrPtr[k-1] =  sysFrameNum.FN;      
      }
      /*
      *  Disable interrupts while decrementing message count
      *  and advancing msgOutIndex, then re-enable
      */ 
      INTR_DISABLE(CPU_INT8);
      qName->msgCount--;                                      
      if ( ++qName->msgOutIndex >= qName->qSize )
      {
         qName->msgOutIndex = 0;
      }
      INTR_ENABLE(CPU_INT8);
   }                          
   return(msgCount);
}                     
                 