
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: jetrtp.cpp													*
 *																						*
 *	Description			: Rtp Module													*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |03/24/99 | DSN    | File created												*
 *		 |		   |		|															*
 *		 |		   |	    |															*
 ****************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <vxWorks.h>
#include <sockLib.h>
#include <zbuflib.h>
#include <zbufsocklib.h>
#include <inetLib.h>
#include <tasklib.h>
#include <memlib.h>
#include <semlib.h>
#include <drv\timer\timerdev.h>

#include "Os/JCTask.h"
#include "GP10OsTune.h"
#include "VOIP/jetrtp.h"
#include "voip/voipapi.h"
#include "cc/hortp.h"
#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"
#include "MnetModuleId.h"
#ifdef VIPER_IP
#include "viperip.h"
#endif

#define IN
#define OUT

/*
 * Global Variables
 */
int	   RtpThreadCounter=0;
int	   RtpPortToDebug=0;
static BOOL RtpDebugPacketIntervalsOn=FALSE;
int    DebugRecvCount=0,DebugSendCount=0;
static rtpPacketBuffer_t *headFreeRtpBufferList,*tailFreeRtpBufferList;
static RTP_BUFFERS_LOCK  rtpBuffersLock;
static RTP_SESSION_INFO RtpSessions[MAX_RTP_SESSIONS];
static int RtpSessionCount=0;
static UINT32 maxInterPacketInterval,sysClockTicksPerSecond;
static char addrStrBuff[INET_ADDR_LEN];

UINT32 BitfieldSet(
	IN  UINT32	value,
	IN  UINT32	bitfield,
	IN  int		nStartBit,
	IN  int		nBits);
UINT32 BitfieldGet(
	IN  UINT32	value,
	IN  int		nStartBit,
	IN  int		nBits);
int ConvertHeader2l (UINT8 *buff,int startIndex,int size);
int ConvertHeader2h (UINT8 *buff,int startIndex,int size);
void jcRtpThread(int Context, int threadIndex);


JCTask *RTPTaskThreads[MAX_ACTIVE_CALLS+MAX_HANDOVER_CHANNELS];


/*
 * InitializeJetRtp
 */
void
InitializeJetRtp()
{
    memset(RtpSessions,0,(sizeof(RTP_SESSION_INFO)*MAX_RTP_SESSIONS));
    RtpSessionCount=0;
    vipInit();
	sysTimestampEnable();
    sysClockTicksPerSecond = sysTimestampFreq();
    maxInterPacketInterval = (sysClockTicksPerSecond / 1000) * 40 ;
}


/*
 * jcRtpInit
 */
INT32 
jcRtpInit(void)
{
    return 0;
}


/*
 * jcRtpEnd
 */
void 
jcRtpEnd(void)
{
	return;
}


/*
 * jcRtpGetAllocationSide
 */
int 
jcRtpGetAllocationSize(void)
{
    return sizeof(jcRtpSession);
}


/*
 * jcRtpOpenFrom
 */
HJCRTPSESSION 
jcRtpOpenFrom(
        IN  UINT16  port,
        IN  UINT32  ssrcPattern,
        IN  UINT32  ssrcMask,
		IN  void*   buffer,
		IN  int	    bufferSize)
{
    jcRtpSession *s = (jcRtpSession *)buffer;
	int sockAddrSize=0;
	struct sockaddr_in serverAddr;

    if (bufferSize < jcRtpGetAllocationSize())
	return NULL;
    
    memset(buffer, 0 , jcRtpGetAllocationSize());

    s->isAllocated    = FALSE;
    s->sSrcPattern    = ssrcPattern;
    s->sSrcMask       = ssrcMask;
    s->sequenceNumber = (UINT16)rand();

	
	s->socket = socket (AF_INET, SOCK_DGRAM, 0);
	if (s->socket == ERROR)   
	{  
		perror ("socket");
		return (NULL);
	}  
/*
	s->sendSocket = socket (AF_INET, SOCK_DGRAM, 0);
	if (s->sendSocket == ERROR)   
	{  
		perror ("socket");
        close(s->socket);
		return (NULL);
	}  
*/	
	sockAddrSize = sizeof (struct sockaddr_in);
	bzero ((char *) &serverAddr, sockAddrSize);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons (port);
   
	s->localPort = port;
#ifndef VIPER_IP
	bind (s->socket, (struct sockaddr *) &serverAddr, sockAddrSize);
#endif
    s->sendSocket = s->socket;
/*
    serverAddr.sin_port = 0;

	bind (s->sendSocket, (struct sockaddr *) &serverAddr, sockAddrSize);
*/
    jcRtpRegenSSRC((HJCRTPSESSION)s); 

    return (HJCRTPSESSION)s;
}




/*
 * jcRtpOpen
 */
HJCRTPSESSION 
jcRtpOpen(
        IN  UINT16  port,
        IN  UINT32  ssrcPattern,
        IN  UINT32  ssrcMask)
{
    return jcRtpOpenEx(port, ssrcPattern, ssrcMask, NULL);
}



/*
 * jcRtpOpenEx
 */
