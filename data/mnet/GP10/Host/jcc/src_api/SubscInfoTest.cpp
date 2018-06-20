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
// File        : SubscInfoTest.cpp
// Author(s)   : Kevin Lim
// Create Date : 06-16-99
// Description : subscriber information structure and en/decoding function
//               for messages
//
// *******************************************************************
#include "stdio.h"
#include "string.h"

// from common vob 
#include "subsc/SubscInfoType.h"
#include "subsc/SubscInfoUtil.h"

#include "JCC/LUDBapi.h"

void ludbMakeDefaultSubscMsg(char *msg, short *size)
{
	T_SUBSC_INFO	test_info;
	short ret;

	test_info.imsi.num_digit = 15;
	test_info.imsi.imsi[0] = 0;
	test_info.imsi.imsi[1] = 0;
	test_info.imsi.imsi[2] = 1;
	test_info.imsi.imsi[3] = 0;
	test_info.imsi.imsi[4] = 1;
	test_info.imsi.imsi[5] = 1;
	test_info.imsi.imsi[6] = 2;
	test_info.imsi.imsi[7] = 3;
	test_info.imsi.imsi[8] = 4;
	test_info.imsi.imsi[9] = 5;
	test_info.imsi.imsi[10] = 6;
	test_info.imsi.imsi[11] = 7;
	test_info.imsi.imsi[12] = 8;
	test_info.imsi.imsi[13] = 9;
	test_info.imsi.imsi[14] = 0;
	test_info.msisdn.num_digit = 13;
    test_info.msisdn.digits[0] = 0; 
    test_info.msisdn.digits[1] = 0; 
    test_info.msisdn.digits[2] = 1; 
    test_info.msisdn.digits[3] = 6; 
    test_info.msisdn.digits[4] = 5; 
    test_info.msisdn.digits[5] = 0; 
    test_info.msisdn.digits[6] = 3; 
    test_info.msisdn.digits[7] = 3; 
    test_info.msisdn.digits[8] = 0; 
    test_info.msisdn.digits[9] = 2; 
    test_info.msisdn.digits[10] = 8; 
    test_info.msisdn.digits[11] = 9; 
    test_info.msisdn.digits[12] = 0; 
	test_info.prov_bs.num_srv = 1;
	test_info.prov_bs.bs_code[0] = DATA_CDA_9600BPS; 
	test_info.prov_ts.num_srv = 3;
	test_info.prov_ts.ts_code[0] = TELEPHONY_SERVICES; 
	test_info.prov_ts.ts_code[1] = EMERGENCY_CALLS; 
	test_info.prov_ts.ts_code[2] = SMS_MT_PP; 
	test_info.prov_ss.num_srv = 4;
	test_info.prov_ss.ss_code[0] = SUPP_SERV_CLIP; 
	test_info.prov_ss.ss_code[1] = SUPP_SERV_COLP; 
	test_info.prov_ss.ss_code[2] = SUPP_SERV_CFU; 
	test_info.prov_ss.ss_code[3] = SUPP_SERV_CW; 
	test_info.status.status = SUBSC_STATUS_ODBARRING;
    test_info.odb_data.odb_general.all_og_barred=false;
	test_info.odb_data.odb_general.intrn_og_barred=false;
	test_info.odb_data.odb_general.intrn_og_not_hplmn_barred=false;
	test_info.odb_data.odb_general.premium_info_og_barred=false;
	test_info.odb_data.odb_general.premium_enter_og_barred=false;
	test_info.odb_data.odb_general.ss_access_barred=true;
	test_info.odb_data.odb_general.intrz_og_barred=false;
	test_info.odb_data.odb_general.intrz_og_not_hplmn_barred=false;
	test_info.odb_data.odb_general.intrz_intrn_og_not_hplmn_barred=false;
	test_info.odb_data.odb_general.all_ect_barred=false;
	test_info.odb_data.odb_general.chargeable_ect_barred=false;
	test_info.odb_data.odb_general.intrn_ect_barred=false;
	test_info.odb_data.odb_general.intrz_ect_barred=false;
	test_info.odb_data.odb_general.double_chargeable_ect_barred=false;
	test_info.odb_data.odb_general.multiple_ect_barred=true;	
	test_info.odb_data.odb_plmn.plmn_specific_type1=false;	
	test_info.odb_data.odb_plmn.plmn_specific_type2=true;	
	test_info.odb_data.odb_plmn.plmn_specific_type3=false;	
	test_info.odb_data.odb_plmn.plmn_specific_type4=false;	
	test_info.ss_info_list.num_ss_info = 3;	
	test_info.ss_info_list.ss_info_forw[0].ss_code = SUPP_SERV_ALL_FWD;
	test_info.ss_info_list.ss_info_forw[0].num_feature = 1;
	test_info.ss_info_list.ss_info_forw[0].list[0].basic_service.service_type = BEARER_SERVICE_TYPE;
	test_info.ss_info_list.ss_info_forw[0].list[0].basic_service.ex_bs_code[0] = DATA_CDA_9600BPS;
	test_info.ss_info_list.ss_info_forw[0].list[0].basic_service.ex_bs_code[1] = 0;	// reserved byte filler
	test_info.ss_info_list.ss_info_forw[0].list[0].basic_service.ex_bs_code[2] = 0;
	test_info.ss_info_list.ss_info_forw[0].list[0].basic_service.ex_bs_code[3] = 0;
	test_info.ss_info_list.ss_info_forw[0].list[0].basic_service.ex_bs_code[4] = 0;
	test_info.ss_info_list.ss_info_forw[0].list[0].ss_status.ss_status_A = 1;
	test_info.ss_info_list.ss_info_forw[0].list[0].ss_status.ss_status_R = 0;
	test_info.ss_info_list.ss_info_forw[0].list[0].ss_status.ss_status_P = 0;
	test_info.ss_info_list.ss_info_forw[0].list[0].ss_status.ss_status_Q = 0;
	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.num_digit = 13;
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.digits[0] = 0; 
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.digits[1] = 0; 
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.digits[2] = 1; 
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.digits[3] = 5; 
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.digits[4] = 1; 
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.digits[5] = 0; 
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.digits[6] = 5; 
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.digits[7] = 7;
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.digits[8] = 4;
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.digits[9] = 0; 
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.digits[10] = 4; 
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.digits[11] = 3; 
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_to_num.digits[12] = 6; 
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_options.notify_forw_party = NOTIFICATION_TO_FORWARDING; 
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_options.notify_calling_party = NOTIFICATION_TO_CALLING; 
    	test_info.ss_info_list.ss_info_forw[0].list[0].forw_options.forw_reason = NO_REPLY; 
	test_info.ss_info_list.ss_info_bar[0].ss_code = SUPP_SERV_BAOC;
	test_info.ss_info_list.ss_info_bar[0].num_feature = 1;
	test_info.ss_info_list.ss_info_bar[0].list[0].basic_service.service_type = TELE_SERVICE_TYPE;
	test_info.ss_info_list.ss_info_bar[0].list[0].basic_service.ex_ts_code[0] = TELEPHONY_SERVICES;
	test_info.ss_info_list.ss_info_bar[0].list[0].basic_service.ex_ts_code[1] = 0;	// reserved byte filler
	test_info.ss_info_list.ss_info_bar[0].list[0].basic_service.ex_ts_code[2] = 0;
	test_info.ss_info_list.ss_info_bar[0].list[0].basic_service.ex_ts_code[3] = 0;
	test_info.ss_info_list.ss_info_bar[0].list[0].basic_service.ex_ts_code[4] = 0;
	test_info.ss_info_list.ss_info_bar[0].list[0].ss_status.ss_status_A = 1;
	test_info.ss_info_list.ss_info_bar[0].list[0].ss_status.ss_status_R = 0;
	test_info.ss_info_list.ss_info_bar[0].list[0].ss_status.ss_status_P = 0;
	test_info.ss_info_list.ss_info_bar[0].list[0].ss_status.ss_status_Q = 0;
	test_info.ss_info_list.ss_info_data[0].ss_code = SUPP_SERV_CW;
	test_info.ss_info_list.ss_info_data[0].ss_status.ss_status_A = 1;
	test_info.ss_info_list.ss_info_data[0].ss_status.ss_status_R = 0;
	test_info.ss_info_list.ss_info_data[0].ss_status.ss_status_P = 1;
	test_info.ss_info_list.ss_info_data[0].ss_status.ss_status_Q = 0;
	test_info.user.len = 5;
	test_info.user.name[0] = 'K';
	test_info.user.name[1] = 'e';
	test_info.user.name[2] = 'v';
	test_info.user.name[3] = 'i';
	test_info.user.name[4] = 'n';

	ret = EncodeSubscInfo((unsigned char *)msg, size, &test_info);
	printf("Encoder return val: %d, msg size return: %d\n", ret, *size);

	printSubscMsg((unsigned char *)msg, *size);

	ret = DecodeSubscInfo((unsigned char *)msg, *size, &test_info);
	printf("Decoder return val: %d\n", ret);

	subscPrintInfo(&test_info);
}

short subscTest1()
{
	T_SUBSC_INFO	subsc;
	char msg[200] = {
	 1, 51, 49, 48, 49, 55, 48, 49, 48, 55,
	50, 55, 57, 52, 56, 56,  2,  4, 50, 50,
	48, 57,  5,  0,  6,  0,  7,  9, 17, 18,
	19, 33, 41, 42, 43, 65, 66,  8,  0,  9,
	 0,  0,  0,  0,  0,  0,  0,  0, 10,  4,
	 1, 33,  1,  1,  0,  0,  0,  0,  0,  1,
	 2,  0,  3,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  1, 41,  0,  1,
	42,  0,  1, 43,  0, 11, 13, 32, 77,111,
   116,111,114,111,108, 97, 32, 45, 32, 49, 
	};

	return DecodeSubscInfo((unsigned char *)msg, 110, &subsc);	
}

void ludbPrintSubscInfoSize()
{
	printf("sizeof(T_SUBSC_INFO) : %d\n", sizeof(T_SUBSC_INFO));
}

void ludbPrintLudbMsgSize()
{
	printf("sizeof(LudbMsgStruct) : %d\n", sizeof(LudbMsgStruct));
}
