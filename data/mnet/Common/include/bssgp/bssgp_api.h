/*
 ****************************************************************************************
 *                                                                                      *
 *  Copyright Cisco Systems, Inc 2000 All rights reserved                               *
 *                                                                                      *
 *  File                : bssgp_api.h                                                   *
 *                                                                                      *
 *  Description         : API data structures for the BSS-GP layer module               *
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
#ifndef _BSSGP_API_HDR_INC_
#define _BSSGP_API_HDR_INC_

#include <stdio.h>
#include <stdlib.h>
#include "bssgp_prot.h"
#include "jcstddef.h"
/*
 * RIL3 (4.08) includes
 */
#include "ril3/ie_tlli.h"
#include "ril3/ie_bvci.h"
#include "ril3/ie_mobile_id.h"
#include "ril3/ie_location_area_id.h"
#include "ril3/ie_qos_profile.h"
#include "ril3/ril3_gmm_msg.h"
#include "ril3/ie_reject_cause.h"
#include "ril3/ie_cell_id.h"
#include "ril3/ie_radio_cause_id.h"
#include "ril3/ie_priority.h"
#include "ril3/ie_pdu_lifetime.h"
#include "ril3/ie_lsa_information.h"
#include "ril3/ie_cell_identifier.h"
#include "ril3/ie_lsa_identifier_list.h"
#include "ril3/ie_bss_area_indication.h"
#include "ril3/ie_bssgp_cause.h"
#include "ril3/ie_bvc_measurement.h"


/*
 * LLC_PDU value; will depend on
 * interface agreed upon and the
 * platform in question
 */
#if defined (MNET_GP10)
typedef struct {
    UINT8       *data;
    UINT16      data_len;
} LLC_PDU;
#elif defined (MNET_GS)
typedef struct {
    UINT8       *data;
    UINT16      data_len;
} LLC_PDU;
#include "llcmsg.h"
#elif defined(WIN32)
typedef struct {
    UINT8       *data;
    UINT16      data_len;
} LLC_PDU;
#endif



/*
 * BSSGP API message type
 */
typedef enum {

    BSSGP_API_MSG_DL_UNITDATA,
    BSSGP_API_MSG_UL_UNITDATA,
    BSSGP_API_MSG_PAGING_PS,
    BSSGP_API_MSG_RADIO_STATUS,
    BSSGP_API_MSG_SUSPEND,
    BSSGP_API_MSG_SUSPEND_ACK,
    BSSGP_API_MSG_SUSPEND_NACK,
    BSSGP_API_MSG_RESUME,
    BSSGP_API_MSG_RESUME_ACK,
    BSSGP_API_MSG_RESUME_NACK,
    BSSGP_API_MSG_LOCATION_UPDATE,
    BSSGP_API_MSG_LOCATION_UPDATE_ACK,
    BSSGP_API_MSG_LOCATION_UPDATE_NACK,
    BSSGP_API_MSG_RADIO_CAP,
    BSSGP_API_MSG_RADIO_CAP_UPDATE,
    BSSGP_API_MSG_RADIO_CAP_UPDATE_ACK,
    BSSGP_API_MSG_FLUSH_LL,
    BSSGP_API_MSG_FLUSH_LL_ACK,
    BSSGP_API_MSG_LLC_DISCARDED,
    BSSGP_API_MSG_FLOW_CONTROL_BVC,
    BSSGP_API_MSG_FLOW_CONTROL_BVC_ACK,
    BSSGP_API_MSG_FLOW_CONTROL_MS,
    BSSGP_API_MSG_FLOW_CONTROL_MS_ACK,
    BSSGP_API_MSG_BVC_BLOCK,
    BSSGP_API_MSG_BVC_BLOCK_ACK,
    BSSGP_API_MSG_BVC_UNBLOCK,
    BSSGP_API_MSG_BVC_UNBLOCK_ACK,
    BSSGP_API_MSG_BVC_RESET,
    BSSGP_API_MSG_BVC_RESET_ACK,
    BSSGP_API_MSG_STATUS,
    BSSGP_API_MAX_MSGS

} BSSGP_API_MSG_TYPE;