HJCRTPSESSION 
jcRtpOpenEx(
        IN  UINT16  port,
        IN  UINT32  ssrcPattern,
        IN  UINT32  ssrcMask,
        IN  char *  cname)
{
    jcRtpSession *s = (jcRtpSession *)calloc(sizeof(jcRtpSession),1);

    if (s==NULL) 
	return NULL;

	s->Magic = JCRTP_MAGIC;
	s->RtpThreadId = 0;
    s->useFastReadHandler = FALSE;

    if ((jcRtpSession *)jcRtpOpenFrom(port, ssrcPattern, ssrcMask, (void*)s, jcRtpGetAllocationSize())==NULL) 
    {
		cfree((char *)s);
		return NULL;
    }
    else {
        s->bytesSent = s->bytesReceived = s->sendErrors = s->recvErrors = 0;
        AddNewRtpSession(s);
    }
    return (HJCRTPSESSION)s;
}



/*
 * jcRtpClose
 */
UINT32 
jcRtpClose(
        IN  HJCRTPSESSION  hRTP)
{
    jcRtpSession *s = (jcRtpSession *)hRTP;


    shutdown(s->socket,1);
	if (s->RtpThreadId)
	{
		taskDelete(s->RtpThreadId);
		RtpThreadCounter--;
	}
    if (s->isAllocated) {
        RemoveRtpSession(s);
	    cfree((char *)s);
    }

    return 0;
}




/*
 * jcRtpGetSSRC
 */
UINT32 
jcRtpGetSSRC(
        IN  HJCRTPSESSION  hRTP)
{
    jcRtpSession *s = (jcRtpSession *)hRTP;

    return s->sSrc;
}




/*
 * jcRtpSetEventHandler
 */
void 
jcRtpSetEventHandler(
        IN  HJCRTPSESSION        hRTP,
        IN  LPJCRTPEVENTHANDLER  eventHandler,
        IN  void *              context,
        IN  char *              taskName)
{
	DBG_FUNC("jcRtpSetEventHandler",H323_RTP_LAYER);
	DBG_ENTER();

    jcRtpSession *s = (jcRtpSession *)hRTP;
	static char jcRtpThreadName[200];


    if (s)
    {
		
		s->Magic = JCRTP_MAGIC;
        s->eventHandler = eventHandler;
        s->context      = context;
    
        memset(jcRtpThreadName,0,200);
		sprintf(jcRtpThreadName,"Rtp%d%s",RtpThreadCounter++,taskName);
        memset(s->RtpThreadName,0,MAX_RTP_THREAD_NAME_LENGTH);
        if (strlen(jcRtpThreadName) < MAX_RTP_THREAD_NAME_LENGTH) {
            strcpy(s->RtpThreadName,jcRtpThreadName);
        }
		DBG_TRACE("Spawning RTP thread %s; handle 0x%x\n",jcRtpThreadName,s);

#ifdef VIPER_IP
        s->semRtpRecv = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
        if (s->semRtpRecv == NULL) {
            DBG_ERROR("Could not create message q for RTP thread %s\n",jcRtpThreadName);
            return;
        }
        else {
            vipAddRtpPort(s->localPort,s->semRtpRecv,s->rtpRecvPacketBuffer);
        }
#endif
        RTPTaskThreads[RtpThreadCounter-1] = new JCTask(jcRtpThreadName);
		s->RtpThreadId=RTPTaskThreads[RtpThreadCounter-1]->JCTaskSpawn(RTPSUB_TASK_PRIORITY, RTPSUB_TASK_OPTION, RTPSUB_TASK_STACK_SIZE,
			(FUNCPTR)jcRtpThread, (int)s,RtpThreadCounter-1,0,0, 0, 0, 0, 0, 0, 0, MODULE_H323,JC_CRITICAL_TASK);


    }
	DBG_LEAVE();
}


/*
 * jcRtpSetFastReadHandler
 */
void
jcRtpSetFastReadHandler(HJCRTPSESSION hRTP,fastReadCallBack readHandlerFunction)
{
    DBG_FUNC("jcRtpSetFastReadHandler",H323_RTP_LAYER);
    DBG_ENTER();
    jcRtpSession *s = (jcRtpSession *)hRTP;

    DBG_TRACE("Fast read handler specified for RTP Handle 0x%x\n",hRTP);

    s->fastReadHandler      = readHandlerFunction;
    s->useFastReadHandler   = TRUE;

    DBG_LEAVE();
}

/*
 * jcRtpThread
 */
