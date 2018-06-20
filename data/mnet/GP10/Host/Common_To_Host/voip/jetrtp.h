/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: JetRtp.h														*
 *																						*
 *	Description			: The JetCell RTP Library Header file							*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |04/21/99 | DSN    | File created												*
 *		 |		   |		|															*
 *		 |		   |	    |															*
 ****************************************************************************************
 */


#ifndef JCRTP_HDR_INC
#define JCRTP_HDR_INC



#include <socklib.h>
#include <hostlib.h>
#include <stdlib.h>
#include <semlib.h>

#ifdef WIN32
#include <windows.h>
#endif


typedef unsigned char BYTE;
typedef void * HJCRTPSESSION;
typedef void * HJCRTCPSESSION;
#define UINT32			unsigned int


#define RTP_PROTOCOL_VERSION		2
#define STD_RTP_HEADER_SIZE			12
#define RTPTHREAD_WORK_PRIORITY		10
#define RTPTHREAD_STACK_SIZE		10240
#define JCRTP_MAGIC					1234
#define MAX_RTP_SESSIONS            50
#define MAX_RTP_PAYLOAD_SIZE		50
#define MAX_RTP_PACKET_BUFFERS		50
#define RTP_PACKET_BUFFER_MAGIC     0x5432
#define MAX_RTP_THREAD_NAME_LENGTH  128




#ifdef WIN32
#define RTP_BUFFERS_LOCK				    CRITICAL_SECTION	
#define INIT_RTP_PACKET_BUFFER_LIST()	    InitializeCriticalSection(&rtpBuffersLock)
#define DELETE_RTP_PACKET_BUFFER_LIST()	    DeleteCriticalSection(&rtpBuffersLock)
#define LOCK_RTP_PACKET_BUFFER_LIST()	    EnterCriticalSection(&rtpBuffersLock)
#define UNLOCK_RTP_PACKET_BUFFER_LIST()		LeaveCriticalSection(&rtpBuffersLock)

#elif defined(__VXWORKS__)
#define RTP_BUFFERS_LOCK				    SEM_ID
#define INIT_RTP_PACKET_BUFFER_LIST()		rtpBuffersLock=semBCreate(SEM_Q_PRIORITY,SEM_FULL)
#define DELETE_RTP_PACKET_BUFFER_LIST()		semDelete(rtpBuffersLock)
#define LOCK_RTP_PACKET_BUFFER_LIST()		semTake(rtpBuffersLock,WAIT_FOREVER)
#define UNLOCK_RTP_PACKET_BUFFER_LIST()		semGive(rtpBuffersLock)
#endif

#define RTP_DISP_NAME_OFFSET    1
#define RTP_DISP_NAME_WIDTH     14
#define RTP_DISP_HANDLE_OFFSET  RTP_DISP_NAME_OFFSET+RTP_DISP_NAME_WIDTH
#define RTP_DISP_HANDLE_WIDTH   14
#define RTP_DISP_SENT_OFFSET    RTP_DISP_HANDLE_OFFSET+RTP_DISP_HANDLE_WIDTH
#define RTP_DISP_SENT_WIDTH     15
#define RTP_DISP_RECV_OFFSET    RTP_DISP_SENT_OFFSET+RTP_DISP_SENT_WIDTH
#define RTP_DISP_RECV_WIDTH     14
#define RTP_DISP_SERR_OFFSET    RTP_DISP_RECV_OFFSET+RTP_DISP_RECV_WIDTH
#define RTP_DISP_SERR_WIDTH     12
#define RTP_DISP_RERR_OFFSET    RTP_DISP_SERR_OFFSET+RTP_DISP_SERR_WIDTH
#define RTP_DISP_RERR_WIDTH     12
#define RTP_DISP_TOTAL_WIDTH    RTP_DISP_RERR_OFFSET+RTP_DISP_RERR_WIDTH+1




typedef struct _rtpPacketBuffer {
    int                     magic;
	unsigned char           packetData[MAX_RTP_PAYLOAD_SIZE];
	struct _rtpPacketBuffer *previous;
	struct _rtpPacketBuffer *next;
} rtpPacketBuffer_t;



typedef struct {
  UINT32			RtpProtocolVersion:2;
  UINT32			Padding:1;
  UINT32			HeaderExtensionFlag:1;
  UINT32			CsrcCount:4;
  UINT32			Marker:1;
  UINT32			PayloadType:7;	
  UINT32			SequenceNumber:16;
  UINT32			TimeStamp;
  UINT32			Ssrc;
  UINT32			Csrc[1];
} RTP_HEADER, *PRTP_HEADER;


