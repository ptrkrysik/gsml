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
// File        : SubscInfoMsgEncoder.cpp
// Author(s)   : Kevin Lim
// Create Date : 06-16-99
// Description : subscriber information structure and encoding function
//               for messages
//
// *******************************************************************
#include "stdio.h"
#include "subsc/SubscInfoType.h"
#include "subsc/SubscInfoUtil.h"

short EncodeVersionNumber(unsigned char *msg, short *i)
{
	msg[(*i)++] = SUBSC_INFO_IE_VERSION_NUMBER;
	msg[(*i)++] = SUBSC_ENC_VER0;
	msg[(*i)++] = SUBSC_ENC_VER1;
	msg[(*i)++] = SUBSC_ENC_VER2;
	return 0;
}

short EncodeMobileId(unsigned char *msg, short *i, T_SUBSC_IE_MOBILE_ID_IMSI *imsi)
{
	int j, num;
	msg[(*i)++] = SUBSC_INFO_IE_MOBILE_ID_IMSI;
	num = msg[(*i)++] = (unsigned char)imsi->num_digit;
	if(num > MAX_NUM_SUBSC_IMSI){
		printf("Err. mobile id too large : %d\n", num);
		return SUBSC_INFO_ERR_IMSI_NUM_DIGIT;
	}
	for(j=0;j<num;j++) msg[(*i)++] = imsi->imsi[j];
	return 0;
}

short EncodeISDN(unsigned char *msg, short *i, T_SUBSC_IE_ISDN *msisdn)
{
	int j, num;
	msg[(*i)++] = SUBSC_INFO_IE_ISDN;
	num = msg[(*i)++] = (unsigned char)msisdn->num_digit;
	if(num > MAX_NUM_SUBSC_ISDN){
		printf("Err. msisdn num digit too large : %d\n", num);
		return SUBSC_INFO_ERR_ISDN_NUM_DIGIT;
	}
	for(j=0;j<num;j++) msg[(*i)++] = msisdn->digits[j];
	for(j=0;j<NUM_PLAN_LEN;j++) msg[(*i)++] = msisdn->num_plan[j];
	return 0;
}

short EncodeSubAddr(unsigned char *msg, short *i, T_SUBSC_IE_SUBADDR *subaddr)
{
	int j;
	msg[(*i)++] = SUBSC_INFO_IE_SUBADDR;
	msg[(*i)++] = (unsigned char)subaddr->subaddr_type;
	for(j=0;j<NUM_SUBADDR_INFO;j++) msg[(*i)++] = subaddr->info[j];
	return 0;
}

short EncodeProvisionBS(unsigned char *msg, short *i, T_SUBSC_IE_PROVISION_BS *bs)
{
	int j, num;
	msg[(*i)++] = SUBSC_INFO_IE_PROVISION_BS;
	num = msg[(*i)++] = (unsigned char)bs->num_srv;
	if(num > MAX_NUM_BEARER_SERVICE){
		printf("Err. bearer service number too large : %d\n", num);
		return SUBSC_INFO_ERR_BS_NUM_SRV;
	}
	for(j=0;j<num;j++) msg[(*i)++] = bs->bs_code[j];
	return 0;
}

short EncodeProvisionTS(unsigned char *msg, short *i, T_SUBSC_IE_PROVISION_TS *ts)
{
	int j, num;
	msg[(*i)++] = SUBSC_INFO_IE_PROVISION_TS;
	num = msg[(*i)++] = (unsigned char)ts->num_srv;
	if(num > MAX_NUM_TELE_SERVICE){
		printf("Err. tele service number too large : %d\n", num);
		return SUBSC_INFO_ERR_TS_NUM_SRV;
	}
	for(j=0;j<num;j++) msg[(*i)++] = ts->ts_code[j];
	return 0;
}

short EncodeProvisionSS(unsigned char *msg, short *i, T_SUBSC_IE_PROVISION_SS *ss)
{
	int j, num;
	msg[(*i)++] = SUBSC_INFO_IE_PROVISION_SS;
	num = msg[(*i)++] = (unsigned char)ss->num_srv;
	if(num > MAX_NUM_SUPP_SERVICE){
		printf("Err. supp service number too large : %d\n", num);
		return SUBSC_INFO_ERR_SS_NUM_SRV;
	}
	for(j=0;j<num;j++) msg[(*i)++] = ss->ss_code[j];
	return 0;
}