void 
jcRtpThread(int Context, int threadIndex)
{
    DBG_FUNC("jcRtpThread",H323_RTP_LAYER);
    DBG_ENTER();
    jcRtpSession *s = (jcRtpSession *)Context;
	struct sockaddr_in	FromAddress;
	int AddrSize;
	int Status;
    unsigned short *readCount;
    RTP_READ_STATUS ReadStatus;
    rtpParam RtpParam;

	if (s->Magic != JCRTP_MAGIC)
	{
		return;
	}
	
	RTPTaskThreads[threadIndex]->JCTaskEnterLoop();

	while(TRUE)
	{

#ifdef VIPER_IP
        memset(&RtpParam,0,sizeof(RtpParam));
        if (semTake(s->semRtpRecv, WAIT_FOREVER) != ERROR) {
            readCount = (unsigned short *)&s->rtpRecvPacketBuffer[0];
            jcRtpProcessFastReadPacket(s,(char *)&s->rtpRecvPacketBuffer[sizeof(unsigned short)],*readCount,&RtpParam,(struct sockaddr_in *)&FromAddress);
            s->bytesReceived += *readCount;
            (*s->fastReadHandler)(s,(unsigned char *)&s->rtpRecvPacketBuffer[sizeof(unsigned short)],*readCount,RtpParam,s->context);
        }
        else {
            DBG_ERROR("Error receiving packet from MSG_Q in RTP thread %s\n",s->RtpThreadName);
        }
#else
		AddrSize = sizeof(FromAddress);
		memset(&FromAddress,0,AddrSize);
            
        if (s->useFastReadHandler) {
            ZBUF_ID zId;
            memset(&RtpParam,0,sizeof(RtpParam));
            int readBytes=MAX_RTP_PAYLOAD_SIZE;
            zId = zbufSockRecvfrom(s->socket,0,&readBytes,(struct sockaddr *)&FromAddress,&AddrSize);
            char *packetBuffer = zbufSegData(zId,NULL);
            jcRtpProcessFastReadPacket(s,(char *)packetBuffer,readBytes,&RtpParam,(struct sockaddr_in *)&FromAddress);
            s->bytesReceived += readBytes;
            (*s->fastReadHandler)(s,(unsigned char *)packetBuffer,readBytes,RtpParam,s->context);
            zbufDelete(zId);
        }
        else {
 		    Status = recvfrom(s->socket,buf,1,MSG_PEEK,(struct sockaddr *)&FromAddress,&AddrSize);
            if (Status != ERROR) {
                ReadStatus = (*s->eventHandler)(s,s->context);
                if (ReadStatus !=  RTP_READ_STATUS_OK) {
                    switch(ReadStatus) {
                        case RTP_READ_STATUS_NOT_CONNECTED:
							inet_ntoa_b(FromAddress.sin_addr,addrStrBuff);
                            DBG_ERROR("RTP packet received from %s,Read routine returns NOT_CONNECTED\n",addrStrBuff);
                            break;
                        case RTP_READ_STATUS_READ_FAILED:
							inet_ntoa_b(FromAddress.sin_addr,addrStrBuff);
                            DBG_ERROR("RTP packet received from %s,Read routine returns READ_FAILED\n",addrStrBuff);
                            break;
                    }
                }
            } 
            else {
                DBG_ERROR("Error reading RTP packet; error %d\n",errno);
            }
        }
#endif

	}

    RTPTaskThreads[threadIndex]->JCTaskNormExit();

    DBG_LEAVE();
	exit(1);    	
}




/*
 * jcRtpSetRemoteAddress
 */
void 
jcRtpSetRemoteAddress(
        IN HJCRTPSESSION  hRTP,	/* RTP Session Opaque Handle */
        IN UINT32       ip,
        IN UINT16       port)
{
    DBG_FUNC("jcRtpSetRemoteAddress",H323_RTP_LAYER);
    DBG_ENTER();
    jcRtpSession *s = (jcRtpSession *)hRTP;
    struct sockaddr_in remoteAddress;

    s->ip   = ip;
    s->port = port;
    memset(&remoteAddress,0,sizeof(remoteAddress));
    remoteAddress.sin_family = AF_INET;
    remoteAddress.sin_port = htons(port);
    remoteAddress.sin_addr.s_addr = ip;
    if (connect(s->sendSocket,(struct sockaddr *)&remoteAddress,sizeof(remoteAddress)) == ERROR) {
        /*
         * No complains for now
         * DBG_ERROR("Connect failed!!\n");
         */
    }


#ifdef VIPER_IP
    



    struct in_addr remoteAddr,localAddr;

    remoteAddr.s_addr = ip;
    localAddr.s_addr = s->remoteIp = inet_addr(vipGetLocalIpAddress());

    /*
     * Ok, it is a mobile-to-mobile call within the same 
     * ViperCell
     */
    if (localAddr.s_addr == remoteAddr.s_addr) {
        int count;
        for(count=0;count<MAX_RTP_SESSIONS;count++) {
            if (RtpSessions[count].used == TRUE) {
                if (RtpSessions[count].rtpSession->localPort == port) {
                    s->IsLocalLoop = TRUE;
                    s->localSession = RtpSessions[count].rtpSession;
                    break;
                }
            }
            else continue;
        }
    }
    else {
        s->IsLocalLoop = FALSE;
        s->localSession = NULL;
    }


    memset(&s->rtpPacketBuffer,0,512);
    s->rtpPacketDataOffset = 0;

    vipConstructNetHeaders(remoteAddr.s_addr,
                        port,
                        s->localPort,
                        s->rtpPacketBuffer,
                        &s->rtpPacketDataOffset);
#endif
    DBG_LEAVE();
}




/*
 * jcRtpWrite
 */
INT32 
jcRtpWrite(
        IN  HJCRTPSESSION  hRTP,
        IN  void *       buf,
        IN  INT32        len,
        IN  rtpParam *   p)
{

    INT32 status;
    status = jcRtpFastWrite(hRTP,buf,len,p,FALSE,NULL);
    return(status);
}



