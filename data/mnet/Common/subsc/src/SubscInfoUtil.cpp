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
// File        : SubscInfoUtil.cpp
// Author(s)   : Kevin Lim
// Create Date : 06-16-99
// Description : subscriber information structure and en/decoding function
//               for messages
//
// *******************************************************************
#include "stdio.h"
#include "string.h"
#include "subsc/SubscInfoType.h"
#include "subsc/SubscInfoUtil.h"

//*****************************************************************************
T_SUBSC_IE_MOBILE_ID_IMSI* subscGetMobileIdPtr(T_SUBSC_INFO *info)
{
	if(info != 0) return(&info->imsi);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_ISDN*			subscGetMSISDNPtr(T_SUBSC_INFO *info)
{
	if(info != 0) return(&info->msisdn);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_PROVISION_BS*	subscGetProvBSPtr(T_SUBSC_INFO *info)
{
	if(info != 0) return(&info->prov_bs);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_PROVISION_TS*	subscGetProvTSPtr(T_SUBSC_INFO *info)
{
	if(info != 0) return(&info->prov_ts);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_PROVISION_SS*	subscGetProvSSPtr(T_SUBSC_INFO *info)
{
	if(info != 0) return(&info->prov_ss);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_SUBSC_STATUS*	subscGetSubscStatusPtr(T_SUBSC_INFO *info)
{
	if(info != 0) return(&info->status);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_ODB_DATA*		subscGetODBDataPtr(T_SUBSC_INFO *info)
{
	if(info != 0) return(&info->odb_data);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_SS_INFO_LIST*	subscGetSSInfoListPtr(T_SUBSC_INFO *info)
{
	if(info != 0) return(&info->ss_info_list);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_USER_INFO*	subscGetUserInfoPtr(T_SUBSC_INFO *info)
{
	if(info != 0) return(&info->user);
	else return 0;
}

//*****************************************************************************
T_SUBSC_IE_GPRS_DATA_INFO*	subscGetGPRSDataPtr(T_SUBSC_INFO *info)
{
	if(info != 0) return(&info->gprs_data);
	else return 0;
}

//*****************************************************************************
// [jihoon 07/13/01]
// 1. 'subscGetCLIROption' added
// 2. When success, it returns 0 and the CLIR option through 'option'.
// 3. When fails, it return non-zero.
// 4. Fail cases include null 'info', CLIR not provisioned, no index for SUPP_SERV_CLIR,
//    CLIR option not present, or wrong option type.
int	subscGetCLIROption(T_SUBSC_INFO *info, T_SS_SUBSC_CLIR_OPTION *option) // 2.1.1
{
	if (info == 0) return -1;

	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	if (subscGetP(info_list, SUPP_SERV_CLIR) == 0) return -2; // not provisioned

	int ind = subscGetSSInfoDataInd(SUPP_SERV_CLIR);
	if(ind == -1) return -3;
	T_SS_SUBSC_OPTION* subsc_option = &info_list->ss_info_data[ind].ss_subsc_option;

	if (subsc_option->isPresent==false) return -4;
	if (subsc_option->option_type!=SS_SUBSC_OPTION_CLIR_OPTION) return -5;

	*option = subsc_option->clir_option;

	return 0; // success
}

//*****************************************************************************
// [jihoon 07/13/01]
// 1. 'subscSetCLIROption' added
// 2. When success, it returns 0.
// 3. When fails, it return non-zero.
// 4. Fail cases include null 'info', no index for SUPP_SERV_CLIR, 'option' of wrong value.
int	subscSetCLIROption(T_SUBSC_INFO *info, T_SS_SUBSC_CLIR_OPTION option) // 2.1.1
{
	if (info == 0) return -1;

	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);

	int ind = subscGetSSInfoDataInd(SUPP_SERV_CLIR);
	if(ind == -1) return -2;
	T_SS_SUBSC_OPTION* subsc_option = &info_list->ss_info_data[ind].ss_subsc_option;

	switch(option)
	{
	case CLIR_PERMANENT:
	case CLIR_TEMP_DFT_RESTRICTED:
	case CLIR_TEMP_DFT_ALLOWED:
		subsc_option->isPresent = true;
		subsc_option->option_type = SS_SUBSC_OPTION_CLIR_OPTION;
		subsc_option->clir_option = option;
		break;

	default:
		return -3;
	}


	return 0; // success
}

//*****************************************************************************
//*****************************************************************************
/*
T_SUBSC_IE_SS_INFO*	subscGetSSInfoPtr(T_SUBSC_INFO *info,
													T_SUBSC_SS_INFO_INDEX ind)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	if(info_list == 0) return 0;
	return( &info_list->ss_info[ind]);
}
*/
//*****************************************************************************
bool	subscIsProvCLIP(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_CLIP) == 1);
}

//*****************************************************************************
bool	subscIsProvCLIR(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_CLIR) == 1);
}

//*****************************************************************************
bool	subscIsProvCOLP(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_COLP) == 1);
}

//*****************************************************************************
bool	subscIsProvCOLR(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_COLR) == 1);
}

//*****************************************************************************
bool	subscIsProvCFU(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_CFU) == 1);
}

//*****************************************************************************
bool	subscIsProvCFB(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_CFB) == 1);
}

//*****************************************************************************
bool	subscIsProvCFNRY(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_CFNRY) == 1);
}

//*****************************************************************************
bool	subscIsProvCFNRC(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_CFNRC) == 1);
}

//*****************************************************************************
bool	subscIsProvECT(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_ECT) == 1);
}

//*****************************************************************************
bool	subscIsProvCW(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_CW) == 1);
}

//*****************************************************************************
bool	subscIsProvHOLD(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_HOLD) == 1);
}

//*****************************************************************************
bool	subscIsProvMPTY(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_MPTY) == 1);
}

//*****************************************************************************
bool	subscIsProvBAOC(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_BAOC) == 1);
}

//*****************************************************************************
bool	subscIsProvBOIC(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_BOIC) == 1);
}

//*****************************************************************************
bool	subscIsProvBOICE(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_BOICE) == 1);
}

//*****************************************************************************
bool	subscIsProvBAIC(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_BAIC) == 1);
}

//*****************************************************************************
bool	subscIsProvBICRoam(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetP(info_list, SUPP_SERV_BIC_ROAM) == 1);
}

//*****************************************************************************
T_SUBSC_FORW_FEATURE*		subscGetForwFeature(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
										  T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code)
{
	if(ss_info_list == 0) return 0;
	int i, j;
	i = subscGetSSInfoForwInd(ss_code);
	int num_feature = ss_info_list->ss_info_forw[i].num_feature;
	for(j=0; j<num_feature; j++){
					// check for BS here
					// for now just one
			return(&ss_info_list->ss_info_forw[i].list[j]);
	}
	return 0;
}

//*****************************************************************************
bool	subscIsActiveCF(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
						T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code)
{
	return (subscGetA(ss_info_list, ss_code) == 1);
}

//*****************************************************************************
bool	subscIsActiveCW(T_SUBSC_IE_SS_INFO_LIST *ss_info_list)
{
	return (subscGetA(ss_info_list, SUPP_SERV_CW) == 1);
}

