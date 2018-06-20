/*******************************************************************************            
 (c) Copyright Cisco 2000
 All Rights Reserved
*******************************************************************************/

/*******************************************************************************            
msgL1Encode

This file contains the functins to encode the messages to be sent to Layer 1. 
There is one function per message type. The caller supplies the message in a
structure and receives the encoded byte array. The memory of the prepared byte 
array is managed by the caller. 
*/

#include "vxworks.h" 
#include "string.h"
#include "msgL1.h"

#if _BYTE_ORDER == _BIG_ENDIAN
#define WORD_TO_BYTES(w,b) *(int*)b=w
#else
#define WORD_TO_BYTES(w,b) LONGSWAP(w),*(int*)b=w
#endif

/*******************************************************************************            
*/
static /*inline*/ void msgEncodeHeader
(
  MsgHeader* msgHeader,
  char** ppMsg
) 
{
  *(*ppMsg)++ = msgHeader->function;
  *(*ppMsg)++ = (msgHeader->type) >> 8;
  *(*ppMsg)++ = (msgHeader->type) & 0xFF;
  *(*ppMsg)++ = msgHeader->trx;
}

/*******************************************************************************            
*/
static /*inline*/ void taiEncode
(
  Tai*   taiIn,
  char** ppTaiOut
)
{
  *(*ppTaiOut)++ = taiIn->ts;
  *(*ppTaiOut)++ = taiIn->tai; 
}

/*******************************************************************************            
*/
static /*inline*/ void timeEncode
(
  Time*   timeIn,
  char**  ppTimeOut
)
{
  *(*ppTimeOut)++ = timeIn->imm;
  *(*ppTimeOut)++ = ((timeIn->frameNum.t1) << 3) | (((timeIn->frameNum.t3) >> 3) & 0x07);
  *(*ppTimeOut)++ = ((timeIn->frameNum.t3) << 5) | ((timeIn->frameNum.t2) & 0x1F);
}

/*******************************************************************************            
*/
int msgEncodePtcch
(
  MsgPtcch* msgIn, 
  char* msg 
)
{
  const int MsgSize = MAX_RLC_CONTROL_MSG + DSP_CHAN_IE_SIZE + DSP_MSG_HDR_SIZE;

  msgEncodeHeader(&msgIn->header, &msg);
  *msg++ = msgIn->channel.typeTs;
  *msg++ = msgIn->channel.ccch;
  bcopy(msgIn->data, msg, MAX_RLC_CONTROL_MSG);
  return MsgSize;
}

/*******************************************************************************            
*/
int msgEncodePacch
(
  MsgPacchReq* msgIn, 
  char* msg 
)
{
  const int MsgSize = MAX_RLC_CONTROL_MSG + DSP_CHAN_IE_SIZE + DSP_MSG_HDR_SIZE + 
    DSP_TAG_SIZE + DSP_BURST_TYPE_SIZE;

  msgEncodeHeader(&msgIn->header, &msg);
  *msg++ = msgIn->channel.typeTs;
  *msg++ = msgIn->channel.ccch;
  *msg++ = msgIn->tbf;
  *msg++ = msgIn->burstType;
  bcopy(msgIn->data, msg, MAX_RLC_CONTROL_MSG);
  return MsgSize;
}

/*******************************************************************************            
*/
int msgEncodePdtch
(
  MsgPdtchReq* msgIn, 
  char* msg,
  int dataBlkSize   
)
{
  const int MsgSize = DSP_CHAN_IE_SIZE + DSP_MSG_HDR_SIZE + DSP_CODE_RATE_SIZE + 
    DSP_TAG_SIZE + DSP_BURST_TYPE_SIZE + dataBlkSize;

  msgEncodeHeader(&msgIn->header, &msg);
  *msg++ = msgIn->channel.typeTs;
  *msg++ = msgIn->channel.ccch;
  *msg++ = msgIn->tbf;
  *msg++ = msgIn->burstType;
  *msg++ = msgIn->codeRate;
  bcopy(msgIn->data, msg, dataBlkSize);
  return MsgSize;
}

/*******************************************************************************            
*/
int msgEncodeUlTbfActivation
(
  MsgUlTbfActivation* msgIn, 
  char* msg
)
{
  const int MsgSize = 18;

  msgEncodeHeader(&msgIn->header, &msg);
  WORD_TO_BYTES(msgIn->tlli, msg);  
  msg += 4;
  *msg++ = msgIn->tbfMode;
  *msg++ = msgIn->groupTfi;
  *msg++ = msgIn->channel;
  *msg++ = msgIn->tsAllocation;
  WORD_TO_BYTES(msgIn->usfTsMap, msg); 
  msg += 4;
  taiEncode(&msgIn->tai, &msg);
  /* timeEncode(&msgIn->startTime, &msg); */
  /* USF granularity */
  return MsgSize;
}   