/*
 * jcRtpFastWrite
 */
INT32
jcRtpFastWrite(IN HJCRTPSESSION hRTP,
                IN void *buf,
                IN INT32 len,
                IN rtpParam *p,
                BOOL fastWrite,
                VOIDFUNCPTR callbackFunction)
{    
    int retVal;

    jcRtpSession *s = (jcRtpSession *)hRTP;
	struct sockaddr_in DestAddress;
	int AddrSize;
    RTP_HEADER  *rtpHdr;
    UINT32 *header=(UINT32*)((char*)buf+p->sByte-12);
    if (s->useSequenceNumber)
        s->sequenceNumber=p->sequenceNumber;
    p->sequenceNumber=s->sequenceNumber;

    header[0]=0;
    rtpHdr = (RTP_HEADER *)header;

    rtpHdr->RtpProtocolVersion  = 2;
    rtpHdr->Marker              = p->marker;
    rtpHdr->PayloadType         = p->payload;
    rtpHdr->SequenceNumber      = s->sequenceNumber++;
/*
    header[0]=BitfieldSet(header[0],2,30,2);	
    header[0]=BitfieldSet(header[0],p->marker,23,1);	
    header[0]=BitfieldSet(header[0],p->payload,16,7);	
    header[0]=BitfieldSet(header[0],s->sequenceNumber++,0,16);	
*/


    header[1]=p->timestamp;
    header[2]=s->sSrc;
	if (p->payload != 3) {
		rtpHdr->Marker = 1;
	}
	       
	ConvertHeader2l((UINT8*)header,0,3);

	AddrSize = sizeof(DestAddress);
	memset(&DestAddress,0,AddrSize);
	DestAddress.sin_family = AF_INET;
	DestAddress.sin_port = htons (s->port);
	DestAddress.sin_addr.s_addr = s->ip;

	if (RtpPortToDebug == s->localPort) {
		DebugSendCount++;
		if (DebugSendCount == 20) {
			inet_ntoa_b(DestAddress.sin_addr,addrStrBuff);
			printf("Sent 20 packets; now sending to %d bytes of data to %s address, port %d\n",
				len,addrStrBuff,s->port);
			DebugSendCount=0;
		}
	}

    if (RtpDebugPacketIntervalsOn) {
        s->currentTimestamp = sysTimestamp();
		s->currentInterval = s->currentTimestamp-s->previousTimestamp;
        if ( s->currentInterval > (maxInterPacketInterval) ) {
            printf("Send drifted on RTP Handle 0x%x\n",s);
            s->previousTimestamp=sysTimestamp();
        }
        else
        s->previousTimestamp = s->currentTimestamp;
		if (s->currentInterval > s->maxInterPacketDelay) 
			s->maxInterPacketDelay = s->currentInterval;
    }

    if (fastWrite == TRUE) {
        int packetLength=len-((char*)header-(char*)buf);

        /*
        retVal = zbufSockBufSendto(s->sendSocket,(char *)buf,packetLength,(VOIDFUNCPTR)callbackFunction,(int)buf,0,(struct sockaddr *)&DestAddress, AddrSize);
         */
        retVal = zbufSockBufSend(s->sendSocket,(char *)buf,packetLength,(VOIDFUNCPTR)callbackFunction,(int)buf,0);
        if (retVal == ERROR) {
            printf("Error in zbufSockSendTo\n");
            s->sendErrors++;
        }
        else {
           s->bytesSent += packetLength;
        }
    }
    else {

#ifdef VIPER_IP
        int pktLength=len-((char*)header-(char*)buf);
        if (s->IsLocalLoop == TRUE) {
            jcRtpSession *peer= (jcRtpSession*)s->localSession;
            unsigned short *count;

            count = (unsigned short *)peer->rtpRecvPacketBuffer;
            *count = pktLength;

            memcpy((char *)&peer->rtpRecvPacketBuffer[sizeof(unsigned short)],
                    header,pktLength);
            semGive(peer->semRtpRecv);
        }
        else {
            memcpy(&s->rtpPacketBuffer[s->rtpPacketDataOffset],header,pktLength);
            vipSend(s->rtpPacketBuffer,pktLength);
        }
        s->bytesSent += pktLength;
#else
        /*
	    retVal = sendto(s->sendSocket, (char *)header, 
                       len-((char*)header-(char*)buf), 0,(struct sockaddr *)&DestAddress, AddrSize);
        */
        retVal = send(s->sendSocket,(char *)header,len-((char*)header-(char*)buf), 0);
        if (retVal == ERROR) {
            printf("Error in sendto, error %d!\n",errno);
            s->sendErrors++;
        }
        else {
            s->bytesSent += len;
        }
#endif
    }
    if (s->hRTCP  &&  retVal >= 0)
    {
        jcRtcpRTPPacketSent(s->hRTCP, p->len, p->timestamp);
    }

    return retVal;
}



/*
 * jcRtpRead
 */
INT32 
jcRtpRead(
        IN  HJCRTPSESSION  hRTP,
        IN  void *buf,
        IN  INT32 len,
        OUT rtpParam* p)
{
    INT32 readStatus;

    readStatus=jcRtpReadPacket(hRTP,(char *)buf,len,p,TRUE,NULL);
    return(readStatus);
}