//*****************************************************************************
int		subscGetTimerCFNRY(T_SUBSC_IE_SS_INFO_LIST *ss_info_list)
{
	if(ss_info_list == 0) return -1;
	T_SUBSC_FORW_FEATURE*	feature = subscGetForwFeature(ss_info_list, SUPP_SERV_CFNRY);
	if(feature == 0) return -1;
	else return feature->no_reply_cond_time;
}

//*****************************************************************************
// print
//*****************************************************************************
void	subscPrintImsi(T_SUBSC_IE_MOBILE_ID_IMSI *imsi)
{
	if(imsi == 0) return;
	int i;
	for(i=0; i<imsi->num_digit; i++) printf("%c", imsi->imsi[i]);
}

//*****************************************************************************
void	subscPrintIsdn(T_SUBSC_IE_ISDN *msisdn)
{
	if(msisdn == 0) return;
	int i;
	int num_digit = (msisdn->num_digit > MAX_NUM_SUBSC_ISDN)?
			MAX_NUM_SUBSC_ISDN: msisdn->num_digit;
	for(i=0; i<num_digit; i++)
		printf("%c", msisdn->digits[i]);
}

//*****************************************************************************
void	subscPrintProvBS(T_SUBSC_IE_PROVISION_BS *prov_bs)
{
	if(prov_bs == 0) return;
	int i;
	int num_srv = (prov_bs->num_srv > MAX_NUM_BEARER_SERVICE)?
			MAX_NUM_BEARER_SERVICE: prov_bs->num_srv;
	for(i=0; i<num_srv; i++)
		printf("%s,", getSubscInfoBSCodeStr(prov_bs->bs_code[i]));
}

//*****************************************************************************
void	subscPrintProvTS(T_SUBSC_IE_PROVISION_TS *prov_ts)
{
	if(prov_ts == 0) return;
	int i;
	int num_srv = (prov_ts->num_srv > MAX_NUM_TELE_SERVICE)?
			MAX_NUM_TELE_SERVICE: prov_ts->num_srv;
	for(i=0; i<num_srv; i++)
		printf("%s,", getSubscInfoTSCodeStr(prov_ts->ts_code[i]));
}

//*****************************************************************************
void	subscPrintProvSS(T_SUBSC_IE_PROVISION_SS *prov_ss)
{
	if(prov_ss == 0) return;
	int i;
	int num_srv = (prov_ss->num_srv > MAX_NUM_SUPP_SERVICE)?
			MAX_NUM_SUPP_SERVICE: prov_ss->num_srv;
	for(i=0; i<num_srv; i++)
		printf("%s,", getSubscInfoSSCodeStr(prov_ss->ss_code[i]));
}

//*****************************************************************************
void	subscPrintSubscStatus(T_SUBSC_IE_SUBSC_STATUS *status)
{
	if(status == 0) return;
	printf("%d", status->status);
}

//*****************************************************************************
void	subscPrintODBData(T_SUBSC_IE_ODB_DATA *odb_data)
{
	if(odb_data == 0) return;
	printf(" ODB General: \n");
	printf("  all og barred                :%d\n", odb_data->odb_general.all_og_barred);
	printf("  intrn og barred              :%d\n", odb_data->odb_general.intrn_og_barred);
	printf("  intrn og not hplmn barred    :%d\n", odb_data->odb_general.intrn_og_not_hplmn_barred);
	printf("  premium info og barred       :%d\n", odb_data->odb_general.premium_info_og_barred);
	printf("  premium enter og barred      :%d\n", odb_data->odb_general.premium_enter_og_barred);
	printf("  ss access barred             :%d\n", odb_data->odb_general.ss_access_barred);
	printf("  intrz og barred              :%d\n", odb_data->odb_general.intrz_og_barred);
	printf("  intrz og not hplmn barred    :%d\n", odb_data->odb_general.intrz_og_not_hplmn_barred);
	printf("  intrz intrn og not hplmn barr:%d\n", odb_data->odb_general.intrz_intrn_og_not_hplmn_barred);
	printf("  all ect barred               :%d\n", odb_data->odb_general.all_ect_barred);
	printf("  chargeable ect barred        :%d\n", odb_data->odb_general.chargeable_ect_barred);
	printf("  intrn ect barred             :%d\n", odb_data->odb_general.intrn_ect_barred);
	printf("  intrz ect barred             :%d\n", odb_data->odb_general.intrz_ect_barred);
	printf("  double chargeable ect barred :%d\n", odb_data->odb_general.double_chargeable_ect_barred);
	printf("  multiple ect barred          :%d\n", odb_data->odb_general.multiple_ect_barred);
	printf(" ODB PLMN specific: \n");
	printf("  type 1               :%d\n", odb_data->odb_plmn.plmn_specific_type1);
	printf("  type 2               :%d\n", odb_data->odb_plmn.plmn_specific_type2);
	printf("  type 3               :%d\n", odb_data->odb_plmn.plmn_specific_type3);
	printf("  type 4               :%d\n", odb_data->odb_plmn.plmn_specific_type4);
}

//*****************************************************************************
void	subscPrintSSStatus(T_SS_STATUS_FLAG	*ss_status)
{
	printf("ss status - A: %d, R: %d, P: %d, Q: %d\n",  ss_status->ss_status_A,
														ss_status->ss_status_R,
														ss_status->ss_status_P,
														ss_status->ss_status_Q);
}

//*****************************************************************************
void	subscPrintSSSubscOption(T_SS_SUBSC_OPTION	*ss_subsc_option) // 2.1.1
{
	if (!ss_subsc_option->isPresent) return; // nothing to print

	printf(" SS SUBSC OPTION \n");
	switch(ss_subsc_option->option_type)
	{
	case SS_SUBSC_OPTION_CLIR_OPTION:
		printf("   CLIR Option             :%d\n",ss_subsc_option->clir_option);
		break;

	case SS_SUBSC_OPTION_OVERRIDE_CATEGORY:
		printf("   Override Category       :%d\n",ss_subsc_option->override_category);
		break;

	default:
		printf("   Unknown Option Type     :%d\n",ss_subsc_option->option_type);
	}
}

//*****************************************************************************
void	subscPrintSSInfoForw(T_SUBSC_IE_SS_INFO_FORW *info_forw)
{
	int j;
	printf(" SS FORW INFO: \n");
	printf("  ss code: %s\n", getSubscInfoSSCodeStr(info_forw->ss_code));
	int num_feature = (info_forw->num_feature > MAX_NUM_EXT_FEATURE)?
					MAX_NUM_EXT_FEATURE : info_forw->num_feature;
	for(j=0; j<num_feature; j++){
		printf("   feature[%d]\n", j);
		printf("    basic service - service type: %d\n", info_forw->list[j].basic_service.service_type);
		printf("    service code: 0x%02x\n", info_forw->list[j].basic_service.ex_bs_code[0]);
		printf("    ");
		subscPrintSSStatus(&info_forw->list[j].ss_status);
		printf("    forward to num: ");
		int num_digit = (info_forw->list[j].forw_to_num.num_digit > MAX_NUM_SUBSC_ISDN)?
						MAX_NUM_SUBSC_ISDN: info_forw->list[j].forw_to_num.num_digit;
		for(int k=0; k<num_digit; k++)
			printf("%c", info_forw->list[j].forw_to_num.digits[k]);
		printf("\n");
		printf("    forward option: \n");
		printf("     notify forwarding party: %d\n", info_forw->list[j].forw_options.notify_forw_party);
		printf("     notify calling party: %d\n",    info_forw->list[j].forw_options.notify_calling_party);
		printf("     forwarding reason: %d\n",       info_forw->list[j].forw_options.forw_reason);
		printf("    no reply condition time: %d\n",  info_forw->list[j].no_reply_cond_time);
	}
}

