// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

#include "string.h"
#include "logging\vcmodules.h"
#include "logging\vclogging.h"
#include "CC/hortp.h"


HANDOVER_CHANNEL HandoverChannelArray[MAX_HANDOVER_CHANNELS];


/*
 * InitializeHandpverRtpPool
 */
BOOL
InitializeHandoverRtpPool()
{
	DBG_FUNC("InitializeHandoverRtpPool",CC_LAYER);
	DBG_ENTER();
	BOOL RetStatus=FALSE;
	PHANDOVER_CHANNEL	pChannel;

	int Counter,ArraySize;
	
	ArraySize = MAX_HANDOVER_CHANNELS * sizeof(HANDOVER_CHANNEL);
		
	memset(HandoverChannelArray,0,ArraySize);

	
	for(Counter=0;Counter<MAX_HANDOVER_CHANNELS;Counter++)
	{
		pChannel = (PHANDOVER_CHANNEL)&HandoverChannelArray[Counter];
		
		pChannel->LocalPort = Counter+HANDOVER_RTP_POOL_BEGIN;

		pChannel->RtpHandle = jcRtpOpenEx(pChannel->LocalPort, 1, 0xff, "JetCellBTS" );
	
		ASSERT(pChannel->RtpHandle != NULL);
		if (pChannel->RtpHandle == NULL)
		{
			DBG_ERROR("Could not open RTP port !!\n");
			DBG_LEAVE();
			return RetStatus;
		}
	
		jcRtpSetEventHandler(pChannel->RtpHandle,HoRtpEventHandler,NULL,"HOver");
	
#ifdef FAST_RTP_READ
        jcRtpSetFastReadHandler(pChannel->RtpHandle,HoRtpFastReadHandler);
#endif
		/*
		 * Notify CSU of the new RTP port
		 */
		T_CSU_PORT_ID CsuPort;
		CsuPort.portType = CSU_RTP_PORT;
		CsuPort.portId.rtpHandler = pChannel->RtpHandle;
		if (csu_RegSrcSnkPort(&CsuPort) != CSU_RESULT_REG_SUCCESS) {
			DBG_ERROR("Unable to register RTP port with CSU!!\n");
		}
		else {
			RetStatus = TRUE;
		}

		DBG_TRACE("RTP %X\n",pChannel->RtpHandle);
	}

	DBG_LEAVE();
	return(RetStatus);
}



/*
 * AssignHandoverRtpChannel
 */
HJCRTPSESSION
AssignHandoverRtpChannel()
{
	DBG_FUNC("AssignHandoverRtpChannel",CC_LAYER);
	DBG_ENTER();
	int Count;
	PHANDOVER_CHANNEL	pChannel;


	for(Count=0;Count<MAX_HANDOVER_CHANNELS;Count++) {
		pChannel = &HandoverChannelArray[Count];
		if (pChannel->Used == FALSE) {
			pChannel->Used = TRUE;
			DBG_LEAVE();
			return(pChannel->RtpHandle);
		}
	}

	DBG_LEAVE();
	return(NULL);
}



/*
 * ReleaseHandoverRtpChannel
 */
void
ReleaseHandoverRtpChannel(HJCRTPSESSION RtpHandle)
{
	DBG_FUNC("ReleaseHandoverRtpChannel",CC_LAYER);
	DBG_ENTER();
	int Count;
	PHANDOVER_CHANNEL	pChannel;

	for(Count=0;Count<MAX_HANDOVER_CHANNELS;Count++) {
		pChannel = &HandoverChannelArray[Count];
		if (pChannel->RtpHandle == RtpHandle) {
			pChannel->Used = FALSE;
			break;
		}
	}
	DBG_LEAVE();
}



/*
 * handoverRtpDataPrint
 */
void
handoverRtpDataPrint(void)
{

  int Count;
  PHANDOVER_CHANNEL	pChannel;


  for(Count=0;Count<MAX_HANDOVER_CHANNELS;Count++) {
    pChannel = &HandoverChannelArray[Count];
    if (pChannel->Used == FALSE) 
      {
        printf("RtpHandle = %p is not used\n", pChannel->RtpHandle);
      }
    else
      {
        printf("RtpHandle = %p is used\n", pChannel->RtpHandle);
      }
  }

}



/*
 * GetIncomingHandoverRtpPort
 */
UINT16
GetIncomingHandoverRtpPort(HJCRTPSESSION RtpHandle)
{
	DBG_FUNC("GetIncomingHandoverRtpPort",CC_LAYER);
	DBG_ENTER();
	int Count;
	PHANDOVER_CHANNEL	pChannel;
	int RetStatus = 0;

	for(Count=0;Count<MAX_HANDOVER_CHANNELS;Count++) {
		pChannel = &HandoverChannelArray[Count];
		if (pChannel->RtpHandle == RtpHandle) {
			RetStatus = pChannel->LocalPort;
			break;
		}
	}

	DBG_LEAVE();
	return(RetStatus);
}



/*
 * SetRemoteHandoverRtpPort
 */
void
SetRemoteHandoverRtpPort(HJCRTPSESSION RtpHandle,UINT32 ip,UINT16 port)
{
	DBG_FUNC("SetRemoteHandoverRtpPort",CC_LAYER);
	DBG_ENTER();

	jcRtpSetRemoteAddress( RtpHandle, ip, port );

	DBG_LEAVE();
}



/*
 * HoRtpEventHandler
 */
RTP_READ_STATUS HoRtpEventHandler(HJCRTPSESSION RtpHandle,void *Context)
{
	RTPEventHandler(RtpHandle, (int)Context,0);
}



#ifdef FAST_RTP_READ 
/*
 * HoRtpFastReadHandler
 */
void
HoRtpFastReadHandler(HJCRTPSESSION RtpHandle,unsigned char *newPacket,int packetLength,rtpParam RtpParam,void *Context)
{
	RTPFastReadHandler(RtpHandle,newPacket,packetLength,RtpParam,(int)Context,0);

}
#endif