/*******************************************************************************            
*/
int msgEncodeUlTbfDeactivation
(
  MsgUlTbfDeactivation* msgIn,
  char* msg
)
{
  const int MsgSize = 8;

  msgEncodeHeader(&msgIn->header, &msg);
  WORD_TO_BYTES(msgIn->tlli, msg);  
  return MsgSize;
}

/*******************************************************************************            
*/
int msgEncodeDlTbfActivation
(
  MsgDlTbfActivation* msgIn, 
  char* msg 
)
{
  const int MsgSize = 13;

  msgEncodeHeader(&msgIn->header, &msg);
  WORD_TO_BYTES(msgIn->tlli, msg);  
  msg += 4;
  *msg++ = msgIn->groupTfi;
  *msg++ = msgIn->channel;
  *msg++ = msgIn->tsAllocation;
  *msg++ = msgIn->msPower;
  *msg++ = msgIn->ta;
  /* timeEncode(&msgIn->startTime, &msg); */
  return MsgSize;
}

/*******************************************************************************            
*/
int msgEncodeDlTbfDeactivation
(
  MsgDlTbfDeactivation* msgIn,
  char* msg
)
{
  const int MsgSize = 8;

  msgEncodeHeader(&msgIn->header, &msg);
  WORD_TO_BYTES(msgIn->tlli, msg);  
  return MsgSize;
}

/*******************************************************************************            
*/
int msgEncodeUlTbfReconfig
(
  MsgUlTbfReconfig* msgIn, 
  char* msg
)
{
  const int MsgSize = 18;

  msgEncodeHeader(&msgIn->header, &msg);
  WORD_TO_BYTES(msgIn->tlli, msg);  
  msg += 4;
  *msg++ = msgIn->groupTfi;
  *msg++ = msgIn->channel;
  *msg++ = msgIn->tbfMode;
  *msg++ = msgIn->tsAllocation;
  WORD_TO_BYTES(msgIn->usfTsMap, msg); 
  msg += 4;
  taiEncode(&msgIn->tai, &msg);
  /* timeEncode(&msgIn->startTime, &msg); */
  return MsgSize;
}   

/*******************************************************************************            
*/
int msgEncodeDlTbfReconfig
(
  MsgDlTbfReconfig* msgIn, 
  char* msg 
)
{
  const int MsgSize = 11;

  msgEncodeHeader(&msgIn->header, &msg);
  WORD_TO_BYTES(msgIn->tlli, msg);  
  msg += 4;
  *msg++ = msgIn->groupTfi;
  *msg++ = msgIn->channel;
  *msg++ = msgIn->tsAllocation;
  /* timeEncode(&msgIn->startTime, &msg); */
  return MsgSize;
}


/*******************************************************************************            
*/
int msgEncodeSingleBlockAssignAlertAck
(
  MsgSingleBlockAssignAlertAck* msgIn, 
  char* msg 
)
{
  const int MsgSize = 10;

  msgEncodeHeader(&msgIn->header, &msg);
  *msg++ = msgIn->channel.typeTs;
  *msg++ = msgIn->channel.ccch;
  WORD_TO_BYTES(msgIn->frameNum, msg);  
  return MsgSize;
}


/*******************************************************************************            
*/
int msgEncodeDLAssignAlertAck
(
  MsgDLAssignAlertAck* msgIn, 
  char* msg 
)
{
  const int MsgSize = 11;

  msgEncodeHeader(&msgIn->header, &msg);
  *msg++ = msgIn->channel.typeTs;
  *msg++ = msgIn->channel.ccch;
  WORD_TO_BYTES(msgIn->frameNum, msg);
  msg+=4;
  *msg = msgIn->tag;  
  return MsgSize;
}


/*******************************************************************************            
*/
int msgEncodeDLAckNackInfo
(
  MsgDLAckNackInfo* msgIn, 
  char* msg 
)
{
  const int MsgSize = 20;

  msgEncodeHeader(&msgIn->header, &msg);
  WORD_TO_BYTES(msgIn->tlli, msg);  
  msg += 4;
  *msg++ = msgIn->groupTfi;
  *msg++ = msgIn->cValue;
  *msg++ = msgIn->rxqual;
  *msg++ = msgIn->iLevelMask;
  for (int i=0; i<8; i++)
    *msg++ = msgIn->iLevelByTs[i];
  return MsgSize;
}
