/*
 ****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: bssgp_decoder.h												*
 *																						*
 *	Description			: Decoding function prototypes for the BSS-GP layer module		*
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
#ifndef BSSGP_DECODER_HDR_INCLUDE
#define BSSGP_DECODER_HDR_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include "bssgp_prot.h"
#include "bssgp_util.h"
#include "bssgp_api.h"

/*
 * Just the function prototypes
 */
BOOL bssgp_decode_msg(UINT8 *msg,UINT16 msg_len,BOOL incoming,
			BSSGP_NETWORK_QOS_LEVEL qos_level,BSSGP_BVCI bvci,BOOL *free_buff);
BOOL bssgp_decode_dl_unitdata(UINT8 *msg,UINT16 *len, BSSGP_NETWORK_QOS_LEVEL qos_level);
BOOL bssgp_decode_ul_unitdata(UINT8 *msg,UINT16 *len,BSSGP_NETWORK_QOS_LEVEL qos_level,BSSGP_BVCI bvci);
BOOL bssgp_decode_paging_ps(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_radio_status(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_suspend(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_suspend_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_suspend_nack(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_resume(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_resume_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_resume_nack(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_location_update(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_location_update_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_location_update_nack(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_radio_cap(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_radio_cap_update(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_radio_cap_update_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_flow_control_ms(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_flow_control_ms_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_flush_ll(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_flush_ll_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_llc_discarded(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_flow_control_bvc(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_flow_control_bvc_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_block(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_block_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_unblock(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_unblock_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_reset(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_reset_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
BOOL bssgp_decode_status(UINT8 *msg,UINT16 *msg_len, BOOL incoming);
 

#ifdef WIN32
void llcwin32_pdu_handler(LLC_PDU);
#endif
#endif /* #ifndef BSSGP_DECODER_HDR_INCLUDE */