short EncodeStatusFlag(unsigned char *msg, short *i, T_SUBSC_IE_SUBSC_STATUS *status)
{
	msg[(*i)++] = SUBSC_INFO_IE_SUBSC_STATUS;
	msg[(*i)++] = (unsigned char)status->status;
	return 0;
}

short EncodeODBData(unsigned char *msg, short *i, T_SUBSC_IE_ODB_DATA *odb_data)
{
	msg[(*i)++] = SUBSC_INFO_IE_ODB_DATA;
	msg[(*i)++] = 0; // reserved
	msg[(*i)++] = 0; // reserved
	msg[(*i)++] = 0 |
		((odb_data->odb_general.multiple_ect_barred & 0x01) 			<< ODB_GEN_BIT_MULTI_ECT) |
		((odb_data->odb_general.double_chargeable_ect_barred & 0x01) 	<< ODB_GEN_BIT_DBL_CHARGE_ECT) |
		((odb_data->odb_general.intrz_ect_barred & 0x01) 				<< ODB_GEN_BIT_INTRZ_ECT) |
		((odb_data->odb_general.intrn_ect_barred & 0x01) 				<< ODB_GEN_BIT_INTRN_ECT) |
		((odb_data->odb_general.chargeable_ect_barred & 0x01) 			<< ODB_GEN_BIT_CHARGE_ECT) |
		((odb_data->odb_general.all_ect_barred & 0x01) 					<< ODB_GEN_BIT_ALL_ECT) |
		((odb_data->odb_general.intrz_intrn_og_not_hplmn_barred & 0x01) << ODB_GEN_BIT_INTRZ_INTRN_NOT_HPLMN);

	msg[(*i)++] = 0 |
		((odb_data->odb_general.intrz_og_not_hplmn_barred & 0x01) 		<< ODB_GEN_BIT_INTRZ_OG_NOT_HPLMN) |
		((odb_data->odb_general.intrz_og_barred & 0x01) 				<< ODB_GEN_BIT_INTRZ_OG) |
		((odb_data->odb_general.ss_access_barred & 0x01) 				<< ODB_GEN_BIT_SS_ACCESS) |
		((odb_data->odb_general.premium_enter_og_barred & 0x01) 		<< ODB_GEN_BIT_PREM_ENT) |
		((odb_data->odb_general.premium_info_og_barred & 0x01) 			<< ODB_GEN_BIT_PREM_INFO) |
		((odb_data->odb_general.intrn_og_not_hplmn_barred & 0x01) 		<< ODB_GEN_BIT_INTRN_OG_NOT_HPLMN) |
		((odb_data->odb_general.intrn_og_barred & 0x01) 				<< ODB_GEN_BIT_INTRN_OG) |
		((odb_data->odb_general.all_og_barred & 0x01) 					<< ODB_GEN_BIT_ALL_OG);

	msg[(*i)++] = 0; // reserved
	msg[(*i)++] = 0; // reserved
	msg[(*i)++] = 0; // reserved
	msg[(*i)++] = 0 |
		((odb_data->odb_plmn.plmn_specific_type4 & 0x01)	<< ODB_HPLMN_BIT_TYPE_4) |
		((odb_data->odb_plmn.plmn_specific_type3 & 0x01)	<< ODB_HPLMN_BIT_TYPE_3) |
		((odb_data->odb_plmn.plmn_specific_type2 & 0x01)	<< ODB_HPLMN_BIT_TYPE_2) |
		((odb_data->odb_plmn.plmn_specific_type1 & 0x01)	<< ODB_HPLMN_BIT_TYPE_1);

	return 0;
}

short EncodeBasicServiceCode(unsigned char *msg, short *i,
							T_SUBSC_INFO_EXT_BASIC_SERVICE *basic_service)
{
	int j;
	msg[(*i)++] = basic_service->service_type;
	switch(basic_service->service_type){
		case BEARER_SERVICE_TYPE:
			for(j=0;j<5;j++) msg[(*i)++] = basic_service->ex_bs_code[j];
			break;
		case TELE_SERVICE_TYPE:
			for(j=0;j<5;j++) msg[(*i)++] = basic_service->ex_ts_code[j];
			break;
		default:
			printf("Unknown Basic Service Type :%d\n", basic_service->service_type);
			return SUBSC_INFO_ERR_UNKNOWN_BASIC;
	}
	return 0;
}