//*****************************************************************************
void	subscPrintSSInfoBar(T_SUBSC_IE_SS_INFO_BAR *info_bar)
{
	int j;
	printf(" SS BARR INFO: \n");
    printf("  ss code: %s\n", getSubscInfoSSCodeStr(info_bar->ss_code));
	int num_feature = (info_bar->num_feature > MAX_NUM_EXT_FEATURE)?
					MAX_NUM_EXT_FEATURE : info_bar->num_feature;
	for(j=0; j<num_feature; j++){
		printf("   feature[%d]\n", j);
		printf("    basic service - service type: %d\n", info_bar->list[j].basic_service.service_type);
		printf("    service code: 0x%02x\n", info_bar->list[j].basic_service.ex_bs_code[0]);
		printf("    ");
		subscPrintSSStatus(&info_bar->list[j].ss_status);
	}
}

//*****************************************************************************
void	subscPrintSSInfoData(T_SUBSC_IE_SS_INFO_DATA *info_data)
{
	printf(" SS DATA INFO: \n");
    printf("  ss code: %s\n", getSubscInfoSSCodeStr(info_data->ss_code));
	subscPrintSSStatus(&info_data->ss_status);
	subscPrintSSSubscOption(&info_data->ss_subsc_option); // 2.1.1
}

//*****************************************************************************
void	subscPrintSSInfoList(T_SUBSC_IE_SS_INFO_LIST *ss_info_list)
{
	if(ss_info_list == 0) return;
	int i;
	for(i=0; i<SS_INFO_IND_FORW_MAX; i++){
		printf("SS Forw Info[%d] \n", i);
		subscPrintSSInfoForw(&ss_info_list->ss_info_forw[i]);
	}
	for(i=0; i<SS_INFO_IND_BAR_MAX; i++){
		printf("SS Bar Info[%d] \n", i);
		subscPrintSSInfoBar(&ss_info_list->ss_info_bar[i]);
	}
	for(i=0; i<SS_INFO_IND_DATA_MAX; i++){
		printf("SS Data Info[%d] \n", i);
		subscPrintSSInfoData(&ss_info_list->ss_info_data[i]);
	}
}

//*****************************************************************************
void	subscPrintUserInfo(T_SUBSC_IE_USER_INFO *user)
{
	if(user == 0) return;
	for(int i=0; i<user->len; i++){
		printf("%c", user->name[i]);
	}
	printf("\n");
}

//*****************************************************************************
void subscPrintPDPType(CNI_RIL3_PDP_ADDRESS_TYPE value)
{
	switch(value){
		case CNI_RIL3_PDP_ADDRESS_TYPE_IETF_IPV4:
			printf("IETF_IPV4\n");
			break;
		case CNI_RIL3_PDP_ADDRESS_TYPE_IETF_IPV6:
			printf("IETF_IPV6\n");
			break;
		case CNI_RIL3_PDP_ADDRESS_TYPE_ETSI_X121:
			printf("ETSI_X121\n");
			break;
		case CNI_RIL3_PDP_ADDRESS_TYPE_ETSI_PPP:
			printf("ETSI_PPP\n");
			break;
		case CNI_RIL3_PDP_ADDRESS_TYPE_ETSI_OSP_IHOSS:
			printf("ETSI_OSP_IHOSS\n");
			break;
		default: printf("Unknown :%d\n", value);
	}
}

/*********************************************************************/
void subscPrintQOSDelay(CNI_RIL3_QOS_DELAY_CLASS value)
{
	switch(value){
		case CNI_RIL3_QOS_DELAY_CLASS_1:
			printf("CLASS_1\n");
			break;
		case CNI_RIL3_QOS_DELAY_CLASS_2:
			printf("CLASS_2\n");
			break;
		case CNI_RIL3_QOS_DELAY_CLASS_3:
			printf("CLASS_3\n");
			break;
		case CNI_RIL3_QOS_DELAY_CLASS_4:
			printf("CLASS_4\n");
			break;
		default: printf("Unknown :%d\n", value);
	}
}

/*********************************************************************/
void subscPrintQOSRely(CNI_RIL3_QOS_RELIABILITY_CLASS value)
{
	switch(value){
		case CNI_RIL3_QOS_RELIABILITY_CLASS_ACK_GTP_LLC_RLC_PROTECTED_DATA:
			printf("ACK_GTP_LLC_RLC_PROTECTED_DATA\n");
			break;
		case CNI_RIL3_QOS_RELIABILITY_CLASS_UNACK_GTP_ACK_LLC_RLC_PROTECTED_DATA:
			printf("UNACK_GTP_ACK_LLC_RLC_PROTECTED_DATA\n");
			break;
		case CNI_RIL3_QOS_RELIABILITY_CLASS_UNACK_GTP_LLC_ACK_RLC_PROTECTED_DATA:
			printf("UNACK_GTP_LLC_ACK_RLC_PROTECTED_DATA\n");
			break;
		case CNI_RIL3_QOS_RELIABILITY_CLASS_UNACK_GTP_LLC_RLC_PROTECTED_DATA:
			printf("UNACK_GTP_LLC_RLC_ACK_PROTECTED_DATA\n");
			break;
		case CNI_RIL3_QOS_RELIABILITY_CLASS_UNACK_GTP_LLC_RLC_UNPROTECTED_DATA:
			printf("UNACK_GTP_LLC_RLC_UNPROTECTED_DATA\n");
			break;
		default: printf("Unknown :%d\n", value);
	}
}

/*********************************************************************/
void subscPrintQOSPrec(CNI_RIL3_QOS_PRECEDENCE_CLASS value)
{
	switch(value){
		case CNI_RIL3_QOS_PRECEDENCE_CLASS_HIGH_PRIORITY:
			printf("HIGH\n");
			break;
		case CNI_RIL3_QOS_PRECEDENCE_CLASS_NORMAL_PRIORITY:
			printf("NORMAL\n");
			break;
		case CNI_RIL3_QOS_PRECEDENCE_CLASS_LOW_PRIORITY:
			printf("LOW\n");
			break;
		default: printf("Unknown :%d\n", value);
	}
}

/*********************************************************************/
void subscPrintQOSPeak(CNI_RIL3_QOS_PEAK_THROUGHPUT value)
{
	switch(value){
		case CNI_RIL3_QOS_PEAK_THROUGHPUT_1_KBPS:
			printf("PEAK 1KBPS\n");
			break;
		case CNI_RIL3_QOS_PEAK_THROUGHPUT_2_KBPS:
			printf("PEAK 2KBPS\n");
			break;
		case CNI_RIL3_QOS_PEAK_THROUGHPUT_4_KBPS:
			printf("PEAK 4KBPS\n");
			break;
		case CNI_RIL3_QOS_PEAK_THROUGHPUT_8_KBPS:
			printf("PEAK 8KBPS\n");
			break;
		case CNI_RIL3_QOS_PEAK_THROUGHPUT_16_KBPS:
			printf("PEAK 16KBPS\n");
			break;
		case CNI_RIL3_QOS_PEAK_THROUGHPUT_32_KBPS:
			printf("PEAK 32KBPS\n");
			break;
		case CNI_RIL3_QOS_PEAK_THROUGHPUT_64_KBPS:
			printf("PEAK 64KBPS\n");
			break;
		case CNI_RIL3_QOS_PEAK_THROUGHPUT_128_KBPS:
			printf("PEAK 128KBPS\n");
			break;
		case CNI_RIL3_QOS_PEAK_THROUGHPUT_256_KBPS:
			printf("PEAK 256KBPS\n");
			break;
		default: printf("Unknown :%d\n", value);
	}
}

