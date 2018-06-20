#ifndef LUDBapi_H
#define LUDBapi_H

// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : LUDBapi.h
// Author(s)   : Kevin Lim 
// Create Date : 08-10-99
// Description : Local User Database 
//
// *******************************************************************

#include "ril3/ril3_common.h"
#include "voip/vblink.h"
#include "jcc/LUDBConfig.h"
#include "jcc/JCCComMsg.h"

typedef enum {
	LUDB_MM_REG_REQ			= 0x40,
	LUDB_MM_UNREG_REQ,
	LUDB_MM_SEC_REQ,
	LUDB_MM_EMERG_REQ,
	LUDB_MM_REM_EMERG_REQ,
	LUDB_MM_CIPHER_SET,

	LUDB_MM_REL_REQ,
	LUDB_MM_REG_CNF,
	LUDB_MM_REG_REJ,
	LUDB_MM_UNREG_CNF,
	LUDB_MM_UNREG_REJ,
	LUDB_MM_SEC_RSP,	
	LUDB_MM_EMERG_RSP,
	LUDB_MM_REM_EMERG_CNF,

	VOIP_LUDB_REG_CNF		= 0x60,
	VOIP_LUDB_REG_REJ,
	VOIP_LUDB_UNREG_CNF,
	VOIP_LUDB_UNREG_REJ,
	VOIP_LUDB_SPI_IND,
	VOIP_LUDB_SEC_RSP,
	

	VOIP_LUDB_UNREG_REQ,

	LUDB_VBREQ_TIMEOUT		= 0x70,
	LUDB_PURGE_TIMEOUT,

} LudbMsgType;

typedef struct {
	/* common members */
	unsigned char				module_id;
	LudbMsgType					msg_type;
	T_CNI_RIL3_IE_MOBILE_ID		mobile_id;
	T_CNI_RIL3_REJECT_CAUSE_VALUE	cause;

	/* mm */
	short						mm_id;
	short						ludb_id;
	short						algo;
	short						forceAuth;

	/* voip */
	short						vb_msg_size;
	char						vb_msg[VB_MAX_MSG_LENGTH];
} LudbMsgStruct;

void ludbSetPurgeTimer(int sec);
void ludbSetAuthRequired(int flag);
void ludbSetAuthFreq(int freq);
int  ludbVBReqTimeout(int ludb_id);	// timeout handler
bool ludbStartVBReqTimer(short ludb_id);
bool ludbStopVBReqTimer(short ludb_id);
int  ludbPurgeTimeout(int ludb_id);	// timeout handler
bool ludbStartPurgeTimer(short ludb_id);
bool ludbStopPurgeTimer(short ludb_id);
char *getLudbMsgTypeString(LudbMsgType msg_type);
bool LudbVBLinkMessageHandler(short ludb_id,
							  VBLINK_MOBILE_REG_EVENT event,
							  VOIP_MOBILE_HANDLE mobile);
 
void ludbSetCipherAlgo(short index, short algo);


bool ludbGetSubscInfoExists(short index);
void ludbSetSubscInfoExists(short index, bool flag);
bool ludbGetSecInfoExists(short index);
void ludbSetSecInfoExists(short index, bool flag);
short ludbCheckAuthCount(short index);
T_AUTH_TRIPLET *ludbCurTriplet(short index);
char ludbCKSN(short index);
char ludbIncCKSN(short index);
void ludbDataPrint();
void ludbDataInit();
void ludbSetMMId(const short index, short mmId);
short ludbGetMMId(const short index);
void ludbEntryInit(const short index);

T_CNI_RIL3_IE_MOBILE_ID		ludbGetMobileId(short index);
short						ludbGetEntryByMobileId(T_CNI_RIL3_IE_MOBILE_ID id);

T_SUBSC_INFO*				ludbGetSubscInfoPtr(const short index);
T_SUBSC_IE_MOBILE_ID_IMSI*	ludbGetMobileIdPtr(const short index);
T_SUBSC_IE_ISDN*			ludbGetMSISDNPtr(const short index);
T_SUBSC_IE_PROVISION_BS*	ludbGetProvBSPtr(const short index);
T_SUBSC_IE_PROVISION_TS*	ludbGetProvTSPtr(const short index);
T_SUBSC_IE_PROVISION_SS*	ludbGetProvSSPtr(const short index);
T_SUBSC_IE_SUBSC_STATUS*	ludbGetSubscStatusPtr(const short index);
T_SUBSC_IE_ODB_DATA*		ludbGetODBDataPtr(const short index);
T_SUBSC_IE_SS_INFO_LIST*	ludbGetSSInfoListPtr(const short index);
T_SUBSC_IE_USER_INFO*		ludbGetUserInfoPtr(const short index);

bool	ludbIsProvSMS_MO(const short index);
bool	ludbIsProvSMS_MT(const short index);

bool	ludbIsProvCLIP(const short index);
bool	ludbIsProvCLIR(const short index);
/* shmin */
int	ludbGetCLIROption(const short index, T_SS_SUBSC_CLIR_OPTION *option, bool *isProvCLIR);

bool	ludbIsProvCOLP(const short index);
bool	ludbIsProvCOLR(const short index);
bool	ludbIsProvCFU(const short index);
bool	ludbIsProvCFB(const short index);
bool	ludbIsProvCFNRY(const short index);
bool	ludbIsProvCFNRC(const short index);
bool	ludbIsProvECT(const short index);
bool	ludbIsProvCW(const short index);
bool	ludbIsProvHOLD(const short index);
bool	ludbIsProvMPTY(const short index);
bool	ludbIsProvBAOC(const short index);
bool	ludbIsProvBOIC(const short index);
bool	ludbIsProvBOICE(const short index);
bool	ludbIsProvBAIC(const short index);

bool	ludbIsActiveCFU(const short index);
bool	ludbIsActiveCFB(const short index);
bool	ludbIsActiveCFNRY(const short index);
int		ludbGetTimerCFNRY(const short index);
bool	ludbIsActiveCFNRC(const short index);
bool	ludbIsActiveCW(const short index);

void	ludbPrintSubscInfo(const short index);
void	ludbPrintImsi(const short index);
void	ludbPrintIsdn(const short index);
void	ludbPrintProvBS(const short index);
void	ludbPrintProvTS(const short index);
void	ludbPrintProvSS(const short index);
void	ludbPrintSubscStatus(const short index);
void	ludbPrintODBData(const short index);
void	ludbPrintSSInfoList(const short index);
void	ludbPrintUserInfo(const short index);
void 	ludbPrintSRS(const short index);

/* return 0 for no error, others are error */
short	ludbSetDecodeSubscInfo(const short index, unsigned char *msg, short size);

/* Voip Support */
bool	ludbGetMobileNumber(const short ludbId, char *num);
bool	ludbGetMobileName(const short ludbId, char *name);
bool	ludbIsMobileRegistered(char *num, short *ludbId);
VOIP_MOBILE_HANDLE ludbGetVoipMobileHandle(const short ludbId);
bool ludbProfileUpdate(T_CNI_RIL3_IE_MOBILE_ID mobile_id, short len, char *buf);
bool ludbSRSUpdate(T_CNI_RIL3_IE_MOBILE_ID mobile_id, short num, T_AUTH_TRIPLET *triplet);

/* CC Support */
short ludbGetCipherAlgo(short ludb_id, IntraL3CipherModeCommand_t *cmd);
 
#endif                                       // LUDBapi_H



