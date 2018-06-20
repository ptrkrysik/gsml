/*
 ****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: bssgp_encoder.h												*
 *																						*
 *	Description			: Encoding function prototypes for the BSS-GP layer module		*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |10/10/00 | DSN    | File created												*
 *		 |		   |		|															*
 ****************************************************************************************
 */
#ifndef BSSGP_ENCODER_HDR_INCLUDE
#define BSSGP_ENCODER_HDR_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include "bssgp_prot.h"
#include "bssgp_util.h"
#include "bssgp_api.h"


BOOL bssgp_encode_msg(BSSGP_API_MSG *api_msg,UINT8 *msg,UINT16 *msg_len,
                BSSGP_NETWORK_QOS_LEVEL *qos_reqd, BOOL *tx_msg);
BOOL bssgp_encode_dl_unitdata(BSSGP_DL_UNITDATA_MSG dl_unitdata_msg ,UINT8 *msg,
            UINT16 *len, BSSGP_NETWORK_QOS_LEVEL *qos_reqd, BOOL *tx_msg);
BOOL bssgp_encode_ul_unitdata(BSSGP_UL_UNITDATA_MSG ul_unitdata_msg,UINT8 *msg,UINT16 *len, 
    BSSGP_NETWORK_QOS_LEVEL *qos_reqd, BOOL *tx_msg);
BOOL bssgp_encode_paging_ps(BSSGP_PAGING_PS_MSG paging_msg ,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_radio_status(BSSGP_RADIO_STATUS_MSG radio_status_msg ,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_suspend(BSSGP_SUSPEND_MSG suspend_msg,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_suspend_ack(BSSGP_SUSPEND_ACK_MSG suspend_ack_msg ,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_suspend_nack(BSSGP_SUSPEND_NACK_MSG suspend_nack_msg ,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_resume(BSSGP_RESUME_MSG resume_msg ,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_resume_ack(BSSGP_RESUME_ACK_MSG resume_ack_msg ,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_resume_nack(BSSGP_RESUME_NACK_MSG resume_nack_msg ,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_location_update(BSSGP_LOCATION_UPDATE_MSG location_update_msg ,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_location_update_ack(BSSGP_LOCATION_UPDATE_ACK_MSG location_update_ack ,
	UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_location_update_nack(BSSGP_LOCATION_UPDATE_NACK_MSG location_update_nack ,UINT8 *msg,
	UINT16 *msg_len);
BOOL bssgp_encode_radio_cap(BSSGP_RADIO_CAP_MSG radio_cap ,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_radio_cap_update(BSSGP_RADIO_CAP_UPDATE_MSG radio_cap_update ,UINT8 *msg,
	UINT16 *msg_len);
BOOL bssgp_encode_radio_cap_update_ack(BSSGP_RADIO_CAP_UPDATE_ACK_MSG radio_cap_update_ack ,UINT8 *msg,
	UINT16 *msg_len);
BOOL bssgp_encode_flow_control_ms(BSSGP_FLOW_CONTROL_MS_MSG flow_control_ms,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_flow_control_ms_ack(BSSGP_FLOW_CONTROL_MS_ACK_MSG flow_control_ms_ack,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_flush_ll(BSSGP_FLUSH_LL_MSG flush_ll,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_flush_ll_ack(BSSGP_FLUSH_LL_ACK_MSG flush_ll_ack,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_flow_control_bvc(BSSGP_FLOW_CONTROL_BVC_MSG flow_control_bvc, UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_flow_control_bvc_ack(BSSGP_FLOW_CONTROL_BVC_ACK_MSG flow_control_bvc_ack, UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_llc_discarded(BSSGP_LLC_DISCARDED_MSG llc_discarded,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_bvc_block(BSSGP_BVC_BLOCK_MSG bvc_block,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_bvc_block_ack(BSSGP_BVC_BLOCK_ACK_MSG bvc_block_ack,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_bvc_unblock(BSSGP_BVC_UNBLOCK_MSG bvc_unblock,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_bvc_unblock_ack(BSSGP_BVC_UNBLOCK_ACK_MSG bvc_unblock_ack,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_bvc_reset(BSSGP_BVC_RESET_MSG bvc_reset,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_bvc_reset_ack(BSSGP_BVC_RESET_ACK_MSG bvc_reset_ack,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_encode_bvc_status(BSSGP_BVC_STATUS_MSG bvc_status,UINT8 *msg,UINT16 *msg_len);


#endif /* #ifndef BSSGP_ENCODER_HDR_INCLUDE */