/*********************************************************************/
void subscPrintQOSMean(CNI_RIL3_QOS_MEAN_THROUGHPUT value)
{
	switch(value){
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_100_OPH:
			printf("MEAN 100 OPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_200_OPH:
			printf("MEAN 200 OPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_500_OPH:
			printf("MEAN 500 OPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_1_KOPH:
			printf("MEAN 1 KOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_2_KOPH:
			printf("MEAN 2 KOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_5_KOPH:
			printf("MEAN 5 KOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_10_KOPH:
			printf("MEAN 10 KOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_20_KOPH:
			printf("MEAN 20 KOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_50_KOPH:
			printf("MEAN 50 KOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_100_KOPH:
			printf("MEAN 100 KOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_200_KOPH:
			printf("MEAN 200 KOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_500_KOPH:
			printf("MEAN 500 KOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_1_MOPH:
			printf("MEAN 1 MOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_2_MOPH:
			printf("MEAN 2 MOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_5_MOPH:
			printf("MEAN 5 MOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_10_MOPH:
			printf("MEAN 10 MOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_20_MOPH:
			printf("MEAN 20 MOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_50_MOPH:
			printf("MEAN 50 MOPH\n");
			break;
		case CNI_RIL3_QOS_MEAN_THROUGHPUT_BEST_EFFORT:
			printf("MEAN BEST EFFORT\n");
			break;
		default: printf("Unknown :%d\n", value);
	}
}

//*****************************************************************************
void subscPrintQoS(T_SUBSC_QOS_DATA *ie)
{
	printf("   delay_class      : "); subscPrintQOSDelay(ie->delay_class);
	printf("   reliability_class: "); subscPrintQOSRely(ie->reliability_class);
	printf("   precedence_class : "); subscPrintQOSPrec(ie->precedence_class);
	printf("   peak_throughput  : "); subscPrintQOSPeak(ie->peak_throughput);
	printf("   mean_throughput  : "); subscPrintQOSMean(ie->mean_throughput);
}

//*****************************************************************************
void	subscPrintPDPData(T_SUBSC_GPRS_DATA *data)
{
	int i;
	if(data == 0) return;
	printf("  pdp context id: %d\n", data->pdpContextId);
	printf("  pdp type: ");
	subscPrintPDPType(data->pdpType);
	printf("  pdp address: ");
	for(i=0; i<MAX_LEN_SUBSC_PDP_ADDR; i++) printf("%02x ", data->pdpAddress[i]);
	printf("\n");
	printf("  qos data: \n");
	subscPrintQoS(&data->qosSubscribed);
	printf("  vplmn address allowed: %d\n", data->vplmnAddressAllowed);
	printf("  apn [%d] : ", data->apn_length);
	for(i=0; i<data->apn_length; i++) printf("%c", data->apn[i]);
	printf("\n");
}

//*****************************************************************************
void	subscPrintGPRSDataPtr(T_SUBSC_IE_GPRS_DATA_INFO *gprs_data)
{
	if(gprs_data == 0) return;
	if(gprs_data->num_data >= MAX_NUM_SUBSC_GPRS_DATA) return;
	int i;
	for(i=0; i<gprs_data->num_data; i++){
		printf(" PDP Data[%d] \n", i);
		subscPrintPDPData(&gprs_data->data[i]);
	}
}

//*****************************************************************************
void subscPrintInfo(T_SUBSC_INFO *info)
{
	printf("IMSI: ");
	subscPrintImsi(subscGetMobileIdPtr(info));
	printf("\n");
	printf("ISDN: ");
	subscPrintIsdn(subscGetMSISDNPtr(info));
	printf("\n");
	printf("Provision Bearer Service: ");
	subscPrintProvBS(subscGetProvBSPtr(info));
	printf("\n");
	printf("Provision Teleservice: ");
	subscPrintProvTS(subscGetProvTSPtr(info));
	printf("\n");
	printf("Provision Supplementary Service: ");
	subscPrintProvSS(subscGetProvSSPtr(info));
	printf("\n");
	printf("Subscriber Status: ");
	subscPrintSubscStatus(subscGetSubscStatusPtr(info));
	printf("\n");
	printf("ODB Data\n");
	subscPrintODBData(subscGetODBDataPtr(info));
	subscPrintSSInfoList(subscGetSSInfoListPtr(info));
	printf("UserInfo :");
	subscPrintUserInfo(subscGetUserInfoPtr(info));
	printf("GPRS :\n");
	subscPrintGPRSDataPtr(subscGetGPRSDataPtr(info));
}

//*****************************************************************************
void subscPrintGPRSInfo(T_SUBSC_INFO *info)
{
	printf("IMSI: ");
	subscPrintImsi(subscGetMobileIdPtr(info));
	printf("\n");
	printf("GPRS :\n");
	subscPrintGPRSDataPtr(subscGetGPRSDataPtr(info));
}

//*****************************************************************************
char *subscGetStringImsi(T_SUBSC_IE_MOBILE_ID_IMSI *imsi)
{
	if(imsi == 0) return 0;
	return (char *)imsi->imsi;
}

char *subscGetStringIsdn(T_SUBSC_IE_ISDN *isdn)
{
	if(isdn == 0) return 0;
	return (char *)isdn->digits;
}

char *subscGetStringUser(T_SUBSC_IE_USER_INFO *user)
{
	if(user == 0) return "Unknown";
	return (char *)user->name;
}

char *subscGetCFNum(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
							T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code)
{
	if(ss_info_list == 0) return "Unknown";
	T_SUBSC_FORW_FEATURE*	feature = subscGetForwFeature(ss_info_list, ss_code);
	if(feature == 0) return "Unknown";
	else return((char *)feature->forw_to_num.digits);
}

T_SUBSC_IE_ISDN*	subscGetCFNumPtr(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
							T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code)
{
	if(ss_info_list == 0) return 0;
	T_SUBSC_FORW_FEATURE*	feature = subscGetForwFeature(ss_info_list, ss_code);
	if(feature == 0) return 0;
	else return(&feature->forw_to_num);
}

void	subscSetIMSI(T_SUBSC_INFO *subsc, char *buf)
{
	T_SUBSC_IE_MOBILE_ID_IMSI*	imsi = subscGetMobileIdPtr(subsc);
	int len = strlen(buf);
	if(len > MAX_NUM_SUBSC_IMSI) imsi->num_digit = len = MAX_NUM_SUBSC_IMSI;
	else imsi->num_digit = len;
	strncpy((char *)imsi->imsi, buf, len);
	imsi->imsi[len] = '\0';
}

void  subscSetMSISDN(T_SUBSC_INFO *subsc, char *buf)
{
	T_SUBSC_IE_ISDN* isdn = subscGetMSISDNPtr(subsc);
	int len = strlen(buf);
	if(len > MAX_NUM_SUBSC_ISDN) isdn->num_digit = len = MAX_NUM_SUBSC_ISDN;
	else isdn->num_digit = len;
	strncpy((char *)isdn->digits, buf, len);
	isdn->digits[len] = '\0';
}

void  subscSetUserInfo(T_SUBSC_INFO *subsc, char *buf)
{
	T_SUBSC_IE_USER_INFO* user = subscGetUserInfoPtr(subsc);
	int len = strlen(buf);
	if(len > MAX_NUM_USER_INFO) user->len = len = MAX_NUM_USER_INFO;
	else user->len = len;
	strncpy((char *)user->name, buf, len);
	user->name[len] = '\0';
}

void	subscSetACF(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
						T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
						int active)
{
	if(ss_info_list == 0) return;
	T_SUBSC_FORW_FEATURE*	feature = subscGetForwFeature(ss_info_list, ss_code);
	if(feature == 0) return;
	else feature->ss_status.ss_status_A = (unsigned char)(active & 0x01);
}

void	subscSetPCF(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
						T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
						int prov)
{
	if(ss_info_list == 0) return;
	T_SUBSC_FORW_FEATURE*	feature = subscGetForwFeature(ss_info_list, ss_code);
	if(feature == 0) return;
	else feature->ss_status.ss_status_P = (unsigned char)(prov & 0x01);
}

void	subscSetRCF(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
						T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
						int reg)
{
	if(ss_info_list == 0) return;
	T_SUBSC_FORW_FEATURE*	feature = subscGetForwFeature(ss_info_list, ss_code);
	if(feature == 0) return;
	else feature->ss_status.ss_status_R = (unsigned char)(reg & 0x01);
}

void	subscSetCFNum(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
						T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
						char *num)
{
	T_SUBSC_IE_ISDN*	forw_num = subscGetCFNumPtr(ss_info_list, ss_code);
	if(forw_num != 0){
		forw_num->num_digit = strlen(num);
		strcpy((char *)forw_num->digits, num);
		forw_num->num_plan[0] = 0x91;	// Set All Call Forwarding Number Type as International
	}
}

void	subscSetTimerCFNRY(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
						int sec)
{
	if(ss_info_list == 0) return;
	T_SUBSC_FORW_FEATURE*	feature = subscGetForwFeature(ss_info_list, SUPP_SERV_CFNRY);
	if(feature == 0) return;
	else feature->no_reply_cond_time = sec;
}

T_SS_STATUS_FLAG* subscGetStatusFlagPtr(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
													T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code)
{
	int ind;
	if(ss_info_list == 0) return 0;
	switch(ss_code){
		case SUPP_SERV_CLIP	:
		case SUPP_SERV_CLIR	:
		case SUPP_SERV_COLP	:
		case SUPP_SERV_COLR	:
		case SUPP_SERV_ECT	:
		case SUPP_SERV_ALL_CC:
		case SUPP_SERV_CW		:
		case SUPP_SERV_HOLD	:
		case SUPP_SERV_ALL_MULT:
		case SUPP_SERV_MPTY:
				ind = subscGetSSInfoDataInd(ss_code);
				if(ind == -1) return 0;
				return &ss_info_list->ss_info_data[ind].ss_status;
		case SUPP_SERV_CFU	:
		case SUPP_SERV_CFB	:
		case SUPP_SERV_CFNRY	:
		case SUPP_SERV_CFNRC	:
				ind = subscGetSSInfoForwInd(ss_code);
				if(ind == -1) return 0;
				return &ss_info_list->ss_info_forw[ind].list[0].ss_status;
		case SUPP_SERV_BAOC	:
		case SUPP_SERV_BOIC	:
		case SUPP_SERV_BOICE	:
		case SUPP_SERV_BAIC	:
		case SUPP_SERV_BIC_ROAM:
				ind = subscGetSSInfoBarInd(ss_code);
				if(ind == -1) return 0;
				return &ss_info_list->ss_info_bar[ind].list[0].ss_status;
		default:
				return 0;
	}
}

int	subscGetP(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code)
{
	T_SS_STATUS_FLAG *ss_status = subscGetStatusFlagPtr(ss_info_list, ss_code);
	if(ss_status == 0) return 0;
	return ss_status->ss_status_P;
}

int  subscGetA(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code)
{
	T_SS_STATUS_FLAG *ss_status = subscGetStatusFlagPtr(ss_info_list, ss_code);
	if(ss_status == 0) return 0;
	return ss_status->ss_status_A;
}

int  subscGetR(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code)
{
	T_SS_STATUS_FLAG *ss_status = subscGetStatusFlagPtr(ss_info_list, ss_code);
	if(ss_status == 0) return 0;
	return ss_status->ss_status_R;
}

int  subscGetQ(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code)
{
	T_SS_STATUS_FLAG *ss_status = subscGetStatusFlagPtr(ss_info_list, ss_code);
	if(ss_status == 0) return 0;
	return ss_status->ss_status_Q;
}

void  subscSetP(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
					int prov)
{
	T_SS_STATUS_FLAG *ss_status = subscGetStatusFlagPtr(ss_info_list, ss_code);
	if(ss_status == 0) return;
	ss_status->ss_status_P = prov;
}

void  subscSetA(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
					int active)
{
	T_SS_STATUS_FLAG *ss_status = subscGetStatusFlagPtr(ss_info_list, ss_code);
	if(ss_status == 0) return;
	ss_status->ss_status_A = active;
}

void  subscSetR(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
					int registered)
{
	T_SS_STATUS_FLAG *ss_status = subscGetStatusFlagPtr(ss_info_list, ss_code);
	if(ss_status == 0) return;
	ss_status->ss_status_R = registered;
}

void  subscSetQ(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
					int quiescent)
{
	T_SS_STATUS_FLAG *ss_status = subscGetStatusFlagPtr(ss_info_list, ss_code);
	if(ss_status == 0) return;
	ss_status->ss_status_Q = quiescent;
}


int subscGetSSInfoInd(T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code)				// 2.0.6
{
	switch (ss_code){
		case SUPP_SERV_CFU	:
		case SUPP_SERV_CFB	:
		case SUPP_SERV_CFNRY	:
		case SUPP_SERV_CFNRC	:
			return subscGetSSInfoForwInd(ss_code);
		case SUPP_SERV_CLIP	:
		case SUPP_SERV_CLIR	:
		case SUPP_SERV_COLP	:
		case SUPP_SERV_COLR	:
		case SUPP_SERV_ECT	:
		case SUPP_SERV_CW		:
		case SUPP_SERV_HOLD	:
		case SUPP_SERV_MPTY	:
			return subscGetSSInfoDataInd(ss_code);
		case SUPP_SERV_BAOC	:
		case SUPP_SERV_BOIC	:
		case SUPP_SERV_BOICE	:
		case SUPP_SERV_BAIC	:
		case SUPP_SERV_BIC_ROAM:
			return subscGetSSInfoBarInd(ss_code);
		default: return -1;
	}
}

int subscGetSSInfoForwInd(T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code)				// 2.0.7
{
	switch (ss_code){
		case SUPP_SERV_CFU	:	return(SS_INFO_IND_FORW_CFU);
		case SUPP_SERV_CFB	:	return(SS_INFO_IND_FORW_CFB);
		case SUPP_SERV_CFNRY	:	return(SS_INFO_IND_FORW_CFNRY);
		case SUPP_SERV_CFNRC	:	return(SS_INFO_IND_FORW_CFNRC);
		default: return -1;
	}
}

int subscGetSSInfoDataInd(T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code)				// 2.0.7
{
	switch (ss_code){
		case SUPP_SERV_CLIP	:	return(SS_INFO_IND_DATA_CLIP);
		case SUPP_SERV_CLIR	:	return(SS_INFO_IND_DATA_CLIR);
		case SUPP_SERV_COLP	:	return(SS_INFO_IND_DATA_COLP);
		case SUPP_SERV_COLR	:	return(SS_INFO_IND_DATA_COLR);
		case SUPP_SERV_ECT	:	return(SS_INFO_IND_DATA_ECT);
		case SUPP_SERV_CW		:	return(SS_INFO_IND_DATA_CW);
		case SUPP_SERV_HOLD	:	return(SS_INFO_IND_DATA_HOLD);
		case SUPP_SERV_MPTY	:	return(SS_INFO_IND_DATA_MPTY);
		default: return -1;
	}
}

int subscGetSSInfoBarInd(T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code)				// 2.0.7
{
	switch (ss_code){
		case SUPP_SERV_BAOC	:	return(SS_INFO_IND_BAR_BAOC);
		case SUPP_SERV_BOIC	:	return(SS_INFO_IND_BAR_BOIC);
		case SUPP_SERV_BOICE	:	return(SS_INFO_IND_BAR_BOICE);
		case 0x96: 			// wrong value, but for older user profile
		case SUPP_SERV_BAIC	:	return(SS_INFO_IND_BAR_BAIC);
		case 0x97: 			// wrong value, but for older user profile
		case SUPP_SERV_BIC_ROAM:	return(SS_INFO_IND_BAR_BIC_ROAM);
		default: return -1;
	}
}

void subscSSInfoListClear(T_SUBSC_IE_SS_INFO_LIST *info_list)						// 2.0.6
{
	if(info_list == 0) return;

	memset(info_list, 0, sizeof(T_SUBSC_IE_SS_INFO_LIST));

	info_list->num_ss_info = SS_INFO_IND_DATA_MAX + SS_INFO_IND_FORW_MAX + SS_INFO_IND_BAR_MAX;

	info_list->ss_info_data[SS_INFO_IND_DATA_CLIP ].ss_code = SUPP_SERV_CLIP;
	info_list->ss_info_data[SS_INFO_IND_DATA_CLIR ].ss_code = SUPP_SERV_CLIR;
	info_list->ss_info_data[SS_INFO_IND_DATA_COLP ].ss_code = SUPP_SERV_COLP;
	info_list->ss_info_data[SS_INFO_IND_DATA_COLR ].ss_code = SUPP_SERV_COLR;
	info_list->ss_info_data[SS_INFO_IND_DATA_ECT  ].ss_code = SUPP_SERV_ECT;
	info_list->ss_info_data[SS_INFO_IND_DATA_CW   ].ss_code = SUPP_SERV_CW;
	info_list->ss_info_data[SS_INFO_IND_DATA_HOLD ].ss_code = SUPP_SERV_HOLD;
	info_list->ss_info_data[SS_INFO_IND_DATA_MPTY ].ss_code = SUPP_SERV_MPTY;

	info_list->ss_info_forw[SS_INFO_IND_FORW_CFU  ].ss_code = SUPP_SERV_CFU;
	info_list->ss_info_forw[SS_INFO_IND_FORW_CFU  ].num_feature = 1;
	info_list->ss_info_forw[SS_INFO_IND_FORW_CFU  ].list[0].basic_service.service_type = TELE_SERVICE_TYPE;
	info_list->ss_info_forw[SS_INFO_IND_FORW_CFB  ].ss_code = SUPP_SERV_CFB;
	info_list->ss_info_forw[SS_INFO_IND_FORW_CFB  ].num_feature = 1;
	info_list->ss_info_forw[SS_INFO_IND_FORW_CFB  ].list[0].basic_service.service_type = TELE_SERVICE_TYPE;
	info_list->ss_info_forw[SS_INFO_IND_FORW_CFNRY].ss_code = SUPP_SERV_CFNRY;
	info_list->ss_info_forw[SS_INFO_IND_FORW_CFNRY].num_feature = 1;
	info_list->ss_info_forw[SS_INFO_IND_FORW_CFNRY].list[0].basic_service.service_type = TELE_SERVICE_TYPE;
	info_list->ss_info_forw[SS_INFO_IND_FORW_CFNRC].ss_code = SUPP_SERV_CFNRC;
	info_list->ss_info_forw[SS_INFO_IND_FORW_CFNRC].num_feature = 1;
	info_list->ss_info_forw[SS_INFO_IND_FORW_CFNRC].list[0].basic_service.service_type = TELE_SERVICE_TYPE;

	info_list->ss_info_bar[SS_INFO_IND_BAR_BAOC ].ss_code = SUPP_SERV_BAOC;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BAOC ].num_feature = 1;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BAOC ].list[0].basic_service.service_type = TELE_SERVICE_TYPE;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BOIC ].ss_code = SUPP_SERV_BOIC;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BOIC ].num_feature = 1;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BOIC ].list[0].basic_service.service_type = TELE_SERVICE_TYPE;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BOICE].ss_code = SUPP_SERV_BOICE;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BOICE ].num_feature = 1;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BOICE ].list[0].basic_service.service_type = TELE_SERVICE_TYPE;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BAIC ].ss_code = SUPP_SERV_BAIC;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BAIC ].num_feature = 1;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BAIC ].list[0].basic_service.service_type = TELE_SERVICE_TYPE;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BIC_ROAM].ss_code = SUPP_SERV_BIC_ROAM;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BIC_ROAM ].num_feature = 1;
	info_list->ss_info_bar[SS_INFO_IND_BAR_BIC_ROAM ].list[0].basic_service.service_type = TELE_SERVICE_TYPE;
}

void subscInfoInitialize(T_SUBSC_INFO *info)									// 2.0.6
{
	memset(info, 0, sizeof(T_SUBSC_INFO));

	info->prov_bs.num_srv = 2;
	info->prov_bs.bs_code[0] = DATA_CDA_9600BPS;
	info->prov_bs.bs_code[1] = DATA_CDA_14400BPS;
	info->prov_ts.num_srv = 2;
	info->prov_ts.ts_code[0] = TELEPHONY_SERVICES;
	info->prov_ts.ts_code[1] = EMERGENCY_CALLS;
//	info->prov_ts.ts_code[2] = SMS_MT_PP;
//	info->prov_ts.ts_code[3] = SMS_MO_PP;
//	info->prov_ts.ts_code[4] = FACS_GRP3_ALT_SPEECH;

	subscSSInfoListClear(subscGetSSInfoListPtr(info));
}

int subscIsODBarred							(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_SUBSC_STATUS*	status_ptr = subscGetSubscStatusPtr(info);
	if(status_ptr != 0){
		return(status_ptr->status);
	}
	else
		return 0;
}

int subscIsODBAllOg							(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.all_og_barred);
	}
	else
		return 0;
}

int subscIsODBIntrnOg						(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.intrn_og_barred);
	}
	else
		return 0;
}

