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
// File        : MMConfig.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "oam_api.h"
#include "LOGGING/vcmodules.h"
#include "LOGGING/vclogging.h"
#include "MnetModuleId.h"
#include "ril3/ie_location_area_id.h"
#include "MM/mmConfig.h"

#include "sysLib.h"

// Global variable
extern MSG_Q_ID mmMsgQId;
extern MobilityConfigData	mmCfg;
extern T_CNI_RIL3_IE_LOCATION_AREA_ID	mmCfgLocId;
extern short ludb_auth_required;
extern int mmCfgIMSIrequired;

void mmConfigPrint()
{
	DBG_FUNC("MMConfigPrint", MM_LAYER);
	DBG_ENTER();
	DBG_TRACE(" MM OAM Config Data ---------------------------------------------------------\n");
	DBG_TRACE("mm_max_mm_connections \t\t:%d\n",mmCfg.mm_max_mm_connections);
	DBG_TRACE("mm_paging_t3113 \t\t\t:%d\n",mmCfg.mm_paging_t3113);
	DBG_TRACE("authenticationRequired \t:%d\n",ludb_auth_required);
	DBG_TRACE("mm_cipheringRequired \t\t:%d\n",mmCfg.mm_cipheringRequired);
	DBG_TRACE("mm_IMEICheckRequired \t\t:%d\n",mmCfg.mm_IMEICheckRequired);
	DBG_TRACE("mcc \t\t:%x%x%x\n",mmCfgLocId.mcc[0], mmCfgLocId.mcc[1], mmCfgLocId.mcc[2]);
	DBG_TRACE("mnc \t\t:%x%x\n",mmCfgLocId.mnc[0], mmCfgLocId.mnc[1],mmCfgLocId.mnc[2] );
	DBG_TRACE("lac \t\t:%d\n",mmCfgLocId.lac);
	DBG_TRACE("-----------------------------------------------------------------------------\n");
	printf(" MM OAM Config Data ---------------------------------------------------------\n");
	printf("mm_max_mm_connections \t\t:%d\n",mmCfg.mm_max_mm_connections);
	printf("mm_paging_t3113 \t\t\t:%d\n",mmCfg.mm_paging_t3113);
	printf("authenticationRequired \t:%d\n",ludb_auth_required);
	printf("mm_cipheringRequired \t\t:%d\n",mmCfg.mm_cipheringRequired);
	printf("mm_IMEICheckRequired \t\t:%d\n",mmCfg.mm_IMEICheckRequired);
	printf("mcc \t\t:%x%x%x\n",mmCfgLocId.mcc[0], mmCfgLocId.mcc[1], mmCfgLocId.mcc[2]);
	printf("mnc \t\t:%x%x\n",mmCfgLocId.mnc[0], mmCfgLocId.mnc[1],mmCfgLocId.mnc[2]);
	printf("lac \t\t:%d\n",mmCfgLocId.lac);
	printf("-----------------------------------------------------------------------------\n");	
	DBG_LEAVE();	
}

void mmCfgSetIMSIRequired(INT_32_T val)
{
	mmCfgIMSIrequired = (int)val;
}

void mmCfgSetMaxMMConnections(INT_32_T val)
{
	mmCfg.mm_max_mm_connections = val;
}
	
void mmCfgSetPagingT3113(INT_32_T val)
{
	mmCfg.mm_paging_t3113 = (val * sysClkRateGet()) / 100;		// convert 10ms to tick
}

void mmCfgSetAuthenticationRequired(INT_32_T val)
{
	ludb_auth_required = (short)val;
}

void mmCfgSetCipheringRequired(INT_32_T val)
{
	mmCfg.mm_cipheringRequired = (Boolean_t)val;
}

void mmCfgSetIMEICheckRequired(INT_32_T val)
{
	mmCfg.mm_IMEICheckRequired = (Boolean_t)val;
}

void mmConvertMCC(INT_32_T oam_mcc)
{
	DBG_FUNC("mmConvertMCC", MM_LAYER);
	DBG_ENTER();
	STATUS	ret = getByteAryByInt(oam_mcc, mmCfgLocId.mcc, 3);
	if(ret != STATUS_OK){
		DBG_ERROR("getByteAryByInt returned err :%d\n", ret);
		DBG_ERROR("MCC restored with default values.\n");
		mmCfgLocId.mcc[0] =	MM_DEFAULT_BTS_MCC[0];
		mmCfgLocId.mcc[1] =	MM_DEFAULT_BTS_MCC[1];
		mmCfgLocId.mcc[2] =	MM_DEFAULT_BTS_MCC[2];	
	}
	DBG_LEAVE();
}

