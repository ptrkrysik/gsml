
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: RtpTask.cpp													*
 *																						*
 *	Description			: All the functions pertaining to RTP 							*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |04/1/99 | DSN    | File created												*
 *		 |		   |		|															*
 *		 |		   |	    |															*
 ****************************************************************************************
 */

#include <vxworks.h>
#include <socklib.h>
#include <inetlib.h>
#include <stdiolib.h>
#include <iolib.h>
#include <fiolib.h>

#include "Os/JCTask.h"
#include "logging\vcmodules.h"
#include "logging\vclogging.h"
#include "ccb.h"
#include "voip\voipapi.h"
#include "h323interface.h"
#include "util.h"
#include "rtptask.h"

#ifndef LOOPBACK_MODE
#include "csunew\csu_intf.h"
#endif

extern PH323TASK_CONFIG	pVoipTaskConfig;
extern CALL_CONTROL_BLOCK	CCBArray[MAX_ACTIVE_CALLS];
extern JCTask RTPMainTaskObj;

#ifdef FAST_RTP_READ
void RtpFastReadHandler(HJCRTPSESSION RtpHandle,unsigned char *newPacket,int packetLength,rtpParam RtpParam,void *Context);
#endif


/*
 * SignalRtpTaskShutdown
 */
void SignalRtpTaskShutdown()
{
	DBG_FUNC("SignalRtpTaskShutdown",H323_RTP_LAYER);
	DBG_ENTER();

   	RtpInputMessage_t Msg;
   	Msg.Type = RtpInputMessageTypeTaskShutdown;
	DBG_LEAVE();
	SendMessageToRtp(pVoipTaskConfig->pRtpInputPipe, &Msg );
   	return ;
}



/*
 * RtpTask
 */
int RtpTask( void )
{
	DBG_FUNC("RTPTask",H323_RTP_LAYER);
	DBG_ENTER();

	PCALL_CONTROL_BLOCK	pCCB;

	pVoipTaskConfig->pRtpInputPipe = RadPipeOpen("RTPInputPipe", sizeof( RtpInputMessage_t ),
                 30, (pfnCallBackFunction)RtpInputMessageHandler) ;

   	if( pVoipTaskConfig->pRtpInputPipe == NULL)
   	{
      	DBG_ERROR( "Error creating RTP Input Pipe\n" );
      	DBG_LEAVE();
      	return FALSE;
   	}

   	if( jcRtpInit() < 0 || jcRtcpInit() < 0 )
   	{
      	DBG_ERROR( "Error initializing RTP/RTCP layer\n" );
		DBG_LEAVE();
      	return FALSE;
   	}
	int Counter=0;

	for(Counter=0;Counter<MAX_ACTIVE_CALLS;Counter++)
	{
		pCCB = &CCBArray[Counter];

		pCCB->RtpHandle = jcRtpOpenEx((17000 + 2 * Counter), 1, 0xff, "JetCellBTS" );
		ASSERT(pCCB->RtpHandle != NULL);
		if (pCCB->RtpHandle == NULL)
		{
			DBG_ERROR("Could not open RTP port !!\n");
			DBG_LEAVE();
			return FALSE;
		}
		jcRtpSetEventHandler(pCCB->RtpHandle,(LPJCRTPEVENTHANDLER)RtpEventHandler,pCCB,"Voip");
#ifdef FAST_RTP_READ
        jcRtpSetFastReadHandler(pCCB->RtpHandle,RtpFastReadHandler);
#endif
		DBG_TRACE("CCB %x RTP %X\n",pCCB,pCCB->RtpHandle);
#ifndef LOOPBACK_MODE
		T_CSU_PORT_ID CsuPort;
		CsuPort.portType = CSU_RTP_PORT;
		CsuPort.portId.rtpHandler = pCCB->RtpHandle;
		if (csu_RegSrcSnkPort(&CsuPort) != CSU_RESULT_REG_SUCCESS) {
			DBG_ERROR("Unable to register RTP port with CSU!!\n");
		}
#endif

	}

   	DBG_TRACE( "RTP Task: Initialization Successful\n" );
    RTPMainTaskObj.JCTaskEnterLoop();
   	WAIT_FOREVER_FOR_RTP_EVENTS();
    RTPMainTaskObj.JCTaskNormExit();

	DBG_LEAVE();
	return TRUE;
}

#ifdef LOOPBACK_MODE_FAST_RTP_READ
void	RTPFastReadHandler(HJCRTPSESSION RtpHandle,unsigned char *newPacket,
                              int packetLength,rtpParam RtpParam,VOIP_CALL_HANDLE CallHandle,T_CNI_IRT_ID MobileHandle);
#endif


/*
 * RtpEventHandler
 */
void RtpEventHandler(HJCRTPSESSION RtpHandle,void *Context)
{

	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)Context;
	ASSERT(pCCB->Magic == H323TASK_MAGIC_VALUE);
	
	RTPEventHandler(RtpHandle, pCCB->Index, (T_CNI_IRT_ID)pCCB->LudbId);


}


#ifdef FAST_RTP_READ 
/*
 * RtpFastReadHandler
 */
void
RtpFastReadHandler(HJCRTPSESSION RtpHandle,unsigned char *newPacket,int packetLength,rtpParam RtpParam,void *Context)
{
	PCALL_CONTROL_BLOCK pCCB = (PCALL_CONTROL_BLOCK)Context;
	
	RTPFastReadHandler(RtpHandle,newPacket,packetLength,RtpParam,pCCB->Index,(T_CNI_IRT_ID)pCCB->LudbId);

}
#endif


/*
 * RtpInputMessageHandler
 */
void RtpInputMessageHandler(RtpInputMessage_t* pMsg )
{
	DBG_FUNC("RtpInputMessageHandler",H323_RTP_LAYER);
	DBG_ENTER();

   	switch( pMsg->Type )
   	{
      case RtpInputMessageTypeTaskShutdown:
         ShutdownRtpTask();
         break;
      default:
         DBG_ERROR( "Unknown message type %d\n", pMsg->Type );
         break;
   	}
	DBG_LEAVE();
   	return;
}



/*
 * ShutdownRtpTask
 */
void ShutdownRtpTask( void )
{
	DBG_FUNC("ShutdownRtpTask",H323_RTP_LAYER);
	DBG_ENTER();

   	int i;
   	PCALL_CONTROL_BLOCK	pCCB;

   	DBG_TRACE("RTP Task is going down...\n");

   	RadPipeClose( pVoipTaskConfig->pRtpInputPipe );

	for(i=0;i<MAX_ACTIVE_CALLS;i++)
	{
		pCCB = &CCBArray[i];
		if (pCCB != NULL) 
		{
			if (pCCB->RtpHandle!=NULL)
			jcRtpClose(pCCB->RtpHandle);
		}
	}

   	jcRtcpEnd();
   	jcRtpEnd();

	DBG_LEAVE();
	exit(1);

}