int subscIsODBIntrnOgNotHplmn			(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.intrn_og_not_hplmn_barred);
	}
	else
		return 0;
}

int subscIsODBPremiumInfoOg				(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.premium_info_og_barred);
	}
	else
		return 0;
}

int subscIsODBPremiumEnterOg				(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.premium_enter_og_barred);
	}
	else
		return 0;
}

int subscIsODBSsAccess						(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.ss_access_barred);
	}
	else
		return 0;
}

int subscIsODBIntrzOg						(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.intrz_og_barred);
	}
	else
		return 0;
}

int subscIsODBIntrzOgNotHplmn			(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.intrz_og_not_hplmn_barred);
	}
	else
		return 0;
}

int subscIsODBIntrzIntrnOgNotHplmn		(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.intrz_intrn_og_not_hplmn_barred);
	}
	else
		return 0;
}

int subscIsODBAllEct						(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.all_ect_barred);
	}
	else
		return 0;
}

int subscIsODBChargeableEct				(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.chargeable_ect_barred);
	}
	else
		return 0;
}

int subscIsODBIntrnEct						(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.intrn_ect_barred);
	}
	else
		return 0;
}

int subscIsODBIntrzEct						(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.intrz_ect_barred);
	}
	else
		return 0;
}

int subscIsODBDoubleChargeableEct		(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.double_chargeable_ect_barred);
	}
	else
		return 0;
}