void mmConvertMNC(INT_32_T oam_mnc)
{
	DBG_FUNC("mmConvertMNC", MM_LAYER);
	DBG_ENTER();
//	STATUS	ret = getByteAryByInt(oam_mnc, mmCfgLocId.mnc, 2);

	STATUS	ret = oam_getMnc((unsigned char *)mmCfgLocId.mnc);
	if(ret != STATUS_OK){
		DBG_ERROR("getByteAryByInt returned err :%d\n", ret);
		DBG_ERROR("MNC restored with default values.\n");
		mmCfgLocId.mnc[0] =	MM_DEFAULT_BTS_MNC[0];
		mmCfgLocId.mnc[1] =	MM_DEFAULT_BTS_MNC[1];
		mmCfgLocId.mnc[2] =	MM_DEFAULT_BTS_MNC[2];
	}
	DBG_LEAVE();
}

void mmConvertLAC(INT_32_T oam_lac)
{
	DBG_FUNC("mmConvertLAC", MM_LAYER);
	DBG_ENTER();
	if(oam_lac > 0xFFFF){
		DBG_ERROR("oam_lac value too big\n");
		DBG_ERROR("LAC restored with default values.\n");
		mmCfgLocId.lac = 	MM_DEFAULT_BTS_LAC;
	}
	else{
		mmCfgLocId.lac = oam_lac;
	}
	DBG_LEAVE();
}
						
void mmSetOAMTraps()
{
	DBG_FUNC("mmSetOAMTraps", MM_LAYER);
	DBG_ENTER();
	STATUS oamStatus;

/*
	if((oamStatus = oam_setTrap(MODULE_MM, mmMsgQId, MIB_mm_max_mm_connections))
		!= STATUS_OK){
		DBG_ERROR("oam_setTrap failed (stauts=%d) on MIB_mm_max_mm_connections\n", oamStatus);
	}
*/
	if((oamStatus = oam_setTrap(MODULE_MM, mmMsgQId, MIB_t3113))
		!= STATUS_OK){
		DBG_ERROR("oam_setTrap failed (stauts=%d) on MIB_t3113\n", oamStatus);
	}
	if((oamStatus = oam_setTrap(MODULE_MM, mmMsgQId, MIB_mm_authenticationRequired))
		!= STATUS_OK){
		DBG_ERROR("oam_setTrap failed (stauts=%d) on MIB_mm_authenticationRequired\n", oamStatus);
	}
	if((oamStatus = oam_setTrap(MODULE_MM, mmMsgQId, MIB_mm_cipheringRequired))
		!= STATUS_OK){
		DBG_ERROR("oam_setTrap failed (stauts=%d) on MIB_mm_cipheringRequired\n", oamStatus);
	}
	if((oamStatus = oam_setTrap(MODULE_MM, mmMsgQId, MIB_mm_IMEICheckRequired))
		!= STATUS_OK){
		DBG_ERROR("oam_setTrap failed (stauts=%d) on MIB_mm_IMEICheckRequired\n", oamStatus);
	}
	if((oamStatus = oam_setTrap(MODULE_MM, mmMsgQId, MIB_rm_nim_1_0))
		!= STATUS_OK){
		DBG_ERROR("oam_setTrap failed (stauts=%d) on MIB_rm_nim_1_0\n", oamStatus);
	}
	if((oamStatus = oam_setTrap(MODULE_MM, mmMsgQId, MIB_bts_mcc))
		!= STATUS_OK){
		DBG_ERROR("oam_setTrap failed (stauts=%d) on MIB_mm_mcc\n", oamStatus);
	}
	if((oamStatus = oam_setTrap(MODULE_MM, mmMsgQId, MIB_bts_mnc))
		!= STATUS_OK){
		DBG_ERROR("oam_setTrap failed (stauts=%d) on MIB_mm_mnc\n", oamStatus);
	}
	if((oamStatus = oam_setTrap(MODULE_MM, mmMsgQId, MIB_bts_lac))
		!= STATUS_OK){
		DBG_ERROR("oam_setTrap failed (stauts=%d) on MIB_bts_lac\n", oamStatus);
	}
	if((oamStatus = oam_setTrap(MODULE_MM, mmMsgQId, MIB_bts_operationalState))
		!= STATUS_OK){
		DBG_ERROR("oam_setTrap failed (stauts=%d) on MIB_bts_operationalState\n", oamStatus);
	}
	DBG_LEAVE();
}

