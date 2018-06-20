/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __RM_IMPORT_H__
#define __RM_IMPORT_H__

#include "ril3\ril3_result.h"
#include "ril3\ril3irt.h"
#include "lapdm\lapdm_l3intf.h"
#include "ril3\ril3msgheader.h"
#include "ril3\ie_rrm_common.h"
#include "ril3\ie_identity_type.h"
#include "ril3\ie_page_mode.h"
#include "ril3\ie_mobile_id.h"
#include "ril3\msg_channel_request.h"
#include "ril3\ril3_rrm_msg.h"
#include "jcc\JCCL3Msg.h"

//
// Imported data types
//
typedef T_CNI_LAPDM_SAPI							rm_LapdmSapi_t;
typedef T_CNI_LAPDM_GsmChannelType					rm_LapdmGsmChanType_t;
typedef T_CNI_LAPDM_ReleaseMode						rm_LapdmRelMode_t;
typedef T_CNI_RIL3_CHANNEL_TYPE						rm_UmEdChanType_t;		// @ie_rrm_common.h
typedef T_CNI_RIL3RRM_MSG							rm_UmMsg_t;             // union of ril3 msgs@ril3_rrm_msg.h
typedef T_CNI_IRT_ID								rm_IrtEntryId_t;        // unsigned short def @ ril3irt.h
typedef T_CNI_LAPDM_OID								rm_LapdmOId_t;          // unsigned long: def @ladpm_l3intf.h
typedef T_CNI_RIL3_RESULT							rm_EdRet_t;             // enum @ ril3_result.h
typedef T_CNI_RIL3RRM_MSG_CHANNEL_REQUEST			rm_ChanReq_t;			// add prefix Um later
typedef T_CNI_RIL3RRM_MSG_SYSTEM_INFO_TYPE2			rm_UmSI2_t; 
typedef T_CNI_RIL3RRM_MSG_SYSTEM_INFO_TYPE3			rm_UmSI3_t; 
typedef T_CNI_RIL3RRM_MSG_SYSTEM_INFO_TYPE4			rm_UmSI4_t; 
typedef T_CNI_RIL3RRM_MSG_SYSTEM_INFO_TYPE5			rm_UmSI5_t; 
typedef T_CNI_RIL3RRM_MSG_SYSTEM_INFO_TYPE6			rm_UmSI6_t; 
typedef	T_CNI_RIL3RRM_MSG_CHANNEL_MODE_MODIFY		rm_UmChanModeModify_t;
typedef	T_CNI_RIL3RRM_MSG_CHANNEL_MODE_MODIFY_ACK   rm_UmChanModeModifyAck_t;
typedef T_CNI_RIL3RRM_MSG_PAGING_REQUEST_TYPE1		rm_UmPageType1_t;
typedef T_CNI_RIL3RRM_MSG_IMMEDIATE_ASSIGNMENT		rm_UmImmAssign_t;
typedef T_CNI_RIL3RRM_MSG_PAGING_RESPONSE			rm_UmPagingRsp_t;
typedef T_CNI_RIL3RRM_MSG_CHANNEL_RELEASE			rm_UmChanRel_t;

typedef IntraL3PageRequest_t						rm_MmPaging_t;
typedef IntraL3PageResponse_t						rm_MmPagingRsp_t;
typedef IntraL3Msg_t 								rm_ItcTxL3Msg_t;
typedef IntraL3ChannAssignCmd_t						rm_MmChannAssign_t;		

typedef T_CNI_LAPDM_L3MessageUnit					rm_L3Data_t;			// @lapdm_l3intf.h
typedef T_CNI_RIL3_IE_MOBILE_ID						rm_IeMobileId_t;		// @ie_mobile_id.h
typedef T_CNI_RIL3_IE_MOBILE_ID						rm_IeImsi_t;			// @ie_mobile_id.h

#endif // __RM_IMPORT_H__
