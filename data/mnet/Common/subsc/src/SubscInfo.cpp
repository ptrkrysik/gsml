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
// File        : SubscInfo.cpp
// Author(s)   : Kevin Lim
// Create Date : 06-16-99
// Description : subscriber information structure and en/decoding function
//               for messages
//
// *******************************************************************
#include "stdio.h"
#include "string.h"
#include "subsc/SubscInfoType.h"
#include "subsc/SubscGPRSInfoType.h"

char * getSubscInfoIETypeStr(T_SUBSC_INFO_IE_TYPE ie_type)
{
	switch(ie_type){
		case SUBSC_INFO_IE_VERSION_NUMBER		: return("VERSION_NUM "); 
		case SUBSC_INFO_IE_MOBILE_ID_IMSI		: return("MOBILE_IMSI "); 
		case SUBSC_INFO_IE_ISDN					: return("ISDN        "); 
		case SUBSC_INFO_IE_SUBADDR				: return("SUBADDR     "); 
		case SUBSC_INFO_IE_PROVISION_BS			: return("PROVISION_BS");	
		case SUBSC_INFO_IE_PROVISION_TS			: return("PROVISION_TS"); 
		case SUBSC_INFO_IE_PROVISION_SS			: return("PROVISION_SS"); 
		case SUBSC_INFO_IE_SUBSC_STATUS			: return("SUBSC_STATUS");
		case SUBSC_INFO_IE_ODB_DATA				: return("ODB_DATA    "); 
		case SUBSC_INFO_IE_SS_INFO_LIST			: return("SS_INFO_LIST"); 
		case SUBSC_INFO_IE_USER_INFO			: return("USER_INFO   ");
		case SUBSC_INFO_IE_GPRS_DATA			: return("GPRS_DATAT  ");
		default: return("Unknown IE Type");
	}
}

