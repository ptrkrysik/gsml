/*******************************************************************************            
 (c) Copyright Cisco 2000
 All Rights Reserved
*******************************************************************************/

/*******************************************************************************            
msgL1Decode

This file contains the functins to deccode the messages received from  Layer 1. 
There is one function per message type. The caller supplies the message in a
byte array and receives the decoded message in a structure.  The memory of the 
prepared structured is managed by the caller. 
*/

#include "vxWorks.h"
#include "string.h"
#include "msgL1.h"

#if _BYTE_ORDER == _BIG_ENDIAN
#define BYTES_TO_WORD(b) *(int*)b
#else 
#define BYTES_TO_WORD(b) LONGSWAP(*(int*)b)
#endif

/*******************************************************************************            
*/
static void msgDecodeHeader
(
  char** ppMsgIn,
  MsgHeader* header
)
{
  header->function = *(*ppMsgIn)++;
  header->type = (*(*ppMsgIn)++ << 8) | *(*ppMsgIn)++;
  header->trx = *(*ppMsgIn)++;
}

/*******************************************************************************            
*/
static void msgDecodeChannel
(
  char** ppMsgIn,
  Channel* channel
)
{
  channel->typeTs = *(*ppMsgIn)++; 
  channel->ccch = *(*ppMsgIn)++;
}

/*******************************************************************************            
*/
static void msgDecodeFrameNum
(
  char** ppMsgIn,
  FrameNum* frame
)
{
  frame->t1 = (*(*ppMsgIn) >> 3) & 0x1F;
  frame->t3 = ((*(*ppMsgIn)++ << 3) & 0x1C) | ((*(*ppMsgIn) >> 5) & 0x07);
  frame->t2 = *(*ppMsgIn)++ & 0x1F;
}

/*******************************************************************************            
*/
void msgDecodePrach
(
  char* msgIn,
  MsgPrach* msgOut
)
{
  msgDecodeHeader(&msgIn, &msgOut->header);
  msgDecodeChannel(&msgIn, &msgOut->channel);
  msgDecodeFrameNum(&msgIn, &msgOut->frameNum);  
  msgOut->packetAccessRef = (*msgIn++ << 8) | (*msgIn++ & 0x0F);
  msgOut->ta = *msgIn++;
  msgOut->power = *msgIn;
}

/*******************************************************************************            
*/
void msgDecodePacch
(
  char* msgIn,
  MsgPacchInd* msgOut
)
{
  msgDecodeHeader(&msgIn, &msgOut->header);
  msgDecodeChannel(&msgIn, &msgOut->channel);
  msgOut->tbf = *msgIn++;
  bcopy(msgIn, msgOut->data, MAX_RLC_CONTROL_MSG);
}

/*******************************************************************************            
*/
void msgDecodePdtch
(
  char* msgIn,
  MsgPdtchInd* msgOut,
  int msgSize
)
{
  msgDecodeHeader(&msgIn, &msgOut->header);
  msgDecodeChannel(&msgIn, &msgOut->channel);
  msgOut->codeRate = *msgIn++;
  bcopy(msgIn, msgOut->data, msgSize - DSP_CHAN_IE_SIZE + DSP_MSG_HDR_SIZE - 
    DSP_CODE_RATE_SIZE);
}

/*******************************************************************************            
*/
void msgDecodeUlPowerTiming
(
  char* msgIn,
  MsgUlPowerTiming* msgOut
)
{
  msgDecodeHeader(&msgIn, &msgOut->header);
  msgDecodeChannel(&msgIn, &msgOut->channel);
  msgOut->groupTfi = *msgIn;
}

/*******************************************************************************            
*/
void msgDecodeUlTbfActivationAck
(
  char* msgIn,
  MsgUlTbfActivationAck* msgOut
)
{
  msgDecodeHeader(&msgIn, &msgOut->header);
  msgOut->tlli = BYTES_TO_WORD(msgIn);
}

/*******************************************************************************            
*/
static void msgDecodeCommon
(
  char* msgIn,
  MsgCommon* msgOut
)
{
  msgDecodeHeader(&msgIn, &msgOut->header);
  msgOut->tlli = BYTES_TO_WORD(msgIn);
}

