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
// File        : SubscInfoMsgDecoder.cpp
// Author(s)   : Kevin Lim
// Create Date : 06-16-99
// Description : subscriber information structure and decoding function
//               for messages
//
// Modification: 11-11-00
//			     ccpentium compiler gives warning of
//               Warning: using `%eax' instead of `%ax' due to `l' suffix
//               if assigning character referenced value to short
//				 kludge added to assign it to int and then assign it to short
// *******************************************************************
#include "stdio.h"
#include "string.h"
#include "subsc/SubscInfoType.h"
#include "subsc/SubscInfoUtil.h"

short isVerGE(T_SUBSC_VERSION v, int v0, int v1, int v2)
{
	int ver1 = v.v0 * 100 + v.v1 * 10 + v.v2;
	int ver2 = v0 * 100 + v1 * 10 + v2;
	if(ver1 >= ver2) return 1;
	else return 0;
}

short DecodeVersionNumber(unsigned char *msg, short *i, T_SUBSC_VERSION *v)
{
	v->v0 = 0;
	v->v1 = 0;
	v->v2 = 0;
	short ret = checkSubscInfoIEType(msg[(*i)], SUBSC_INFO_IE_VERSION_NUMBER);
	if(ret){
		ret = checkSubscInfoIEType(msg[(*i)], SUBSC_INFO_IE_MOBILE_ID_IMSI);
		if(ret) return SUBSC_INFO_ERR_UNEXP_IE;
		else return -1;	// missing version number but has valid imsi field
	}
	else (*i)++;

	int kludge;
	v->v0 = kludge = msg[(*i)++];
	v->v1 = kludge = msg[(*i)++];
	v->v2 = kludge = msg[(*i)++];
	if(v->v0 == SUBSC_ENC_VER0){
		if(v->v1 == SUBSC_ENC_VER1){
			if(v->v2 == SUBSC_ENC_VER2){
				return 0;
			}
		}
	}
	return 1;
}

short DecodeMobileId1(unsigned char *msg, short *i, T_SUBSC_IE_MOBILE_ID_IMSI *imsi)
{
	int j;
	short ret = checkSubscInfoIEType(msg[(*i)++], SUBSC_INFO_IE_MOBILE_ID_IMSI);
	if(ret) return SUBSC_INFO_ERR_UNEXP_IE;

	imsi->num_digit = SUBSC_MCC_LEN + SUBSC_MNC_LEN + SUBSC_MSIN_LEN;
	for(j=0;j<imsi->num_digit;j++) imsi->imsi[j] = msg[(*i)++];
	imsi->imsi[j] = '\0';	// null term
	return 0;
}

short DecodeMobileId2(unsigned char *msg, short *i, T_SUBSC_IE_MOBILE_ID_IMSI *imsi)
{
	int j, num;
	short ret = checkSubscInfoIEType(msg[(*i)++], SUBSC_INFO_IE_MOBILE_ID_IMSI);
	if(ret) return SUBSC_INFO_ERR_UNEXP_IE;

	num = imsi->num_digit = msg[(*i)++];
	if(num > MAX_NUM_SUBSC_IMSI){
		printf("Err. imsi num digit too large : %d\n", num);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_IMSI_NUM_DIGIT;
	}
	for(j=0;j<num;j++) imsi->imsi[j] = msg[(*i)++];
	imsi->imsi[j] = '\0';	// null term
	return 0;
}

short DecodeISDN(unsigned char *msg, short *i, T_SUBSC_IE_ISDN *msisdn, T_SUBSC_VERSION *v)
{
	int j, num;
	short ret = checkSubscInfoIEType(msg[(*i)++], SUBSC_INFO_IE_ISDN);
	if(ret) return SUBSC_INFO_ERR_UNEXP_IE;

	msisdn->num_digit = num = msg[(*i)++];
	if(num > MAX_NUM_SUBSC_ISDN){
		printf("Err. msisdn num digit too large : %d\n", num);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_ISDN_NUM_DIGIT;
	}
	for(j=0;j<num;j++) msisdn->digits[j] = msg[(*i)++];
	msisdn->digits[j] = '\0';	// null term

	if(isVerGE(*v, 2, 0, 4)){	// version 2.0.4 up
		for(j=0; j<NUM_PLAN_LEN; j++){
			msisdn->num_plan[j] = msg[(*i)++];
		}
	}
	return 0;
}