/*
 * jcRtpReadPacket
 */
INT32 
jcRtpReadPacket(
        IN  HJCRTPSESSION  hRTP,
        IN  char *buf,
        IN  INT32 len,
        OUT rtpParam* p,
        BOOL readPacket,
        struct sockaddr_in *fromAddress)
{
    jcRtpSession *s = (jcRtpSession *)hRTP;
    UINT32 ip;
    UINT16 port;
    UINT32 *header=(UINT32 *)buf;
	struct sockaddr_in FromAddress;
	int AddrSize;

	AddrSize = sizeof(FromAddress);
	memset(&FromAddress,0,AddrSize);
    
    if (readPacket) {
        p->len=recvfrom(s->socket,(char *)buf,len,0,(struct sockaddr *)&FromAddress,&AddrSize);
        if (p->len == ERROR) {
            s->recvErrors++;
        }
        else {
            s->bytesReceived += p->len;
        }

    }
    else {
        memcpy(&FromAddress,fromAddress,sizeof(FromAddress));
        p->len = len;
    }

	ip = FromAddress.sin_addr.s_addr;
	port = ntohs(FromAddress.sin_port);

    ConvertHeader2h((UINT8*)buf,0,3);

	if (RtpPortToDebug == s->localPort) {
		DebugRecvCount++;
		if (DebugRecvCount == 20) {
			inet_ntoa_b(FromAddress.sin_addr,addrStrBuff);
			printf("Received 20 packets; now received %d bytes of data from %s address, port %d\n",
					p->len,addrStrBuff,port);
			DebugRecvCount=0;
		}
	}

    if (p->len==ERROR) return ERROR;
    if (p->len<12) return ERROR;
    
    ConvertHeader2h((UINT8*)buf,3,BitfieldGet(header[0],24,4));
    p->timestamp=header[1];
    p->sequenceNumber=(UINT16)BitfieldGet(header[0],0,16);
    p->sSrc=header[2];
    p->marker=BitfieldGet(header[0],23,1);
    p->payload=(unsigned char)BitfieldGet(header[0],16,7);
    
    p->sByte=12+BitfieldGet(header[0],24,4)*sizeof(UINT32);
    if (BitfieldGet(header[0],28,1))
    {
        int xStart=p->sByte/sizeof(UINT32);
        ConvertHeader2h((UINT8*)buf,xStart,1);
        ConvertHeader2h((UINT8*)buf,xStart+1,BitfieldGet(header[xStart],0,16));
        p->sByte+=BitfieldGet(header[xStart],0,16)*sizeof(UINT32);
    }

    return 0;
}



/*
 * jcRtpProcessFastReadPacket
 */
void
jcRtpProcessFastReadPacket(
        IN  HJCRTPSESSION  hRTP,
        IN  char *buf,
        IN  INT32 len,
        OUT rtpParam* p,
        struct sockaddr_in *fromAddress)
{
    INT32 readStatus;

    readStatus=jcRtpReadPacket(hRTP,(char *)buf,len,p,FALSE,fromAddress);
    return;

}

/*
 * jcRtpReadEx
 */
INT32 
jcRtpReadEx(
        IN  HJCRTPSESSION  hRTP,
        IN  void *       buf,
        IN  INT32        len,
        IN  UINT32       timestamp,
        OUT rtpParam *   p)
{
    jcRtpSession *s = (jcRtpSession *)hRTP;
    int retVal;

    retVal = jcRtpRead(hRTP, buf, len, p);

    if (s->hRTCP  &&  retVal >= 0)
    {
        jcRtcpRTPPacketRecv(s->hRTCP, p->sSrc, timestamp,  p->timestamp, p->sequenceNumber);
    }

    return retVal;
}



/*
 * jcRtpGetPort
 */
UINT16 
jcRtpGetPort(
        IN HJCRTPSESSION  hRTP)	/* RTP Session Opaque Handle */
{
    jcRtpSession *s = (jcRtpSession *)hRTP;
	return (s->localPort);
}






/*
 * jcRtpGetRTCPSession
 */
HJCRTCPSESSION 
jcRtpGetRTCPSession(
        IN  HJCRTPSESSION  hRTP)
{
    jcRtpSession *s = (jcRtpSession *)hRTP;

    return (s->hRTCP);
}



/*
 * jcRtpSetRTCPSession
 */
INT32 
jcRtpSetRTCPSession(
        IN  HJCRTPSESSION   hRTP,
        IN  HJCRTCPSESSION  hRTCP)
{
    jcRtpSession *s = (jcRtpSession *)hRTP;

    s->hRTCP = hRTCP;

    return 0;
}


/*
 * jcRtpGetHeaderLength
 */
INT32 
jcRtpGetHeaderLength(void)
{
    return 12;
}



/*
 * jcRtpRegenSSRC
 */
UINT32 
jcRtpRegenSSRC(
        IN  HJCRTPSESSION  hRTP)
{
    jcRtpSession *s = (jcRtpSession *)hRTP;

    s->sSrc = rand() ;
    s->sSrc &= ~s->sSrcMask;
    s->sSrc |= s->sSrcPattern;

    return s->sSrc;
}




