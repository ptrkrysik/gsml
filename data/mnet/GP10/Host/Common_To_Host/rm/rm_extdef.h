/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_EXTDEF_H__
#define __RM_EXTDEF_H__

//
// Imported function names
//
#define  rm_SendDlRelReq				CNI_LAPDM_Dl_Release_Request
#define  rm_SendMdlRelReq				CNI_LAPDM_Mdl_Release_Request
#define  rm_SendDlRelReq				CNI_LAPDM_Dl_Release_Request
#define  rm_SendDlDataReq				CNI_LAPDM_Dl_Data_Request

//
// Imported symbolic constants
//
#define RM_CELL_NOT_BARRED				CNI_RIL3_CELL_NOT_BARRED  					//0
#define RM_CELL_BARRED	     			CNI_RIL3_CELL_BARRED_ACCESS					//1
#define RM_REST_NOT_ALLOWED				CNI_RIL3_REESTABLISHMENT_NOT_ALLOWED    	//1
#define RM_REST_ALLOWD					CNI_RIL3_REESTABLISHMENT_ALLOWED			//0
#define RM_EC_NOT_ALLOWED				CNI_RIL3_EMERGENCY_CALL_HIGHCLASS_MS_ONLY   //1
#define RM_EC_ALLOWED					CNI_RIL3_EMERGENCY_CALL_ALLOWED_TO_ALL_MS   //0

#define RM_EXTIND_0						CNI_RIL3_EXT_IND_BA_COMPLETE
#define RM_EXTIND_1  					CNI_RIL3_EXT_IND_BA_PARTIAL
#define RM_P_NOT_PRESENT				CNI_RIL3_FREQ_PARAM_FIELD_NOT_PRESENT
#define RM_P_PRESENT					CNI_RIL3_FREQ_PARAM_FIELD_PRESENT
#define RM_BA_WHOLE						CNI_RIL3_EXT_IND_BA_COMPLETE
#define RM_FRMT_VARBITMAP				CNI_RIL3_FREQ_LIST_FORMAT_VARIABLE_BIT_MAP

//
// ED-relaged definitions for RM-ED interface
//
#define RM_RAED_SUCCESS					CNI_RIL3_RESULT_SUCCESS	 // enum @ ril3_result.h
#define RM_RRED_SUCCESS					CNI_RIL3_RESULT_SUCCESS  // enum @ ril3_result.h
#define RM_UMED_SUCCESS					CNI_RIL3_RESULT_SUCCESS  // enum @ ril3_result.h

#define RM_PHYCHAN_TCHF_S 				CNI_RIL3_TCH_ACCH_FULL   // @ ie_rrm_common.h
#define RM_PHYCHAN_SDCCH4				CNI_RIL3_SDCCH_ACCH_4orCBCH
#define RM_NORMAL_EVENT	  				CNI_RIL3_RR_NORMAL_EVENT
#define RM_CHANMODESP_VER1  			CNI_RIL3_CHANNELMODE_SPEECH_VER1
#define RM_CTFACCH_F					CT_FACCH_F
#define RM_CTSDCCH						CT_SDCCH
#define RM_CTSACCH						CT_SACCH
#define RM_CTPCH						CT_PCH
#define RM_CTAGCH						CT_AGCH
#define RM_CTRACH						CT_RACH
#define RM_CTBCCH						CT_BCCH

#define RM_PD	 						CNI_RIL3_PD_RRM			// enum @ ril3msgheader.h
#define MM_PD	 						CNI_RIL3_PD_MM			// enum @ ril3msgheader.h

#define RM_SI2							CNI_RIL3RRM_MSGID_SYSTEM_INFORMATION_2
#define RM_SI3							CNI_RIL3RRM_MSGID_SYSTEM_INFORMATION_3
#define RM_SI4							CNI_RIL3RRM_MSGID_SYSTEM_INFORMATION_4
#define RM_SI5							CNI_RIL3RRM_MSGID_SYSTEM_INFORMATION_5
#define RM_SI6							CNI_RIL3RRM_MSGID_SYSTEM_INFORMATION_6
#define RM_PAGING_REQUEST_TYPE_1		CNI_RIL3RRM_MSGID_PAGING_REQUEST_TYPE_1
#define RM_IMMEDIATE_ASSIGNMENT			CNI_RIL3RRM_MSGID_IMMEDIATE_ASSIGNMENT	
#define RM_PAGING_RESPONSE				CNI_RIL3RRM_MSGID_PAGING_RESPONSE
#define RM_CHANNEL_RELEASE				CNI_RIL3RRM_MSGID_CHANNEL_RELEASE
#define RM_CHANNEL_MODE_MODIFY			CNI_RIL3RRM_MSGID_CHANNEL_MODE_MODIFY
#define RM_CHANNEL_MODE_MODIFY_ACK		CNI_RIL3RRM_MSGID_CHANNEL_MODE_MODIFY_ACKNOWLEDGE
			
#define RM_NORMAL_PAGING				CNI_RIL3_NORMAL_PAGING				
#define RM_CHANNEEDED_TCHF				CNI_RIL3_CHANNEL_NEEDED_TCHF
#define RM_IMSI	 						CNI_RIL3_IDENTITY_TYPE_IMSI      
#define RM_IMEI							CNI_RIL3_IMEI
#define RM_IMEISV						CNI_RIL3_IMEISV
#define RM_TMSI							CNI_RIL3_TMSI
#define RM_NOMOBILEID					CNI_RIL3_NO_MOBILE_ID

//
// Message-related definitions for RM-MM interface
//
#define MM_PAGE_REQ   					INTRA_L3_RR_PAGE_REQ
#define MM_PAGE_RSP						INTRA_L3_RR_PAGE_RSP

#define MM_CHAN_ASSIGN_CMD				INTRA_L3_RR_CHANN_ASSIGN_CMD
#define MM_CHAN_ASSIGN_COM				INTRA_L3_RR_CHANN_ASSIGN_COM

#define MM_BEARER_SPEECH				BEARER_SPEECH
#define MM_BEARER_DATA					BEARER_DATA
#define MM_BEARER_SIGNALING				BEARER_SIGNALING
#define RM_NORREL						RM_NORMAL
#define RM_LOCREL						RM_LOCAL_RELEASE

#endif // __RM_EXTDEF_H__
