/****************************************************************************************
 *                                                                                      *
 *  Copyright Cisco Systems, Inc 2000 All rights reserved                               *
 *                                                                                      *
 *  File                : bssgp_ie_decoder.h                                            *
 *                                                                                      *
 *  Description         : Function prototypes for the IEI decoder module of BSSGP       *
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

#ifndef BSSGP_IE_DEC_HDR_INCLUDE
#define BSSGP_IE_DEC_HDR_INCLUDE

#include <stdio.h>
#include "bssgp_prot.h"
#include "bssgp_api.h"


/*
 * All the function prototpes
 */
BOOL bssgp_ie_decode_alignment_octets(UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_bmax_def_ms(UINT16 *bmax, UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_bss_area_ind(RIL3_BSS_AREA_INDICATION *bss_area, UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_bucket_leak_rate(UINT16 *r_value, UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_bvc_bucket_size(UINT16 *bmax, UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_bvci(RIL3_BVCI *bvci,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_bvc_measurement(RIL3_BVC_MEASUREMENT *delay_val, UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_exception_cause(RIL3_BSSGP_CAUSE *cause,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_cell_id(RIL3_IE_CELL_IDENTIFIER *cell_id,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_channel_needed(UINT8 *val, UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_drx_params(T_CNI_RIL3_IE_DRX_PARAMETER *drx_params,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_emlpp_priority(UINT8 *val, UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_flush_action(BSSGP_FLUSH_ACTION *action, UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_imsi(T_CNI_RIL3_IE_MOBILE_ID *imsi,UINT8 *msg,UINT16 *msg_len);
BOOL bssgp_ie_decode_llc_pdu(BSSGP_DL_UNITDATA_MSG *dl_unitdata,UINT8 *msg,UINT16 *len);
UINT8* bssgp_ie_decode_llc_pdu_ext(UINT16 *pdu_len,UINT8 *msg,UINT16 *len);

BOOL bssgp_ie_decode_llc_frames_discarded(UINT8 *frames, UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_location_area(T_CNI_RIL3_IE_LOCATION_AREA_ID *location_area,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_lsa_identifier_list(RIL3_IE_LSA_IDENTIFIER_LIST *lsa_id_list,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_lsa_info(RIL3_IE_LSA_INFORMATION *lsa_info, UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_mobile_id(UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_ms_bucket_size(UINT16 *bmax, UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_ms_ra_cap(T_CNI_RIL3_IE_MS_RADIO_ACCESS_CAPABILITY *ra_cap,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_omc_id(UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_pdu_in_error(UINT8 *llc_pdu,UINT16 *pdu_len,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_pdu_lifetime(RIL3_IE_PDU_LIFETIME *delay, UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_priority(RIL3_IE_PRIORITY *priority, UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_qos_params(T_CNI_RIL3_IE_QOS *qos_params, UINT8 *msg, UINT16 *len);
BOOL bssgp_ie_decode_qos_profile(T_RIL3_IE_QOS_PROFILE_ID *qos_profile,UINT8 *msg,UINT16 *len, BSSGP_IEI_FORMAT format);
BOOL bssgp_ie_decode_radio_cause(T_RIL3_IE_RADIO_CAUSE_ID *radio_cause,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_ra_cap_update_cause(RA_CAP_UPD_CAUSE *ra_cause,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_reject_cause(T_CNI_RIL3_IE_REJECT_CAUSE *reject_cause, UINT8 *msg, UINT16 *len);
BOOL bssgp_ie_decode_routing_area(T_CNI_RIL3_IE_ROUTING_AREA_ID *routing_area,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_r_def_ms(UINT16 *r_default,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_suspend_ref_num(UINT8 *suspend_ref_num,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_tag(UINT8 *tag,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_tlli(RIL3_TLLI *tlli,UINT8 *msg,UINT16 *len, BSSGP_IEI_FORMAT format);
BOOL bssgp_ie_decode_tmsi(T_CNI_RIL3_IE_MOBILE_ID *tmsi,UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_trace_ref(UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_trace_type(UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_txn_id(UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_trigger_id(UINT8 *msg,UINT16 *len);
BOOL bssgp_ie_decode_num_octets_affected(UINT32 *num,UINT8 *msg,UINT16 *len);
UINT16 bssgp_ie_decode_iei_length(UINT8 *msg,UINT16 *msg_len);


#endif /* #ifndef BSSGP_IE_DEC_HDR_INCLUDE */