typedef struct {
	unsigned short	Profile;
	unsigned short	Length;
} RTP_EXTENDED_HEADER, *PRTP_EXTENDED_HEADER;

typedef struct {
  UINT32		FirstWord;
  UINT32		SecondWord;
  UINT32		LastWord;
} RTP_NETWORK_ORDER_HEADER, *PRTP_NETWORK_ORDER_HEADER;


typedef enum {
    RTP_READ_STATUS_OK,
    RTP_READ_STATUS_NOT_CONNECTED,
    RTP_READ_STATUS_READ_FAILED
} RTP_READ_STATUS;

typedef RTP_READ_STATUS (*LPJCRTPEVENTHANDLER)
    (
          HJCRTPSESSION  hRTP,
          void *       context
    );




typedef struct {
       UINT32  timestamp;
       BOOL    marker;
       BYTE    payload;
    
         UINT32  sSrc;
         UINT16  sequenceNumber;
         int     sByte;
         int     len;
} rtpParam;



#define SWAP_LONG_TO_HOST_ORDER(Word) Word = ntohl(Word)
#define SWAP_LONG_TO_NETWORK_ORDER(Word) Word = htonl(Word)


typedef void (*fastReadCallBack)(void *, unsigned char *, int, struct rtpParam, void *);

typedef struct _JCRTP_SESSION
{
    int		       		isAllocated;
    int                	socket;
    int                 sendSocket;
    UINT32             	sSrc;
    UINT32             	sSrcMask;
    UINT32             	sSrcPattern;
    LPJCRTPEVENTHANDLER eventHandler;
    void *             	context;
    UINT16             	sequenceNumber;
    UINT32             	ip;
    UINT16             	port;
	UINT16				localPort;
	UINT32				remoteIp;
    BOOL               	useSequenceNumber;
    HJCRTCPSESSION      hRTCP;
	int					Magic;
	int					RtpThreadId;
    BYTE                RtpPayloadType;
    char                RtpThreadName[MAX_RTP_THREAD_NAME_LENGTH];
    BOOL                useFastReadHandler;
    fastReadCallBack    fastReadHandler;
    unsigned long       bytesSent;
    unsigned long       bytesReceived;
    unsigned long       sendErrors;
    unsigned long       recvErrors;
    UINT32              previousTimestamp;
	UINT32				maxInterPacketDelay;
    UINT32              currentTimestamp;
    UINT32              currentInterval;
#ifdef VIPER_IP
    BOOL                IsLocalLoop;
    struct _JCRTP_SESSION *localSession;
    unsigned char       rtpPacketBuffer[512];
    unsigned short      rtpPacketDataOffset;
    SEM_ID              semRtpRecv;
    unsigned char       rtpRecvPacketBuffer[512];
#endif
} jcRtpSession,*pjcRtpSession;


typedef struct {
    jcRtpSession *rtpSession;
    BOOL         used;
}  RTP_SESSION_INFO;

/*
 * Function prototypes
 */
INT32 jcRtpInit(void);

void jcRtpEnd(void);

int jcRtpGetAllocationSize(void);

HJCRTPSESSION jcRtpOpenFrom(
          UINT16  port,
          UINT32  ssrcPattern,
          UINT32  ssrcMask,
	  void*   buffer,
	  int	    bufferSize);

HJCRTPSESSION jcRtpOpen(
          UINT16  port,
          UINT32  ssrcPattern,
          UINT32  ssrcMask);

HJCRTPSESSION jcRtpOpenEx(
          UINT16  port,
          UINT32  ssrcPattern,
          UINT32  ssrcMask,
          char *  cname);

UINT32 jcRtpClose(
          HJCRTPSESSION  hRTP);

UINT32 jcRtpGetSSRC(
          HJCRTPSESSION  hRTP);

void jcRtpSetEventHandler(
          HJCRTPSESSION        hRTP,
          LPJCRTPEVENTHANDLER  eventHandler,
          void *               context,
          char *               taskName);

void jcRtpSetFastReadHandler(HJCRTPSESSION hRTP,
            fastReadCallBack readHandlerFunction);

void jcRtpSetRemoteAddress(
          HJCRTPSESSION  hRTP,
          UINT32       ip,
          UINT16       port);

