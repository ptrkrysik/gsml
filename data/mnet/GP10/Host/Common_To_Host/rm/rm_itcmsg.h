/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __RM_ITCMSG_H__
#define __RM_ITCMSG_H__

#include "ril3\ril3md.h"
#include "jcc\JCCL3Msg.h"
#include "oam_api.h"
#include "jcc\JCCbcCb.h"
#include "grr\grr_type.h"
#include "rm_types.h"


// 
// ITC message definition for RM inbound messages
//

#define RM_MAX_QMSG_LEN						200
#define RM_MAX_RXQMSG_LEN					RM_MAX_QMSG_LEN
#define RM_MAX_TXQMSG_LEN					RM_MAX_QMSG_LEN

typedef T_CNI_RIL3MD_RRM_MSG				rm_ItcRxMdMsg_t;  // struct @ ril3md.h
typedef IntraL3Msg_t						rm_ItcRxMmMsg_t;  // struct @ JCCL3Msg.h
typedef IntraL3Msg_t						rm_ItcRxBgMsg_t;  // struct @ JCCL3Msg.h
typedef IntraL3Msg_t						rm_ItcRxCcMsg_t;  // struct @ JCCL3Msg.h
typedef TrapMsg								rm_ItcRxOamMsg_t;

typedef struct {
					u8	module_id;
					u8	entry_id;
					u8	primitive_type;
					u8	timerId;

					struct {
								u16	msgLength;
								u8	buffer[RM_MAX_RXQMSG_LEN];
						} l3_data;

			} rm_ItcRxL1Msg_t, rm_ItcRxRmMsg_t;

typedef InterCbcCbData_t  rm_ItcRxVbMsg_t;

//GP2
typedef grr_ItcMsg_t	  rm_ItcRxGrrMsg_t;

typedef union {
					u8					module_id;
					rm_ItcRxMdMsg_t		itcRxMdMsg;
					rm_ItcRxL1Msg_t		itcRxL1Msg;
					rm_ItcRxMmMsg_t		itcRxMmMsg;
					rm_ItcRxBgMsg_t		itcRxBgMsg;
					rm_ItcRxCcMsg_t		itcRxCcMsg;
					rm_ItcRxRmMsg_t		itcRxRmMsg;
					rm_ItcRxOamMsg_t	itcRxOamMsg;
					rm_ItcRxVbMsg_t 	itcRxVbMsg;
					rm_ItcRxGrrMsg_t    itcRxGrrMsg;   //GP2

			} rm_ItcRxMsg_t;


//
// ITC message format def for RM outbound messages
//
typedef struct {
					u16	msgLength;
					u8	buffer[RM_MAX_TXQMSG_LEN];

			} rm_ItcTxMsg_t;

#endif // __RM_ITCMSG_H__
