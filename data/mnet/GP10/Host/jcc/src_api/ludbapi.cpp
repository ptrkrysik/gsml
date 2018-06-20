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
// File        : LUDBapi.cpp
// Author(s)   : Kevin Lim 
// Create Date : 08-10-99
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included L3 Message header for messages from other Layer 3 modules (CC, RR)

// from common vob
#include "subsc/SubscInfoType.h"
#include "subsc/SubscInfoUtil.h"

#include "MnetModuleId.h"
#include "JCC/LUDB.h"
#include "JCC/LUDBapi.h"
#include "LUDBVoipMsgProc.h"
#include "LudbHashTbl.h"

#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"


// global variables
int 		ludb_purge_timer_sec = 1;
MSG_Q_ID 	ludbMsgQId;
LUDB	 	gLUDB;
short		ludb_auth_required = 0;
short		ludb_auth_freq = 1;
LudbHashTbl *ludbImsiHash;

// ****************************************************************************
void ludbSetPurgeTimer(int sec)
{
	if(sec == 0){
		printf("usage: ludbSetPurgeTimer(seconds)\n");
		printf("current: %d\n", ludb_purge_timer_sec);
	}
	else{
		ludb_purge_timer_sec = sec;
	}
}

// ****************************************************************************
void ludbSetAuthRequired(int flag)
{
	if(ludb_auth_required != flag){
		ludb_auth_required = flag;
		printf("ludb_auth_required is now set to %d\n", ludb_auth_required);
	}
	else{
		printf("ludb_auth_required is already set to %d\n", ludb_auth_required);
	}
}

// ****************************************************************************
void ludbSetAuthFreq(int freq)
{
	if(freq == 0){
		printf("usage: ludbSetAuthFreq(freq)\n");
		printf("freq : 1..5\n");
		printf("current: %d\n", ludb_auth_freq);
	}
	else{
		ludb_auth_freq = freq;
	}
}

// ****************************************************************************
int ludbVBReqTimeout(int ludb_id)
{
	DBG_FUNC("ludbVBReqTimeout", LUDB_LAYER);
	DBG_ENTER();
	DBG_TRACE("VBReq Timeout on entry: %d\n", ludb_id);

	LudbMsgStruct timeout;

	timeout.module_id = MODULE_LUDB;
	timeout.msg_type = LUDB_VBREQ_TIMEOUT;
	timeout.ludb_id = ludb_id;
	if (ERROR == msgQSend(	ludbMsgQId, 
							(char *) &timeout, 
							sizeof(LudbMsgStruct), 
							NO_WAIT,	// important, timer handler shouldn't wait
							MSG_PRI_NORMAL
							))
	{
		DBG_ERROR("ludbVBReqTimeout msgQSend (QID=%d) error\n", (int)ludbMsgQId);
	}
	DBG_LEAVE();
	return 0;
}

// ****************************************************************************
bool ludbStartVBReqTimer(short ludb_id)
{
	return gLUDB.startVBReqTimer(ludb_id);
}

// ****************************************************************************
bool ludbStopVBReqTimer(short ludb_id)
{
	return gLUDB.stopVBReqTimer(ludb_id);
}

// ****************************************************************************
int ludbPurgeTimeout(int ludb_id)
{
	DBG_FUNC("ludbPurgeTimeout", LUDB_LAYER);
	DBG_ENTER();
	DBG_TRACE("Purge Timeout on entry: %d\n", ludb_id);

	LudbMsgStruct timeout;

	timeout.module_id = MODULE_LUDB;
	timeout.msg_type = LUDB_PURGE_TIMEOUT;
	timeout.ludb_id = ludb_id;
	if (ERROR == msgQSend(	ludbMsgQId, 
							(char *) &timeout, 
							sizeof(LudbMsgStruct), 
							NO_WAIT,	// important, timer handler shouldn't wait
							MSG_PRI_NORMAL
							))
	{
		DBG_ERROR("ludbPurgeTimeout msgQSend (QID=%d) error\n", (int)ludbMsgQId);
	}
	DBG_LEAVE();
	return 0;
}