/*
 * The downlink & uplink unitdata messages are
 * handled slightly differently from the other
 * signalling messages; basically in the message
 * only the pointer to the actual msg is passed
 * via regular interfaces, and actual content will
 * be in a mbuf somewhere.... Dinesh N.
 */

/*
 * DL-UnitData
 */

typedef struct {

    RIL3_TLLI                                   tlli;
    T_RIL3_IE_QOS_PROFILE_ID                    qos_profile;
    /* Cisco proprietary */
    T_CNI_RIL3_IE_QOS                           qos_params;
    RIL3_IE_PDU_LIFETIME                        pdu_lifetime;
    T_CNI_RIL3_IE_MS_RADIO_ACCESS_CAPABILITY    radio_access_cap;
    RIL3_IE_PRIORITY                            priority; /* GSM 08.08 */
    T_CNI_RIL3_IE_DRX_PARAMETER                 drx_params;
    T_CNI_RIL3_IE_MOBILE_ID                     imsi;
    RIL3_TLLI                                   tlli_old;
    RIL3_IE_LSA_INFORMATION                     lsa_info;
    LLC_PDU                                     pdu;
        
} BSSGP_DL_UNITDATA_MSG;




/*
 * UL-UnitData
 */
typedef struct {

    RIL3_TLLI                                   tlli;
    T_RIL3_IE_QOS_PROFILE_ID                    qos_profile;
    RIL3_IE_CELL_IDENTIFIER                     cell_id;
    RIL3_IE_LSA_IDENTIFIER_LIST                 lsa_id_list;
    LLC_PDU                                     pdu;    

} BSSGP_UL_UNITDATA_MSG;





/*
 * The Paging msg api structure & defns
 *
 * 
 */
typedef struct {

    T_CNI_RIL3_IE_MOBILE_ID         imsi;
    RIL3_BVCI                       bvci;
    T_CNI_RIL3_IE_LOCATION_AREA_ID  location_area;
    T_CNI_RIL3_IE_ROUTING_AREA_ID   routing_area;
    RIL3_BSS_AREA_INDICATION        bss_area;
    T_CNI_RIL3_IE_DRX_PARAMETER     drx_params;
    T_RIL3_IE_QOS_PROFILE_ID        qos_profile;
    T_CNI_RIL3_IE_MOBILE_ID         tmsi;

} BSSGP_PAGING_PS_MSG;

/*
 * Radio Status msg
 */
typedef struct {

    RIL3_TLLI                       tlli;
    T_CNI_RIL3_IE_MOBILE_ID         tmsi;
    T_CNI_RIL3_IE_MOBILE_ID         imsi;
    T_RIL3_IE_RADIO_CAUSE_ID        cause;

} BSSGP_RADIO_STATUS_MSG;


/*
 * Suspend msg
 */
typedef struct {

    RIL3_TLLI                       tlli;
    T_CNI_RIL3_IE_ROUTING_AREA_ID   routing_area;

} BSSGP_SUSPEND_MSG;

/*
 * Suspend Ack
 */
typedef struct {

    RIL3_TLLI                       tlli;
    T_CNI_RIL3_IE_ROUTING_AREA_ID   routing_area;
    char                            suspend_ref_num;

} BSSGP_SUSPEND_ACK_MSG;


/*
 * Suspend Nack
 */
typedef struct {

    RIL3_TLLI                       tlli;
    T_CNI_RIL3_IE_ROUTING_AREA_ID   routing_area;
    RIL3_BSSGP_CAUSE                exception_cause;

} BSSGP_SUSPEND_NACK_MSG;

/*
 * Resume
 */
typedef struct {

    RIL3_TLLI                       tlli;
    T_CNI_RIL3_IE_ROUTING_AREA_ID   routing_area;
    char                            suspend_ref_num;

} BSSGP_RESUME_MSG;


