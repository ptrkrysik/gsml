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
// File        : LUDBInstance.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : Local User Database. 
//
// Modified History:
// merge mmcc-h323 with integration. -03/04/99
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// from common vob 
#include "subsc/SubscInfoType.h"
#include "subsc/SubscInfoUtil.h"

#include "JCC/LUDB.h"
#include "JCC/LUDBConfig.h"
#include "JCC/JCCL3Msg.h"
#include "JCC/JCCUtil.h"
#include "JCC/LUDBApi.h"

#include "LudbHashTbl.h"

#include "semLib.h"
#include "stdioLib.h"
#include "sysLib.h"

#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

// *******************************************************************
// Constant Definitions
// *******************************************************************
extern int ludb_purge_timer_sec;

extern LUDB				gLUDB;
extern short			ludb_auth_required;
extern short			ludb_auth_freq;
bool srs_debug_mode = false;

extern LudbHashTbl		*ludbImsiHash;

//*****************************************************************************
LUDBEntry::LUDBEntry()
{
	ludbSemId = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);
	VBReqTimer = new JCCTimer((FUNCPTR)ludbVBReqTimeout);
	PurgeTimer = new JCCTimer((FUNCPTR)ludbPurgeTimeout);
	init();
}


//*****************************************************************************
void LUDBEntry::init()
{
	// take the semaphore 
	semTake(ludbSemId, WAIT_FOREVER);
	mobileId.mobileIdType		= CNI_RIL3_NO_MOBILE_ID;
	classmarkType				= NULL_CLASSMARK;
	free						= true;
	state						= IDLE;
	mobileHandle				= NULL;
	mm_id						= -1;
	subsc_exist					= false;
	stopVBReqTimer();
	stopPurgeTimer();
	auth_reuse_count			= 0;
	sec_exist					= false;
	cur_cksn					= 0x07;
	cur_algo					= 0;

	// Now let go of the semaphores.
	semGive(ludbSemId);
}


//*****************************************************************************
void LUDBEntry::isFree (bool free)
{
	semTake(ludbSemId, WAIT_FOREVER);
	LUDBEntry::free = free;
	semGive(ludbSemId);
}

//*****************************************************************************
void LUDBEntry::setSubscExist (bool flag)
{
	semTake(ludbSemId, WAIT_FOREVER);
	LUDBEntry::subsc_exist = flag;
	semGive(ludbSemId);
}

//*****************************************************************************
void LUDBEntry::setSecExist (bool flag)
{
	semTake(ludbSemId, WAIT_FOREVER);
	LUDBEntry::sec_exist = flag;
	semGive(ludbSemId);
}

//*****************************************************************************
char *LUDBEntry::getStateString()
{
	switch(state){
		case IDLE:			return("IDLE      ");
		case REGREQ:		return("WAIT4REG  ");
		case REGTHENSEC:	return("WAIT4REG* ");
		case SECREQ:		return("WAIT4SEC  ");
		case SECINREG:		return("WAIT4SEC* ");
		case ACTIVE:		return("ACTIVE    ");
		case UNREGREQ:		return("WAIT4UNREG");
		case INACTIVE:		return("INACTIVE  ");
		case UNREGRJT:		return("UNREG_REJ ");
		case EMERGENCY:		return("EMERGENCY ");
		default:			return("UNKNOWN   ");
	} 
}

//*****************************************************************************
void LUDBEntry::startVBReqTimer(int sec, short ludb_id)
{
	int tick = sec * sysClkRateGet();
	VBReqTimer->setTimer(tick, (int)ludb_id);
}

//*****************************************************************************
void LUDBEntry::stopVBReqTimer()
{
	VBReqTimer->cancelTimer();
}

//*****************************************************************************
void LUDBEntry::startPurgeTimer(int sec, short ludb_id)
{
	int tick = sec * sysClkRateGet();
	PurgeTimer->setTimer(tick, (int)ludb_id);
}

//*****************************************************************************
void LUDBEntry::stopPurgeTimer()
{
	PurgeTimer->cancelTimer();
}

//*****************************************************************************
T_AUTH_TRIPLET* LUDBEntry::curTripletPtr()
{
	if(secExist()){
		return &sec_info;
	}
	else{
		return 0;	// error
	}
}

//*****************************************************************************
// LUDB
//*****************************************************************************
LUDB::LUDB()
{
	semId = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);
	init();
}


//*****************************************************************************
void LUDB::init()
{
    for(int i=0; i< LUDB_MAX_NUM_ENTRY; i++) {
		if(!entries[i].isFree()){
			int bucket_id;
			if(ludbImsiHash->find(i, bucket_id)) ludbImsiHash->remove(i);
		}
		entries[i].init();
	}
}


