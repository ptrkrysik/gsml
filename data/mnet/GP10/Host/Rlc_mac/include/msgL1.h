/*******************************************************************************            
 (c) Copyright Cisco 2000
 All Rights Reserved
*******************************************************************************/

 
/*******************************************************************************            
msgL1

Structure and function interface definitions for encoding and decoding of
messages between RLC/MAC and layer 1.
*/

#ifndef __MSGL1_H__
#define __MSGL1_H__

#include "RlcMacCommon.h"

#define MAX_RLC_CONTROL_MSG     23
#define MAX_RLC_DATA_MSG        53
#define DSP_MSG_HDR_SIZE        4
#define DSP_CHAN_IE_SIZE        2
#define DSP_CODE_RATE_SIZE      1
#define DSP_TAG_SIZE            1
#define DSP_BURST_TYPE_SIZE     1

/*----- Structure definitions for layer 1 messages -----*/

/*----- Sub-structures -----*/
typedef struct  
{
  char function;
  short int type;
  char trx;
} MsgHeader;

typedef struct
{
  char typeTs;
  char ccch;
} Channel;

typedef struct
{
  char ts;
  char tai;
} Tai;

typedef struct
{
  char t1;
  char t2;
  char t3;  
} FrameNum;

typedef struct
{
  char imm;
  FrameNum frameNum;
} Time;


/* 
Structures for encoding messages from RLC/MAC to Layer 1 
*/

typedef struct 
{
  MsgHeader header;
  Channel   channel;
  char      tbf;
  char      burstType;
  char      data[MAX_RLC_CONTROL_MSG];
} MsgPacchReq;

typedef struct
{
  MsgHeader header;
  Channel   channel;
  char      tbf;
  char      burstType;
  char      codeRate;
  char      data[MAX_RLC_DATA_MSG];
} MsgPdtchReq;

typedef struct 
{
  MsgHeader header;
  Channel channel;
  char    data[MAX_RLC_CONTROL_MSG];
} MsgPtcch;

typedef struct
{
  MsgHeader header;
  int tlli;
  char tbfMode;
  char groupTfi;
  int channel;
  int tsAllocation;
  int usfTsMap;
  Tai tai;
  Time startTime;
  /* USF granularity */
} MsgUlTbfActivation;

typedef struct
{
  MsgHeader header;
  int tlli;
} MsgUlTbfDeactivation, MsgDlTbfDeactivation;;

typedef struct
{
  MsgHeader header;
  int tlli;
  char groupTfi;
  int channel;
  int tsAllocation;
  int msPower;
  int ta;
  Time startTime;
} MsgDlTbfActivation;

typedef struct
{
  MsgHeader header;
  int tlli;
  char groupTfi;
  int channel;
  char tbfMode;
  int tsAllocation;
  int usfTsMap;
  Tai tai;
  Time startTime;
} MsgUlTbfReconfig;

typedef struct
{
  MsgHeader header;
  int tlli;
  char groupTfi;
  int channel;
  int tsAllocation;
  int msPower;
  Time startTime;
} MsgDlTbfReconfig;

typedef struct
{
  MsgHeader header;
  Channel channel;
  unsigned int frameNum;
} MsgSingleBlockAssignAlertAck;

typedef struct
{
  MsgHeader header;
  Channel channel;
  unsigned int frameNum;
  char tag;
} MsgDLAssignAlertAck;

typedef struct
{
  MsgHeader header;
  int tlli;
  char groupTfi;
  char cValue;
  char rxqual;
  char iLevelMask;
  char iLevelByTs[MAX_TIMESLOTS];
} MsgDLAckNackInfo;


/*
Structures for decoding messages from Layer 1 to RLC/MAC 
*/

typedef struct 
{
  MsgHeader header;
  Channel   channel;
  char      tbf;
  char      data[MAX_RLC_CONTROL_MSG];
} MsgPacchInd;

typedef struct
{
  MsgHeader header;
  Channel   channel;
  char      codeRate;
  char      data[MAX_RLC_DATA_MSG];
} MsgPdtchInd;

typedef struct
{
  MsgHeader header;
  Channel channel;
  FrameNum frameNum;
  short int packetAccessRef;
  char ta;
  char power;
} MsgPrach;

typedef struct
{
  MsgHeader header;
  Channel channel;
  char groupTfi;
  /* Global Power Control IE */
  /* Power Control IE */
  /* Global Packet Timing Adv. */
} MsgUlPowerTiming;

typedef struct
{
  MsgHeader header;
  int tlli;
} MsgUlTbfActivationAck, MsgUlTbfDeactivationAck, 
  MsgDlTbfActivationAck, MsgDlTbfDeactivationAck,
  MsgUlTbfReconfigAck, MsgDlTbfReconfigAck,
  MsgCommon;

typedef struct
{
  MsgHeader header;
  Channel channel;
  char link;  /* link identifier */
  char numBuffs;  /* number of buffers ready */
} MsgReadyToSend;

typedef struct
{
  MsgHeader header;
  Channel channel;
  char tbf;
} MsgPacchNack;