// ****************************************************************************
bool ludbStartPurgeTimer(short ludb_id)
{
	return gLUDB.startPurgeTimer(ludb_id);
}

// ****************************************************************************
bool ludbStopPurgeTimer(short ludb_id)
{
	return gLUDB.stopPurgeTimer(ludb_id);
}

// ****************************************************************************
char *getLudbMsgTypeString(LudbMsgType msg_type)
{
	switch(msg_type){
		case LUDB_MM_REG_REQ:		return("Registration Request");		break;
		case LUDB_MM_UNREG_REQ:		return("UnRegistration Request");	break;
		case LUDB_MM_SEC_REQ:		return("SecurityInfo Request");		break;
		case LUDB_MM_REG_CNF:		return("Registration Confirm");		break;
		case LUDB_MM_REG_REJ:		return("Registration Reject");		break;
		case LUDB_MM_UNREG_CNF:		return("UnRegistration Confirm");	break;
		case LUDB_MM_UNREG_REJ:		return("UnRegistration Reject");	break;
		case LUDB_MM_SEC_RSP:		return("Security Response");		break;

		case VOIP_LUDB_REG_CNF:		return("Registration Confirm");		break;
		case VOIP_LUDB_REG_REJ:		return("Registration Reject");		break;
		case VOIP_LUDB_UNREG_CNF:	return("UnRegistration Confirm");	break;
		case VOIP_LUDB_UNREG_REJ:	return("UnRegistration Reject");	break;
		case VOIP_LUDB_SPI_IND:		return("Subscriber Profile");		break;
		case VOIP_LUDB_SEC_RSP:		return("SecurityInfo Response");	break;

		case VOIP_LUDB_UNREG_REQ:	return("UnRegistration Request");	break;
		default:					return("Unknown");					break;
	}
}

// ****************************************************************************
bool ludbProfileUpdate(T_CNI_RIL3_IE_MOBILE_ID mobile_id, short len, char *buf)
{
	DBG_FUNC("ludbProfileUpdate", LUDB_LAYER);
	DBG_ENTER();

	MSG_Q_ID qid = ludbMsgQId;
	LudbMsgStruct	ludbInMsg;
	ludbInMsg.module_id  = MODULE_H323;
	ludbInMsg.mobile_id = mobile_id;
	ludbInMsg.msg_type   = VOIP_LUDB_SPI_IND;

	if(len <= VB_MAX_MSG_LENGTH){
		ludbInMsg.vb_msg_size = len;
		for(int i=0; i<len; i++)
			ludbInMsg.vb_msg[i] = buf[i];
	}
	else{
		DBG_ERROR("msg size bigger than %d\n", VB_MAX_MSG_LENGTH);
		DBG_LEAVE();
		return (false);
	}

	STATUS stat = msgQSend( qid, (char *) &ludbInMsg, 
						sizeof(LudbMsgStruct), 
						NO_WAIT, MSG_PRI_NORMAL ); 
	if(stat == ERROR){
		DBG_ERROR("msgQSend error\n");
		DBG_LEAVE();
		return (false);
	}
	else {
		DBG_TRACE("Voip->Ludb MsgHandler MsgType: %s\n", 
					getLudbMsgTypeString(ludbInMsg.msg_type));
		DBG_LEAVE();
		return (true);
	}
}

