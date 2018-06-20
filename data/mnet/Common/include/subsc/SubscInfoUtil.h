
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
// File        : SubscInfoUtil.h
// Author(s)   : Kevin Lim
// Create Date : 06-16-99
// Description : message and ie interface among LUDB and other modules
//
// *******************************************************************

#ifndef SUBSC_INFO_UTIL_H
#define SUBSC_INFO_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

T_SUBSC_IE_MOBILE_ID_IMSI*	subscGetMobileIdPtr(T_SUBSC_INFO *info);
T_SUBSC_IE_ISDN*				subscGetMSISDNPtr(T_SUBSC_INFO *info);
T_SUBSC_IE_PROVISION_BS*	subscGetProvBSPtr(T_SUBSC_INFO *info);
T_SUBSC_IE_PROVISION_TS*	subscGetProvTSPtr(T_SUBSC_INFO *info);
T_SUBSC_IE_PROVISION_SS*	subscGetProvSSPtr(T_SUBSC_INFO *info);
T_SUBSC_IE_SUBSC_STATUS*	subscGetSubscStatusPtr(T_SUBSC_INFO *info);
T_SUBSC_IE_ODB_DATA*			subscGetODBDataPtr(T_SUBSC_INFO *info);
T_SUBSC_IE_SS_INFO_LIST*	subscGetSSInfoListPtr(T_SUBSC_INFO *info);
T_SUBSC_IE_USER_INFO*		subscGetUserInfoPtr(T_SUBSC_INFO *info);
T_SUBSC_IE_GPRS_DATA_INFO*	subscGetGPRSDataPtr(T_SUBSC_INFO *info);

bool	subscIsProvCLIP	(T_SUBSC_INFO *info);
bool	subscIsProvCLIR	(T_SUBSC_INFO *info);
bool	subscIsProvCOLP	(T_SUBSC_INFO *info);
bool	subscIsProvCOLR	(T_SUBSC_INFO *info);
bool	subscIsProvCFU		(T_SUBSC_INFO *info);
bool	subscIsProvCFB		(T_SUBSC_INFO *info);
bool	subscIsProvCFNRY	(T_SUBSC_INFO *info);
bool	subscIsProvCFNRC	(T_SUBSC_INFO *info);
bool	subscIsProvECT		(T_SUBSC_INFO *info);
bool	subscIsProvCW		(T_SUBSC_INFO *info);
bool	subscIsProvHOLD	(T_SUBSC_INFO *info);
bool	subscIsProvMPTY	(T_SUBSC_INFO *info);
bool	subscIsProvBAOC	(T_SUBSC_INFO *info);
bool	subscIsProvBOIC	(T_SUBSC_INFO *info);
bool	subscIsProvBOICE	(T_SUBSC_INFO *info);
bool	subscIsProvBAIC	(T_SUBSC_INFO *info);
bool	subscIsProvBICRoam(T_SUBSC_INFO *info);

T_SUBSC_FORW_FEATURE*		subscGetForwFeature(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
										  T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code);
bool	subscIsActiveCF(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
							T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code);
bool	subscIsActiveCW(T_SUBSC_IE_SS_INFO_LIST *ss_info_list);
int	subscGetTimerCFNRY(T_SUBSC_IE_SS_INFO_LIST *ss_info_list);

void	subscPrintImsi(T_SUBSC_IE_MOBILE_ID_IMSI *imsi);
void	subscPrintIsdn(T_SUBSC_IE_ISDN *msisdn);
void	subscPrintProvBS(T_SUBSC_IE_PROVISION_BS *prov_bs);
void	subscPrintProvTS(T_SUBSC_IE_PROVISION_TS *prov_ts);
void	subscPrintProvSS(T_SUBSC_IE_PROVISION_SS *prov_ss);
void	subscPrintSubscStatus(T_SUBSC_IE_SUBSC_STATUS *status);
void	subscPrintODBData(T_SUBSC_IE_ODB_DATA *odb_data);
void	subscPrintSSInfoList(T_SUBSC_IE_SS_INFO_LIST *ss_info_list);
void	subscPrintUserInfo(T_SUBSC_IE_USER_INFO *user);
void	subscPrintGPRSDataPtr(T_SUBSC_IE_GPRS_DATA_INFO *gprs_data);

void 	subscPrintInfo(T_SUBSC_INFO *info);
void 	subscPrintGPRSInfo(T_SUBSC_INFO *info);
char *subscGetStringImsi(T_SUBSC_IE_MOBILE_ID_IMSI *imsi);
char *subscGetStringIsdn(T_SUBSC_IE_ISDN *isdn);
char *subscGetStringUser(T_SUBSC_IE_USER_INFO *user);
char *subscGetCFNum(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
							T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code);