char * getSubscInfoBSCodeStr(T_SUBSC_INFO_BEARER_SERVICE_CODE code)
{
	switch(code){
		case ALL_BEARER_SERVICES		: return("ALL_BEARER");

		case ALL_DATA_CDA				: return("ALL_DATA_CDA");
		case DATA_CDA_300BPS			: return("DATA_CDA_300");
		case DATA_CDA_1200BPS			: return("DATA_CDA_1200");
		case DATA_CDA_1200_75BPS		: return("DATA_CDA_1200_75");
		case DATA_CDA_2400BPS			: return("DATA_CDA_2400");
		case DATA_CDA_4800BPS			: return("DATA_CDA_4800");
		case DATA_CDA_9600BPS			: return("DATA_CDA_9600");		// supported by ViperCell
		case DATA_CDA_14400BPS			: return("DATA_CDA_14400");		// supported by ViperCell
		case DATA_CDA_GENERAL			: return("DATA_CDA_GENERAL");

		case ALL_DATA_CDS				: return("ALL_DATA_CDS");
		case DATA_CDS_1200BPS			: return("DATA_CDS_1200");
		case DATA_CDS_2400BPS			: return("DATA_CDS_2400");
		case DATA_CDS_4800BPS			: return("DATA_CDS_4800");
		case DATA_CDS_9600BPS			: return("DATA_CDS_9600");
		case DATA_CDS_GENERAL			: return("DATA_CDS_GENERAL");

		case ALL_PAD_ACCESS_CA			: return("ALL_PAD_ACCESS_CA");
		case PAD_ACCESS_CA_300BPS		: return("PAD_ACC_CA_300");
		case PAD_ACCESS_CA_1200BPS		: return("PAD_ACC_CA_1200");
		case PAD_ACCESS_CA_1200_75BPS	: return("PAD_ACC_CA_1200_75");
		case PAD_ACCESS_CA_2400BPS		: return("PAD_ACC_CA_2400");
		case PAD_ACCESS_CA_4800BPS		: return("PAD_ACC_CA_4800");
		case PAD_ACCESS_CA_9600BPS		: return("PAD_ACC_CA_9600");
		case PAD_ACCESS_CA_GENERAL		: return("PAD_ACC_CA_GENERAL");

		case ALL_DATA_PDS				: return("ALL_DATA_PDS");
		case DATA_PDS_2400BPS			: return("DATA_PDS_2400");
		case DATA_PDS_4800BPS			: return("DATA_PDS_4800");
		case DATA_PDS_9600BPS			: return("DATA_PDS_9600");
		case DATA_PDS_GENERAL			: return("DATA_PDS_GENERAL");

		case ALL_ALT_SPEECH_DATA_CDA	: return("ALL_ALT_SPCH_CDA");
		case ALL_ALT_SPEECH_DATA_CDS	: return("ALL_ALT_SPCH_CDS");
		case ALL_SPEECH_FOLL_DATA_CDA	: return("ALL_SPCH_FOLL_CDA");
		case ALL_SPEECH_FOLL_DATA_CDS	: return("ALL_SPCH_FOLL_CDS");

		case ALL_PLMN_SPECIFIC_BS		: return("ALL_PLMN_SPEC_BS");
		case PLMN_SPECIFIC_BS_1			: return("PLMN_SPEC_BS_1");
		case PLMN_SPECIFIC_BS_2			: return("PLMN_SPEC_BS_2");
		case PLMN_SPECIFIC_BS_3			: return("PLMN_SPEC_BS_3");
		case PLMN_SPECIFIC_BS_4			: return("PLMN_SPEC_BS_4");
		case PLMN_SPECIFIC_BS_5			: return("PLMN_SPEC_BS_5");
		case PLMN_SPECIFIC_BS_6			: return("PLMN_SPEC_BS_6");
		case PLMN_SPECIFIC_BS_7			: return("PLMN_SPEC_BS_7");
		case PLMN_SPECIFIC_BS_8			: return("PLMN_SPEC_BS_8");
		case PLMN_SPECIFIC_BS_9			: return("PLMN_SPEC_BS_9");
		case PLMN_SPECIFIC_BS_A			: return("PLMN_SPEC_BS_A");
		case PLMN_SPECIFIC_BS_B			: return("PLMN_SPEC_BS_B");
		case PLMN_SPECIFIC_BS_C			: return("PLMN_SPEC_BS_C");
		case PLMN_SPECIFIC_BS_D			: return("PLMN_SPEC_BS_D");
		case PLMN_SPECIFIC_BS_E			: return("PLMN_SPEC_BS_E");
		case PLMN_SPECIFIC_BS_F			: return("PLMN_SPEC_BS_F");
		default: return("Unknown BS Code");
	}
}