//*****************************************************************************
// clean up entry - klim 3/29/99
void LUDB::init(const short index)
{
	if(inRange(index)){
		entries[index].init();
		int bucket_id;
		if(ludbImsiHash->find(index, bucket_id)) ludbImsiHash->remove(index);
	}
}


//*****************************************************************************
short LUDB::entryAlloc()
{
	semTake(semId, WAIT_FOREVER);

	for(short i=0; i< LUDB_MAX_NUM_ENTRY;i++)
	{
		if (entries[i].isFree() == true)
		{
			entries[i].isFree(false);
			semGive(semId);
			return i;
		}
	}
	semGive(semId);
	return JCC_ERROR;
}


//*****************************************************************************
void LUDB::entryDealloc(short i)
{
	if(inRange(i)){
		semTake(semId, WAIT_FOREVER);
		entries[i].init();
		semGive(semId);
	}
}


//*****************************************************************************
bool LUDB::startVBReqTimer(short i)
{
	if(inRange(i)){
		entries[i].startVBReqTimer(VB_REQ_TIMER_SEC, i);
		return true;
	}
	else{
		return false;
	}
}

//*****************************************************************************
bool LUDB::stopVBReqTimer(short i)
{
	if(inRange(i)){
		entries[i].stopVBReqTimer();
		return true;
	}
	else{
		return false;
	}
}

//*****************************************************************************
bool LUDB::startPurgeTimer(short i)
{
	if(inRange(i)){
		entries[i].startPurgeTimer(ludb_purge_timer_sec, i);
		return true;
	}
	else{
		return false;
	}
}

//*****************************************************************************
bool LUDB::stopPurgeTimer(short i)
{
	if(inRange(i)){
		entries[i].stopPurgeTimer();
		return true;
	}
	else{
		return false;
	}
}

//*****************************************************************************
void LUDB::setMMId(const short i, short mmId)
{
	if(inRange(i)){
		entries[i].mm_id = mmId;
	}
}

//*****************************************************************************
short LUDB::getMMId(const short i)
{
	if(inRange(i)){
		return(entries[i].mm_id);
	}
	else{
		return -1;
	}
}

//*****************************************************************************
// Print LUDB Data
void LUDB::print()
{
	printf(" [ID] IMSI            STATE      Voip     Number\n");
	for(short i=0; i<LUDB_MAX_NUM_ENTRY;i++)
	{
		if (entries[i].isFree())
		{
			//	printf("LUDB Entry %d is free.\n", i);
		}
		else
		{
			printf(" [%2d] ", i);
			printMobileId(entries[i].mobileId);
			printf(" %s", entries[i].getStateString());
			printf(" %08x ", entries[i].mobileHandle);	
			ludbPrintIsdn(i);	
			printf("\n");
		}
	}
  
}
  
//*****************************************************************************
// Register user
short LUDB::registerUser(T_CNI_RIL3_IE_MOBILE_ID mobileId)
{
	short i = JCC_ERROR;

	if ( (i = entryAlloc()) != JCC_ERROR)
	{
		// take the semaphore 
		semTake(semId, WAIT_FOREVER);
		entries[i].mobileId = mobileId;
		semGive(semId);

		ludbImsiHash->add(mobileId.digits, i);
	}

	return i;
}


//*****************************************************************************
//get LUDB index for given IMSI
short LUDB::getEntry (T_CNI_RIL3_IE_MOBILE_ID mobileId)
{
    short index = JCC_ERROR;

	int bucket_id;
	LudbLinkedListItem *item = ludbImsiHash->find(mobileId.digits, bucket_id);
	if(item)
		index = item->value;
 
    return index;
} 


//*****************************************************************************
void  LUDB::update (short index, LUDBEntry::LUDBState state)
{
	DBG_FUNC("LUDB::update(state)", LUDB_LAYER);
	DBG_ENTER();

    //  index range check and state check.
	if ((state >= LUDBEntry::IDLE)&&(state <= LUDBEntry::MAXLUDBSTATE)
	  &&(inRange(index)))
		entries[index].state = state;
	else
		DBG_ERROR("invalid state %d or invalid index %d\n", state, index);

	DBG_LEAVE();
}

//*****************************************************************************
void LUDB::update (short index, VOIP_MOBILE_HANDLE mobileHandle)
{
	DBG_FUNC("LUDB::update(voip_handle)", LUDB_LAYER);
	DBG_ENTER();

	if(inRange(index))
	{
		// check if ludb is free
		if(entries[index].isFree())
			DBG_ERROR("ludb entry is Free, index %d\n", index);
		else
			entries[index].mobileHandle = mobileHandle;
	}
	else
		DBG_ERROR("ludb entry invalid index %d\n", index);

	DBG_LEAVE();
}