T_SUBSC_IE_ISDN*	subscGetCFNumPtr(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
							T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code);

void  subscSetIMSI(T_SUBSC_INFO *subsc, char *buf);
void  subscSetMSISDN(T_SUBSC_INFO *subsc, char *buf);
void  subscSetUserInfo(T_SUBSC_INFO *subsc, char *buf);
void	subscSetACF(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
						T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
						int active);
void	subscSetPCF(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
						T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
						int prov);		// use subscSetP instead
void	subscSetRCF(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
						T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
						int reg);

void	subscSetCFNum(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
						T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
						char *num);
void	subscSetTimerCFNRY(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
						int sec);

T_SS_STATUS_FLAG* subscGetStatusFlagPtr(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
													T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code);

int	subscGetP(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code);

int  subscGetA(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code);

int  subscGetR(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code);

int  subscGetQ(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code);

void	subscSetP(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
					int prov);

void  subscSetA(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
					int active);

void  subscSetR(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
					int registered);

void  subscSetQ(T_SUBSC_IE_SS_INFO_LIST *ss_info_list,
					T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code,
					int quiescent);

int subscGetSSInfoInd(T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code);	// 2.0.6
int subscGetSSInfoForwInd(T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code);	// 2.0.7
int subscGetSSInfoDataInd(T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code);	// 2.0.7
int subscGetSSInfoBarInd(T_SUBSC_INFO_SUPP_SERVICE_CODE ss_code);	// 2.0.7
void subscSSInfoListClear(T_SUBSC_IE_SS_INFO_LIST *info_list);		// 2.0.6


void subscInfoInitialize(T_SUBSC_INFO *info);						// 2.0.6

int subscIsODBarred							(T_SUBSC_INFO *info);
int subscIsODBAllOg							(T_SUBSC_INFO *info);
int subscIsODBIntrnOg						(T_SUBSC_INFO *info);
int subscIsODBIntrnOgNotHplmn				(T_SUBSC_INFO *info);
int subscIsODBPremiumInfoOg				(T_SUBSC_INFO *info);
int subscIsODBPremiumEnterOg				(T_SUBSC_INFO *info);
int subscIsODBSsAccess						(T_SUBSC_INFO *info);
int subscIsODBIntrzOg						(T_SUBSC_INFO *info);
int subscIsODBIntrzOgNotHplmn				(T_SUBSC_INFO *info);
int subscIsODBIntrzIntrnOgNotHplmn		(T_SUBSC_INFO *info);
int subscIsODBAllEct							(T_SUBSC_INFO *info);
int subscIsODBChargeableEct				(T_SUBSC_INFO *info);
int subscIsODBIntrnEct						(T_SUBSC_INFO *info);
int subscIsODBIntrzEct						(T_SUBSC_INFO *info);
int subscIsODBDoubleChargeableEct		(T_SUBSC_INFO *info);
int subscIsODBMultipleEct					(T_SUBSC_INFO *info);

void subscSetODB								(T_SUBSC_INFO *info, int flag);
void subscSetODBAllOg						(T_SUBSC_INFO *info, int flag);
void subscSetODBIntrnOg						(T_SUBSC_INFO *info, int flag);
void subscSetODBIntrnOgNotHplmn			(T_SUBSC_INFO *info, int flag);
void subscSetODBPremiumInfoOg				(T_SUBSC_INFO *info, int flag);
void subscSetODBPremiumEnterOg			(T_SUBSC_INFO *info, int flag);
void subscSetODBSsAccess					(T_SUBSC_INFO *info, int flag);
void subscSetODBIntrzOg						(T_SUBSC_INFO *info, int flag);
void subscSetODBIntrzOgNotHplmn			(T_SUBSC_INFO *info, int flag);
void subscSetODBIntrzIntrnOgNotHplmn	(T_SUBSC_INFO *info, int flag);
void subscSetODBAllEct						(T_SUBSC_INFO *info, int flag);
void subscSetODBChargeableEct				(T_SUBSC_INFO *info, int flag);
void subscSetODBIntrnEct					(T_SUBSC_INFO *info, int flag);
void subscSetODBIntrzEct					(T_SUBSC_INFO *info, int flag);
void subscSetODBDoubleChargeableEct		(T_SUBSC_INFO *info, int flag);
void subscSetODBMultipleEct				(T_SUBSC_INFO *info, int flag);