// ****************************************************************************
bool ludbSRSUpdate(T_CNI_RIL3_IE_MOBILE_ID mobile_id, short num, T_AUTH_TRIPLET *triplet)
{
	DBG_FUNC("ludbSRSUpdate", LUDB_LAYER);
	DBG_ENTER();

	MSG_Q_ID qid = ludbMsgQId;
	LudbMsgStruct	ludbInMsg;
	ludbInMsg.module_id  = MODULE_H323;
	ludbInMsg.mobile_id = mobile_id;
	ludbInMsg.msg_type   = VOIP_LUDB_SEC_RSP;

	if(num == 1){
		ludbInMsg.vb_msg_size = 1;
		memcpy(ludbInMsg.vb_msg, triplet, sizeof(T_AUTH_TRIPLET));
	}
	else{
		ludbInMsg.vb_msg_size = 0;
	}

	STATUS stat = msgQSend( qid, (char *) &ludbInMsg, 
						sizeof(LudbMsgStruct), 
						NO_WAIT, MSG_PRI_NORMAL ); 
	if(stat == ERROR){
		DBG_ERROR("msgQSend error\n");
		DBG_LEAVE();
		return (false);
	}
	else {
		DBG_TRACE("Voip->Ludb MsgHandler MsgType: %s\n", 
					getLudbMsgTypeString(ludbInMsg.msg_type));
		DBG_LEAVE();
		return (true);
	}
}

// ****************************************************************************
bool LudbVBLinkMessageHandler(VBLINK_MOBILE_REG_EVENT event,
							  T_CNI_RIL3_IE_MOBILE_ID mobile_id)
{ 
	DBG_FUNC("LudbVBLinkMessageHandler", LUDB_LAYER);
	DBG_ENTER();

	MSG_Q_ID qid = ludbMsgQId;
	LudbMsgStruct	ludbInMsg;
 
	ludbInMsg.module_id = MODULE_H323;
	ludbInMsg.mobile_id = mobile_id;

	switch (event) {
		case VBLINK_MOBILE_REG_EVENT_RECV_URQ:
			ludbInMsg.msg_type   = VOIP_LUDB_UNREG_REQ;
			break;

		case VBLINK_MOBILE_REG_EVENT_RECV_RCF:
			ludbInMsg.msg_type   = VOIP_LUDB_REG_CNF;
			break;

		case VBLINK_MOBILE_REG_EVENT_RECV_RRJ:
			ludbInMsg.msg_type   = VOIP_LUDB_REG_REJ;
			break;

		case VBLINK_MOBILE_REG_EVENT_RECV_UCF:
			ludbInMsg.msg_type   = VOIP_LUDB_UNREG_CNF;
			break;

		case VBLINK_MOBILE_REG_EVENT_RECV_URJ:
			ludbInMsg.msg_type   = VOIP_LUDB_UNREG_REJ;
			break;

		default:
			DBG_ERROR("Unexpected RegEvent Type : %d\n", event);
			DBG_LEAVE();
			return (false);
	}

	// send the message.
	STATUS stat = msgQSend( qid, (char *) &ludbInMsg, 
						sizeof(LudbMsgStruct), 
						NO_WAIT, MSG_PRI_NORMAL ); 
	if(stat == ERROR){
		DBG_ERROR("msgQSend error\n");
		DBG_LEAVE();
		return (false);
	}
	else {
		DBG_TRACE("Voip->Ludb MsgHandler MsgType: %s\n", 
					getLudbMsgTypeString(ludbInMsg.msg_type));
		DBG_LEAVE();
		return (true);
	}
}


//*****************************************************************************
//support legcy functions.
//*****************************************************************************
bool ludbGetSubscInfoExists(short index)
{
	return gLUDB.getSubscExist(index);
}


//*****************************************************************************
void ludbSetSubscInfoExists(short index, bool flag)
{
	gLUDB.setSubscExist(index, flag);
}


//*****************************************************************************
bool ludbGetSecInfoExists(short index)
{
	return gLUDB.getSecExist(index);
}


//*****************************************************************************
void ludbSetSecInfoExists(short index, bool flag)
{
	gLUDB.setSecExist(index, flag);
}


//*****************************************************************************
short ludbCheckAuthCount(short index)
{
	gLUDB.checkAuthCount(index);
}

//*****************************************************************************
T_AUTH_TRIPLET *ludbCurTriplet(short index)
{
	return gLUDB.curTripletPtr(index);
}

//*****************************************************************************
char ludbCKSN(short index)
{
	return gLUDB.curCKSN(index);
}

//*****************************************************************************
char ludbIncCKSN(short index)
{
	return gLUDB.incCKSN(index);
}