short DecodeSubAddr(unsigned char *msg, short *i, T_SUBSC_IE_SUBADDR *subaddr)
{
	int j, kludge;
	short ret = checkSubscInfoIEType(msg[(*i)++], SUBSC_INFO_IE_SUBADDR);
	if(ret) return SUBSC_INFO_ERR_UNEXP_IE;

	subaddr->subaddr_type = kludge = msg[(*i)++];
	for(j=0; j<NUM_SUBADDR_INFO; j++) subaddr->info[j] = msg[(*i)++];
	return 0;
}

short DecodeProvisionBS(unsigned char *msg, short *i, T_SUBSC_IE_PROVISION_BS *bs)
{
	int j, num;
	short ret = checkSubscInfoIEType(msg[(*i)++], SUBSC_INFO_IE_PROVISION_BS);
	if(ret) return SUBSC_INFO_ERR_UNEXP_IE;

	bs->num_srv = num = msg[(*i)++];
	if(num > MAX_NUM_BEARER_SERVICE){
		printf("Err. bearer service number too large : %d\n", num);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_BS_NUM_SRV;
	}
	for(j=0;j<num;j++) bs->bs_code[j] = (T_SUBSC_INFO_BEARER_SERVICE_CODE)msg[(*i)++];
	return 0;
}

short DecodeProvisionTS(unsigned char *msg, short *i, T_SUBSC_IE_PROVISION_TS *ts)
{
	int j, num;
	short ret = checkSubscInfoIEType(msg[(*i)++], SUBSC_INFO_IE_PROVISION_TS);
	if(ret) return SUBSC_INFO_ERR_UNEXP_IE;

	ts->num_srv = num = msg[(*i)++];
	if(num > MAX_NUM_TELE_SERVICE){
		printf("Err. tele service number too large : %d\n", num);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_TS_NUM_SRV;
	}
	for(j=0;j<num;j++) ts->ts_code[j] = (T_SUBSC_INFO_TELE_SERVICE_CODE)msg[(*i)++];
	return 0;
}

short DecodeProvisionSS(unsigned char *msg, short *i, T_SUBSC_IE_PROVISION_SS *ss,
								T_SUBSC_IE_SS_INFO_LIST *ss_info_list, T_SUBSC_VERSION *v)
{
	int j, num;
	short ret = checkSubscInfoIEType(msg[(*i)++], SUBSC_INFO_IE_PROVISION_SS);
	if(ret) return SUBSC_INFO_ERR_UNEXP_IE;

	ss->num_srv = num = msg[(*i)++];
	if(num > MAX_NUM_SUPP_SERVICE){
		printf("Err. supp. service number too large : %d\n", num);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_SS_NUM_SRV;
	}
	for(j=0;j<num;j++){
		T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code =
				(T_SUBSC_INFO_SUPP_SERVICE_CODE)msg[(*i)++];

                /* fix wrong code */
                if(ss_code == 0x96) ss_code = SUPP_SERV_BAIC;
                if(ss_code == 0x97) ss_code = SUPP_SERV_BIC_ROAM;

		ss->ss_code[j] = ss_code;
		if(!isVerGE(*v, 2, 0, 6)){
			subscSetP(ss_info_list, ss_code, 1);
		}
	}
	return 0;
}

short DecodeStatusFlag(unsigned char *msg, short *i, T_SUBSC_IE_SUBSC_STATUS *status)
{
	short ret = checkSubscInfoIEType(msg[(*i)++], SUBSC_INFO_IE_SUBSC_STATUS);
	if(ret) return SUBSC_INFO_ERR_UNEXP_IE;

	status->status = (T_SUBSC_STATUS_TYPE)msg[(*i)++];
	return 0;
}