/*
 * jcRtpUseSequenceNumber
 */
INT32 
jcRtpUseSequenceNumber(
                IN HJCRTPSESSION  hRTP)
{
    jcRtpSession *s = (jcRtpSession *)hRTP;

    s->useSequenceNumber = 1;

    return 0;
}



/*
 * jcRtcpSetRemoteAddress
 */
void 
jcRtcpSetRemoteAddress(
                IN  HJCRTCPSESSION  hRTCP,     /* RTCP Session Opaque Handle */
                IN  UINT32        ip,        /* target ip address */
                IN  UINT16        port)      /* target UDP port */
{
	return;
}


/*
 * jcRtcpRTPPacketSent
 */
void 
jcRtcpRTPPacketSent(HJCRTCPSESSION What, int Len, UINT32 timestamp)
{
	return;
}

/*
 * jcRtcpRTPPacketRecv
 */
void 
jcRtcpRTPPacketRecv(HJCRTCPSESSION What, UINT32 sSrc, UINT32 timestamp,  UINT32 timestamp2, UINT16 sequenceNumber)
{
	return;
}


/*
 * jcRtcpInit
 */
INT32 
jcRtcpInit(void)
{
    return 0;
}


/*
 * jcRtcpEnd
 */
INT32 
jcRtcpEnd(void)
{
    return (0);
}



/*
 * BitfieldSet
 */
UINT32 
BitfieldSet(
	IN  UINT32	value,
	IN  UINT32	bitfield,
	IN  int		nStartBit,
	IN  int		nBits)
{
    int mask = (1 << nBits) - 1;
    
    return (value & ~(mask << nStartBit)) + 
           ((bitfield & mask) << nStartBit);
}

/*
 * BitfieldGet
 */
UINT32 
BitfieldGet(
	IN  UINT32	value,
	IN  int		nStartBit,
	IN  int		nBits)
{
    int mask = (1 << nBits) - 1;

    return (value >> nStartBit) & mask; 
}



/*
 * ConvertHeader2l
 */
int
ConvertHeader2l (UINT8 *buff,int startIndex,int size)
{
 int i;

 for (i=startIndex;i< (startIndex+size);i++) 
   ((UINT32*)buff)[i]=htonl(((UINT32*)buff)[i]);

 return (0);
}


/*
 * ConvertHeader2h
 */
int
ConvertHeader2h (UINT8 *buff,int startIndex,int size)
{
  int i;

  for (i=startIndex;i< (startIndex+size);i++) 
    ((UINT32*)buff)[i]= ntohl (((UINT32*)buff)[i]);
  
  return (0);
}



/*
 * VoipCallGetRtpPayloadType
 */
int
VoipCallGetRtpPayloadType(HJCRTPSESSION RtpHandle)
{
    jcRtpSession *RtpSession = (jcRtpSession*)RtpHandle;
    return(RtpSession->RtpPayloadType);
}


/*
 * jcRtpSetPayloadType
 */
void 
jcRtpSetPayloadType(HJCRTPSESSION RtpHandle,BYTE Payload)
{
    jcRtpSession *RtpSession = (jcRtpSession*)RtpHandle;
    RtpSession->RtpPayloadType = Payload;
}


/*
 * RtpPrintHandleInfo
 */
void
RtpPrintHandleInfo(HJCRTPSESSION hRTP)
{
	struct sockaddr_in	DestAddress;
	jcRtpSession *RtpHandle = (jcRtpSession*)hRTP;
	
	if (RtpHandle->Magic != JCRTP_MAGIC) {
		printf("Invalid RTP Handle specified\n");
		return;
	}
	memset(&DestAddress,0,sizeof(struct sockaddr_in));

	DestAddress.sin_family = AF_INET;
	DestAddress.sin_port = htons(RtpHandle->localPort);
	DestAddress.sin_addr.s_addr = RtpHandle->ip;

	inet_ntoa_b(DestAddress.sin_addr,addrStrBuff);
	printf("Remote IP %s, remote port %d, local port %d, task name %s\n",
 		addrStrBuff,RtpHandle->port,RtpHandle->localPort,RtpHandle->RtpThreadName);

	return;
}






/*
 * RtpDebugPort
 */
void
RtpDebugPort(UINT16 port)
{
	RtpPortToDebug = port;
	printf("Turning on debugging on RTP port %d\n",RtpPortToDebug);	
}



/*
 * RtpDebugHandle
 */
void
RtpDebugHandle(HJCRTPSESSION hRTP)
{
	jcRtpSession *RtpHandle = (jcRtpSession*)hRTP;
	
	if (RtpHandle->Magic != JCRTP_MAGIC) {
		printf("Invalid RTP Handle specified\n");
		return;
	}
	RtpPortToDebug = RtpHandle->localPort;
	printf("Turning on debugging on RTP port %d\n",RtpPortToDebug);	

	DebugRecvCount = DebugSendCount=0;
}







/*
 * InitializeRtpPacketBuffers
 */
