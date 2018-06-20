/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: vblink.h  													*
 *																						*
 *	Description			: Data Structures  for proprietary msging between VC & VB       *
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
#ifndef VBLINK_HDR_INCLUDE
#define VBLINK_HDR_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <vxworks.h>
#include <socklib.h>
#include <wdlib.h>
#include <sysLib.h>
#include <string.h>
#include <socklib.h>

#include "subsc/SubscInfoType.h"
#include "voipapi.h"
#include "exchange.h"
#include "mncalxfr.h"

// ext-HO <chenj:05-15-01>
#include "Jcc/JCCEHoMsg.h"
#include "exchangeho.h"

#define VBLINK_NULL_HANDLE                  VOIP_NULL_MOBILE_HANDLE 
#define VBLINK_MAX_REGISTERED_MOBILES       MAX_REGISTERED_MOBILES
#define VBLINK_PORT                         2121
#define VBLINK_MAX_MESSAGE_SIZE             1460
#define VBLINK_MAX_IP_ADDR_STR_LEN          32
#define VBLINK_CONNECT_TIMEOUT_SECS         30



typedef struct {
    LUDB_ID                         LudbId;
    unsigned short                  Index;
   	NSRASHANDLE 				    VBHandle;
} MOBILE_INFO, *PMOBILE_INFO;



typedef struct {
    int                 VBSocket;
    BOOL                ConnectedToViperBase;
    struct sockaddr_in  ViperBaseAddress;
    char                szViperBaseAddress[VBLINK_MAX_IP_ADDR_STR_LEN];
	fd_set				ReadSet;
    BOOL                RetryConnects;
	unsigned long		KeepAliveInterval; 
	unsigned long		LastKeepAliveTimestamp;
	unsigned long		KeepAliveTxCount;
	unsigned long		MaxKeepAliveTxCount;
} VBLINK_CONFIG;


/*
 * The Mobile registration section
 */
typedef enum {
	VBLINK_MOBILE_REG_EVENT_BASE=0,
	VBLINK_MOBILE_REG_EVENT_RECV_RCF,
	VBLINK_MOBILE_REG_EVENT_RECV_RRJ,
	VBLINK_MOBILE_REG_EVENT_RECV_UCF,
	VBLINK_MOBILE_REG_EVENT_RECV_URJ,
	VBLINK_MOBILE_REG_EVENT_RECV_URQ,
	MAX_VBLINK_MOBILE_REG_EVENTS
} VBLINK_MOBILE_REG_EVENT;


typedef enum {
    VBLINK_OAM_TRAP_NULL,
    VBLINK_VIPERBASE_LINK_LOST,
    VBLINK_VIPERBASE_LINK_RESTORED,
    VBLINK_VIPERBASE_LINK_NOT_UP,
    VBLINK_MAX_OAM_TRAPS
}VBLINK_OAM_TRAP;


/*
 * Voip API message structures
 */

typedef struct {

	char        ViperCellId[MAX_VC_ID_STRING_LENGTH];

    TXN_ID      TxnId;

} VBLINK_API_CELL_LOCATION_REQUEST_MSG;


typedef struct {
	char ViperCellId[MAX_VC_ID_STRING_LENGTH];
	char ViperCellIpAddress[MAX_VC_IP_ADDRESS_STRING_LENGTH];
} VBLINK_API_VC_IP_ADDRESS_RESPONSE_MSG;


typedef struct {
	unsigned char  rejectCause;
	unsigned short reqSeqNumber;
	T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER cpn;
} VBLINK_API_VC_BCT_NUMBER_RESPONSE_MSG;

/* 
 * ViperCell update status 
 */
typedef enum
{
    VC_STATUS_UNKNOWN = 1,
	VC_STATUS_DOWN,
    VC_STATUS_UP,
    MAX_VC_STATUS_EVENTS
} VBLINK_VC_STATUS_INFO;


/*
 * Function prototypes
 */
BOOL VBLinkStatus(void);
BOOL VBLinkGetInterval(struct timeval *WaitTime);
BOOL VBLinkSendKeepAlive(void);
BOOL InitializeVBLink(void);
BOOL VBLinkPrepareSocket(void);
void VBLink(void);
BOOL ProcessViperBaseMessage(unsigned char *buffer,int nDataSize);




/*
 * API Function Prototypes defined by VBLink
 */

