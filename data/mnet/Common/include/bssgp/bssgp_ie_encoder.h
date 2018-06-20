/****************************************************************************************
 *                                                                                      *
 *  Copyright Cisco Systems, Inc 2000 All rights reserved                               *
 *                                                                                      *
 *  File                : bssgp_ie_encoder.h                                            *
 *                                                                                      *
 *  Description         : Function prototypes for the IEI encoder module of BSSGP       *
 *                                                                                      *
 *  Author              : Dinesh Nambisan                                               *
 *                                                                                      *
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description                                               *
 *--------------------------------------------------------------------------------------*
 * 00:00 |10/10/00 | DSN    | File created                                              *
 *       |         |        |                                                           *
 ****************************************************************************************
 */

#ifndef BSSGP_IE_ENC_HDR_INCLUDE
#define BSSGP_IE_ENC_HDR_INCLUDE

#include <stdio.h>
#include "bssgp_prot.h"
#include "bssgp_api.h"


/*
 * All the function prototpes
 */
void bssgp_ie_encode_alignment_octets(UINT8 count,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_bmax_def_ms(UINT16 bmax,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_bss_area_ind(RIL3_BSS_AREA_INDICATION bss_area,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_bucket_leak_rate(UINT16 leak_rate,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_bvc_bucket_size(UINT16 bucket_size,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_bvci(RIL3_BVCI bvci,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_bvc_measurement(RIL3_BVC_MEASUREMENT bvc_meas,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_exception_cause(RIL3_BSSGP_CAUSE cause,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_cell_id(RIL3_IE_CELL_IDENTIFIER cell_id,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_channel_needed(UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_drx_params(T_CNI_RIL3_IE_DRX_PARAMETER drx_params,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_emlpp_priority(UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_flush_action(BSSGP_FLUSH_ACTION flush_action,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_imsi(T_CNI_RIL3_IE_MOBILE_ID imsi,UINT8 *msg,UINT16 *msg_len);
void bssgp_ie_encode_llc_pdu(UINT8* llc_pdu, UINT16 pdu_len,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_llc_frames_discarded(UINT8 num_frames,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_location_area(T_CNI_RIL3_IE_LOCATION_AREA_ID location_area,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_lsa_identifier_list(RIL3_IE_LSA_IDENTIFIER_LIST lsa_id_list,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_lsa_info(RIL3_IE_LSA_INFORMATION lsa_info,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_mobile_id(UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_ms_bucket_size(UINT16 bucket_size,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_ms_ra_cap(T_CNI_RIL3_IE_MS_RADIO_ACCESS_CAPABILITY ra_cap,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_omc_id(UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_pdu_in_error(BSSGP_PDU_TYPE pdu_type,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_pdu_lifetime(RIL3_IE_PDU_LIFETIME pdu_lifetime,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_priority(RIL3_IE_PRIORITY priority,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_qos_params(T_CNI_RIL3_IE_QOS qos_params, UINT8 *msg, UINT16 *len);
void bssgp_ie_encode_qos_profile(T_RIL3_IE_QOS_PROFILE_ID qos_profile,UINT8 *msg,UINT16 *len, BSSGP_IEI_FORMAT format);
void bssgp_ie_encode_radio_cause(T_RIL3_IE_RADIO_CAUSE_ID radio_cause,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_reject_cause(T_CNI_RIL3_IE_REJECT_CAUSE reject_cause, UINT8 *msg, UINT16 *len);
void bssgp_ie_encode_ra_cap_update_cause(RA_CAP_UPD_CAUSE ra_cause,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_routing_area(T_CNI_RIL3_IE_ROUTING_AREA_ID routing_area,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_r_def_ms(UINT16 r_default,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_suspend_ref_num(UINT8 suspend_ref_num,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_tag(UINT8 tag,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_tlli(RIL3_TLLI tlli,UINT8 *msg,UINT16 *len, BSSGP_IEI_FORMAT format);
void bssgp_ie_encode_tmsi(T_CNI_RIL3_IE_MOBILE_ID tmsi,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_trace_ref(UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_trace_type(UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_txn_id(UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_trigger_id(UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_num_octets_affected(UINT32 num_octets,UINT8 *msg,UINT16 *len);
void bssgp_ie_encode_iei_length(int iei_len,UINT8 *msg,UINT16 *msg_len);


#endif /* BSSGP_IE_ENC_HDR_INCLUDE */