short EncodeSSStatus(unsigned char *msg, short *i,
					T_SS_STATUS_FLAG	*ss_status)
{
	msg[(*i)++] = 0 |
		((ss_status->ss_status_A & 0x01)	<< SS_STATUS_A_BIT) |
		((ss_status->ss_status_R & 0x01)	<< SS_STATUS_R_BIT) |
		((ss_status->ss_status_P & 0x01)	<< SS_STATUS_P_BIT) |
		((ss_status->ss_status_Q & 0x01)	<< SS_STATUS_Q_BIT);
	return 0;
}

short EncodeSSSubscOption(unsigned char *msg, short *i, T_SS_SUBSC_OPTION *ss_subsc_option) // 2.1.1
{
	if (ss_subsc_option->isPresent==false)
	{
		msg[(*i)++] = 0;
		return 0;
	}

	msg[(*i)++] = 1;
	msg[(*i)++] = (unsigned char) ss_subsc_option->option_type;

	switch(ss_subsc_option->option_type)
	{
	case SS_SUBSC_OPTION_CLIR_OPTION:
		msg[(*i)++] = (unsigned char) ss_subsc_option->clir_option;
		break;

	case SS_SUBSC_OPTION_OVERRIDE_CATEGORY:
		msg[(*i)++] = (unsigned char) ss_subsc_option->override_category;
		break;

	default:
		printf("Unsupported SS Subscription Option Type : %d\n", ss_subsc_option->option_type);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_SS_SUBSC_OPTION;
	}

	return 0;
}

short EncodeForwOption(unsigned char *msg, short *i,
					T_SS_FORW_OPTION	*forw)
{
	msg[(*i)++] = 0 |
		((forw->notify_forw_party & 0x01) 	<< SS_NOTIFY_FORW_FLAG_BIT) |
		((forw->notify_calling_party & 0x01)<< SS_NOTIFY_CALLING_FLAG_BIT) |
		((forw->forw_reason & 0x03) 		<< SS_FORW_REASON_FLAG_BIT);
	return 0;
}

short EncodeSSInfoForw(unsigned char *msg, short *i, T_SUBSC_IE_SS_INFO_FORW *forw)
{
	int j, num;
	msg[(*i)++] = (unsigned char)forw->ss_code;
	num = msg[(*i)++] = (unsigned char)forw->num_feature;
	if(num > MAX_NUM_EXT_FEATURE){
		printf("Err. forw feature number too large : %d\n", num);
		return SUBSC_INFO_ERR_FORW_NUM_FEATURE;
	}
	for(j=0; j<num; j++){
		EncodeBasicServiceCode(msg, i, &forw->list[j].basic_service);
		EncodeSSStatus(msg, i, &forw->list[j].ss_status);
		EncodeISDN(msg, i, &forw->list[j].forw_to_num);
		EncodeSubAddr(msg, i, &forw->list[j].forw_to_sub);
		EncodeForwOption(msg, i, &forw->list[j].forw_options);
		msg[(*i)++] = (unsigned char)forw->list[j].no_reply_cond_time;
	}
	return 0;
}

short EncodeSSInfoBar(unsigned char *msg, short *i, T_SUBSC_IE_SS_INFO_BAR *bar)
{
	int j, num;
	msg[(*i)++] = (unsigned char)bar->ss_code;
	num = msg[(*i)++] = (unsigned char)bar->num_feature;
	if(num > MAX_NUM_EXT_FEATURE){
		printf("Err. bar feature number too large : %d\n", num);
		return SUBSC_INFO_ERR_BAR_NUM_FEATURE;
	}
	for(j=0; j<num; j++){
		EncodeBasicServiceCode(msg, i, &bar->list[j].basic_service);
		EncodeSSStatus(msg, i, &bar->list[j].ss_status);
	}
	return 0;
}

short EncodeSSInfoData(unsigned char *msg, short *i, T_SUBSC_IE_SS_INFO_DATA *data)	// 2.0.5
{
	msg[(*i)++] = (unsigned char)data->ss_code;
	EncodeSSStatus(msg, i, &data->ss_status);
	EncodeSSSubscOption(msg, i, &data->ss_subsc_option); // 2.1.1
	return 0;
}