int subscIsODBMultipleEct					(T_SUBSC_INFO *info)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		return(odb_ptr->odb_general.multiple_ect_barred);
	}
	else
		return 0;
}


void subscSetODB (T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_SUBSC_STATUS*	status_ptr = subscGetSubscStatusPtr(info);
	if(status_ptr != 0){
		if(flag){
			status_ptr->status = SUBSC_STATUS_ODBARRING;
		}
		else{
			status_ptr->status = SUBSC_STATUS_SERVICE_GRANTED;
		}
	}
}

void subscSetODBAllOg (T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.all_og_barred = 1;
		}
		else{
			odb_ptr->odb_general.all_og_barred = 0;
		}
	}
}

void subscSetODBIntrnOg	(T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.intrn_og_barred = 1;
		}
		else{
			odb_ptr->odb_general.intrn_og_barred = 0;
		}
	}
}

void subscSetODBIntrnOgNotHplmn (T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.intrn_og_not_hplmn_barred = 1;
		}
		else{
			odb_ptr->odb_general.intrn_og_not_hplmn_barred = 0;
		}
	}
}

void subscSetODBPremiumInfoOg	(T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.premium_info_og_barred = 1;
		}
		else{
			odb_ptr->odb_general.premium_info_og_barred = 0;
		}
	}
}