/*
 * Resume Ack
 */
typedef struct {

    RIL3_TLLI                       tlli;
    T_CNI_RIL3_IE_ROUTING_AREA_ID   routing_area;

} BSSGP_RESUME_ACK_MSG;

/*
 * Resume Nack
 */
typedef struct {

    RIL3_TLLI                       tlli;
    T_CNI_RIL3_IE_ROUTING_AREA_ID   routing_area;
    RIL3_BSSGP_CAUSE           exception_cause;

} BSSGP_RESUME_NACK_MSG;



/*
 * Location Update
 */
typedef struct {

    T_CNI_RIL3_IE_MOBILE_ID         imsi;
    RIL3_BVCI                       bvci;
    T_CNI_RIL3_IE_LOCATION_AREA_ID  location_area;
    T_CNI_RIL3_IE_ROUTING_AREA_ID   routing_area;

} BSSGP_LOCATION_UPDATE_MSG;


/*
 * Location Update Ack
 */
typedef struct {

    T_CNI_RIL3_IE_MOBILE_ID         imsi;
    T_CNI_RIL3_IE_LOCATION_AREA_ID  location_area;

} BSSGP_LOCATION_UPDATE_ACK_MSG;


/*
 * Location Update Nack
 */
typedef struct {

    T_CNI_RIL3_IE_MOBILE_ID         imsi;
    T_CNI_RIL3_IE_LOCATION_AREA_ID  location_area;
    T_CNI_RIL3_IE_REJECT_CAUSE      reject_cause;

} BSSGP_LOCATION_UPDATE_NACK_MSG;


/*
 * Radio-Access-Capability 
 */
typedef struct {

    RIL3_TLLI                                   tlli;
    T_CNI_RIL3_IE_MS_RADIO_ACCESS_CAPABILITY    ra_cap;

} BSSGP_RADIO_CAP_MSG;


/*
 * Radio-Access-Capability-Update
 */
typedef struct {

    RIL3_TLLI           tlli;
    UINT8               tag;

} BSSGP_RADIO_CAP_UPDATE_MSG;


/*
 * Radio-Access-Capability-Update-Ack
 */
typedef struct {

    RIL3_TLLI                                   tlli;
    UINT8                                       tag;
    T_CNI_RIL3_IE_MOBILE_ID                     imsi;
    RA_CAP_UPD_CAUSE                            cause;
    T_CNI_RIL3_IE_MS_RADIO_ACCESS_CAPABILITY    ra_cap;

} BSSGP_RADIO_CAP_UPDATE_ACK_MSG;


/*
 * Flush-LL
 */
typedef struct {

    RIL3_TLLI                                   tlli;
    RIL3_BVCI                                   bvci_old;
    RIL3_BVCI                                   bvci_new;

} BSSGP_FLUSH_LL_MSG;


/*
 * Flush-LL-ACK
 */
typedef struct {

    RIL3_TLLI                                   tlli;
    BSSGP_FLUSH_ACTION                          flush_action;
    RIL3_BVCI                                   bvci_new;
    UINT32                                      num_octets;

} BSSGP_FLUSH_LL_ACK_MSG;


/*
 * LLC-Discarded
 */
typedef struct {

    RIL3_TLLI                                   tlli;
    UINT8                                       num_frames;
    RIL3_BVCI                                   bvci;
    UINT32                                      num_octets;

} BSSGP_LLC_DISCARDED_MSG;


/*
 * Flow-Control-BVC
 */
typedef struct {

    UINT8                                       tag;
    UINT16                                      bvc_bucket_size;
    UINT16                                      bucket_leak_rate;
    UINT16                                      bmax;
    UINT16                                      r_default;
    RIL3_BVC_MEASUREMENT                        bvc_meas;

} BSSGP_FLOW_CONTROL_BVC_MSG;


/*
 * Flow-Control-BVC-ACK
 */
typedef struct {

    UINT8                                       tag;

} BSSGP_FLOW_CONTROL_BVC_ACK_MSG;