short DecodeODBData(unsigned char *msg, short *i, T_SUBSC_IE_ODB_DATA *odb_data)
{
	int temp;
	short ret = checkSubscInfoIEType(msg[(*i)++], SUBSC_INFO_IE_ODB_DATA);
	if(ret) return SUBSC_INFO_ERR_UNEXP_IE;

	temp = msg[(*i)++]; // reserved
	temp = msg[(*i)++]; // reserved
	temp = msg[(*i)++];
	odb_data->odb_general.multiple_ect_barred =
			(temp >> ODB_GEN_BIT_MULTI_ECT) & 0x01;
	odb_data->odb_general.double_chargeable_ect_barred =
			(temp >> ODB_GEN_BIT_DBL_CHARGE_ECT) & 0x01;
	odb_data->odb_general.intrz_ect_barred =
			(temp >> ODB_GEN_BIT_INTRZ_ECT) & 0x01;
	odb_data->odb_general.intrn_ect_barred =
			(temp >> ODB_GEN_BIT_INTRN_ECT) & 0x01;
	odb_data->odb_general.chargeable_ect_barred =
			(temp >> ODB_GEN_BIT_CHARGE_ECT) & 0x01;
	odb_data->odb_general.all_ect_barred =
			(temp >> ODB_GEN_BIT_ALL_ECT) & 0x01;
	odb_data->odb_general.intrz_intrn_og_not_hplmn_barred =
			(temp >> ODB_GEN_BIT_INTRZ_INTRN_NOT_HPLMN) & 0x01;

	temp = msg[(*i)++];
	odb_data->odb_general.intrz_og_not_hplmn_barred =
			(temp >> ODB_GEN_BIT_INTRZ_OG_NOT_HPLMN) & 0x01;
	odb_data->odb_general.intrz_og_barred =
			(temp >> ODB_GEN_BIT_INTRZ_OG) & 0x01;
	odb_data->odb_general.ss_access_barred =
			(temp >> ODB_GEN_BIT_SS_ACCESS) & 0x01;
	odb_data->odb_general.premium_enter_og_barred =
			(temp >> ODB_GEN_BIT_PREM_ENT) & 0x01;
	odb_data->odb_general.premium_info_og_barred =
			(temp >> ODB_GEN_BIT_PREM_INFO) & 0x01;
	odb_data->odb_general.intrn_og_not_hplmn_barred =
			(temp >> ODB_GEN_BIT_INTRN_OG_NOT_HPLMN) & 0x01;
	odb_data->odb_general.intrn_og_barred =
			(temp >> ODB_GEN_BIT_INTRN_OG) & 0x01;
	odb_data->odb_general.all_og_barred =
			(temp >> ODB_GEN_BIT_ALL_OG) & 0x01;

	temp = msg[(*i)++];	// reserved
	temp = msg[(*i)++]; // reserved
	temp = msg[(*i)++]; // reserved
	temp = msg[(*i)++];
	odb_data->odb_plmn.plmn_specific_type4 = (temp >> ODB_HPLMN_BIT_TYPE_4) & 0x01;
	odb_data->odb_plmn.plmn_specific_type3 = (temp >> ODB_HPLMN_BIT_TYPE_3) & 0x01;
	odb_data->odb_plmn.plmn_specific_type2 = (temp >> ODB_HPLMN_BIT_TYPE_2) & 0x01;
	odb_data->odb_plmn.plmn_specific_type1 = (temp >> ODB_HPLMN_BIT_TYPE_1) & 0x01;
	return 0;
}

short DecodeBasicServiceCode(unsigned char *msg, short *i,
							T_SUBSC_INFO_EXT_BASIC_SERVICE *basic_service)
{
	int j;
	basic_service->service_type = (T_SUBSC_INFO_BASIC_SERVICE_TYPE)msg[(*i)++];
	switch(basic_service->service_type){
		case BEARER_SERVICE_TYPE:
			for(j=0;j<5;j++) basic_service->ex_bs_code[j] = msg[(*i)++];
			break;
		case TELE_SERVICE_TYPE:
			for(j=0;j<5;j++) basic_service->ex_ts_code[j] = msg[(*i)++];
			break;
		default:
			printf("Unknown Basic Service Type :%d\n", basic_service->service_type);
			return SUBSC_INFO_ERR_UNKNOWN_BASIC;
	}
	return 0;
}