//*****************************************************************************
short LUDB::update (short index, T_SUBSC_INFO info)
{
	DBG_FUNC("LUDB::update(subsc_info)", LUDB_LAYER);
	DBG_ENTER();

	if(inRange(index))
	{
		// check if ludb is free
		if(entries[index].isFree()){
			DBG_ERROR("ludb entry is Free, index %d\n", index);
			DBG_LEAVE();
			return 1;
		}
		else{
			entries[index].subsc_info = info;
			DBG_LEAVE();
			return 0;
		}
	}
	else{
		DBG_ERROR("ludb entry invalid index %d\n", index);
		DBG_LEAVE();
		return 1;
	}
}

//*****************************************************************************
void LUDB::printSRS(short index)
{
	int i;
	if(!inRange(index)) return;
	
	printf("SRS_DEBUG---------------------------------------------------\n");
	printf("RAND: ");
	for(i=0; i<SEC_RAND_LEN; i++) printf("%02X ", entries[index].sec_info.RAND[i]);
	printf("\n");
	printf("SRES: ");
	for(i=0; i<SEC_SRES_LEN; i++) printf("%02X ", entries[index].sec_info.SRES[i]);
	printf("\n");
	printf("KC: ");
	for(i=0; i<SEC_KC_LEN; i++) printf("%02X ", entries[index].sec_info.Kc[i]);
	printf("\n");
	printf("------------------------------------------------------------\n");
}

//*****************************************************************************
short LUDB::update (short index, T_AUTH_TRIPLET info)
{
	DBG_FUNC("LUDB::update(sec_info)", LUDB_LAYER);
	DBG_ENTER();

	if(inRange(index)){
		if(entries[index].isFree()){		// check if ludb is free
			DBG_ERROR("ludb entry is Free, index %d\n", index);
			DBG_LEAVE();
			return 1;
		}
		else{
			setSecExist(index, true);
			incCKSN(index);
			entries[index].sec_info = info;
			entries[index].auth_reuse_count = 0;
			if(srs_debug_mode) ludbPrintSRS(index);
			DBG_LEAVE();
			return 0;
		}
	}
	else{
		DBG_ERROR("ludb entry invalid index %d\n", index);
		DBG_LEAVE();
		return 1;
	}
}

//*****************************************************************************
LUDBEntry::LUDBState LUDB::getState(const short index)
{
	if(inRange(index))
   		return entries[index].state;
	else
		return LUDBEntry::IDLE;
}

//*****************************************************************************
bool LUDB::isFree(const short index)
{
	if(inRange(index))
		return entries[index].isFree();
	else 
		return false;
}

//*****************************************************************************
bool LUDB::getSubscExist(short index)
{
	if(inRange(index))
		return entries[index].subscExist();
	else
		return false;
}

//*****************************************************************************
void LUDB::setSubscExist(short index, bool flag)
{
	if(inRange(index))
		entries[index].setSubscExist(flag);
}

//*****************************************************************************
bool LUDB::getSecExist(short index)
{
	if(inRange(index))
		return entries[index].secExist();
	else
		return false;
}

//*****************************************************************************
void LUDB::setSecExist(short index, bool flag)
{
	if(inRange(index))
		entries[index].setSecExist(flag);
}

//*****************************************************************************
short LUDB::checkAuthCount(const short index)
{
	short count;
    
	DBG_FUNC("LUDB::checkAuthCount(index)", LUDB_LAYER);
	DBG_ENTER();

	if(inRange(index)){
		if(!ludbGetSecInfoExists(index)){
			DBG_TRACE("no sec info exist\n");
			DBG_LEAVE();
			return 1;
		}

		if(++entries[index].auth_reuse_count >= ludb_auth_freq){
			DBG_TRACE("Hit ludb_auth_freq! resetting ludb_auth_count to 0\n");
			DBG_LEAVE();
			return 1;	// need to fetch new set 
		}
	}
	else{
		DBG_LEAVE();
		return -1;
	}

	DBG_TRACE("auth_reuse_count [%d]: %d\n", index, entries[index].auth_reuse_count);
	DBG_LEAVE();
	return 0;	// okay to use current key
}

//*****************************************************************************
T_AUTH_TRIPLET* LUDB::curTripletPtr(short index)
{
	if(inRange(index)){
		return entries[index].curTripletPtr(); 
	}
	else
		return 0;
}

//*****************************************************************************
short	LUDB::curAlgo(short i)
{
	if(inRange(i)){
		return entries[i].cur_algo; 
	}
	else
		return -1;
}

//*****************************************************************************
void	LUDB::setAlgo(short i, short algo)
{
	DBG_FUNC("LUDB::setAlgo", LUDB_LAYER);
	DBG_ENTER();
	if(inRange(i)){
		if(!entries[i].isFree()){
			DBG_TRACE("setting ludb %d algo to %d\n", i, algo);
			entries[i].cur_algo = algo; 
		}
	}
	DBG_LEAVE();
}