INT32 jcRtpWrite(
          HJCRTPSESSION  hRTP,
          void  *      buf,
          INT32        len,
          rtpParam *   p);

INT32
jcRtpFastWrite(HJCRTPSESSION hRTP,
                void *buf,
                INT32 len,
                rtpParam *p,
                BOOL fastWrite,
                VOIDFUNCPTR callbackFunction);

INT32 jcRtpRead(
           HJCRTPSESSION  hRTP,
           void *       buf,
           INT32        len,
          rtpParam *   p);

INT32 jcRtpReadEx(
           HJCRTPSESSION  hRTP,
           void *       buf,
           INT32        len,
           UINT32       timestamp,
          rtpParam *   p);

INT32 
jcRtpReadPacket(
        HJCRTPSESSION  hRTP,
        char *buf,
        INT32 len,
        rtpParam* p,
        BOOL readPacket,
        struct sockaddr_in *fromAddress);

void
jcRtpProcessFastReadPacket(
        HJCRTPSESSION  hRTP,
        char *buf,
        INT32 len,
        rtpParam* p,
        struct sockaddr_in *fromAddress);

UINT16 jcRtpGetPort(
          HJCRTPSESSION  hRTP);

HJCRTCPSESSION jcRtpGetRTCPSession(
          HJCRTPSESSION  hRTP);

INT32 jcRtpSetRTCPSession(
          HJCRTPSESSION   hRTP,
          HJCRTCPSESSION  hRTCP);

INT32 jcRtpGetHeaderLength(void);

UINT32 jcRtpRegenSSRC(
          HJCRTPSESSION  hRTP);

INT32 jcRtpUseSequenceNumber(
          HJCRTPSESSION  hRTP);

void jcRtpThread(int Context);

void jcRtcpRTPPacketSent(HJCRTCPSESSION What, int Len, UINT32 timestamp);

void jcRtcpRTPPacketRecv(HJCRTCPSESSION What, UINT32 sSrc, UINT32 timestamp,  UINT32 timestamp2, UINT16 sequenceNumber);

INT32 jcRtcpInit(void);

INT32 jcRtcpEnd(void);

void jcRtcpSetRemoteAddress(
                  HJCRTCPSESSION  hRTCP,     /* RTCP Session Opaque Handle */
                  UINT32        ip,        /* target ip address */
                  UINT16        port);

inline void SWAP_TO_NETWORK_ORDER(PRTP_NETWORK_ORDER_HEADER pHdr)
{
	SWAP_LONG_TO_NETWORK_ORDER(pHdr->FirstWord);
	SWAP_LONG_TO_NETWORK_ORDER(pHdr->SecondWord);
	SWAP_LONG_TO_NETWORK_ORDER(pHdr->LastWord);
};

inline void SWAP_TO_HOST_ORDER(PRTP_NETWORK_ORDER_HEADER pHdr)
{
	SWAP_LONG_TO_HOST_ORDER(pHdr->FirstWord);
	SWAP_LONG_TO_HOST_ORDER(pHdr->SecondWord);
	SWAP_LONG_TO_HOST_ORDER(pHdr->LastWord);
};
void RtpPrintHandleInfo(HJCRTPSESSION RtpHandle);
void RtpDebugPort(UINT16 port);
void RtpDebugHandle(HJCRTPSESSION hRTP);

void InitializeRtpPacketBuffers(void);
void ShutdownRtpPacketBuffers(void);
void InsertPacketBufferIntoFreePool(unsigned char  *Buffer);
unsigned char *GetPacketBufferFromFreePool(void);
int VoipCallGetRtpPayloadType(HJCRTPSESSION RtpHandle);
void jcRtpSetPayloadType(HJCRTPSESSION RtpHandle,BYTE Payload);
void AddNewRtpSession(jcRtpSession *rtpSession);
void RemoveRtpSession(jcRtpSession *rtpSession);
void RtpDisplaySessions(void);
void RtpDisplayHeader(void);
void RtpDisplayLine(void);
void RtpResetSessionCounters(HJCRTPSESSION RtpHandle);
void RtpResetAllCounters(void);
char *RtpGetRemoteTsapString(HJCRTPSESSION hRTP);
char *RtpGetLocalTsapString(HJCRTPSESSION hRTP);
unsigned long RtpGetSessionBytesSent(HJCRTPSESSION hRTP);
unsigned long RtpGetSessionBytesRecvd(HJCRTPSESSION hRTP);
void InitializeJetRtp(void);


#endif  /* __RTP_H */