short DecodeSSStatus(unsigned char *msg, short *i,
					T_SS_STATUS_FLAG	*ss_status)
{
	int temp = msg[(*i)++];
	ss_status->ss_status_A = (temp >> SS_STATUS_A_BIT) & 0x01;
	ss_status->ss_status_R = (temp >> SS_STATUS_R_BIT) & 0x01;
	ss_status->ss_status_P = (temp >> SS_STATUS_P_BIT) & 0x01;
	ss_status->ss_status_Q = (temp >> SS_STATUS_Q_BIT) & 0x01;
	return 0;
}

short DecodeSSSubscOptionV210OrOlder(unsigned char *msg, short *i, T_SS_SUBSC_OPTION *ss_subsc_option) // 2.1.1
{
	ss_subsc_option->isPresent = true;
	ss_subsc_option->option_type = SS_SUBSC_OPTION_CLIR_OPTION;
	ss_subsc_option->clir_option = CLIR_PERMANENT;

	return 0;
}

short DecodeSSSubscOptionV211OrLater(unsigned char *msg, short *i, T_SS_SUBSC_OPTION *ss_subsc_option) // 2.1.1
{
	ss_subsc_option->isPresent = (msg[(*i)++]==0)? false : true;

	if (ss_subsc_option->isPresent==false) return 0;

	ss_subsc_option->option_type = (T_SS_SUBSC_OPTION_TYPE) msg[(*i)++];

	switch(ss_subsc_option->option_type)
	{
	case SS_SUBSC_OPTION_CLIR_OPTION:
		ss_subsc_option->clir_option = (T_SS_SUBSC_CLIR_OPTION) msg[(*i)++];
		break;

	case SS_SUBSC_OPTION_OVERRIDE_CATEGORY:
		ss_subsc_option->override_category = (T_SS_SUBSC_OVERRIDE_CATEGORY) msg[(*i)++];
		break;

	default:
		printf("Unsupported SS Subscription Option Type : %d\n", ss_subsc_option->option_type);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_SS_SUBSC_OPTION;
	}

	return 0;
}

short DecodeSSSubscOption(unsigned char *msg, short *i, T_SS_SUBSC_OPTION *ss_subsc_option, T_SUBSC_VERSION *v) // 2.1.1
{
	if (!isVerGE(*v,2,1,1))
	{
		return DecodeSSSubscOptionV210OrOlder(msg, i, ss_subsc_option);
	}

	return DecodeSSSubscOptionV211OrLater(msg, i, ss_subsc_option);
}

short DecodeForwOption(unsigned char *msg, short *i,
					T_SS_FORW_OPTION	*forw)
{
	int temp = msg[(*i)++];
	forw->notify_forw_party  	= (T_NOTIFY_FORW_FLAG)
								((temp >> SS_NOTIFY_FORW_FLAG_BIT) & 0x01);
	forw->notify_calling_party 	= (T_NOTIFY_CALLING_FLAG)
								((temp >> SS_NOTIFY_CALLING_FLAG_BIT) & 0x01);
	forw->forw_reason 			= (T_FORW_REASON_FLAG)
								((temp >> SS_FORW_REASON_FLAG_BIT) & 0x03);
	return 0;
}

short DecodeSSInfoForw(unsigned char *msg, short *i, T_SUBSC_IE_SS_INFO_FORW *forw, T_SUBSC_VERSION *v)
{
	int j, num, kludge;
	forw->ss_code = (T_SUBSC_INFO_SUPP_SERVICE_CODE)msg[(*i)++];
	forw->num_feature = num = msg[(*i)++];
	if(num > MAX_NUM_EXT_FEATURE){
		printf("Err. forw feature number too large : %d\n", num);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_FORW_NUM_FEATURE;
	}
	for(j=0; j<num; j++){
		DecodeBasicServiceCode(msg, i, &forw->list[j].basic_service);
		DecodeSSStatus(msg, i, &forw->list[j].ss_status);
		DecodeISDN(msg, i, &forw->list[j].forw_to_num, v);
		DecodeSubAddr(msg, i, &forw->list[j].forw_to_sub);
		DecodeForwOption(msg, i, &forw->list[j].forw_options);
		forw->list[j].no_reply_cond_time = kludge = msg[(*i)++];
	}
	return 0;
}

