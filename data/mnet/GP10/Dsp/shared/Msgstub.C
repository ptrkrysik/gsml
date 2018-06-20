/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* file name: msg_test.c               
*
*****************************************************************************/
#include "stdlib.h" 
#include "string.h" 
#include "gsmdata.h"
#include "dsp/dsphmsg.h"
#include "dsp6201.h"                 
#include "comdata.h"                                      

Void send_test_msg(Void)
{                 
  volatile msgUnionStruc *TestMsg; 
  ccchInfoMsgStruc ccchMsg;
  Uint16 ts;
  Uint16 chType;
  
  /*
  * maps test message to HPI message buf
  */

  TestMsg = (msgUnionStruc *)hpiRxMsgPtr;
/*                       
  TestMsg->SyncWord = 0xAAAA;            
  TestMsg->tag = 0x1234; 
*/    
  TestMsg->trx = 0;   
  TestMsg->function = DIAGNOSTIC_MSG;     
  TestMsg->typeLoByte = MPH_DSP_SET_ID_REQ;
  TestMsg->typeHiByte = MPH_DSP_SET_ID_REQ>>8;  
  TestMsg->msgSize = MSG_HEAD_SIZE + 2;
  TestMsg->u.params[0] = 0x10;
  TestMsg->u.params[1] =  2;
   
  TestMsg += 1;                           
  TestMsg->trx = 0;   
  TestMsg->function = DIAGNOSTIC_MSG;       
  TestMsg->typeLoByte = MPH_DSP_DIAG_PING_REQ;
  TestMsg->typeHiByte = MPH_DSP_DIAG_PING_REQ>>8;
  
  TestMsg->msgSize = MSG_HEAD_SIZE + 2;  
  TestMsg->u.params[0] = 0;
  TestMsg->u.params[1] =  2;
 
  HostMsgNumWord = 2;                    
                        
  TestMsg += 1;
  memset((Uint8 *) TestMsg, 0, sizeof(msgStruc));
    
  TestMsg->trx = 0;                              
  TestMsg->function = CCCH_MANAGE_MSG;       
  TestMsg->typeLoByte = BCCH_INFO_REQ;
  TestMsg->typeHiByte = BCCH_INFO_REQ >> 8;  
  TestMsg->msgSize = MSG_HEAD_SIZE + sizeof(ccchInfoMsgStruc);

  ts = 0;
  chType = 0x10;
  TestMsg->u.ccchInfo.chanNum = (ts<<5) | chType;
  
  TestMsg->u.ccchInfo.msgSize = 23;
  
  TestMsg->u.ccchInfo.sysInfoType = SYS_INFO_2; 
  
  
  TestMsg->u.ccchInfo.sysInfo[0] = 0x59; /* L2 pseudo length */ 
  TestMsg->u.ccchInfo.sysInfo[1] = 0x06; 
  
  TestMsg->u.ccchInfo.sysInfo[2] = 0x1A; /* message type */ 
  
  TestMsg->u.ccchInfo.sysInfo[3] = 0x8E;   
  
  TestMsg->u.ccchInfo.sysInfo[19] = 0x01;
                                                                                                  
                                                                                                  
  TestMsg->u.ccchInfo.sysInfo[20] = 0x01;
  TestMsg->u.ccchInfo.sysInfo[21] = 0x04;
  TestMsg->u.ccchInfo.sysInfo[22] = 0x00;
        
  TestMsg->u.ccchInfo.startTime.IMM = 1;
/*  
22 00 00 00 0c 00 11 00 80 00 02 17 16 06 1a 8e 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 
01 04 08 01 00 00 ee ee ee ee ee ee ee ee ee ee 
ee ee ee ee ee ee ee ee ee ee ee ee ac 2c 16 00   
*/                           
  HostMsgNumWord += 1;

  
  TestMsg += 1;
  memset((Uint8 *) TestMsg, 0, sizeof(msgStruc));

  TestMsg->trx = 0;                              
  TestMsg->function = CCCH_MANAGE_MSG;       
  
  TestMsg->typeLoByte = BCCH_INFO_REQ;
  TestMsg->typeHiByte = BCCH_INFO_REQ >> 8;  
    
  TestMsg->msgSize = MSG_HEAD_SIZE + sizeof(ccchInfoMsgStruc);

  ts = 0;
  chType = 0x10;
  TestMsg->u.ccchInfo.chanNum = (ts<<5) | chType;
  
  TestMsg->u.ccchInfo.msgSize = 23;
  
  TestMsg->u.ccchInfo.sysInfoType = SYS_INFO_3; 
  
  TestMsg->u.ccchInfo.sysInfo[0] = 0x49; /* L2 pseudo length */ 
  TestMsg->u.ccchInfo.sysInfo[1] = 0x06; 
  
  TestMsg->u.ccchInfo.sysInfo[2] = 0x1B; /* message type */
  
  TestMsg->u.ccchInfo.sysInfo[3] = 0x00; /* Cell Identity */
  TestMsg->u.ccchInfo.sysInfo[4] = 0x01;
         
  
  TestMsg->u.ccchInfo.sysInfo[5] = 0x13; /* MCC */
  TestMsg->u.ccchInfo.sysInfo[6] = 0xF0;
  
  
  TestMsg->u.ccchInfo.sysInfo[7] = 0x71; /* MNC */
  
  TestMsg->u.ccchInfo.sysInfo[8] = 0x00; /* Loaction area identify */
  TestMsg->u.ccchInfo.sysInfo[9] = 0x01;
    
  
  TestMsg->u.ccchInfo.sysInfo[10] = 0x01; /* Control Channel Description */
  TestMsg->u.ccchInfo.sysInfo[11] = 0x02; /* Num Paging Group */
  TestMsg->u.ccchInfo.sysInfo[12] = 0x00;      
  TestMsg->u.ccchInfo.sysInfo[13] = 0x23;
  TestMsg->u.ccchInfo.sysInfo[14] = 0x07; 
  
  TestMsg->u.ccchInfo.sysInfo[16] = 0x01;
  TestMsg->u.ccchInfo.sysInfo[17] = 0x04;
  
  TestMsg->u.ccchInfo.sysInfo[19] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[20] = 0x2b;    
  TestMsg->u.ccchInfo.sysInfo[21] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[22] = 0x2b;    
    
  
  TestMsg->u.ccchInfo.startTime.IMM = 1;

/* 
   22 00 00 00 0c 00 11 00 80 00 03 17 12 06 1b 00 
   01 13 f0 71 00 01 01 02 00 23 07 00 01 04 00 2b 
   2b 2b 2b 01 00 00 ee ee ee ee ee ee ee ee ee ee 
*/   
  HostMsgNumWord += 1;                  
  
                            
  TestMsg += 1;
  memset((Uint8 *) TestMsg, 0, sizeof(msgStruc));
    
  TestMsg->trx = 0;                              
  TestMsg->function = CCCH_MANAGE_MSG;       
  TestMsg->typeLoByte = BCCH_INFO_REQ;
  TestMsg->typeHiByte = BCCH_INFO_REQ >> 8;  
  TestMsg->msgSize = MSG_HEAD_SIZE + sizeof(ccchInfoMsgStruc);

  ts = 0;
  chType = 0x10;
  TestMsg->u.ccchInfo.chanNum = (ts<<5) | chType;
  
  TestMsg->u.ccchInfo.msgSize = 23;
  
  TestMsg->u.ccchInfo.sysInfoType = SYS_INFO_4; 
  
  TestMsg->u.ccchInfo.sysInfo[0] = 0x49;  /* L2 pseudo length */ 
  TestMsg->u.ccchInfo.sysInfo[1] = 0x06; 
  
  TestMsg->u.ccchInfo.sysInfo[2] = 0x1C;  /* message type */

  
  TestMsg->u.ccchInfo.sysInfo[3] = 0x13;  /* MCC */
  TestMsg->u.ccchInfo.sysInfo[4] = 0xF0;
         
  
  TestMsg->u.ccchInfo.sysInfo[5] = 0x71;
  TestMsg->u.ccchInfo.sysInfo[6] = 0x00;
  
  
  TestMsg->u.ccchInfo.sysInfo[7] = 0x01; /* MNC */
  TestMsg->u.ccchInfo.sysInfo[8] = 07;

  TestMsg->u.ccchInfo.sysInfo[10] = 01;
  TestMsg->u.ccchInfo.sysInfo[11] = 04;

  TestMsg->u.ccchInfo.sysInfo[13] = 0x2b;  
  TestMsg->u.ccchInfo.sysInfo[14] = 0x2b;  
  TestMsg->u.ccchInfo.sysInfo[15] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[16] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[17] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[18] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[19] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[20] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[21] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[22] = 0x2b;              
      
  TestMsg->u.ccchInfo.startTime.IMM = 1;
/*
   22 00 00 00 0c 00 11 00 80 00 04 17 0c 06 1c 13 
   f0 71 00 01 07 00 01 06 00 2b 2b 2b 2b 2b 2b 2b 
   2b 2b 2b 01 00 00                           
*/
  HostMsgNumWord += 1;

 
  TestMsg += 1;
  memset((Uint8 *) TestMsg, 0, sizeof(msgStruc));

  TestMsg->trx = 0; 
  
  TestMsg->function = DCCH_MANAGE_MSG;       
  
  TestMsg->typeLoByte = DCH_CH_ACTIVATION_REQ;
  TestMsg->typeHiByte = DCH_CH_ACTIVATION_REQ >> 8;  
  TestMsg->msgSize = MSG_HEAD_SIZE + sizeof(chActMsgStruc);
  
  ts = 1;
  chType = 1;
  TestMsg->u.ccchInfo.chanNum = (ts<<5) | chType;

  TestMsg->u.chActInfo.chModes.dtxSelect = 0; 
  TestMsg->u.chActInfo.chModes.chSelect = 1;  	/* select Speech */
  TestMsg->u.chActInfo.chModes.chType = 0x08;   /* TCHF */
  TestMsg->u.chActInfo.chModes.dataRate = 0x01; /* GSM Speech Codec 1 */
  
  HostMsgNumWord += 1;    


  TestMsg += 1;
  memset((Uint8 *) TestMsg, 0, sizeof(msgStruc));

  TestMsg->trx = 0; 
  
  TestMsg->function = TRX_MANAGE_MSG;       
  
  TestMsg->typeLoByte = DCH_SACCH_INFO_MOD_REQ;
  TestMsg->typeHiByte = DCH_SACCH_INFO_MOD_REQ >> 8;  
  TestMsg->msgSize = MSG_HEAD_SIZE + 24;
  
  TestMsg->u.params[0] = SYS_INFO_5;

  TestMsg->u.ccchInfo.sysInfo[0] = 0x49;  /* L2 pseudo length */ 
  TestMsg->u.ccchInfo.sysInfo[1] = 0x06; 
  
  TestMsg->u.ccchInfo.sysInfo[2] = 0x1D;  /* message type */

  TestMsg->u.ccchInfo.sysInfo[3] = 0x8E;  /* MCC */

  TestMsg->u.ccchInfo.sysInfo[12] = 0x01;
  
  TestMsg->u.ccchInfo.sysInfo[15] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[16] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[17] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[18] = 0x2b;              
        
  TestMsg->u.params[21] = 1;  
/*
 1b 00 00 00 10 00 1a 00 05 12 06 1d 8e 00 00 00 
 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 2b 
 2b 2b 2b 01 00 00  
*/ 
  HostMsgNumWord += 1;    
                           

  TestMsg += 1;
  memset((Uint8 *) TestMsg, 0, sizeof(msgStruc));

  TestMsg->trx = 0; 
  
  TestMsg->function = TRX_MANAGE_MSG;       
  
  TestMsg->typeLoByte = DCH_SACCH_INFO_MOD_REQ;
  TestMsg->typeHiByte = DCH_SACCH_INFO_MOD_REQ >> 8;  
  TestMsg->msgSize = MSG_HEAD_SIZE + 24;
  
  TestMsg->u.params[0] = SYS_INFO_6;

  TestMsg->u.ccchInfo.sysInfo[0] = 0x49;  /* L2 pseudo length */ 
  TestMsg->u.ccchInfo.sysInfo[1] = 0x06; 
  
  TestMsg->u.ccchInfo.sysInfo[2] = 0x1E;  /* message type */

  TestMsg->u.ccchInfo.sysInfo[3] = 0x00; 
  TestMsg->u.ccchInfo.sysInfo[4] = 0x01;
  
  TestMsg->u.ccchInfo.sysInfo[5] = 0x13;
  TestMsg->u.ccchInfo.sysInfo[6] = 0xF0;
  
  
  TestMsg->u.ccchInfo.sysInfo[7] = 0x71; /* MNC */
  TestMsg->u.ccchInfo.sysInfo[8] = 00;

  TestMsg->u.ccchInfo.sysInfo[9] = 01;
  TestMsg->u.ccchInfo.sysInfo[10] = 23;

  TestMsg->u.ccchInfo.sysInfo[11] = 0x01;
  
  TestMsg->u.ccchInfo.sysInfo[12] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[13] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[14] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[15] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[16] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[17] = 0x2b;
  TestMsg->u.ccchInfo.sysInfo[18] = 0x2b;              
                                                
  TestMsg->u.params[21] = 1;  
/*
 1b 00 00 00 10 00 1a 00 06 12 06 1e 00 01 13 f0 
 71 00 01 23 01 2b 2b 2b 2b 2b 2b 2b 01 00 00 
*/ 
  HostMsgNumWord += 1;    
                                                                                 
}                     