/*
 * Flow-Control-MS
 */
typedef struct {

    RIL3_TLLI                                   tlli;
    UINT8                                       tag;
    UINT16                                      ms_bucket_size;
    UINT16                                      bucket_leak_rate;

} BSSGP_FLOW_CONTROL_MS_MSG;


/*
 * Flow-Control-MS-ACK
 */
typedef struct {

    RIL3_TLLI                                   tlli;
    UINT8                                       tag;

} BSSGP_FLOW_CONTROL_MS_ACK_MSG;


/*
 * BVC-Block
 */
typedef struct {

    RIL3_BVCI                                   bvci;
    RIL3_BSSGP_CAUSE                            cause;

} BSSGP_BVC_BLOCK_MSG;


/*
 * BVC-Block-ACK
 */
typedef struct {

    RIL3_BVCI                                   bvci;

} BSSGP_BVC_BLOCK_ACK_MSG;


/*
 * BVC-Unblock
 */
typedef struct {

    RIL3_BVCI                                   bvci;

} BSSGP_BVC_UNBLOCK_MSG;


/*
 * BVC-Unblock-ACK
 */
typedef struct {

    RIL3_BVCI                                   bvci;

} BSSGP_BVC_UNBLOCK_ACK_MSG;


/*
 * BVC-Reset
 */
typedef struct {

    RIL3_BVCI                                   bvci;
    RIL3_BSSGP_CAUSE                            cause;
    RIL3_IE_CELL_IDENTIFIER                     cell_id;

} BSSGP_BVC_RESET_MSG;


/*
 * BVC-Reset-ACK
 */
typedef struct {

    RIL3_BVCI                                   bvci;
    RIL3_IE_CELL_IDENTIFIER                     cell_id;

} BSSGP_BVC_RESET_ACK_MSG;


/*
 * BVC-Status
 */
typedef struct {

    RIL3_BSSGP_CAUSE                            cause;
    RIL3_BVCI                                   bvci;

} BSSGP_BVC_STATUS_MSG;


/*
 * The main API msg; which is overloaded with
 * all the individual msgs
 */
typedef struct {

    BSSGP_API_MSG_TYPE                          msg_type;
    BSSGP_BVCI                                  bvci;
    int                                         magic;

    union {
        BSSGP_DL_UNITDATA_MSG                   dl_unitdata; /* From GS --> GP-10 */
        BSSGP_UL_UNITDATA_MSG                   ul_unitdata; /* From GP10 --> GS */
        BSSGP_PAGING_PS_MSG                     paging_ps; /* From GS --> GP-10 */
        BSSGP_RADIO_STATUS_MSG          radio_status; /* From GP-10 --> GS */
        BSSGP_SUSPEND_MSG               suspend; /* From GP-10 --> GS */
        BSSGP_SUSPEND_ACK_MSG           suspend_ack; /* From GS --> GP-10 */
        BSSGP_SUSPEND_NACK_MSG          suspend_nack; /* From GS --> GP-10 */
        BSSGP_RESUME_MSG                resume; /* From GP-10 --> GS */
        BSSGP_RESUME_ACK_MSG            resume_ack; /* From GS --> GP-10 */
        BSSGP_RESUME_NACK_MSG           resume_nack; /* From GS --> GP-10 */
        BSSGP_LOCATION_UPDATE_MSG       location_update; /* From GS --> GP-10 */
        BSSGP_LOCATION_UPDATE_ACK_MSG   location_update_ack; /* From GP-10 --> GS */
        BSSGP_LOCATION_UPDATE_NACK_MSG  location_update_nack; /* From GP-10 --> GS */
        BSSGP_RADIO_CAP_MSG             radio_cap; /* From GS --> GP-10 */
        BSSGP_RADIO_CAP_UPDATE_MSG      radio_cap_update; /* From GP-10 --> GS */
        BSSGP_RADIO_CAP_UPDATE_ACK_MSG  radio_cap_update_ack; /* From GS --> GP-10 */
        BSSGP_FLUSH_LL_MSG              flush_ll; /* From GS --> GP-10 */
        BSSGP_FLUSH_LL_ACK_MSG          flush_ll_ack; /* From GP-10 --> GS */
        BSSGP_LLC_DISCARDED_MSG         llc_discarded; /* From GP-10 --> GS */
        BSSGP_FLOW_CONTROL_BVC_MSG      flow_control_bvc;  /* From GP-10 --> GS */
        BSSGP_FLOW_CONTROL_BVC_ACK_MSG  flow_control_bvc_ack; /* From GS --> GP-10 */
        BSSGP_FLOW_CONTROL_MS_MSG       flow_control_ms;  /* From GP-10 --> GS */
        BSSGP_FLOW_CONTROL_MS_ACK_MSG   flow_control_ms_ack; /* From GS --> GP-10 */
        BSSGP_BVC_BLOCK_MSG             bvc_block; /* From GS --> GP-10 */
        BSSGP_BVC_BLOCK_ACK_MSG         bvc_block_ack; /* From GP-10 --> GS */
        BSSGP_BVC_UNBLOCK_MSG           bvc_unblock; /* From GP-10 --> GS */
        BSSGP_BVC_UNBLOCK_ACK_MSG       bvc_unblock_ack; /* From GS --> GP-10 */
        BSSGP_BVC_RESET_MSG             bvc_reset; /* From GS <--> GP-10 */
        BSSGP_BVC_RESET_ACK_MSG         bvc_reset_ack; /* From GS <--> GP-10 */
        BSSGP_BVC_STATUS_MSG            bvc_status; /* From GS <--> GP-10 */
    } msg;

} BSSGP_API_MSG;