void subscSetODBPremiumEnterOg (T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.premium_enter_og_barred = 1;
		}
		else{
			odb_ptr->odb_general.premium_enter_og_barred = 0;
		}
	}
}

void subscSetODBSsAccess (T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.ss_access_barred = 1;
		}
		else{
			odb_ptr->odb_general.ss_access_barred = 0;
		}
	}
}

void subscSetODBIntrzOg	(T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.intrz_og_barred = 1;
		}
		else{
			odb_ptr->odb_general.intrz_og_barred = 0;
		}
	}
}

void subscSetODBIntrzOgNotHplmn (T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.intrz_og_not_hplmn_barred = 1;
		}
		else{
			odb_ptr->odb_general.intrz_og_not_hplmn_barred = 0;
		}
	}
}

void subscSetODBIntrzIntrnOgNotHplmn (T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.intrz_intrn_og_not_hplmn_barred = 1;
		}
		else{
			odb_ptr->odb_general.intrz_intrn_og_not_hplmn_barred = 0;
		}
	}
}

void subscSetODBAllEct (T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.all_ect_barred = 1;
		}
		else{
			odb_ptr->odb_general.all_ect_barred = 0;
		}
	}
}

void subscSetODBChargeableEct (T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.chargeable_ect_barred = 1;
		}
		else{
			odb_ptr->odb_general.chargeable_ect_barred = 0;
		}
	}
}

void subscSetODBIntrnEct (T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.intrn_ect_barred = 1;
		}
		else{
			odb_ptr->odb_general.intrn_ect_barred = 0;
		}
	}
}

void subscSetODBIntrzEct (T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.intrz_ect_barred = 1;
		}
		else{
			odb_ptr->odb_general.intrz_ect_barred = 0;
		}
	}
}

void subscSetODBDoubleChargeableEct	(T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.double_chargeable_ect_barred = 1;
		}
		else{
			odb_ptr->odb_general.double_chargeable_ect_barred = 0;
		}
	}
}

void subscSetODBMultipleEct (T_SUBSC_INFO *info, int flag)
{
	T_SUBSC_IE_ODB_DATA*	odb_ptr = subscGetODBDataPtr(info);
	if(odb_ptr != 0){
		if(flag){
			odb_ptr->odb_general.multiple_ect_barred = 1;
		}
		else{
			odb_ptr->odb_general.multiple_ect_barred = 0;
		}
	}
}

int subscIsSSBaoc						(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetA(info_list, SUPP_SERV_BAOC) == 1);
}

int subscIsSSBoic						(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetA(info_list, SUPP_SERV_BOIC) == 1);
}

int subscIsSSBoicExHC					(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetA(info_list, SUPP_SERV_BOICE) == 1);
}

int subscIsSSBaic						(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetA(info_list, SUPP_SERV_BAIC) == 1);
}

int subscIsSSBicRoam					(T_SUBSC_INFO *info)
{
	if(info == 0) return 0;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	return (subscGetA(info_list, SUPP_SERV_BIC_ROAM) == 1);
}

void subscSetSSBaoc						(T_SUBSC_INFO *info, int flag)
{
	if(info == 0) return;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	if(info_list == 0) return;
	subscSetA(info_list, SUPP_SERV_BAOC, flag);
}

void subscSetSSBoic						(T_SUBSC_INFO *info, int flag)
{
	if(info == 0) return;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	if(info_list == 0) return;
	subscSetA(info_list, SUPP_SERV_BOIC, flag);
}

void subscSetSSBoicExHC					(T_SUBSC_INFO *info, int flag)
{
	if(info == 0) return;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	if(info_list == 0) return;
	subscSetA(info_list, SUPP_SERV_BOICE, flag);
}

void subscSetSSBaic						(T_SUBSC_INFO *info, int flag)
{
	if(info == 0) return;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	if(info_list == 0) return;
	subscSetA(info_list, SUPP_SERV_BAIC, flag);
}

void subscSetSSBicRoam					(T_SUBSC_INFO *info, int flag)
{
	if(info == 0) return;
	T_SUBSC_IE_SS_INFO_LIST *info_list = subscGetSSInfoListPtr(info);
	if(info_list == 0) return;
	subscSetA(info_list, SUPP_SERV_BIC_ROAM, flag);
}

int subscGetTSIndex(T_SUBSC_IE_PROVISION_TS *ts_list,
						  T_SUBSC_INFO_TELE_SERVICE_CODE code)
{
	int index = -1;
	for(int i=0; i<ts_list->num_srv; i++){
		if(ts_list->ts_code[i] == code){
			index = i;
			break;
		}
	}
	return index;
}