int subscIsSSBaoc						(T_SUBSC_INFO *info);
int subscIsSSBoic						(T_SUBSC_INFO *info);
int subscIsSSBoicExHC					(T_SUBSC_INFO *info);
int subscIsSSBaic						(T_SUBSC_INFO *info);
int subscIsSSBicRoam					(T_SUBSC_INFO *info);

void subscSetSSBaoc						(T_SUBSC_INFO *info, int flag);
void subscSetSSBoic						(T_SUBSC_INFO *info, int flag);
void subscSetSSBoicExHC					(T_SUBSC_INFO *info, int flag);
void subscSetSSBaic						(T_SUBSC_INFO *info, int flag);
void subscSetSSBicRoam					(T_SUBSC_INFO *info, int flag);

int subscGetTSIndex(T_SUBSC_IE_PROVISION_TS *ts_list,
						  T_SUBSC_INFO_TELE_SERVICE_CODE code);
int subscExistTS(T_SUBSC_INFO *info, T_SUBSC_INFO_TELE_SERVICE_CODE code);
int subscAddTS(T_SUBSC_INFO *info, T_SUBSC_INFO_TELE_SERVICE_CODE code);
int subscDelTS(T_SUBSC_INFO *info, T_SUBSC_INFO_TELE_SERVICE_CODE code);

int subscProvSMS_MO_PP(T_SUBSC_INFO *info);
int subscProvSMS_MT_PP(T_SUBSC_INFO *info);

void subscProvSSCode(T_SUBSC_INFO *info, T_SUBSC_INFO_SUPP_SERVICE_CODE code, int add);

int	subscGetCLIROption(T_SUBSC_INFO *info, T_SS_SUBSC_CLIR_OPTION *option); // 2.1.1
int	subscSetCLIROption(T_SUBSC_INFO *info, T_SS_SUBSC_CLIR_OPTION option); // 2.1.1

/* GPRS */
T_SUBSC_GPRS_DATA *subscGetGprsPdpDataPtr(T_SUBSC_INFO *info, int index);

/* find entry by pdpContextId */
/* return 0 and index value -1 if not found */
/* return pointer to T_SUBSC_GPRS_DATA and index of T_SUBSC_IE_GPRS_DATA_INFO's data array */

T_SUBSC_GPRS_DATA *subscFindGprsPdpData(T_SUBSC_INFO *info, unsigned char pdpContextId, int *index);

/* copy data to the new entry after the last entry and increase num_data of the list */
/* return 0 on success */
/* return -1 if can't get pointer to T_SUBSC_IE_GPRS_DATA_INFO */
/* return 1 if T_SUBSC_GPRS_DATA already exists with pdpContextId same data->pdpContextId */
/* return 2 internal error */
/* return 3 T_SUBSC_IE_GPRS_DATA_INFO's data array is full */

int subscAddGprsPdpData(T_SUBSC_INFO *info, T_SUBSC_GPRS_DATA *data);

/* copy the last entry data to the delete target entry and decrease num_data of the list */
/* return 0 on success */
/* return -1 if can't get pointer to T_SUBSC_IE_GPRS_DATA_INFO */
/* return 1 if T_SUBSC_GPRS_DATA not found with pdpContextId */
/* return 2 internal error */
/* return 3 internal error, T_SUBSC_IE_GPRS_DATA_INFO's data array is empty */

int subscDelGprsPdpData(T_SUBSC_INFO *info, unsigned char pdpContextId);

/* find and modify the passed data in the list */
/* return 0 on success */
/* return -1 if can't get pointer to T_SUBSC_IE_GPRS_DATA_INFO */
/* return 1 if T_SUBSC_GPRS_DATA not found with pdpContextId */
/* return 2 internal error */
/* return 3 internal error, T_SUBSC_IE_GPRS_DATA_INFO's data array is empty */

int subscModifyGprsPdpData(T_SUBSC_INFO *info, T_SUBSC_GPRS_DATA *data);

/* IN PROFILE */

int	subscGetClassInfo(T_SUBSC_INFO *info, unsigned char* classId); // 2.1.2
int	subscGetPrepaidIndicator(T_SUBSC_INFO *info, bool* isPrepaid); // 2.1.2
int subscSetInProfile(T_SUBSC_INFO *info, T_SUBSC_IE_IN_PROFILE *inProf); // 2.1.2

#ifdef __cplusplus
}
#endif

#endif // SUBSC_INFO_UTIL_H