void mmCfgRead()
{
	INT_32_T	oam_t3113, oam_auth, oam_ciph, oam_imei; 
	INT_32_T 	oam_mcc, oam_mnc, oam_lac;
	INT_32_T	oam_imsi_required;
	DBG_FUNC("mmCfgRead", MM_LAYER);
	DBG_ENTER();

	STATUS ret;

	mmCfgSetMaxMMConnections(MM_DEFAULT_MAX_MM_CONNECTION);

	if(ret = oam_getMibIntVar(MIB_t3113, &oam_t3113)){
		DBG_ERROR("Failed to initialize mm_paging_t3113 with OAM:%d\n", ret);
		DBG_ERROR("Initialized with hard coded default values.\n");
		mmCfgSetPagingT3113(MM_DEFAULT_PAGING_T3113);
	}
	else{
		mmCfgSetPagingT3113(oam_t3113);
	}

	if(ret = oam_getMibIntVar(MIB_mm_authenticationRequired, &oam_auth)){
		DBG_ERROR("Failed to initialize mm_authenticationRequired with OAM:%d\n", ret);
		DBG_ERROR("Initialized with hard coded default values.\n");
		mmCfgSetAuthenticationRequired(MM_DEFAULT_AUTHENTICATION_REQUIRED);
	}
	else{
		mmCfgSetAuthenticationRequired(oam_auth);
	}

	if(ret = oam_getMibIntVar(MIB_mm_cipheringRequired, &oam_ciph)){
		DBG_ERROR("Failed to initialize mm_cipheringRequired with OAM:%d\n", ret);
		DBG_ERROR("Initialized with hard coded default values.\n");
		mmCfgSetCipheringRequired(MM_DEFAULT_CIPHERING_REQUIRED);
	}
	else{
		mmCfgSetCipheringRequired(oam_ciph);
	}

	if(ret = oam_getMibIntVar(MIB_mm_IMEICheckRequired, &oam_imei)){
		DBG_ERROR("Failed to initialize mm_IMEICheckRequired with OAM:%d\n", ret);
		DBG_ERROR("Initialized with hard coded default values.\n");
		mmCfgSetIMEICheckRequired(MM_DEFAULT_IMEI_CHECK_REQUIRED);
	}
	else{
		mmCfgSetIMEICheckRequired(oam_imei);
	}

	if(ret = oam_getMibIntVar(MIB_rm_nim_1_0, &oam_imsi_required)){
		DBG_ERROR("Failed to initialize mm_IMSIRequired with OAM:%d\n", ret);
		DBG_ERROR("Initialized with hard coded default values.\n");
		mmCfgSetIMSIRequired(0);
	}
	else{
		mmCfgSetIMSIRequired(oam_imsi_required);
	}

	DBG_TRACE("mm_max_mm_connections \t\t:%d\n",mmCfg.mm_max_mm_connections);
	DBG_TRACE("mm_paging_t3113 \t\t\t:%d\n",mmCfg.mm_paging_t3113);
	DBG_TRACE("authenticationRequired \t:%d\n",ludb_auth_required);
	DBG_TRACE("mm_cipheringRequired \t\t:%d\n",mmCfg.mm_cipheringRequired);
	DBG_TRACE("mm_IMEICheckRequired \t\t:%d\n",mmCfg.mm_IMEICheckRequired);

	if(ret = getMibIntVar(MIB_bts_mcc, &oam_mcc)){
		DBG_ERROR("Failed to initialize mcc value with OAM:%d\n", ret);
		DBG_ERROR("Initialized with hard coded default values.\n");
		mmCfgLocId.mcc[0] = 				MM_DEFAULT_BTS_MCC[0];
		mmCfgLocId.mcc[1] = 				MM_DEFAULT_BTS_MCC[1];
		mmCfgLocId.mcc[2] = 				MM_DEFAULT_BTS_MCC[2];	
	}
	else {
		mmConvertMCC(oam_mcc);
		DBG_TRACE("Reading mcc from OAM completed\n");
	}
	DBG_TRACE("mcc \t\t:%x%x%x\n",mmCfgLocId.mcc[0], mmCfgLocId.mcc[1], mmCfgLocId.mcc[2]);

//	if(ret = getMibIntVar(MIB_bts_mnc, &oam_mnc)){
	if(ret = oam_getMnc((unsigned char *)mmCfgLocId.mnc)){
		DBG_ERROR("Failed to initialize mnc value with OAM:%d\n", ret);
		DBG_ERROR("Initialized with hard coded default values.\n");
		mmCfgLocId.mnc[0] = 				MM_DEFAULT_BTS_MNC[0];
		mmCfgLocId.mnc[1] = 				MM_DEFAULT_BTS_MNC[1];
		mmCfgLocId.mnc[2] = 				MM_DEFAULT_BTS_MNC[2];
	}
	else {
		mmConvertMNC(oam_mnc);
		DBG_TRACE("Reading mnc from OAM completed\n");
	}
	DBG_TRACE("mnc \t\t:%x%x\n",mmCfgLocId.mnc[0], mmCfgLocId.mnc[1], mmCfgLocId.mnc[2]);

	if(ret = getMibIntVar(MIB_bts_lac, &oam_lac)){
		DBG_ERROR("Failed to initialize lac value with OAM:%d\n", ret);
		DBG_ERROR("Initialized with hard coded default values.\n");
		mmCfgLocId.lac = 				MM_DEFAULT_BTS_LAC;
	}
	else {
		mmConvertLAC(oam_lac);
		DBG_TRACE("Reading lac from OAM completed\n");
	}
	DBG_TRACE("lac \t\t:%d\n",mmCfgLocId.lac);

	mmSetOAMTraps();
  
	DBG_LEAVE();
}

bool mmIsAuthenticationRequired()
{
	return((bool)ludb_auth_required);
}

bool mmIsCipheringRequired()
{
	return((bool)mmCfg.mm_cipheringRequired);
}

bool mmIsIMEICheckRequired()
{
	return((bool)mmCfg.mm_IMEICheckRequired);
}