//*****************************************************************************
short ludbCountBeingUsed()
{
	short count=0;
	for(int i=0; i<LUDB_MAX_NUM_ENTRY; i++){
		if(!gLUDB.isFree(i)) count++;
	}
	return count;
}

//*****************************************************************************
short ludbPercentBeingUsed()
{
	return (ludbCountBeingUsed() * 100)/LUDB_MAX_NUM_ENTRY;
}

//*****************************************************************************
void ludbDataPrint()
{
    printf("LUDB Data *********************************************************\n");
	printf(" %d out of %d entries (%d percent) is being used.\n",
			ludbCountBeingUsed(), LUDB_MAX_NUM_ENTRY, ludbPercentBeingUsed()); 
    gLUDB.print();
    printf("*******************************************************************\n");
}

//*****************************************************************************
void ludbHashPrint()
{
    printf("LUDB Hash *********************************************************\n");
    ludbImsiHash->print();
    printf("*******************************************************************\n");
}

//*****************************************************************************
void ludbDataInit()
{
    gLUDB.init();
}


//*****************************************************************************
void ludbEntryInit(const short index) // clean up entry - klim 3/29/99 
{
	DBG_FUNC("ludbEntryInit", LUDB_LAYER);
	DBG_ENTER();
	gLUDB.init(index);
    DBG_TRACE("entry %d initialized\n", index);
	DBG_LEAVE();
}

//*****************************************************************************
void ludbSetMMId(const short index, short mmId)
{
	DBG_FUNC("ludbSetMMId", LUDB_LAYER);
	DBG_ENTER();
	gLUDB.setMMId(index, mmId);
	DBG_LEAVE();
}

//*****************************************************************************
short ludbGetMMId(const short index)
{
	DBG_FUNC("ludbSetMMId", LUDB_LAYER);
	DBG_ENTER();
	DBG_LEAVE();
	return(gLUDB.getMMId(index));
}

//*****************************************************************************
T_CNI_RIL3_IE_MOBILE_ID ludbGetMobileId(short index)
{
	return gLUDB.getMobileId(index);
}

//*****************************************************************************
short ludbGetEntryByMobileId(T_CNI_RIL3_IE_MOBILE_ID id)
{
	return gLUDB.getEntry(id);
}

//*****************************************************************************
T_SUBSC_INFO* ludbGetSubscInfoPtr(const short index)
{
	if(ludbGetSubscInfoExists(index))
		return gLUDB.getSubscInfoPtr(index);
	else
		return 0;
}

//*****************************************************************************
T_SUBSC_IE_MOBILE_ID_IMSI* ludbGetMobileIdPtr(const short index)
{
	if(ludbGetSubscInfoExists(index))
		return gLUDB.getMobileIdPtr(index);
	else
		return 0;
}

//*****************************************************************************
T_SUBSC_IE_ISDN*			ludbGetMSISDNPtr(const short index)
{
	if(ludbGetSubscInfoExists(index))
		return gLUDB.getMSISDNPtr(index);
	else
		return 0;
}

//*****************************************************************************
T_SUBSC_IE_PROVISION_BS*	ludbGetProvBSPtr(const short index)
{
	if(ludbGetSubscInfoExists(index))
		return gLUDB.getProvBSPtr(index);
	else
		return 0;
}

//*****************************************************************************
T_SUBSC_IE_PROVISION_TS*	ludbGetProvTSPtr(const short index)
{
	if(ludbGetSubscInfoExists(index))
		return gLUDB.getProvTSPtr(index);
	else
		return 0;
}

//*****************************************************************************
T_SUBSC_IE_PROVISION_SS*	ludbGetProvSSPtr(const short index)
{
	if(ludbGetSubscInfoExists(index))
		return gLUDB.getProvSSPtr(index);
	else
		return 0;
}

//*****************************************************************************
T_SUBSC_IE_SUBSC_STATUS*	ludbGetSubscStatusPtr(const short index)
{
	if(ludbGetSubscInfoExists(index))
		return gLUDB.getSubscStatusPtr(index);
	else
		return 0;
}

