/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_TYPE_H__
#define __GRR_TYPE_H__

#include "oam_api.h"  //#include "oam\oam_api.h"


//Message types between GRR and RLC/RRM
//
typedef enum {
	    GRR_MESSAGE_TYPE_BASE = 0,

        GRR_CB_FEATURE_CONTROL,
        GRR_GPRS_FEATURE_CONTROL,

		GRR_READY_TO_GO_REQ,
		GRR_READY_TO_GO_ACK,

		GRR_TRX_SLOT_CHANCOMB,
				
		//GRR_CHAN_USAGE_REPORT

		//GRR_BTS_LOCK_REQ,
		//GRR_BTS_LOCK_ACK,
		//GRR_BTS_UNLOCK_REQ,
		//GRR_BTS_UNLOCK_ACK,
		//GRR_BTS_SHUTDOWN_REQ,
		//GRR_BTS_SHUTDOWN_ACK,

		//GRR_TRX_LOCK_REQ,
		//GRR_TRX_LOCK_ACK,
		//GRR_TRX_UNLOCK_REQ,
		//GRR_TRX_UNLOCK_ACK,
		//GRR_TRX_SHUTDOWN_REQ,
		//GRR_TRX_SHUTDOWN_ACK,

		GRR_TRX_SLOT_LOCK_REQ,
		GRR_TRX_SLOT_LOCK_ACK,
		GRR_TRX_SLOT_SHUTDOWN_REQ,
		GRR_TRX_SLOT_SHUTDOWN_ACK,
		//GRR_TRX_SLOT_UNLOCK_REQ,
		//GRR_TRX_SLOT_UNLOCK_ACK,

} grr_MessageType_t;


//Message structure between GRR and RLC/RRM
//
typedef struct {

	unsigned char		module_id;
    int					primitive_type;
	grr_MessageType_t	message_type;
	unsigned char		trx;
	unsigned char		slot;

	union {
						unsigned char		feat_ctrl;
						unsigned char		chan_comb;
						unsigned char		timer_id;
						unsigned char		buffer[100];//GRR_MAX_RXQMSG_LENGTH];
	};

} grr_ItcMsg_t;


//GRR internal-use data structures
//
typedef grr_ItcMsg_t		grr_ItcRxGrrMsg_t;
typedef grr_ItcMsg_t		grr_ItcRxRlcMsg_t;
typedef grr_ItcMsg_t		grr_ItcRxRrmMsg_t;
typedef grr_ItcMsg_t		grr_ItcRxDspMsg_t;
typedef TrapMsg				grr_ItcRxOamMsg_t;

typedef union {
	    unsigned char		module_id;

		grr_ItcRxGrrMsg_t	itcRxGrrMsg;
		grr_ItcRxRrmMsg_t	itcRxRrmMsg;
		grr_ItcRxRlcMsg_t	itcRxRlcMsg;
		grr_ItcRxDspMsg_t	itcRxDspMsg;
		grr_ItcRxOamMsg_t   itcRxOamMsg;

} grr_ItcRxMsg_t;

#endif //__GRR_TYPE_H__