char * getSubscInfoTSCodeStr(T_SUBSC_INFO_TELE_SERVICE_CODE code)
{
	switch(code){
		case ALL_TELE_SERVICES			: return("ALL_TELE");

		case ALL_SPEECH_SERVICES		: return("ALL_SPCH_SRVC");
		case TELEPHONY_SERVICES			: return("TELE_SRVC");
		case EMERGENCY_CALLS			: return("EMERGENCY");

		case ALL_SMS					: return("ALL_SMS");
		case SMS_MT_PP					: return("SMS_MT_PP");
		case SMS_MO_PP					: return("SMS_MO_PP");

		case ALL_FACS					: return("ALL_FACS");
		case FACS_GRP3_ALT_SPEECH		: return("FACS_GRP3_ALT_SPCH");
		case AUTO_FACS_GRP3				: return("AUTO_FACS_GRP3");
		case FACS_GRP4					: return("FACS_GRP4");

		case ALL_VOICE_GROUP_CALL		: return("ALL_VOICE_GROUP");
		case VOICE_GROUP_CALL			: return("VOICE_GROUP_CALL");
		case VOICE_BROADCAST			: return("VOICE_BROADCAST");

		case ALL_PLMN_SPECIFIC_TS		: return("ALL_PLMN_SPEC_TS");
		case PLMN_SPECIFIC_TS_1			: return("PLMN_SPEC_TS_1");
		case PLMN_SPECIFIC_TS_2			: return("PLMN_SPEC_TS_2");
		case PLMN_SPECIFIC_TS_3			: return("PLMN_SPEC_TS_3");
		case PLMN_SPECIFIC_TS_4			: return("PLMN_SPEC_TS_4");
		case PLMN_SPECIFIC_TS_5			: return("PLMN_SPEC_TS_5");
		case PLMN_SPECIFIC_TS_6			: return("PLMN_SPEC_TS_6");
		case PLMN_SPECIFIC_TS_7			: return("PLMN_SPEC_TS_7");
		case PLMN_SPECIFIC_TS_8			: return("PLMN_SPEC_TS_8");
		case PLMN_SPECIFIC_TS_9			: return("PLMN_SPEC_TS_9");
		case PLMN_SPECIFIC_TS_A			: return("PLMN_SPEC_TS_A");
		case PLMN_SPECIFIC_TS_B			: return("PLMN_SPEC_TS_B");
		case PLMN_SPECIFIC_TS_C			: return("PLMN_SPEC_TS_C");
		case PLMN_SPECIFIC_TS_D			: return("PLMN_SPEC_TS_D");
		case PLMN_SPECIFIC_TS_E			: return("PLMN_SPEC_TS_E");
		case PLMN_SPECIFIC_TS_F			: return("PLMN_SPEC_TS_F");
		default: return("Unknown TS Code");
	}
}

char * getSubscInfoSSCodeStr(T_SUBSC_INFO_SUPP_SERVICE_CODE code)
{
	switch(code){
		case SUPP_SERV_ALL_LI			: return("ALL_LI");
		case SUPP_SERV_CLIP				: return("CLIP");
		case SUPP_SERV_CLIR				: return("CLIR");
		case SUPP_SERV_COLP				: return("COLP");
		case SUPP_SERV_COLR				: return("COLR");
		case SUPP_SERV_ALL_FWD			: return("ALL_FWD");
		case SUPP_SERV_CFU				: return("CFU");
		case SUPP_SERV_CFB				: return("CFB");
		case SUPP_SERV_CFNRY			: return("CFNRY");
		case SUPP_SERV_CFNRC			: return("CFNRC");
		case SUPP_SERV_ALL_CO			: return("ALL_CO");
		case SUPP_SERV_ECT				: return("ECT");
		case SUPP_SERV_ALL_CC			: return("ALL_CC");
		case SUPP_SERV_CW				: return("CW");
		case SUPP_SERV_HOLD				: return("HOLD");
		case SUPP_SERV_ALL_MULT			: return("ALL_MULT");
		case SUPP_SERV_MPTY				: return("MPTY");
		case SUPP_SERV_ALL_BAR			: return("ALL_BAR");
		case SUPP_SERV_BAOC				: return("BAOC");
		case SUPP_SERV_BOIC				: return("BOIC");
		case SUPP_SERV_BOICE			: return("BOICE");
		case SUPP_SERV_BAIC				: return("BAIC");
		case SUPP_SERV_BIC_ROAM			: return("BICRoam");
		default: return("Unknown SS Code");
	}
}

short	checkSubscInfoIEType(unsigned char ie_type, T_SUBSC_INFO_IE_TYPE exp)
{
	if(ie_type != (unsigned char)exp){
		printf("Expected SubscInfoIEType:    %s\n", 
				getSubscInfoIETypeStr(exp));
		printf("Encountered SubscInfoIEType: %s\n", 
				getSubscInfoIETypeStr((T_SUBSC_INFO_IE_TYPE)ie_type)); 
		return 1;
	}
	else{
		return 0;
	}
}

void printSubscMsg(unsigned char *msg, short size)
{
	int i;
	for(i=0; i<size; i++){
		if(!(i%16)) printf("\n");
		printf("%02x ", msg[i]);
	}
	printf("\n\n");
}