//*****************************************************************************
T_SUBSC_IE_ODB_DATA*		ludbGetODBDataPtr(const short index)
{
	if(ludbGetSubscInfoExists(index))
		return gLUDB.getODBDataPtr(index);
	else
		return 0;
}

//*****************************************************************************
T_SUBSC_IE_SS_INFO_LIST*	ludbGetSSInfoListPtr(const short index)
{
	if(ludbGetSubscInfoExists(index))
		return gLUDB.getSSInfoListPtr(index);
	else
		return 0;
}

//*****************************************************************************
T_SUBSC_IE_USER_INFO*		ludbGetUserInfoPtr(const short index)
{
	if(ludbGetSubscInfoExists(index))
		return gLUDB.getUserInfoPtr(index);
	else
		return 0;
}

//*****************************************************************************
//*****************************************************************************
bool	ludbIsProvSMS_MT(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscProvSMS_MT_PP(ss));
}

//*****************************************************************************
bool	ludbIsProvSMS_MO(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscProvSMS_MO_PP(ss));
}

//*****************************************************************************
bool	ludbIsProvCLIP(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvCLIP(ss));
}

//*****************************************************************************
bool	ludbIsProvCLIR(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvCLIR(ss));
}

//*****************************************************************************
// shmin 

int	ludbGetCLIROption(const short index, T_SS_SUBSC_CLIR_OPTION *option, bool *isProvCLIR)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);
	if(subscIsProvCLIR(ss) == 0){
		*isProvCLIR = 1;
		return 0;
	}
	else{
		*isProvCLIR = 0;
		return(subscGetCLIROption(ss,option));
	}
}

void 	testludbGetCLIROption(const short index)
{
int	ret;
T_SS_SUBSC_CLIR_OPTION *option;
bool 	*isProvCLIR;

	ret = ludbGetCLIROption(index, option, isProvCLIR);
	printf("LUDB return %d ludbGetCLIROption index %d, isProvCLIR %d, option %d", ret, index, *isProvCLIR, *option);
}

//*****************************************************************************
bool	ludbIsProvCOLP(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvCOLP(ss));
}

//*****************************************************************************
bool	ludbIsProvCOLR(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvCOLR(ss));
}

//*****************************************************************************
bool	ludbIsProvCFU(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvCFU(ss));
}

//*****************************************************************************
bool	ludbIsProvCFB(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvCFB(ss));
}

//*****************************************************************************
bool	ludbIsProvCFNRY(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvCFNRY(ss));
}

//*****************************************************************************
bool	ludbIsProvCFNRC(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvCFNRC(ss));
}

//*****************************************************************************
bool	ludbIsProvECT(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvECT(ss));
}

//*****************************************************************************
bool	ludbIsProvCW(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvCW(ss));
}

//*****************************************************************************
bool	ludbIsProvHOLD(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvHOLD(ss));
}

//*****************************************************************************
bool	ludbIsProvMPTY(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvMPTY(ss));
}

//*****************************************************************************
bool	ludbIsProvBAOC(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvBAOC(ss));
}

//*****************************************************************************
bool	ludbIsProvBOIC(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvBOIC(ss));
}

//*****************************************************************************
bool	ludbIsProvBOICE(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvBOICE(ss));
}

//*****************************************************************************
bool	ludbIsProvBAIC(const short index)
{
	T_SUBSC_INFO* ss = ludbGetSubscInfoPtr(index);	
	return(subscIsProvBAIC(ss));
}

//*****************************************************************************
bool	ludbIsActiveCFU(const short index)
{
	T_SUBSC_IE_SS_INFO_LIST	*info_list = ludbGetSSInfoListPtr(index);
	return(subscIsActiveCF(info_list, SUPP_SERV_CFU));
}

//*****************************************************************************
bool	ludbIsActiveCFB(const short index)
{
	T_SUBSC_IE_SS_INFO_LIST	*info_list = ludbGetSSInfoListPtr(index);
	return(subscIsActiveCF(info_list, SUPP_SERV_CFB));
}