short EncodeSSInfo(unsigned char *msg, short *i, T_SUBSC_IE_SS_INFO *ss_info)
{
	msg[(*i)++] = (unsigned char)ss_info->info_type;
	switch(ss_info->info_type){
		case SUBSC_IE_SS_INFO_FORW_TYPE:
			EncodeSSInfoForw(msg, i, &ss_info->info_forw);
			break;
		case SUBSC_IE_SS_INFO_BARR_TYPE:
			EncodeSSInfoBar(msg, i, &ss_info->info_bar);
			break;
		case SUBSC_IE_SS_INFO_DATA_TYPE:						// 2.0.5
			EncodeSSInfoData(msg, i, &ss_info->info_data);
			break;
		default:
			printf("Unsupported SS Info Type encountered :%d\n", ss_info->info_type);
			return SUBSC_INFO_ERR_SS_INFO_TYPE;
	}
	return 0;
}

short EncodeSSInfoList(unsigned char *msg, short *i, T_SUBSC_IE_SS_INFO_LIST *ss_info_list)
{
	int j, num;
	short ret;
	msg[(*i)++] = SUBSC_INFO_IE_SS_INFO_LIST;
	num = msg[(*i)++] = (unsigned char)ss_info_list->num_ss_info;
	if(num > MAX_NUM_SUPP_SERVICE){
		printf("Err. ss info list number too large : %d\n", num);
		return SUBSC_INFO_ERR_SS_NUM_INFO;
	}
	for(j=0; j<SS_INFO_IND_FORW_MAX; j++){
		msg[(*i)++] = (unsigned char)SUBSC_IE_SS_INFO_FORW_TYPE;
		ret = EncodeSSInfoForw(msg, i, &ss_info_list->ss_info_forw[j]);
		if(ret) return ret;
	}
	for(j=0; j<SS_INFO_IND_DATA_MAX; j++){
		msg[(*i)++] = (unsigned char)SUBSC_IE_SS_INFO_DATA_TYPE;
		ret = EncodeSSInfoData(msg, i, &ss_info_list->ss_info_data[j]);
		if(ret) return ret;
	}
	for(j=0; j<SS_INFO_IND_BAR_MAX; j++){
		msg[(*i)++] = (unsigned char)SUBSC_IE_SS_INFO_BARR_TYPE;
		ret = EncodeSSInfoBar(msg, i, &ss_info_list->ss_info_bar[j]);
		if(ret) return ret;
	}
	return 0;
}

short EncodeUserInfo(unsigned char *msg, short *i, T_SUBSC_IE_USER_INFO *user)
{
	int j, num;

	msg[(*i)++] = SUBSC_INFO_IE_USER_INFO;
	num = msg[(*i)++] = (unsigned char)user->len;
	if(num > MAX_NUM_USER_INFO){
		printf("Err. user info too large: %d\n", num);
		return SUBSC_INFO_ERR_USER_INFO;
	}
	for(j=0; j<num; j++){
		msg[(*i)++] = user->name[j];
	}
	return 0;
}

short EncodeGPRSData(unsigned char *msg, short *i, T_SUBSC_GPRS_DATA *data)
{
	int j, num;

	msg[(*i)++] = data->pdpContextId;
	msg[(*i)++] = (unsigned char)data->pdpType;
	for(j=0; j<MAX_LEN_SUBSC_PDP_ADDR; j++)
		msg[(*i)++] = data->pdpAddress[j];
	msg[(*i)++] = (unsigned char)data->qosSubscribed.delay_class;
	msg[(*i)++] = (unsigned char)data->qosSubscribed.reliability_class;
	msg[(*i)++] = (unsigned char)data->qosSubscribed.precedence_class;
	msg[(*i)++] = (unsigned char)data->qosSubscribed.peak_throughput;
	msg[(*i)++] = (unsigned char)data->qosSubscribed.mean_throughput;
	msg[(*i)++] = data->vplmnAddressAllowed;
	num = msg[(*i)++] = data->apn_length;
	if(num > MAX_LEN_SUBSC_APN){
		printf("Err. apn length too large: %d\n", num);
		return SUBSC_INFO_ERR_APN_LEN;
	}
	for(j=0; j<num; j++)
		msg[(*i)++] = data->apn[j];
	return 0;
}

