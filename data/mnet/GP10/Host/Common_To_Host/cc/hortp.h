#ifndef HORTP_HDR_INC
#define HORTP_HDR_INC

// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <vxworks.h>
#include "voip/jetrtp.h"
#include "csunew/csu_intf.h"

#define HANDOVER_RTP_POOL_BEGIN			18000
#define MAX_HANDOVER_CHANNELS			12


typedef struct {
	HJCRTPSESSION	RtpHandle;
	BOOL			Used;
	short			LocalPort;
} HANDOVER_CHANNEL, *PHANDOVER_CHANNEL;


BOOL InitializeHandoverRtpPool(void);
HJCRTPSESSION AssignHandoverRtpChannel(void);
void ReleaseHandoverRtpChannel(HJCRTPSESSION RtpHandle);
UINT16	GetIncomingHandoverRtpPort(HJCRTPSESSION RtpHandle);
void SetRemoteHandoverRtpPort(HJCRTPSESSION RtpHandle,UINT32 ip,UINT16 port);
RTP_READ_STATUS  HoRtpEventHandler(HJCRTPSESSION RtpHandle,void *Context);
void HoRtpFastReadHandler(HJCRTPSESSION RtpHandle,unsigned char *newPacket,int packetLength,rtpParam RtpParam,void *Context);

#endif /* #ifndef HORTP_HDR_INC */