//*****************************************************************************
bool	ludbIsActiveCFNRY(const short index)
{
	T_SUBSC_IE_SS_INFO_LIST	*info_list = ludbGetSSInfoListPtr(index);
	return(subscIsActiveCF(info_list, SUPP_SERV_CFNRY));
}

//*****************************************************************************
int		ludbGetTimerCFNRY(const short index)
{
	T_SUBSC_IE_SS_INFO_LIST	*info_list = ludbGetSSInfoListPtr(index);
	return(subscGetTimerCFNRY(info_list));
}

//*****************************************************************************
bool	ludbIsActiveCFNRC(const short index)
{
	T_SUBSC_IE_SS_INFO_LIST	*info_list = ludbGetSSInfoListPtr(index);
	return(subscIsActiveCF(info_list, SUPP_SERV_CFNRC));
}

//*****************************************************************************
bool	ludbIsActiveCW(const short index)
{
	T_SUBSC_IE_SS_INFO_LIST	*info_list = ludbGetSSInfoListPtr(index);
	return(subscIsActiveCW(info_list));
}

//*****************************************************************************
// prints
//*****************************************************************************
void	ludbPrintSubscInfo(const short index)
{
	subscPrintInfo(ludbGetSubscInfoPtr(index));
}

//*****************************************************************************
void	ludbPrintImsi(const short index)
{
	subscPrintImsi(ludbGetMobileIdPtr(index));
}

//*****************************************************************************
void	ludbPrintIsdn(const short index)
{
	subscPrintIsdn(ludbGetMSISDNPtr(index));
}

//*****************************************************************************
void	ludbPrintProvBS(const short index)
{
	subscPrintProvBS(ludbGetProvBSPtr(index));
}

//*****************************************************************************
void	ludbPrintProvTS(const short index)
{
	subscPrintProvTS(ludbGetProvTSPtr(index));
}

//*****************************************************************************
void	ludbPrintProvSS(const short index)
{
	subscPrintProvSS(ludbGetProvSSPtr(index));
}

//*****************************************************************************
void	ludbPrintSubscStatus(const short index)
{
	subscPrintSubscStatus(ludbGetSubscStatusPtr(index));
}

//*****************************************************************************
void	ludbPrintODBData(const short index)
{
	subscPrintODBData(ludbGetODBDataPtr(index));
}

//*****************************************************************************
void	ludbPrintSSInfoList(const short index)
{
	subscPrintSSInfoList(ludbGetSSInfoListPtr(index));
}

//*****************************************************************************
void	ludbPrintUserInfo(const short index)
{
	subscPrintUserInfo(ludbGetUserInfoPtr(index));
}

//*****************************************************************************
void 	ludbPrintSRS(const short index)
{
	gLUDB.printSRS(index);
}

//*****************************************************************************
//*****************************************************************************
short	ludbSetDecodeSubscInfo(const short index, unsigned char *msg, short size)
{
	T_SUBSC_INFO	subsc;
	short	ret;

	ret = DecodeSubscInfo(msg, size, &subsc);	
	if(ret)
		return ret;
	else 
		return gLUDB.update(index, subsc);
} 

//*****************************************************************************
// VOIP support
//*****************************************************************************
bool	ludbGetMobileNumber(const short ludbId, char *num)
{
	if(num == 0) return false;

	T_SUBSC_IE_ISDN* msisdn;
	msisdn = ludbGetMSISDNPtr(ludbId);
	if(msisdn == 0) return false;
	if(msisdn->num_digit > MAX_NUM_SUBSC_ISDN) return false;
	for(int i=0; i<msisdn->num_digit; i++)
		num[i] = msisdn->digits[i];
	num[i] = '\0';
	return true;
}

//*****************************************************************************
bool	ludbGetMobileName(const short ludbId, char *name)
{
	if(name == 0) return false;

	T_SUBSC_IE_USER_INFO* user;
	user = ludbGetUserInfoPtr(ludbId);
	if(user == 0) return false;
	if(user->len > MAX_NUM_USER_INFO) return false;
	for(int i=0; i<user->len; i++)
		name[i] = user->name[i];
	name[i] = '\0';
	return true;
}