short DecodeSSInfoBar(unsigned char *msg, short *i, T_SUBSC_IE_SS_INFO_BAR *bar, T_SUBSC_VERSION *v)
{
	int j, num;
	bar->ss_code = (T_SUBSC_INFO_SUPP_SERVICE_CODE)msg[(*i)++];
        /* fix wrong code */
        if(bar->ss_code == 0x96) bar->ss_code = SUPP_SERV_BAIC;
        if(bar->ss_code == 0x97) bar->ss_code = SUPP_SERV_BIC_ROAM;
	bar->num_feature = num = msg[(*i)++];
	if(num > MAX_NUM_EXT_FEATURE){
		printf("Err. bar feature number too large : %d\n", num);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_BAR_NUM_FEATURE;
	}
	for(j=0; j<num; j++){
		DecodeBasicServiceCode(msg, i, &bar->list[j].basic_service);
		DecodeSSStatus(msg, i, &bar->list[j].ss_status);
	}
	if(!isVerGE(*v, 2, 0, 8)){
		bar->num_feature = 1;
		bar->list[0].basic_service.service_type = TELE_SERVICE_TYPE;
	}
	return 0;
}

short DecodeSSInfoData(unsigned char *msg, short *i, T_SUBSC_IE_SS_INFO_DATA *data, T_SUBSC_VERSION *v)		// 2.0.5
{
	data->ss_code = (T_SUBSC_INFO_SUPP_SERVICE_CODE)msg[(*i)++];
	DecodeSSStatus(msg, i, &data->ss_status);
	DecodeSSSubscOption(msg, i, &data->ss_subsc_option, v); // 2.1.1
	return 0;
}

short DecodeSSInfo(unsigned char *msg, short *i, T_SUBSC_IE_SS_INFO *ss_info, T_SUBSC_VERSION *v)
{
	short ret = 0;
	ss_info->info_type =  (T_SUBSC_IE_SS_INFO_TYPE)msg[(*i)++];
	switch(ss_info->info_type){
		case SUBSC_IE_SS_INFO_FORW_TYPE:
			DecodeSSInfoForw(msg, i, &ss_info->info_forw, v);
			break;
		case SUBSC_IE_SS_INFO_BARR_TYPE:
			DecodeSSInfoBar(msg, i, &ss_info->info_bar, v);
			break;
		case SUBSC_IE_SS_INFO_DATA_TYPE:							// 2.0.5
			DecodeSSInfoData(msg, i, &ss_info->info_data, v); // 2.1.1
			break;
		default:
			printf("Unsupported SS Info Type encountered :%d\n", ss_info->info_type);
			printf("msg index value: %d\n", *i);
			ret = SUBSC_INFO_ERR_SS_INFO_TYPE;
	}
	return ret;
}