BOOL VBLinkRegisterMobile(MOBILE_ID   MobileId);										/* LUDB -> GMC */	
BOOL VBLinkUnregisterMobile(MOBILE_ID	MobileId);
BOOL VBLinkSecurityRequest(MOBILE_ID	MobileId);

bool VBLinkMsToNetCiss(LUDB_ID ludbIdx, unsigned char *cissMsg, int msgLen);		/* CC -> GMC */	
bool VBLinkMsToNetSmspp(LUDB_ID	ludbIdx, unsigned char *smsppMsg, unsigned int msgLen);

BOOL VBLinkLocationRequest(VBLINK_API_CELL_LOCATION_REQUEST_MSG Msg);					/* GP -> GMC */	



/*
 * API Function Prototypes defined by other modules; but called by VBLink
 */

bool LudbVBLinkMessageHandler(VBLINK_MOBILE_REG_EVENT RegEvent, MOBILE_ID	MobileId);	/* LUDB <- GMC */	

bool CcVBLinkMessageHandler(char *CellId,char *IpAddress,TXN_ID TxnId);					/* CC <- GMC */	
bool CcVBLinkTransferResponse(unsigned short reqSeqNum,unsigned char rejectCause,
			VOIP_CALL_DESTINATION_ADDRESS CalledParty);

BOOL CcVBLinkCellStatusUpdate(char *pszCellId,VBLINK_VC_STATUS_INFO info);				/* GP <- GMC */	




void VBLinkSendCellId(void);
void VBLinkGetVoipNumberType(
		T_CNI_RIL3_TYPE_OF_NUMBER numType,
		MnetH323AliasAddress_t *address
		);
void VBLinkGetGsmNumberType(
		T_CNI_RIL3_TYPE_OF_NUMBER *numType,
		MnetH323AliasAddress_t address
		);

BOOL VBLinkReconnectLoop(void);
BOOL VBLinkConnect(void);
void VBLinkSetOAMTrap(VBLINK_OAM_TRAP Message);
void VBLinkTimerHandler(int arg);
void SendVBLinkRegEvent(VBLINK_MOBILE_REG_EVENT RegEvent, MOBILE_ID	 MobileId);
void SendIpAddressResponse(char *CellId,char *IpAddress,TXN_ID TxnId);
bool ccCissProcessViperBaseMsg(LUDB_ID LudbId, char * pMsgData, int nMsgSize);
bool smsReceiveSmrMsg(LUDB_ID LudbId, unsigned char * pMsgData, unsigned int nMsgSize);

int VBLinkShowKeepAliveInterval(void);
int VBLinkShowMaxKeepAliveCount(void);
int VBLinkSetKeepAliveInterval(int interval);
int VBLinkSetMaxKeepAliveCount(int count);


// ext-HO <chenj:05-15-01>
void copyGenericToGP10Header( InterHoaVcHeader_t* genheader, InterHoaVcMsg_t* gp10header );
bool ExtHOGenericMessageHandler(InterHoaVcMsg_t* Msg);
BOOL VBLinkPerformHandoverRequest(NonStdRasMessagePerformHandoverRQ_t* Msg);
BOOL VBLinkEndHandover(NonStdRasMessageEndHandover_t* Msg);
BOOL VBLinkPostHandoverMobEvent(NonStdRasMessagePostHandoverMobEvent_t* Msg);
BOOL VBLinkPerformHandoverAck(NonStdRasMessagePerformHandoverAck_t* Msg);
BOOL VBLinkHandoverAccess(NonStdRasMessageHandoverAccess_t* Msg);
BOOL VBLinkHandoverSuccess(NonStdRasMessageHandoverSuccess_t* Msg);
BOOL VBLinkPerformHandback(NonStdRasMessagePerformHandbackRQ_t* Msg);
BOOL VBLinkPerformHandbackAck(NonStdRasMessagePerformHandbackAck_t* Msg);
BOOL VBLinkReleaseHOA(NonStdRasMessageReleaseHOA_t* Msg);

// MGPLA <chenj:08-06-01>
BOOL VBLinkPageRequest(NonStdRasMessagePageRequest_t* Msg);
BOOL VBLinkPageResponse(NonStdRasMessagePageResponse_t* Msg);

#endif /* #ifndef VBAPI_HDR_INCLUDE */