//*****************************************************************************
bool	isSameISDN(T_SUBSC_IE_ISDN *isdn, char *num)
{
	if(isdn->num_digit == 0) return false;
	if(isdn->num_digit != strlen(num)) return false;
	for(int i=0; i<isdn->num_digit; i++){
		if(isdn->digits[i] != num[i])
			return false;
	}
	return true;
}

//*****************************************************************************
bool	ludbIsMobileRegistered(char *num, short *ludbId)
{
	T_SUBSC_IE_ISDN* isdn;
	DBG_FUNC("ludbIsMobileRegistered", LUDB_LAYER);
	DBG_ENTER();
	for(int i=0; i<LUDB_MAX_NUM_ENTRY; i++){
		if((isdn = ludbGetMSISDNPtr(i)) != 0){
			if(isSameISDN(isdn, num)){
				*ludbId = i;
				DBG_TRACE("Found Mobile %s in LUDB [%d]\n", num, i);
				DBG_LEAVE();
				return true;
			}
		}
	}
	DBG_WARNING("Mobile %s not registered in LUDB\n", num);
	DBG_LEAVE();
	return false;
}

//*****************************************************************************
VOIP_MOBILE_HANDLE ludbGetVoipMobileHandle(const short ludbId)
{
// printf("[ludbGetVoipMobileHandle] Voip Mobile Handle is no longer supported\n");
	return 0; 
}


//*****************************************************************************
void ludbSetCipherAlgo(short index, short algo)
{
	gLUDB.setAlgo(index, algo);
}

//*****************************************************************************
short ludbGetCipherAlgo(short index, IntraL3CipherModeCommand_t *cmd)
{
	DBG_FUNC("ludbGetCipherAlgo", LUDB_LAYER);
	DBG_ENTER();
	short algo = gLUDB.curAlgo(index);
	if(algo == -1){
		DBG_ERROR("index out of range %d\n", index);
		DBG_LEAVE();
		return -1;
	}
	if(cmd == 0){
		DBG_ERROR("null pointer for cmd field\n");
		DBG_LEAVE();
		return -1;
	}

	if(algo == 0){
		cmd->cmd.cipherModeSetting.ciphering = CNI_RIl3_CIPHER_NO_CIPHERING;
		for(int i=0; i<SEC_KC_LEN; i++) cmd->Kc[i] = 0xFF; 
		DBG_LEAVE();
		return 0;
	}
	else if(algo == 1){
		cmd->cmd.cipherModeSetting.ciphering = CNI_RIl3_CIPHER_START_CIPHERING;
		cmd->cmd.cipherModeSetting.algorithm = CNI_RIL3_CIPHER_ALGORITHM_A51;
	}
	else if(algo == 2){
		cmd->cmd.cipherModeSetting.ciphering = CNI_RIl3_CIPHER_START_CIPHERING;
		cmd->cmd.cipherModeSetting.algorithm = CNI_RIL3_CIPHER_ALGORITHM_A52;
	}
	else{
		DBG_ERROR("ludb %d, cipher mode was not set previously\n");
		cmd->cmd.cipherModeSetting.ciphering = CNI_RIl3_CIPHER_NO_CIPHERING;
	}
	DBG_TRACE("ludb %d, cipherModeSetting.ciphering: %d\n", index, cmd->cmd.cipherModeSetting.ciphering);
	DBG_TRACE("ludb %d, cipherModeSetting.algorithm: %d\n", index, cmd->cmd.cipherModeSetting.algorithm);

	T_AUTH_TRIPLET *triplet = ludbCurTriplet(index);
	if(triplet == 0){
		DBG_ERROR("triplet returned 0 in CipherModeCmd for ludb %d\n", index);
		DBG_LEAVE();
		return -1;
	}
	else{
		for(int i=0; i<SEC_KC_LEN; i++) 
			cmd->Kc[i] = triplet->Kc[i];
	}

	DBG_LEAVE();
	return 0;
}

