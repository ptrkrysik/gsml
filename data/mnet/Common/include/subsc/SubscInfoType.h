
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
// File        : SubscInfoType.h
// Author(s)   : Kevin Lim
// Create Date : 06-16-99
// Description : message and ie interface among LUDB and other modules
//
// *******************************************************************

#ifndef SUBSC_INFO_TYPE_H
#define SUBSC_INFO_TYPE_H

#include "SubscGPRSInfoType.h"
#include "SubscINProfileType.h"

#ifdef __cplusplus
extern "C" {
#endif

// version history
// 2.0.3 combine IMSI fields as one
// 2.0.4 added num_plan in ISDN
// 2.0.5 added SS_DATA type in SS_INFO
// 2.0.6 added SS_INFO_INDEX
// 2.0.7 breaking SS_INFO_TYPE into three structure instead of union
// 2.0.8 added SUPP_SERV_BIC_ROAM
// 2.1.0 added GPRS data with encoder/decoder change - G.2.
// 2.1.1 added CLIR feature support
// 2.1.2 added IN profile support

#define		SUBSC_ENC_VER0		2
#define		SUBSC_ENC_VER1		1
#define		SUBSC_ENC_VER2		2

typedef struct {
	short v0;
	short v1;
	short v2;
} T_SUBSC_VERSION;

typedef enum {
	SUBSC_INFO_IE_VERSION_NUMBER		= 0x00,
	SUBSC_INFO_IE_MOBILE_ID_IMSI		= 0x01,
	SUBSC_INFO_IE_ISDN					= 0x02,
	SUBSC_INFO_IE_SUBADDR				= 0x03,
	SUBSC_INFO_IE_PROVISION_BS			= 0x05,
	SUBSC_INFO_IE_PROVISION_TS			= 0x06,
	SUBSC_INFO_IE_PROVISION_SS			= 0x07,
	SUBSC_INFO_IE_SUBSC_STATUS			= 0x08,
	SUBSC_INFO_IE_ODB_DATA				= 0x09,
	SUBSC_INFO_IE_SS_INFO_LIST			= 0x0A,
	SUBSC_INFO_IE_USER_INFO				= 0x0B,
	SUBSC_INFO_IE_GPRS_DATA				= 0x0C,

	SUBSC_INFO_IE_AUTH_LIST				= 0x11,
} T_SUBSC_INFO_IE_TYPE;

typedef enum {
	ALL_BEARER_SERVICES					= 0x00,

	ALL_DATA_CDA						= 0x10,
	DATA_CDA_300BPS						= 0x11,
	DATA_CDA_1200BPS					= 0x12,
	DATA_CDA_1200_75BPS					= 0x13,
	DATA_CDA_2400BPS					= 0x14,
	DATA_CDA_4800BPS					= 0x15,
	DATA_CDA_9600BPS					= 0x16,		// supported by ViperCell
	DATA_CDA_14400BPS					= 0x66,		// supported by ViperCell
	DATA_CDA_GENERAL					= 0x17,

	ALL_DATA_CDS						= 0x18,
	DATA_CDS_1200BPS					= 0x1A,
	DATA_CDS_2400BPS					= 0x1C,
	DATA_CDS_4800BPS					= 0x1D,
	DATA_CDS_9600BPS					= 0x1E,
	DATA_CDS_GENERAL					= 0x1F,

	ALL_PAD_ACCESS_CA					= 0x20,
	PAD_ACCESS_CA_300BPS				= 0x21,
	PAD_ACCESS_CA_1200BPS				= 0x22,
	PAD_ACCESS_CA_1200_75BPS			= 0x23,
	PAD_ACCESS_CA_2400BPS				= 0x24,
	PAD_ACCESS_CA_4800BPS				= 0x25,
	PAD_ACCESS_CA_9600BPS				= 0x26,
	PAD_ACCESS_CA_GENERAL				= 0x27,

	ALL_DATA_PDS						= 0x28,
	DATA_PDS_2400BPS					= 0x2C,
	DATA_PDS_4800BPS					= 0x2D,
	DATA_PDS_9600BPS					= 0x2E,
	DATA_PDS_GENERAL					= 0x2F,

	ALL_ALT_SPEECH_DATA_CDA				= 0x30,
	ALL_ALT_SPEECH_DATA_CDS				= 0x38,
	ALL_SPEECH_FOLL_DATA_CDA			= 0x40,
	ALL_SPEECH_FOLL_DATA_CDS			= 0x48,

	ALL_PLMN_SPECIFIC_BS				= 0xD0,
	PLMN_SPECIFIC_BS_1					= 0xD1,
	PLMN_SPECIFIC_BS_2					= 0xD2,
	PLMN_SPECIFIC_BS_3					= 0xD3,
	PLMN_SPECIFIC_BS_4					= 0xD4,
	PLMN_SPECIFIC_BS_5					= 0xD5,
	PLMN_SPECIFIC_BS_6					= 0xD6,
	PLMN_SPECIFIC_BS_7					= 0xD7,
	PLMN_SPECIFIC_BS_8					= 0xD8,
	PLMN_SPECIFIC_BS_9					= 0xD9,
	PLMN_SPECIFIC_BS_A					= 0xDA,
	PLMN_SPECIFIC_BS_B					= 0xDB,
	PLMN_SPECIFIC_BS_C					= 0xDC,
	PLMN_SPECIFIC_BS_D					= 0xDD,
	PLMN_SPECIFIC_BS_E					= 0xDE,
	PLMN_SPECIFIC_BS_F					= 0xDF,

} T_SUBSC_INFO_BEARER_SERVICE_CODE;

typedef enum {
	ALL_TELE_SERVICES					= 0x00,

	ALL_SPEECH_SERVICES					= 0x10,
	TELEPHONY_SERVICES					= 0x11,		// supported by ViperCell
	EMERGENCY_CALLS						= 0x12,		// supported by ViperCell

	ALL_SMS								= 0x20,
	SMS_MT_PP							= 0x21,		// supported by ViperCell
	SMS_MO_PP							= 0x22,		// supported by ViperCell

	ALL_FACS							= 0x60,
	FACS_GRP3_ALT_SPEECH				= 0x61,		// supported by ViperCell
	AUTO_FACS_GRP3						= 0x62,		// supported by ViperCell
	FACS_GRP4							= 0x63,

	ALL_VOICE_GROUP_CALL				= 0x90,
	VOICE_GROUP_CALL					= 0x91,
	VOICE_BROADCAST						= 0x92,

	ALL_PLMN_SPECIFIC_TS				= 0xD0,
	PLMN_SPECIFIC_TS_1					= 0xD1,
	PLMN_SPECIFIC_TS_2					= 0xD2,
	PLMN_SPECIFIC_TS_3					= 0xD3,
	PLMN_SPECIFIC_TS_4					= 0xD4,
	PLMN_SPECIFIC_TS_5					= 0xD5,
	PLMN_SPECIFIC_TS_6					= 0xD6,
	PLMN_SPECIFIC_TS_7					= 0xD7,
	PLMN_SPECIFIC_TS_8					= 0xD8,
	PLMN_SPECIFIC_TS_9					= 0xD9,
	PLMN_SPECIFIC_TS_A					= 0xDA,
	PLMN_SPECIFIC_TS_B					= 0xDB,
	PLMN_SPECIFIC_TS_C					= 0xDC,
	PLMN_SPECIFIC_TS_D					= 0xDD,
	PLMN_SPECIFIC_TS_E					= 0xDE,
	PLMN_SPECIFIC_TS_F					= 0xDF,

} T_SUBSC_INFO_TELE_SERVICE_CODE;

typedef enum {
	SUPP_SERV_ALL_LI					= 0x10,
	SUPP_SERV_CLIP						= 0x11,
	SUPP_SERV_CLIR						= 0x12,
	SUPP_SERV_COLP						= 0x13,
	SUPP_SERV_COLR						= 0x14,
	SUPP_SERV_ALL_FWD					= 0x20,
	SUPP_SERV_CFU						= 0x21,
	SUPP_SERV_ALL_CCF					= 0x28,	// all conditional call forwarding
	SUPP_SERV_CFB						= 0x29,
	SUPP_SERV_CFNRY						= 0x2A,
	SUPP_SERV_CFNRC						= 0x2B,
	SUPP_SERV_ALL_CO					= 0x30,
	SUPP_SERV_ECT						= 0x31,
	SUPP_SERV_ALL_CC					= 0x40,
	SUPP_SERV_CW						= 0x41,
	SUPP_SERV_HOLD						= 0x42,
	SUPP_SERV_ALL_MULT					= 0x50,
	SUPP_SERV_MPTY						= 0x51,
	SUPP_SERV_ALL_BAR					= 0x90,
	SUPP_SERV_BAOC						= 0x92,
	SUPP_SERV_BOIC						= 0x93,
	SUPP_SERV_BOICE						= 0x94,
	SUPP_SERV_BAIC						= 0x9A,
	SUPP_SERV_BIC_ROAM					= 0x9B, // added 06/19/2000 kevinlim
} T_SUBSC_INFO_SUPP_SERVICE_CODE;

typedef enum {									// 2.0.7
	SS_INFO_IND_FORW_CFU					= 0,
	SS_INFO_IND_FORW_CFB					= 1,
	SS_INFO_IND_FORW_CFNRC				= 2,
	SS_INFO_IND_FORW_CFNRY				= 3,
	SS_INFO_IND_FORW_MAX
} T_SUBSC_SS_INFO_FORW_INDEX;

typedef enum {
	SS_INFO_IND_DATA_CW					= 0,
	SS_INFO_IND_DATA_HOLD				= 1,
	SS_INFO_IND_DATA_CLIP				= 2,
	SS_INFO_IND_DATA_CLIR				= 3,
	SS_INFO_IND_DATA_COLP				= 4,
	SS_INFO_IND_DATA_COLR				= 5,
	SS_INFO_IND_DATA_ECT					= 6,
	SS_INFO_IND_DATA_MPTY				= 7,
	SS_INFO_IND_DATA_MAX
} T_SUBSC_SS_INFO_DATA_INDEX;

typedef enum {
	SS_INFO_IND_BAR_BAOC					= 0,
	SS_INFO_IND_BAR_BOIC					= 1,
	SS_INFO_IND_BAR_BOICE				= 2,
	SS_INFO_IND_BAR_BAIC					= 3,
	SS_INFO_IND_BAR_BIC_ROAM			= 4,
	SS_INFO_IND_BAR_MAX
} T_SUBSC_SS_INFO_BAR_INDEX;

/*
typedef enum {									// 2.0.6
	SS_INFO_IND_CFU						= 0,
	SS_INFO_IND_CFB						= 1,
	SS_INFO_IND_CFNRC					= 2,
	SS_INFO_IND_CFNRY					= 3,
	SS_INFO_IND_CW						= 4,
	SS_INFO_IND_HOLD					= 5,
	SS_INFO_IND_CLIP					= 6,
	SS_INFO_IND_CLIR					= 7,
	SS_INFO_IND_COLP					= 8,
	SS_INFO_IND_COLR					= 9,
	SS_INFO_IND_ECT						= 10,
	SS_INFO_IND_MPTY					= 11,
	SS_INFO_IND_BAOC					= 12,
	SS_INFO_IND_BOIC					= 13,
	SS_INFO_IND_BOICE					= 14,
	SS_INFO_IND_BAIC					= 15,
} T_SUBSC_SS_INFO_INDEX;
*/

typedef enum {
	ODB_GEN_BIT_ALL_OG					= 0,
	ODB_GEN_BIT_INTRN_OG				= 1,
	ODB_GEN_BIT_INTRN_OG_NOT_HPLMN		= 2,
	ODB_GEN_BIT_PREM_INFO				= 3,
	ODB_GEN_BIT_PREM_ENT				= 4,
	ODB_GEN_BIT_SS_ACCESS				= 5,
	ODB_GEN_BIT_INTRZ_OG				= 6,
	ODB_GEN_BIT_INTRZ_OG_NOT_HPLMN		= 7,

	ODB_GEN_BIT_INTRZ_INTRN_NOT_HPLMN	= 0,
	ODB_GEN_BIT_ALL_ECT					= 1,
	ODB_GEN_BIT_CHARGE_ECT				= 2,
	ODB_GEN_BIT_INTRN_ECT				= 3,
	ODB_GEN_BIT_INTRZ_ECT				= 4,
	ODB_GEN_BIT_DBL_CHARGE_ECT			= 5,
	ODB_GEN_BIT_MULTI_ECT				= 6,

} T_ODB_GEN_BIT;

typedef enum {
	ODB_HPLMN_BIT_TYPE_1				= 0,
	ODB_HPLMN_BIT_TYPE_2				= 1,
	ODB_HPLMN_BIT_TYPE_3				= 2,
	ODB_HPLMN_BIT_TYPE_4				= 3,
} T_ODB_HPLMN_BIT;

typedef enum {
	SS_STATUS_A_BIT						= 0,
	SS_STATUS_R_BIT						= 1,
	SS_STATUS_P_BIT						= 2,
	SS_STATUS_Q_BIT						= 3,
} T_SS_STATUS_BIT;


#define		MAX_NUM_SUBSC_IMSI			18

#define		SUBSC_MCC_LEN			3
#define		SUBSC_MNC_LEN			2
#define		SUBSC_MSIN_LEN			10

typedef struct {
//	T_SUBSC_INFO_IE_TYPE				ie_type;	// only in encoded message
	unsigned char						num_digit;
	unsigned char						imsi[MAX_NUM_SUBSC_IMSI];
} T_SUBSC_IE_MOBILE_ID_IMSI;


// this struct can be changed to 9 byte long TBCD format, if necessary.
#define MAX_NUM_SUBSC_ISDN				18
#define NUM_PLAN_LEN					1
typedef struct {
//	T_SUBSC_INFO_IE_TYPE				ie_type;	// only in encoded message
	short								num_digit;
	unsigned char						digits[MAX_NUM_SUBSC_ISDN];
	unsigned char						num_plan[NUM_PLAN_LEN];
} T_SUBSC_IE_ISDN;

#define NUM_SUBADDR_INFO				1			// was 20
typedef struct {
//	T_SUBSC_INFO_IE_TYPE				ie_type;	// only in encoded message
	short								subaddr_type;
	unsigned char						info[NUM_SUBADDR_INFO];
} T_SUBSC_IE_SUBADDR;

#define MAX_NUM_BEARER_SERVICE			5			// was 50
typedef struct {
//	T_SUBSC_INFO_IE_TYPE				ie_type;	// only in encoded message
	short								num_srv;
	T_SUBSC_INFO_BEARER_SERVICE_CODE	bs_code[MAX_NUM_BEARER_SERVICE];
} T_SUBSC_IE_PROVISION_BS;

#define MAX_NUM_TELE_SERVICE			10			// was 20
typedef struct {
//	T_SUBSC_INFO_IE_TYPE				ie_type;	// only in encoded message
	short								num_srv;
	T_SUBSC_INFO_TELE_SERVICE_CODE		ts_code[MAX_NUM_TELE_SERVICE];
} T_SUBSC_IE_PROVISION_TS;

#define MAX_NUM_SUPP_SERVICE			21			// was 30
typedef struct {
//	T_SUBSC_INFO_IE_TYPE				ie_type;	// only in encoded message
	short								num_srv;
	T_SUBSC_INFO_SUPP_SERVICE_CODE		ss_code[MAX_NUM_SUPP_SERVICE];
} T_SUBSC_IE_PROVISION_SS;

typedef enum {
	SUBSC_STATUS_SERVICE_GRANTED		= 0,
	SUBSC_STATUS_ODBARRING				= 1,
} T_SUBSC_STATUS_TYPE;

typedef struct {
//	T_SUBSC_INFO_IE_TYPE				ie_type;	// only in encoded message
	T_SUBSC_STATUS_TYPE					status;
} T_SUBSC_IE_SUBSC_STATUS;

typedef struct {
	unsigned char						all_og_barred;
	unsigned char						intrn_og_barred;
	unsigned char						intrn_og_not_hplmn_barred;
	unsigned char						premium_info_og_barred;
	unsigned char						premium_enter_og_barred;
	unsigned char						ss_access_barred;
	unsigned char						intrz_og_barred;
	unsigned char						intrz_og_not_hplmn_barred;
	unsigned char						intrz_intrn_og_not_hplmn_barred;
	unsigned char						all_ect_barred;
	unsigned char						chargeable_ect_barred;
	unsigned char						intrn_ect_barred;
	unsigned char						intrz_ect_barred;
	unsigned char						double_chargeable_ect_barred;
	unsigned char						multiple_ect_barred;
} T_ODB_GEN_FLAGS;

typedef struct {
	unsigned char						plmn_specific_type1;
	unsigned char						plmn_specific_type2;
	unsigned char						plmn_specific_type3;
	unsigned char						plmn_specific_type4;
} T_ODB_PLMN_FLAGS;

typedef enum {
	BEARER_SERVICE_TYPE					= 0,
	TELE_SERVICE_TYPE					= 1,
} T_SUBSC_INFO_BASIC_SERVICE_TYPE;

typedef struct {
	T_SUBSC_INFO_BASIC_SERVICE_TYPE		service_type;
	union {
		unsigned char					ex_bs_code[5];	// octet 2-5 reserved
		unsigned char					ex_ts_code[5];	// octet 2-5 reserved
	};
} T_SUBSC_INFO_EXT_BASIC_SERVICE;

typedef struct {
	unsigned char						ss_status_A;
	unsigned char						ss_status_R;
	unsigned char						ss_status_P;
	unsigned char						ss_status_Q;
} T_SS_STATUS_FLAG;

typedef enum {
	NO_NOTIFICATION_TO_FORWARDING		= 0,
	NOTIFICATION_TO_FORWARDING			= 1,
} T_NOTIFY_FORW_FLAG;

typedef enum {
	NO_NOTIFICATION_TO_CALLING			= 0,
	NOTIFICATION_TO_CALLING				= 1,
} T_NOTIFY_CALLING_FLAG;

typedef enum {
	MS_NOT_REACHABLE					= 0,
	MS_BUSY								= 1,
	NO_REPLY							= 2,
	UNCONDITIONAL						= 3,
} T_FORW_REASON_FLAG;

typedef enum {
	SS_NOTIFY_FORW_FLAG_BIT				= 0,
	SS_NOTIFY_CALLING_FLAG_BIT			= 1,
	SS_FORW_REASON_FLAG_BIT				= 2,
} T_SS_FORW_OPTION_BITS;

typedef struct {
	T_NOTIFY_FORW_FLAG					notify_forw_party;
	T_NOTIFY_CALLING_FLAG				notify_calling_party;
	T_FORW_REASON_FLAG					forw_reason;
} T_SS_FORW_OPTION;

typedef struct {
//	T_SUBSC_INFO_IE_TYPE				ie_type;	// only in encoded message
	T_ODB_GEN_FLAGS						odb_general;
	T_ODB_PLMN_FLAGS 					odb_plmn;
} T_SUBSC_IE_ODB_DATA;

typedef struct {
	T_SUBSC_INFO_EXT_BASIC_SERVICE		basic_service;
	T_SS_STATUS_FLAG 					ss_status;
	T_SUBSC_IE_ISDN						forw_to_num;
	T_SUBSC_IE_SUBADDR					forw_to_sub;
	T_SS_FORW_OPTION					forw_options;
	short								no_reply_cond_time;
} T_SUBSC_FORW_FEATURE;

typedef struct {
	T_SUBSC_INFO_EXT_BASIC_SERVICE		basic_service;
	T_SS_STATUS_FLAG 					ss_status;
} T_SUBSC_BARR_FEATURE;

#define MAX_NUM_EXT_FEATURE				1			// was 20 -> 5 -> 1 (2.0.7)

typedef struct {
	T_SUBSC_INFO_SUPP_SERVICE_CODE		ss_code;
	short								num_feature;
	T_SUBSC_FORW_FEATURE				list[MAX_NUM_EXT_FEATURE];
} T_SUBSC_IE_SS_INFO_FORW;

typedef struct {
	T_SUBSC_INFO_SUPP_SERVICE_CODE 		ss_code;
	short								num_feature;
	T_SUBSC_BARR_FEATURE				list[MAX_NUM_EXT_FEATURE];
} T_SUBSC_IE_SS_INFO_BAR;

typedef enum {
	SS_SUBSC_OPTION_CLIR_OPTION			= 2,
	SS_SUBSC_OPTION_OVERRIDE_CATEGORY	= 1,
} T_SS_SUBSC_OPTION_TYPE;	// 2.1.1

typedef enum {
	CLIR_PERMANENT						= 0,
	CLIR_TEMP_DFT_RESTRICTED			= 1,
	CLIR_TEMP_DFT_ALLOWED				= 2,
} T_SS_SUBSC_CLIR_OPTION;	// 2.1.1

typedef enum {
	OVERRIDE_ENABLED					= 0,
	OVERRIDE_DISABLED					= 1,
} T_SS_SUBSC_OVERRIDE_CATEGORY;	// 2.1.1

typedef struct {
	bool isPresent;
	T_SS_SUBSC_OPTION_TYPE				option_type;
	union {
		T_SS_SUBSC_CLIR_OPTION			clir_option;
		T_SS_SUBSC_OVERRIDE_CATEGORY	override_category;
	};
} T_SS_SUBSC_OPTION;	// 2.1.1

typedef struct {
	T_SUBSC_INFO_SUPP_SERVICE_CODE		ss_code;
	T_SS_STATUS_FLAG					ss_status;
	T_SS_SUBSC_OPTION					ss_subsc_option;	// 2.1.1
} T_SUBSC_IE_SS_INFO_DATA;								// 2.0.5

typedef enum {
	SUBSC_IE_SS_INFO_FORW_TYPE		=	1,
	SUBSC_IE_SS_INFO_BARR_TYPE		=	2,
	SUBSC_IE_SS_INFO_DATA_TYPE		=	3,				// 2.0.5
} T_SUBSC_IE_SS_INFO_TYPE;

typedef struct {
	T_SUBSC_IE_SS_INFO_TYPE				info_type;
	union {
		T_SUBSC_IE_SS_INFO_FORW			info_forw;
		T_SUBSC_IE_SS_INFO_BAR			info_bar;
		T_SUBSC_IE_SS_INFO_DATA			info_data;		// 2.0.5
	};
} T_SUBSC_IE_SS_INFO;

typedef struct {
//	T_SUBSC_INFO_IE_TYPE				ie_type;			// only in encoded message
	short										num_ss_info;	// FORW_MAX + DATA_MAX + BAR_MAX
	T_SUBSC_IE_SS_INFO_FORW				ss_info_forw[SS_INFO_IND_FORW_MAX];
	T_SUBSC_IE_SS_INFO_DATA				ss_info_data[SS_INFO_IND_DATA_MAX];
	T_SUBSC_IE_SS_INFO_BAR				ss_info_bar [SS_INFO_IND_BAR_MAX];
} T_SUBSC_IE_SS_INFO_LIST;

#define MAX_NUM_USER_INFO				32
typedef struct {
//	T_SUBSC_INFO_IE_TYPE				ie_type;	// only in encoded message
	short								len;
	unsigned char						name[MAX_NUM_USER_INFO];
} T_SUBSC_IE_USER_INFO;

typedef struct {
	T_SUBSC_IE_MOBILE_ID_IMSI			imsi;
	T_SUBSC_IE_ISDN						msisdn;
	T_SUBSC_IE_PROVISION_BS				prov_bs;
	T_SUBSC_IE_PROVISION_TS				prov_ts;
	T_SUBSC_IE_PROVISION_SS				prov_ss;
	T_SUBSC_IE_SUBSC_STATUS				status;
	T_SUBSC_IE_ODB_DATA					odb_data;
	T_SUBSC_IE_SS_INFO_LIST				ss_info_list;
	T_SUBSC_IE_USER_INFO				user;
	T_SUBSC_IE_GPRS_DATA_INFO			gprs_data;		// 2.1.0
	T_SUBSC_IE_IN_PROFILE				in_profile;		// 2.1.2
} T_SUBSC_INFO;


typedef enum {
	SUBSC_INFO_NO_ERR					= 0,
	SUBSC_INFO_ERR_UNEXP_IE				= 1,
	SUBSC_INFO_ERR_ISDN_NUM_DIGIT		= 2,
	SUBSC_INFO_ERR_BS_NUM_SRV			= 3,
	SUBSC_INFO_ERR_TS_NUM_SRV			= 4,
	SUBSC_INFO_ERR_SS_NUM_SRV			= 5,
	SUBSC_INFO_ERR_UNKNOWN_BASIC		= 6,
	SUBSC_INFO_ERR_FORW_NUM_FEATURE		= 7,
	SUBSC_INFO_ERR_BAR_NUM_FEATURE		= 8,
	SUBSC_INFO_ERR_SS_INFO_TYPE			= 9,
	SUBSC_INFO_ERR_SS_NUM_INFO			= 10,
	SUBSC_INFO_ERR_USER_INFO			= 11,
	SUBSC_INFO_ERR_IMSI_NUM_DIGIT		= 12,
	SUBSC_INFO_ERR_VERSION				= 13,
	SUBSC_INFO_ERR_GPRS_DATA_LEN		= 14,
	SUBSC_INFO_ERR_APN_LEN				= 15,

	SUBSC_INFO_ERR_AUTH_NUM_LIST		= 20,
	SUBSC_INFO_ERR_SS_SUBSC_OPTION		= 21, // 2.1.1
} SUBSC_INFO_ERR;

#define		SEC_RAND_LEN				16
#define		SEC_SRES_LEN				4
#define		SEC_KC_LEN					8

typedef struct {
	unsigned char	RAND[SEC_RAND_LEN];
	unsigned char	SRES[SEC_SRES_LEN];
	unsigned char	Kc[SEC_KC_LEN];
} T_AUTH_TRIPLET;

#define		SEC_MAX_TRIPLET				5

typedef struct {
//	T_SUBSC_INFO_IE_TYPE				ie_type;	// only in encoded message
	short								num_set;
	T_AUTH_TRIPLET						triplet[SEC_MAX_TRIPLET];
} T_SEC_IE_AUTH_LIST;

typedef struct {
	T_SUBSC_IE_MOBILE_ID_IMSI			imsi;
	T_SEC_IE_AUTH_LIST					auth_set;
} T_SEC_INFO;


// Function prototypes

/* SubscInfo.cpp */

char * getSubscInfoIETypeStr(T_SUBSC_INFO_IE_TYPE ie_type);
char * getSubscInfoBSCodeStr(T_SUBSC_INFO_BEARER_SERVICE_CODE code);
char * getSubscInfoTSCodeStr(T_SUBSC_INFO_TELE_SERVICE_CODE code);
char * getSubscInfoSSCodeStr(T_SUBSC_INFO_SUPP_SERVICE_CODE code);
short	checkSubscInfoIEType(unsigned char ie_type, T_SUBSC_INFO_IE_TYPE exp);

void printSubscMsg(unsigned char *msg, short size);

/* SubscInfoMsgEncoder.cpp */

// Encode SubscInfo to Message Buffer (Byte Stream) provided.
// size - size of msg buffer, returned with actual used size
short	EncodeSubscInfo(unsigned char 	*msg,
						short 			*size,
						T_SUBSC_INFO	*subsc);

// Encode SecurityInfo to Message Buffer (Byte Stream) provided.
// size - size of msg buffer, returned with actual used size
short	EncodeSecInfo(unsigned char *msg,
						short		*size,
						T_SEC_INFO	*sec);

/* SubscInfoMsgDecoder.cpp */

// Decode SubscInfo from Message Buffer to the structure.
// size - size of message in msg buffer
short	DecodeSubscInfo(unsigned char	*msg,
						short			size,
						T_SUBSC_INFO	*subsc);

// Decode SecInfo from Message Buffer to the structure.
// size - size of message in msg buffer
short	DecodeSecInfo(unsigned char	*msg,
						short		size,
						T_SEC_INFO	*sec);


#ifdef __cplusplus
}
#endif

#endif // SUBSC_INFO_TYPE_H