void
InitializeRtpPacketBuffers()
{
	int count;
	rtpPacketBuffer_t *newBuffer;

	headFreeRtpBufferList = tailFreeRtpBufferList = NULL;

    INIT_RTP_PACKET_BUFFER_LIST();

	for(count=0;count<MAX_RTP_PACKET_BUFFERS;count++) {
			newBuffer=(rtpPacketBuffer_t *)malloc(sizeof(rtpPacketBuffer_t));
			if (newBuffer == NULL) {
				printf("Unable to allocate buffers for speech frames!\n");
				assert(0);
			}
			else {
                memset(newBuffer->packetData,0,MAX_RTP_PAYLOAD_SIZE);
                newBuffer->magic = RTP_PACKET_BUFFER_MAGIC;
				InsertPacketBufferIntoFreePool(newBuffer->packetData);
			}
	}
}



/*
 * ShutdownRtpPacketBuffers
 */
void
ShutdownRtpPacketBuffers()
{
	LOCK_RTP_PACKET_BUFFER_LIST();
	rtpPacketBuffer_t *tempBuffer;
	int count=0;

	tempBuffer = headFreeRtpBufferList;

	while(tempBuffer != NULL) {
		headFreeRtpBufferList = headFreeRtpBufferList->next;
		free(tempBuffer);
		tempBuffer = headFreeRtpBufferList;
		count++;
		if (count == 48) {
			printf("Freed all buffers\n");
		}
	}
	headFreeRtpBufferList = tailFreeRtpBufferList = NULL;

	UNLOCK_RTP_PACKET_BUFFER_LIST();
    DELETE_RTP_PACKET_BUFFER_LIST();
}


/*
 * InsertPacketBufferIntoFreePool
 */
void
InsertPacketBufferIntoFreePool(unsigned char  *Buffer)
{
	LOCK_RTP_PACKET_BUFFER_LIST();
    rtpPacketBuffer_t *newBuffer;
    unsigned char *tempByte;
    short offSet;
    
    offSet = sizeof(int);
    tempByte = (unsigned char *)&Buffer[-offSet];
    newBuffer = (rtpPacketBuffer_t*)tempByte;
    assert(newBuffer->magic = RTP_PACKET_BUFFER_MAGIC);
	if ( (headFreeRtpBufferList == NULL) && (tailFreeRtpBufferList == NULL) ){
		/*
		 * First element in list
		 */
		headFreeRtpBufferList = tailFreeRtpBufferList = newBuffer;
		newBuffer->previous = newBuffer->next = NULL;
	}
	else if (tailFreeRtpBufferList != NULL) {
		tailFreeRtpBufferList->next = newBuffer;
		newBuffer->previous 		= tailFreeRtpBufferList;
		tailFreeRtpBufferList 		= newBuffer;
        tailFreeRtpBufferList->next = NULL;
	}
	UNLOCK_RTP_PACKET_BUFFER_LIST();
}


/*
 * GetPacketBufferFromFreePool
 */
unsigned char *
GetPacketBufferFromFreePool()
{
	unsigned char *retVal=NULL;
	if (headFreeRtpBufferList != NULL) {
		retVal = headFreeRtpBufferList->packetData;
		headFreeRtpBufferList = headFreeRtpBufferList->next;
		headFreeRtpBufferList->previous = NULL;
	}
    return(retVal);
}


    jcRtpSession *rtpSession;
    BOOL         used;


/*
 * AddNewRtpSession
 */
void
AddNewRtpSession(jcRtpSession *rtpSession)
{
    int Count;
    for(Count=0;Count<MAX_RTP_SESSIONS;Count++) {
        if (RtpSessions[Count].used ==  FALSE) {
            RtpSessions[Count].rtpSession = rtpSession;
            RtpSessions[Count].used = TRUE;
            RtpSessionCount++;
            break;
        }
    }
}



/*
 * RemoveRtpSession
 */
void
RemoveRtpSession(jcRtpSession *rtpSession)
{
    int Count;
    for(Count=0;Count<MAX_RTP_SESSIONS;Count++) {
        if (RtpSessions[Count].rtpSession == rtpSession) {
            RtpSessions[Count].used = FALSE;
            RtpSessions[Count].rtpSession = NULL;
            RtpSessionCount--;
            break;
        }
    }
}



/*
 * RtpResetAllCounters
 */
void
RtpResetAllCounters()
{
    int Count;
    for(Count=0;Count<MAX_RTP_SESSIONS;Count++) {
        if (RtpSessions[Count].used == TRUE) {
            RtpResetSessionCounters((HJCRTPSESSION)RtpSessions[Count].rtpSession);
        }
    }
    printf("All RTP Counters reset\n");
}


/*
 * RtpResetSessionCounters
 */
void
RtpResetSessionCounters(HJCRTPSESSION hRTP)
{
	jcRtpSession *rtpSession = (jcRtpSession*)hRTP;

	if (rtpSession->Magic != JCRTP_MAGIC)
	{
        printf("Invalid RTP handle specified\n");
		return;
	}
    rtpSession->bytesSent = rtpSession->bytesReceived =
    rtpSession->sendErrors = rtpSession->recvErrors = 0;
}


/*
 * RtpDisplaySessions
 */