short EncodeGPRSDataInfo(unsigned char *msg, short *i, T_SUBSC_IE_GPRS_DATA_INFO *gprs_data)
{
	int j, num;
	short ret;

	msg[(*i)++] = SUBSC_INFO_IE_GPRS_DATA;
	num = msg[(*i)++] = (unsigned char)gprs_data->num_data;
	if(num > MAX_NUM_SUBSC_GPRS_DATA){
		printf("Err. gprs data too many: %d\n", num);
		return SUBSC_INFO_ERR_GPRS_DATA_LEN;
	}
	for(j=0; j<num; j++){
		ret = EncodeGPRSData(msg, i, &gprs_data->data[j]);
		if(ret) return ret;
	}
	return 0;
}

// EncodeSubscInfo or DecodeSubscInfo does not allocate memory spaces
// for message buffer nor subsc info sturcture.  It should be allocated
// prior to the calls and pass pointer to the functions.

// Encode SubscInfo to Message Buffer (Byte Stream) provided.
// size - size of msg buffer, returned with actual used size
bool dbgEncodeSubscInfoFlag = false;

extern "C" void dbgEncodeSubscInfoFlagToggle(void)
{
	dbgEncodeSubscInfoFlag = !dbgEncodeSubscInfoFlag;
	printf("[dbgEncodeSubscInfoFlagToggle] debugging is %s\n",
		(dbgEncodeSubscInfoFlag)? "enabled" : "disabled");
}

short	EncodeSubscInfo(unsigned char 	*msg,
						short 			*size,
						T_SUBSC_INFO	*subsc)
{
	short	i = 0;
	short 	ret;

	if (dbgEncodeSubscInfoFlag)
	{
		printf("[EncodeSubscInfo] \n");
		subscPrintInfo(subsc);
	}

	if(ret = EncodeVersionNumber(msg, &i)) return ret;
	if(ret = EncodeMobileId(msg, &i, &subsc->imsi)) return ret;
	if(ret = EncodeISDN(msg, &i, &subsc->msisdn)) return ret;
	if(ret = EncodeProvisionBS(msg, &i, &subsc->prov_bs)) return ret;
	if(ret = EncodeProvisionTS(msg, &i, &subsc->prov_ts)) return ret;
	if(ret = EncodeProvisionSS(msg, &i, &subsc->prov_ss)) return ret;
	if(ret = EncodeStatusFlag(msg, &i, &subsc->status)) return ret;
	if(ret = EncodeODBData(msg, &i, &subsc->odb_data)) return ret;
	if(ret = EncodeSSInfoList(msg, &i, &subsc->ss_info_list)) return ret;
	if(ret = EncodeUserInfo(msg, &i, &subsc->user)) return ret;
	if(ret = EncodeGPRSDataInfo(msg, &i, &subsc->gprs_data)) return ret;
	*size = i;

	return 0;
}


short EncodeAuthTriplet(unsigned char *msg, short *i, T_AUTH_TRIPLET *triplet)
{
	int j;
	for(j=0; j<SEC_RAND_LEN; j++){ msg[(*i)++] = triplet->RAND[j]; }
	for(j=0; j<SEC_SRES_LEN; j++){ msg[(*i)++] = triplet->SRES[j]; }
	for(j=0; j<SEC_KC_LEN; j++){ msg[(*i)++] = triplet->Kc[j]; }
	return 0;
}

short EncodeAuthList(unsigned char *msg, short *i, T_SEC_IE_AUTH_LIST *auth_set)
{
	int j, num;
	short ret;
	msg[(*i)++] = SUBSC_INFO_IE_AUTH_LIST;
	num = msg[(*i)++] = (unsigned char)auth_set->num_set;
	if(num > SEC_MAX_TRIPLET){
		printf("Err. auth triplet list number too large : %d\n", num);
		return SUBSC_INFO_ERR_AUTH_NUM_LIST;
	}
	for(j=0; j<num; j++){
		ret = EncodeAuthTriplet(msg, i, &auth_set->triplet[j]);
		if(ret) return ret;
	}
	return 0;
}

// EncodeSecInfo or DecodeSecInfo does not allocate memory spaces
// for message buffer nor sec info sturcture.  It should be allocated
// prior to the calls and pass pointer to the functions.

// Encode SecInfo to Message Buffer (Byte Stream) provided.
// size - size of msg buffer, returned with actual used size
short	EncodeSecInfo(unsigned char 	*msg,
						short 			*size,
						T_SEC_INFO		*sec)
{
	short	i = 0;
	short 	ret;

	if((ret = EncodeMobileId(msg, &i, &sec->imsi))) return ret;
	if((ret = EncodeAuthList(msg, &i, &sec->auth_set))) return ret;
	*size = i;
	return 0;
}