/*******************************************************************************            
*/
void msgDecodeUlTbfDeactivationAck
(
  char* msgIn,
  MsgUlTbfDeactivationAck* msgOut
)
{
  msgDecodeCommon(msgIn, (MsgCommon *)msgOut);
}

/*******************************************************************************            
*/
void msgDecodeDlTbfActivationAck
(
  char* msgIn,
  MsgDlTbfActivationAck* msgOut
)
{
  msgDecodeCommon(msgIn, (MsgCommon *)msgOut);
}

/*******************************************************************************            
*/
void msgDecodeDlTbfDeactivationAck
(
  char* msgIn,
  MsgDlTbfDeactivationAck* msgOut
)
{
  msgDecodeCommon(msgIn, (MsgCommon *)msgOut);
}

/*******************************************************************************            
*/
void msgDecodeUlTbfReconfigAck
(
  char* msgIn,
  MsgUlTbfReconfigAck* msgOut
)
{
  msgDecodeCommon(msgIn, (MsgCommon *)msgOut);
}

/*******************************************************************************            
*/
void msgDecodeDlTbfReconfigAck
(
  char* msgIn,
  MsgDlTbfReconfigAck* msgOut
)
{
  msgDecodeCommon(msgIn, (MsgCommon *)msgOut);
}

/*******************************************************************************            
*/
void msgDecodeReadyToSend
(
  char* msgIn,
  MsgReadyToSend* msgOut
)
{
  msgDecodeHeader(&msgIn, &msgOut->header);
  msgDecodeChannel(&msgIn, &msgOut->channel);
  msgOut->link = *msgIn++;
  msgOut->numBuffs = *msgIn;
}

/*******************************************************************************            
*/
void msgDecodePacchNack
(
  char* msgIn,
  MsgPacchNack* msgOut
)
{
  msgDecodeHeader(&msgIn, &msgOut->header);
  msgDecodeChannel(&msgIn, &msgOut->channel);
  msgOut->tbf = *msgIn;
}


/*******************************************************************************            
*/
void msgDecodeSingleBlockAssignAlert
(
  char* msgIn,
  MsgSingleBlockAssignAlert* msgOut
)
{
  msgDecodeHeader(&msgIn, &msgOut->header);
  msgOut->ts = *msgIn++;
  msgOut->frameNum = BYTES_TO_WORD(msgIn);
  msgIn += 4;
  msgOut->ARFCN = 0;
  msgOut->ARFCN |= (*msgIn++ << 8);
  msgOut->ARFCN |= *msgIn;
}


/*******************************************************************************            
*/
void msgDecodeDLAssignAlert
(
  char* msgIn,
  MsgDLAssignAlert* msgOut
)
{
  msgDecodeHeader(&msgIn, &msgOut->header);
  msgOut->ts = *msgIn++;
  msgOut->frameNum = BYTES_TO_WORD(msgIn);
  msgIn += 4;
  msgOut->ARFCN = 0;
  msgOut->ARFCN |= (*msgIn++ << 8);
  msgOut->ARFCN |= *msgIn++;
  msgOut->tag = *msgIn;
}


/*******************************************************************************            
*/
void msgDecodePacchAbInd
(
  char* msgIn,
  MsgPacchAbInd* msgOut
)
{
  msgDecodeHeader(&msgIn, &msgOut->header);
  msgDecodeChannel(&msgIn, &msgOut->channel);
  msgOut->tag = *msgIn++;
  msgOut->data[0] = *msgIn++;
  msgOut->data[1] = *msgIn++ << 5;
  msgOut->ta = *msgIn;
}


/*******************************************************************************            
*/
void msgDecodeCodeRateReq
(
  char* msgIn,
  MsgCodeRateReq* msgOut
)
{
  msgDecodeHeader(&msgIn, &msgOut->header);
  msgOut->tbfType = *msgIn++;
  msgOut->groupTfi = *msgIn++;
  msgOut->codeRate = *msgIn;
}