short DecodeSSInfoList(unsigned char *msg, short *i, T_SUBSC_IE_SS_INFO_LIST *ss_info_list, T_SUBSC_VERSION *v)
{
	int j, num, ind;
	short ret = checkSubscInfoIEType(msg[(*i)++], SUBSC_INFO_IE_SS_INFO_LIST);
	if(ret) return SUBSC_INFO_ERR_UNEXP_IE;

//	num = ss_info_list->num_ss_info = msg[(*i)++];
	num = msg[(*i)++];
	if(num > MAX_NUM_SUPP_SERVICE){
		printf("Err. ss info list number too large : %d\n", num);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_SS_NUM_INFO;
	}
	for(j=0; j<num; j++){
		switch(msg[(*i)++]){
			case SUBSC_IE_SS_INFO_FORW_TYPE:
				ind = subscGetSSInfoForwInd((T_SUBSC_INFO_SUPP_SERVICE_CODE)msg[*i]);
				if(ind == -1) return SUBSC_INFO_ERR_SS_INFO_TYPE;
				DecodeSSInfoForw(msg, i, &ss_info_list->ss_info_forw[ind], v);
				break;
			case SUBSC_IE_SS_INFO_DATA_TYPE:
				ind = subscGetSSInfoDataInd((T_SUBSC_INFO_SUPP_SERVICE_CODE)msg[*i]);
				if(ind == -1) return SUBSC_INFO_ERR_SS_INFO_TYPE;
				DecodeSSInfoData(msg, i, &ss_info_list->ss_info_data[ind], v); // 2.1.1
				break;
			case SUBSC_IE_SS_INFO_BARR_TYPE:
				ind = subscGetSSInfoBarInd((T_SUBSC_INFO_SUPP_SERVICE_CODE)msg[*i]);
				if(ind == -1) return SUBSC_INFO_ERR_SS_INFO_TYPE;
				DecodeSSInfoBar(msg, i, &ss_info_list->ss_info_bar[ind], v);
				break;
		}
	}
	return 0;
}

short DecodeUserInfo(unsigned char *msg, short *i, T_SUBSC_IE_USER_INFO *user)
{
	int j, num;
	short ret = checkSubscInfoIEType(msg[(*i)++], SUBSC_INFO_IE_USER_INFO);
	if(ret) return SUBSC_INFO_ERR_UNEXP_IE;

	user->len = num = msg[(*i)++];
	if(num > MAX_NUM_USER_INFO){
		printf("Err. user info too large : %d\n", num);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_USER_INFO;
	}
	for(j=0; j<num; j++){
		user->name[j] = msg[(*i)++];
	}
	user->name[j] = '\0';	// null term
	return 0;
}

short DecodeGPRSData(unsigned char *msg, short *i, T_SUBSC_GPRS_DATA *data)
{
	int j, num;

	data->pdpContextId = msg[(*i)++];
	data->pdpType = (CNI_RIL3_PDP_ADDRESS_TYPE)msg[(*i)++];
	for(j=0; j<MAX_LEN_SUBSC_PDP_ADDR; j++)
		data->pdpAddress[j] = msg[(*i)++];
	data->qosSubscribed.delay_class = (CNI_RIL3_QOS_DELAY_CLASS)msg[(*i)++];
	data->qosSubscribed.reliability_class = (CNI_RIL3_QOS_RELIABILITY_CLASS)msg[(*i)++];
	data->qosSubscribed.precedence_class = (CNI_RIL3_QOS_PRECEDENCE_CLASS)msg[(*i)++];
	data->qosSubscribed.peak_throughput = (CNI_RIL3_QOS_PEAK_THROUGHPUT)msg[(*i)++];
	data->qosSubscribed.mean_throughput = (CNI_RIL3_QOS_MEAN_THROUGHPUT)msg[(*i)++];
	data->vplmnAddressAllowed = msg[(*i)++];
	data->apn_length = num = msg[(*i)++];
	if(num > MAX_LEN_SUBSC_APN){
		printf("Err. apn length too large : %d\n", num);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_APN_LEN;
	}
	for(j=0; j<num; j++){
		data->apn[j] = msg[(*i)++];
	}
	data->apn[j] = '\0';	// null term
	return 0;
}

short DecodeGPRSDataInfo(unsigned char *msg, short *i, T_SUBSC_IE_GPRS_DATA_INFO *gprs_data, T_SUBSC_VERSION *v)
{
	int j, num;
	short ret;

	if(!isVerGE(*v, 2, 1, 0)){
		gprs_data->num_data = 0;
		return 0;
	}

	ret = checkSubscInfoIEType(msg[(*i)++], SUBSC_INFO_IE_GPRS_DATA);
	num = gprs_data->num_data = msg[(*i)++];
	if(num > MAX_NUM_SUBSC_GPRS_DATA){
		printf("Err. gprs data too many : %d\n", num);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_GPRS_DATA_LEN;
	}
	for(j=0; j<num; j++){
		ret = DecodeGPRSData(msg, i, &gprs_data->data[j]);
		if(ret) return ret;
	}
	return 0;
}