typedef struct
{
  MsgHeader header;
  char ts;
  unsigned int frameNum;
  short ARFCN;
} MsgSingleBlockAssignAlert;

typedef struct
{
  MsgHeader header;
  char ts;
  unsigned int frameNum;
  short ARFCN;
  char tag;
} MsgDLAssignAlert;

typedef struct
{
  MsgHeader header;
  Channel channel;
  char tag;
  char data[2];
  char ta;
} MsgPacchAbInd;

typedef struct
{
  MsgHeader header;
  char tbfType;
  char groupTfi;
  char codeRate;
} MsgCodeRateReq;

/*----- Functions to encode messages to Layer 1 -----*/

/*******************************************************************************            
*/
int msgEncodePtcch
(
  MsgPtcch* msgIn, 
  char* msg 
);

/*******************************************************************************            
*/
int msgEncodePacch
(
  MsgPacchReq* msgIn, 
  char* msg 
);

/*******************************************************************************            
*/
int msgEncodePdtch
(
  MsgPdtchReq* msgIn, 
  char* msg,
  int dataBlkSize 
);

/*******************************************************************************            
*/
int msgEncodeUlTbfActivation
(
  MsgUlTbfActivation* msgIn, 
  char* msg
);

/*******************************************************************************            
*/
int msgEncodeUlTbfDeactivation
(
  MsgUlTbfDeactivation* msgIn,
  char* msg
);

/*******************************************************************************            
*/
int msgEncodeDlTbfActivation
(
  MsgDlTbfActivation* msgIn, 
  char* msg 
);

/*******************************************************************************            
*/
int msgEncodeDlTbfDeactivation
(
  MsgDlTbfDeactivation* msgIn,
  char* msg
);

/*******************************************************************************            
*/
int msgEncodeUlTbfReconfig
(
  MsgUlTbfReconfig* msgIn, 
  char* msg
);

/*******************************************************************************            
*/
int msgEncodeDlTbfReconfig
(
  MsgDlTbfReconfig* msgIn, 
  char* msg 
);

/*******************************************************************************            
*/
int msgEncodeSingleBlockAssignAlertAck
(
  MsgSingleBlockAssignAlertAck* msgIn, 
  char* msg 
);

/*******************************************************************************            
*/
int msgEncodeDLAssignAlertAck
(
  MsgDLAssignAlertAck* msgIn, 
  char* msg 
);

/*******************************************************************************            
*/
int msgEncodeDLAckNackInfo
(
  MsgDLAckNackInfo* msgIn, 
  char* msg 
);


/*----- Functions to decode messages from Layer 1 -----*/

/*******************************************************************************            
*/
void msgDecodePrach
(
  char* msgIn,
  MsgPrach* msgOut
);

/*******************************************************************************            
*/
void msgDecodePacch
(
  char* msgIn,
  MsgPacchInd* msgOut
);

/*******************************************************************************            
*/
void msgDecodePdtch
(
  char* msgIn,
  MsgPdtchInd* msgOut,
  int msgSize        /* size of the messege received from layer 1 */
);

/*******************************************************************************            
*/
void msgDecodeUlPowerTiming
(
  char* msgIn,
  MsgUlPowerTiming* msgOut
);

/*******************************************************************************            
*/
void msgDecodeUlTbfActivationAck
(
  char* msgIn,
  MsgUlTbfActivationAck* msgOut
);

/*******************************************************************************            
*/
void msgDecodeUlTbfDeactivationAck
(
  char* msgIn,
  MsgUlTbfDeactivationAck* msgOut
);

/*******************************************************************************            
*/
void msgDecodeDlTbfActivationAck
(
  char* msgIn,
  MsgDlTbfActivationAck* msgOut
);

/*******************************************************************************            
*/
void msgDecodeDlTbfDeactivationAck
(
  char* msgIn,
  MsgDlTbfDeactivationAck* msgOut
);

/*******************************************************************************            
*/
void msgDecodeUlTbfReconfigAck
(
  char* msgIn,
  MsgUlTbfReconfigAck* msgOut
);

/*******************************************************************************            
*/
void msgDecodeDlTbfReconfigAck
(
  char* msgIn,
  MsgDlTbfReconfigAck* msgOut
);

/*******************************************************************************            
*/
void msgDecodeReadyToSend
(
  char* msgIn,
  MsgReadyToSend* msgOut
);

/*******************************************************************************            
*/
void msgDecodePacchNack
(
  char* msgIn,
  MsgPacchNack* msgOut
);

/*******************************************************************************            
*/
void msgDecodeSingleBlockAssignAlert
(
  char* msgIn,
  MsgSingleBlockAssignAlert* msgOut
);

/*******************************************************************************            
*/
void msgDecodeDLAssignAlert
(
  char* msgIn,
  MsgDLAssignAlert* msgOut
);

/*******************************************************************************            
*/
void msgDecodePacchAbInd
(
  char* msgIn,
  MsgPacchAbInd* msgOut
);

/*******************************************************************************            
*/
void msgDecodeCodeRateReq
(
  char* msgIn,
  MsgCodeRateReq* msgOut
);

#endif