//*****************************************************************************
char LUDB::curCKSN(short index)
{
	if(inRange(index)){
		return entries[index].cur_cksn; 
	}
	else
		return (char)0x07;
}

//*****************************************************************************
char LUDB::incCKSN(short index)
{
	if(inRange(index)){
		entries[index].cur_cksn = (entries[index].cur_cksn + 1) % 7; 
		return entries[index].cur_cksn; 
	}
	else
		return (char)0x07;
}

//*****************************************************************************
VOIP_MOBILE_HANDLE LUDB::getMobileHandle (const short index)
{
	DBG_FUNC("LUDB::getMobileHandle", LUDB_LAYER);
	DBG_ENTER();
	VOIP_MOBILE_HANDLE retVal = VOIP_NULL_MOBILE_HANDLE;
	
	if(inRange(index))
	{
		//check if ludb is free
		if(entries[index].isFree())
			DBG_ERROR("ludb entry is Free, index %d\n", index);
		else
			retVal = entries[index].mobileHandle;
	}
	else
		DBG_ERROR("ludb entry invalid index %d\n", index);

	DBG_LEAVE();
	return retVal;
}



//*****************************************************************************
bool LUDB::authTerm(const short index,
		   			const T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER& cpn)
{
	// For now, don't have to check anything, no profile yet.
	if(inRange(index)){
		if (!isFree(index)) {
			return true;	
		}
	}
	return false;
}

//*****************************************************************************
bool LUDB::authOrig(const short index,
					const T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER& cpn)
{
	// For now, don't have to check anything, no profile yet.
	if(inRange(index)){
		if(!isFree(index)){
			return true;
		}
	}
	return false;
}

//*****************************************************************************
// Stage 2 LUDB changes needed for Orig and Term Calls - Begin
T_CNI_RIL3_IE_MOBILE_ID
LUDB::getMobileId (const short index)
{
  T_CNI_RIL3_IE_MOBILE_ID retval;

  retval.mobileIdType = CNI_RIL3_NO_MOBILE_ID;

	if(inRange(index)&&(!entries[index].isFree()))
	{
	  return (entries[index].mobileId);
	}

  return (retval);

}
// Stage 2 LUDB changes needed for Orig and Term Calls - End

//*****************************************************************************
bool	LUDB::inRange(short index)
{
	if((index >= 0)&&(index < LUDB_MAX_NUM_ENTRY)){
		return true;
	}
	else{
		return false;
	}
}

//*****************************************************************************
T_SUBSC_INFO* LUDB::getSubscInfoPtr(const short index)
{
	if(inRange(index))
		return(&entries[index].subsc_info);
	else{ 
		printf("LUDB::getSubscInfoPtr - index out of bound\n");
		return	0;
	}
}

//*****************************************************************************
T_SUBSC_IE_MOBILE_ID_IMSI* LUDB::getMobileIdPtr(const short index)
{
	T_SUBSC_INFO *info = getSubscInfoPtr(index);
	if(info != 0) return(&info->imsi);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_ISDN* LUDB::getMSISDNPtr(const short index)
{
	T_SUBSC_INFO *info = getSubscInfoPtr(index);
	if(info != 0) return(&info->msisdn);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_PROVISION_BS* LUDB::getProvBSPtr(const short index)
{
	T_SUBSC_INFO *info = getSubscInfoPtr(index);
	if(info != 0) return(&info->prov_bs);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_PROVISION_TS* LUDB::getProvTSPtr(const short index)
{
	T_SUBSC_INFO *info = getSubscInfoPtr(index);
	if(info != 0) return(&info->prov_ts);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_PROVISION_SS* LUDB::getProvSSPtr(const short index)
{
	T_SUBSC_INFO *info = getSubscInfoPtr(index);
	if(info != 0) return(&info->prov_ss);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_SUBSC_STATUS* LUDB::getSubscStatusPtr(const short index)
{
	T_SUBSC_INFO *info = getSubscInfoPtr(index);
	if(info != 0) return(&info->status);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_ODB_DATA* LUDB::getODBDataPtr(const short index)
{
	T_SUBSC_INFO *info = getSubscInfoPtr(index);
	if(info != 0) return(&info->odb_data);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_SS_INFO_LIST* LUDB::getSSInfoListPtr(const short index)
{
	T_SUBSC_INFO *info = getSubscInfoPtr(index);
	if(info != 0) return(&info->ss_info_list);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_USER_INFO* LUDB::getUserInfoPtr(const short index)
{
	T_SUBSC_INFO *info = getSubscInfoPtr(index);
	if(info != 0) return(&info->user);
	else return 0;
}

