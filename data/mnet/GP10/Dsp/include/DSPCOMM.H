/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* file name: dspcomm.h
* 
* Description: Globals assignment for general purpose dsp code
*
*****************************************************************************/

#define MSG_OVERFLOW_MAX_COUNT 0xffffffff

#undef GPGLOB                
#ifdef GPDEFINE
#define GPGLOB
#else
#define GPGLOB extern
#endif            
                          

#define QUE_HEAD_SIZE 12

typedef struct 
{           
  Uint32 msgOverflow;
  Int16 msgCount;
  Int16 qSize;
  Int16 qAttrb;   
  Int8 msgInIndex;
  Int8 msgOutIndex;
  msgStruc **qMsg;
}qRspStruc, qCmdStruc, qStruc; 
                            
                                            
#pragma DATA_SECTION (DspMsgNumWord, ".hpiTxMem");
GPGLOB volatile Int32 DspMsgNumWord;
 
#pragma DATA_SECTION (txMsgBuf, ".hpiTxMem");
GPGLOB Uint8 txMsgBuf[0x200];


#pragma DATA_SECTION (HostMsgNumWord, ".hpiRxMem");
GPGLOB volatile Int32 HostMsgNumWord;    

#pragma DATA_SECTION (rxMsgBuf, ".hpiRxMem");             
GPGLOB Uint8 rxMsgBuf[0x200];

/*
 * DSP HOST PORT buffer pointer
 */                           
GPGLOB volatile cmdStruc *hpiRxMsgPtr;  
GPGLOB volatile rspStruc *hpiTxMsgPtr;  
                                               
/*
 * Message queue for HPI
 */                              
GPGLOB qCmdStruc *FastHmsgQue;  
GPGLOB qCmdStruc *HmsgQue;    

GPGLOB qRspStruc *FastRspQue; 
GPGLOB qRspStruc *RspQue;  
                                 
 
GPGLOB Uint32 TimerIsrCount;


GPGLOB Uint16 DspId;