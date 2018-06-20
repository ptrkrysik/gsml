#ifndef MMMdMsgProc_H
#define MMMdMsgProc_H

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
// File        : MMMdMsgProc.h
// Author(s)   : Kevin Lim
// Create Date : 07-26-99
// Description : 
//
// *******************************************************************

// *******************************************************************
// function prototypes
// *******************************************************************


// test approval support functions
int taHoldSdcchOn();
int taHoldSdcchOff();

int StartCipherMode(int start, int algo, unsigned char lsb);
int StopCipherMode();


short initNewConn(	T_CNI_IRT_ID               entryId,
					T_CNI_LAPDM_OID            oid,
					T_CNI_LAPDM_SAPI           sapi,
					MMProcedureType_t          procType);

void sendLocationUpdateAccept(	T_CNI_LAPDM_OID   oid,  
								T_CNI_LAPDM_SAPI sapi, 
								T_CNI_IRT_ID entryId,
						const	T_CNI_RIL3_IE_MOBILE_ID& mobileId);

void sendLocationUpdateReject(	T_CNI_LAPDM_OID   oid,  
								T_CNI_LAPDM_SAPI sapi ,
								T_CNI_IRT_ID entryId,
								T_CNI_RIL3_REJECT_CAUSE_VALUE cause);

void sendIdentityRequest(short mmId);

void sendAuthReqToMobile(short mmId, short ludbId);

void sendAuthRejToMobile(short mmId);

short sendMobileRegister(short mmId);

short sendLocUpInCMServReq(	T_CNI_RIL3_IE_MOBILE_ID mobileId,
						 	short mmId);

short sendMobileUnRegister(	T_CNI_RIL3_IE_MOBILE_ID &mobileId,
							const T_CNI_RIL3MD_CCMM_MSG *msInMsg);

void processLocationUpdate(T_CNI_RIL3_IE_MOBILE_ID& mobileId, 
							const T_CNI_RIL3MD_CCMM_MSG * msInMsg,
							T_CNI_RIL3_IE_CIPHER_KEY *keySeq);

void MM_MD_LocUpdateReqProc(
			T_CNI_RIL3MM_MSG_LOCATION_UPDATE_REQUEST *locUpdReq,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg);

void MM_MD_IdentityRespProc(
			T_CNI_RIL3MM_MSG_IDENTITY_RESPONSE *identityResponse,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg);

bool sendRRCipherModeCmd(short mmId, MMProcedureType_t proc);
bool sendCMServiceAccept(short mmId);
bool sendCMServiceReqToCC(short mmId);

void MM_MD_CMServiceReqProc(
			T_CNI_RIL3MM_MSG_CM_SERVICE_REQUEST *cmServReq,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg);
void MM_MD_CMServiceReqEmergencyProc(
			T_CNI_RIL3MM_MSG_CM_SERVICE_REQUEST *cmServReq,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg);
void MM_ProceedEmergencyIMSI(short mmId);
void MM_MD_CMServiceReqNormalProc(
			T_CNI_RIL3MM_MSG_CM_SERVICE_REQUEST *cmServReq,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg);

void MMCMServiceCleanUp(short mmId);
void MMProceedCMServiceReqAuth(short mmId);
void MMProceedCMServiceReqCipher(short mmId);
void MMProceedCMServiceReqAccept(short mmId);

void MM_MD_IMSIDetachIndProc(
			T_CNI_RIL3MM_MSG_IMSI_DETACH_INDICATION *imsiDetachInd,
			T_CNI_RIL3MD_CCMM_MSG *msInMsg);

void MM_MD_MsgProc(T_CNI_RIL3MD_CCMM_MSG *msInMsg);


#endif                                 // MMMdMsgProc_H