void
RtpDisplaySessions()
{
    BOOL headerDisplayed=FALSE;
    int Count,counter,sessionCounter=0;;
    jcRtpSession *s;
    char displayBuffer[RTP_DISP_TOTAL_WIDTH];

    for(Count=0;Count<MAX_RTP_SESSIONS;Count++) {
        if (RtpSessions[Count].used == TRUE) {
            sessionCounter++;
            if (!headerDisplayed) { RtpDisplayHeader(); headerDisplayed=TRUE; }
            s = (jcRtpSession *)RtpSessions[Count].rtpSession;
            memset(displayBuffer,0,RTP_DISP_TOTAL_WIDTH);
            sprintf(&displayBuffer[RTP_DISP_NAME_OFFSET],"%s",s->RtpThreadName);
            sprintf(&displayBuffer[RTP_DISP_HANDLE_OFFSET],"0x%x",s);
            sprintf(&displayBuffer[RTP_DISP_SENT_OFFSET],"%8ld",s->bytesSent);
            sprintf(&displayBuffer[RTP_DISP_RECV_OFFSET],"%8ld",s->bytesReceived);

            sprintf(&displayBuffer[RTP_DISP_SERR_OFFSET],"%s",
            		RtpGetRemoteTsapString(RtpSessions[Count].rtpSession));

            for(counter=0;counter<RTP_DISP_TOTAL_WIDTH;counter++) {
                if (displayBuffer[counter] == 0) displayBuffer[counter] = 32;
            }
            displayBuffer[RTP_DISP_TOTAL_WIDTH-1] = 0;
            printf("%s\n",displayBuffer);
        }    
    }
    RtpDisplayLine();
    printf("Total RTP Sessions %d\n",sessionCounter);
    RtpDisplayLine();
}




/*
 * RtpDisplayHeader
 */
void
RtpDisplayHeader()
{
    RtpDisplayLine();
    printf("  RTP TASK      HANDLE      BYTES SENT    BYTES RECVD   REMOTE ADDRESS:PORT       \n");
    RtpDisplayLine();
}


/*
 * RtpDisplayLine
 */
void
RtpDisplayLine()
{
    printf("**********************************************************************************\n");
}



/*
 * RtpGetRemoteTsapString
 */
char *
RtpGetRemoteTsapString(HJCRTPSESSION hRTP)
{
	static char remoteTsapStr[128];
	jcRtpSession *s = (jcRtpSession*)hRTP;
	struct in_addr addr;
	memset(remoteTsapStr,0,128);

	addr.s_addr = s->ip;
	inet_ntoa_b(addr,addrStrBuff);
   	sprintf(remoteTsapStr,"%s:%d",addrStrBuff,s->port);

	return(remoteTsapStr);
}

/*
 * RtpGetLocalTsapString
 */
char *
RtpGetLocalTsapString(HJCRTPSESSION hRTP)
{
	static char localTsapStr[128];
	jcRtpSession *s = (jcRtpSession*)hRTP;
	struct in_addr addr;
	memset(localTsapStr,0,128);

	addr.s_addr = s->remoteIp;
	inet_ntoa_b(addr,addrStrBuff);
   	sprintf(localTsapStr,"%s:%d",addrStrBuff,s->localPort);

	return(localTsapStr);
}

/*
 * RtpGetSessionBytesSent
 */
unsigned long
RtpGetSessionBytesSent(HJCRTPSESSION hRTP)
{
	jcRtpSession *s = (jcRtpSession*)hRTP;
	return(s->bytesSent);
}

/*
 * RtpGetSessionBytesRecvd
 */
unsigned long
RtpGetSessionBytesRecvd(HJCRTPSESSION hRTP)
{
	jcRtpSession *s = (jcRtpSession*)hRTP;
	return(s->bytesReceived);
}




/*
 * RtpDebugPacketIntervalsOn
 */
void
RtpDebugPacketIntervals(int value)
{
  if (value > 0) {
    printf("Turned ON debugs of packet intervals on all RTP ports\n");
    RtpDebugPacketIntervalsOn = TRUE;
  }
  else {
    printf("Turned OFF debugs of packet intervals on all RTP ports\n");
    RtpDebugPacketIntervalsOn = FALSE;
  }
}


/*
 * RtpDebugSetIntervalWatermark
 */
void
RtpDebugSetIntervalWatermark(int maxMilliseconds)
{
    maxInterPacketInterval = (sysClockTicksPerSecond / 1000) * maxMilliseconds;
    printf("MAX inter-packet delay watermark now set to %d milliseconds\n",maxMilliseconds);
}


/*
 * RtpDebugShowMaxIntervals
 */
void
RtpDebugShowMaxIntervals()
{
	int Count;
    jcRtpSession *s;
    unsigned long ticksPerMs = sysClockTicksPerSecond/1000;
	printf("\n********************************************************************\n");
    for(Count=0;Count<MAX_RTP_SESSIONS;Count++) {
        if (RtpSessions[Count].used == TRUE) {
            s = (jcRtpSession *)RtpSessions[Count].rtpSession;
			printf("RTP Handle 0x%x : max delay %ld ms\n",s,s->maxInterPacketDelay/ticksPerMs);
		}
	}
	printf("********************************************************************\n");
}