int subscExistTS(T_SUBSC_INFO *info, T_SUBSC_INFO_TELE_SERVICE_CODE code)
{
	T_SUBSC_IE_PROVISION_TS* ts_list = subscGetProvTSPtr(info);
	if(ts_list == 0) return 0;

	int index = subscGetTSIndex(ts_list, code);
	if(index == -1)
		return 0;
	else
		return 1;
}

int subscAddTS(T_SUBSC_INFO *info, T_SUBSC_INFO_TELE_SERVICE_CODE code)
{
	T_SUBSC_IE_PROVISION_TS* ts_list = subscGetProvTSPtr(info);
	if(ts_list == 0) return -1;

	int index = subscGetTSIndex(ts_list, code);
	if(index != -1) // already exists
		return 1;
	if(ts_list->num_srv == MAX_NUM_TELE_SERVICE) // list full
		return -1;
	ts_list->ts_code[ts_list->num_srv] = code;
	ts_list->num_srv++;
	return 0;
}

int subscDelTS(T_SUBSC_INFO *info, T_SUBSC_INFO_TELE_SERVICE_CODE code)
{
	T_SUBSC_IE_PROVISION_TS* ts_list = subscGetProvTSPtr(info);
	if(ts_list == 0) return -1;

	int index = subscGetTSIndex(ts_list, code);
	if(index == -1) // doesn't exist
		return 1;
	if(ts_list->num_srv == 1){ // last one
		ts_list->ts_code[index] = ALL_TELE_SERVICES;
	}
	else{
		ts_list->ts_code[index] = ts_list->ts_code[ts_list->num_srv - 1];
		ts_list->ts_code[ts_list->num_srv - 1] = ALL_TELE_SERVICES;
	}
	ts_list->num_srv--;
	return 0;
}

int subscProvSMS_MT_PP(T_SUBSC_INFO *info)
{
	return( subscExistTS(info, ALL_SMS) || subscExistTS(info, SMS_MT_PP) );
}

int subscProvSMS_MO_PP(T_SUBSC_INFO *info)
{
	return( subscExistTS(info, ALL_SMS) || subscExistTS(info, SMS_MO_PP) );
}

void subscProvSSCode(T_SUBSC_INFO *info, T_SUBSC_INFO_SUPP_SERVICE_CODE code, int add)
{
	T_SUBSC_IE_PROVISION_SS* list =	subscGetProvSSPtr(info);

	int found = 0;
	int i;
	for(i=0; i<list->num_srv; i++){
		if(list->ss_code[i] == code){
			found = 1;
			break;
		}
	}
	if(found){
		if(!add){
			list->ss_code[i] = list->ss_code[list->num_srv - 1];
			list->num_srv--;
		}
	}
	else{
		if(add){
			list->ss_code[list->num_srv++] = code;
		}
	}
}

/* GPRS */
T_SUBSC_GPRS_DATA *subscGetGprsPdpDataPtr(T_SUBSC_INFO *info, int index)
{
	T_SUBSC_IE_GPRS_DATA_INFO* gprs_data = subscGetGPRSDataPtr(info);
	if(gprs_data == 0) return 0;
	else return &gprs_data->data[index];
}

T_SUBSC_GPRS_DATA *subscFindGprsPdpData(T_SUBSC_INFO *info, unsigned char pdpContextId, int *index)
{
	T_SUBSC_IE_GPRS_DATA_INFO* gprs_data = subscGetGPRSDataPtr(info);
	*index = -1;
	int i;

	if(gprs_data == 0) return 0;

	for(i=0; i<gprs_data->num_data; i++){
		if(gprs_data->data[i].pdpContextId == pdpContextId){
			*index = i;
			return &gprs_data->data[i];
		}
	}

	return 0;
}

int subscAddGprsPdpData(T_SUBSC_INFO *info, T_SUBSC_GPRS_DATA *data)
{
	T_SUBSC_IE_GPRS_DATA_INFO* gprs_data = subscGetGPRSDataPtr(info);
	if(gprs_data == 0) return -1;	// can't get pointer to the gprs_data
	if(data == 0) return -1;		// just being cautious
	int index;
	T_SUBSC_GPRS_DATA *pdp_data = subscFindGprsPdpData(info, data->pdpContextId, &index);
	if(pdp_data != 0) return 1;		// pdp data exists
	if(index != -1) return 2;		// internal error

	if(gprs_data->num_data == MAX_NUM_SUBSC_GPRS_DATA) return 3;	// list full

	memcpy(&gprs_data->data[gprs_data->num_data], data, sizeof(T_SUBSC_GPRS_DATA));
	gprs_data->num_data++;

	return 0;
}

int subscDelGprsPdpData(T_SUBSC_INFO *info, unsigned char pdpContextId)
{
	T_SUBSC_IE_GPRS_DATA_INFO* gprs_data = subscGetGPRSDataPtr(info);
	if(gprs_data == 0) return -1;	// can't get pointer to the gprs_data
	int index;
	T_SUBSC_GPRS_DATA *pdp_data = subscFindGprsPdpData(info, pdpContextId, &index);
	if(pdp_data == 0) return 1;		// pdp data doesn't exists
	if(index == -1) return 2;		// internal error

	if(gprs_data->num_data == 0) return 3;	// internal error, empty list

	if(gprs_data->num_data != 1){ 	// not the last one
		memcpy(pdp_data, &gprs_data->data[gprs_data->num_data-1], sizeof(T_SUBSC_GPRS_DATA));
	}
	gprs_data->num_data--;

	return 0;
}

int subscModifyGprsPdpData(T_SUBSC_INFO *info, T_SUBSC_GPRS_DATA *data)
{
	T_SUBSC_IE_GPRS_DATA_INFO* gprs_data = subscGetGPRSDataPtr(info);
	if(gprs_data == 0) return -1;	// can't get pointer to the gprs_data
	if(data == 0) return -1;		// just being cautious
	int index;
	T_SUBSC_GPRS_DATA *pdp_data = subscFindGprsPdpData(info, data->pdpContextId, &index);

	if(index == -1) return 2;		// internal error

	if(gprs_data->num_data == MAX_NUM_SUBSC_GPRS_DATA) return 3;	// list full

	memcpy(&gprs_data->data[index], data, sizeof(T_SUBSC_GPRS_DATA));
//	gprs_data->num_data++;

	return 0;
}

int	subscGetClassInfo(T_SUBSC_INFO *info, unsigned char* classId) // 2.1.2
{
	if (info == 0) return -1;

	T_SUBSC_IE_IN_PROFILE* in_profile = &(info->in_profile);

	if (in_profile->isPresent == false) return -2;

	*classId = in_profile->classInfo.classId;

	return 0; // success
}

int	subscGetPrepaidIndicator(T_SUBSC_INFO *info, bool* isPrepaid) // 2.1.2
{
	if (info == 0) return -1;

	T_SUBSC_IE_IN_PROFILE* in_profile = &(info->in_profile);

	if (in_profile->isPresent == false) return -2;

	*isPrepaid = in_profile->prepaidIndicator.isPrepaid;

	return 0; // success
}

int subscSetInProfile(T_SUBSC_INFO *info, T_SUBSC_IE_IN_PROFILE *inProf)
{
	if (info == 0) return -1;

	info->in_profile = *inProf;

	return 0; // success
}