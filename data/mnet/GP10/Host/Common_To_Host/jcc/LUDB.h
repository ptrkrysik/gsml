#ifndef LUDB_H
#define LUDB_H

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
// File        : LUDB.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : Local User Database. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "ril3/ril3irt.h"
#include "ril3/ie_mobile_id.h"
#include "ril3/ie_classmark_1.h"
#include "ril3/ie_classmark_2.h"

#include "subsc/SubscInfoType.h"

#include "semLib.h"
#include "VOIP/voipapi.h"
#include "JCCTimer.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

// *******************************************************************
// Constant Definitions
// *******************************************************************
#define  LUDB_MAX_EMERG_ENTRY		20
#define  LUDB_MAX_NUM_ENTRY 		(300 + LUDB_MAX_EMERG_ENTRY)	

#define  LUDB_IMSI_HASH_NUM_BUCKET	97

const short NULL_CLASSMARK = 0;

union ClassmarkData_t{
  T_CNI_RIL3_IE_MS_CLASSMARK_1       classmark1;
  T_CNI_RIL3_IE_MS_CLASSMARK_2       classmark2;
} ;



class LUDBEntry {
	public:
		enum LUDBState
		{
			IDLE			= 1,	//IDLE state
			REGREQ			,		// Wait for response of registration only
			REGTHENSEC		,		// Wait for response of registration then request security
			SECREQ			,		// Wait for response of security request only
			SECINREG		,		// Wait for response of security request then confirm registration
			ACTIVE			,		// Mobile is active. RCF from H.323
			UNREGREQ		,		// wait for response of H323MobileUnresgier. (treat as Inactive.)
			INACTIVE		,		// Inactive, unregister confirmed from H.323
			UNREGRJT		,		// Inactive, unregister rejected from H.323
			EMERGENCY		,
			MAXLUDBSTATE	,		// for error check.
		};

		LUDBEntry();
		void						init();	
		bool						isFree() { return free;};
		void						isFree(bool free);
		bool						subscExist() { return subsc_exist;};
		bool						secExist() { return sec_exist;};
		void						setSubscExist(bool flag);
		void						setSecExist(bool flag);
		char*						getStateString();			
		void						startVBReqTimer(int sec, short ludb_Id);
		void						stopVBReqTimer();	
		void						startPurgeTimer(int sec, short ludb_Id);
		void						stopPurgeTimer();	
		T_AUTH_TRIPLET*				curTripletPtr();
		
		T_CNI_RIL3_IE_MOBILE_ID		mobileId;
		short						classmarkType;
		ClassmarkData_t				classmarkData;
		LUDBState					state;
		VOIP_MOBILE_HANDLE			mobileHandle;
		short						mm_id;
		JCCTimer					*VBReqTimer;
		JCCTimer					*PurgeTimer;
		T_SUBSC_INFO				subsc_info;
		T_AUTH_TRIPLET				sec_info;
		char						cur_cksn;
		short						auth_reuse_count;
		short						cur_algo;

	private:
		bool						free;
		bool						subsc_exist;
		bool						sec_exist;
		SEM_ID						ludbSemId;

};

class LUDB
{
	public:
		LUDB();
		virtual ~LUDB() {};
		void	init();
		void	init(const short index);   // klim 3/29/99
		void	print();
		short	getEntry		(T_CNI_RIL3_IE_MOBILE_ID mobileId);
		short	registerUser	(T_CNI_RIL3_IE_MOBILE_ID mobileId );
		void	update			(short index, LUDBEntry::LUDBState state);
		void	update			(short index, VOIP_MOBILE_HANDLE mobileHandle);
		short	update			(short index, T_SUBSC_INFO info);
		short	update			(short index, T_AUTH_TRIPLET info);
		void    printSRS		(short index);
		void	setMMId			(const  short index, short mmId); 
		short	getMMId			(const  short index); 
		LUDBEntry::LUDBState 	getState( const short index);
		bool	isFree			(const short i);
		bool	getSubscExist	(short i);
		void	setSubscExist	(short i, bool flag);
		bool	getSecExist		(short i);
		void	setSecExist		(short i, bool flag);
		short   checkAuthCount  (short i);
		T_AUTH_TRIPLET*			curTripletPtr(short i);
		short	curAlgo			(short i);
		void	setAlgo			(short i, short algo);
		char	curCKSN(short i);
		char	incCKSN(short i);
		void	entryDealloc	(short i);
		void	entryDealloc	(const T_CNI_RIL3_IE_MOBILE_ID& );
		bool	startVBReqTimer(short ludb_Id);
		bool	stopVBReqTimer(short ludb_Id);	
		bool	startPurgeTimer(short ludb_Id);
		bool	stopPurgeTimer(short ludb_Id);	
		
		bool 	authTerm(	const short index, 
			  			const T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER& cpn);

		VOIP_MOBILE_HANDLE getMobileHandle (const short index);
		bool 	authOrig(	const short index,
			       		const T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER& cpn);
		// stage 2 changes
		T_CNI_RIL3_IE_MOBILE_ID		getMobileId (const short index);
		bool						inRange(short index);
		T_SUBSC_INFO* 				getSubscInfoPtr(const short index);
		T_SUBSC_IE_MOBILE_ID_IMSI*	getMobileIdPtr(const short index);
		T_SUBSC_IE_ISDN*			getMSISDNPtr(const short index);
		T_SUBSC_IE_PROVISION_BS*	getProvBSPtr(const short index);
		T_SUBSC_IE_PROVISION_TS*	getProvTSPtr(const short index);
		T_SUBSC_IE_PROVISION_SS*	getProvSSPtr(const short index);
		T_SUBSC_IE_SUBSC_STATUS*	getSubscStatusPtr(const short index);
		T_SUBSC_IE_ODB_DATA*		getODBDataPtr(const short index);
		T_SUBSC_IE_SS_INFO_LIST*	getSSInfoListPtr(const short index);
		T_SUBSC_IE_USER_INFO*		getUserInfoPtr(const short index);
	private:
		SEM_ID		semId;	
		short		entryAlloc();
		LUDBEntry	entries[LUDB_MAX_NUM_ENTRY];

};

#endif                                       // LUDB_H