// Decode SubscInfo from Message Buffer to the structure.
// size - size of message in msg buffer
bool dbgDecodeSubscInfoFlag = false;

extern "C" void dbgDecodeSubscInfoFlagToggle(void)
{
	dbgDecodeSubscInfoFlag = !dbgDecodeSubscInfoFlag;
	printf("[dbgDecodeSubscInfoFlagToggle] debugging is %s\n",
		(dbgDecodeSubscInfoFlag)? "enabled" : "disabled");
}

short	DecodeSubscInfo(unsigned char	*msg,
						short			size,
						T_SUBSC_INFO	*subsc)
{
	short i = 0;
	short ret;
	T_SUBSC_VERSION	v;

	memset(subsc, 0, sizeof(T_SUBSC_INFO)); // 2.1.2

	ret = DecodeVersionNumber(msg, &i, &v);
	if(ret == -1){
		printf("decoding older version subsc info\n");
		if(ret = DecodeMobileId1(msg, &i, &subsc->imsi)) return ret;
	}
	else{
		if(ret = DecodeMobileId2(msg, &i, &subsc->imsi)) return ret;
	}
	if(ret = DecodeISDN(msg, &i, &subsc->msisdn, &v)) return ret;
	if(ret = DecodeProvisionBS(msg, &i, &subsc->prov_bs)) return ret;
	if(ret = DecodeProvisionTS(msg, &i, &subsc->prov_ts)) return ret;
	subscSSInfoListClear(&subsc->ss_info_list);		// 2.0.6
	if(ret = DecodeProvisionSS(msg, &i, &subsc->prov_ss,
										&subsc->ss_info_list, &v)) return ret;
	if(ret = DecodeStatusFlag(msg, &i, &subsc->status)) return ret;
	if(ret = DecodeODBData(msg, &i, &subsc->odb_data)) return ret;
	if(ret = DecodeSSInfoList(msg, &i, &subsc->ss_info_list, &v)) return ret;
	if(ret = DecodeUserInfo(msg, &i, &subsc->user)) return ret;
	if(ret = DecodeGPRSDataInfo(msg, &i, &subsc->gprs_data, &v)) return ret;

	if (dbgDecodeSubscInfoFlag)
	{
		printf("[DecodeSubscInfo] \n");
		subscPrintInfo(subsc);
	}

	return 0;
}

short DecodeAuthTriplet(unsigned char *msg, short *i, T_AUTH_TRIPLET *triplet)
{
	int j;

	for(j=0;j<3;j++) triplet->RAND[j] = msg[(*i)++];
	for(j=0;j<2;j++) triplet->SRES[j] = msg[(*i)++];
	for(j=0;j<10;j++) triplet->Kc[j] = msg[(*i)++];
	return 0;
}

short DecodeAuthList(unsigned char *msg, short *i, T_SEC_IE_AUTH_LIST *auth_set)
{
	int j, num;
	short ret = checkSubscInfoIEType(msg[(*i)++], SUBSC_INFO_IE_AUTH_LIST);
	if(ret) return SUBSC_INFO_ERR_UNEXP_IE;

	auth_set->num_set = num = msg[(*i)++];
	if(num > SEC_MAX_TRIPLET){
		printf("Err. auth triplet list number too large : %d\n", num);
		printf("msg index value: %d\n", *i);
		return SUBSC_INFO_ERR_AUTH_NUM_LIST;
	}
	for(j=0; j<num; j++){
		ret = DecodeAuthTriplet(msg, i, &auth_set->triplet[j]);
		if(ret) return ret;
	}
	return 0;
}

// Decode SecInfo from Message Buffer to the structure.
// size - size of message in msg buffer
short	DecodeSecInfo(unsigned char		*msg,
						short			size,
						T_SEC_INFO		*sec)
{
	short i = 0;
	short ret;

	if((ret = DecodeMobileId2(msg, &i, &sec->imsi))) return ret;
	if((ret = DecodeAuthList(msg, &i, &sec->auth_set))) return ret;
	return 0;
}