/*
 * API function prototypes
 */

/*
 * This function below will be called by :-
 * On GP-10 : GMM1 and RLC/MAC layers
 * On GS    : GMM2 and LLC
 */
BOOL bssgp_api_send_msg(BSSGP_API_MSG msg);

/* 
 * Callback function provided by :
 *  GMM1 on GP-10
 *  GMM2 on GS
 */
BOOL gmm_bssgp_msg_callback(BSSGP_API_MSG msg);
/* GRR needs to be called for packet-switched paging msg only */
BOOL rm_bssgp_msg_callback(BSSGP_API_MSG msg);


/*
 * Common facility on both platforms to add a BVCI
 */
BSSGP_BVCI bssgp_net_add_bvci(char *peer_ip_addr, T_CNI_RIL3_IE_CELL_ID cell_id,UINT8 rac);


#if defined(MNET_GP10)
/*
 * Callbacks defined by
 * RLC/MAC on the GP-10
 */
BOOL rlc_bssgp_msg_callback(BSSGP_API_MSG msg);

#elif defined(MNET_GS)

/*
 * interface to LLC on the GS
 */
BOOL bssgp_llc_api_handler(GS_BssgpDlUnitdataReq_t dl_unitdata);

/*
 *
 */
void bssgp_api_send_ul_unitdata(TLLI tlli,T_CNI_RIL3_IE_CELL_ID cell_id,T_CNI_RIL3_IE_ROUTING_AREA_ID rai,UINT8 *pdu_data,UINT16 pdu_len,BSSGP_BVCI bvci);


UINT16 bssgp_api_get_bvc_leak_rate_by_cell_id_rac(T_CNI_RIL3_IE_CELL_ID cell_id,UINT8 rac);
BSSGP_BVCI bssgp_net_get_bvci_by_cell_id_rac(T_CNI_RIL3_IE_CELL_ID cell_id,UINT8 rac);
void bssgp_net_delete_bvci(BSSGP_BVCI bvci);
void bssgp_net_delete_bvci_by_cell_id_rac(T_CNI_RIL3_IE_CELL_ID cell_id,UINT8 rac);

#endif


#endif /* #ifndef _BSSGP_API_HDR_INC_ */