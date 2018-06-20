/*
 ****************************************************************************************
 *                                                                                      *
 *    Copyright Cisco Systems, Inc 2000 All rights reserved                             *
 *                                                                                      *
 *    File                : gglink_api.h                                                *
 *                                                                                      *
 *    Description            : API data structures for the SGSN-GMC layer module        *
 *                                                                                      *
 *    Author                : Dinesh Nambisan                                           *
 *                                                                                      *
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description                                               *
 *--------------------------------------------------------------------------------------*
 * 00:00 |12/14/00 | DSN    | File created                                              *
 *       |         |        |                                                           *
 ****************************************************************************************
 */
#ifndef _GGLINK_API_HDR_INC_
#define _GGLINK_API_HDR_INC_

#include <stdio.h>
#include <stdlib.h>

#include "subsc/SubscInfoType.h"
#include "subsc/SubscGPRSInfoType.h"

/*
 * RIL3 (4.08) includes
 */
#include "ril3/ie_tlli.h"
#include "ril3/ie_mobile_id.h"
#include "ril3/ie_location_area_id.h"
#include "ril3/ie_qos_profile.h"
#include "ril3/ril3_gmm_msg.h"
#include "ril3/ie_reject_cause.h"
#include "ril3/ie_cell_id.h"

#define GGLINK_API_MAGIC_VALUE        1111

#define MAX_GSN_ADDR_LENGTH            16

/*
 * GGLINK API message type
 */
typedef enum {

    GGLINK_API_MSG_GS_SRQ,                /* GS -> GMC */
    GGLINK_API_MSG_GS_SRS,                /* GMC -> GS */
    GGLINK_API_MSG_GS_RRQ,                /* GS -> GMC */
    GGLINK_API_MSG_GS_RCF,                /* GMC -> GS */
    GGLINK_API_MSG_GS_RRJ,                /* GMC -> GS */
    GGLINK_API_MSG_GS_PRQ,                /* GS -> GMC */
    GGLINK_API_MSG_GS_PRS,                /* GMC -> GS */
    GGLINK_API_MSG_GS_URQ,                /* GMC -> GS */
    GGLINK_API_MSG_SRI_REQ,               /* GS -> GMC , send routing info*/
    GGLINK_API_MSG_SRI_ACK,               /* GMC -> GS , send routing info*/
    GGLINK_API_MSG_FR,                    /* GS -> GMC , failure report */
    GGLINK_API_MSG_FR_ACK,                /* GMC -> GS , failure report Ack*/
    GGLINK_API_MSG_GPRS_PRE,              /* GMC -> GS , Note MS GPRS Present */
    GGLINK_API_MSG_GPRS_PRE_ACK,          /* GS -> GMC , Note MS GPRS Present Ack*/  
    GGLINK_API_MSG_SSRI_REQ,              /* GS -> GMC , Send SGSN Routing Info Request */
    GGLINK_API_MSG_SSRI_RSP,              /* GMC -> GS , Send SGSN Routing Info Response */    
    GGLINK_API_MSG_ECHO_REQ,              /* GS -> GMC */
    GGLINK_API_MSG_ECHO_RSP,              /* GMC -> GS */
    GGLINK_API_MAX_MSGS

} GGLINK_API_MSG_TYPE;


#define GGLINK_CAUSE_REQ_ACCEPTED                  0    
#define GGLINK_CAUSE_NO_RESOURCES_AVAIL            1
#define GGLINK_CAUSE_SERVICE_NOT_SUPPORTED         2
#define GGLINK_CAUSE_SYSTEM_FAILURE                3
#define GGLINK_CAUSE_UNKNOWN_ROUTING_AREA          4


/*
 *
 */
typedef struct {
    bool                ie_present;
    unsigned char       cause_code;    
} GGLINK_IE_CAUSE;

/*
 *
 */
typedef struct {
    bool                ie_present;
    unsigned char       map_error_code;    
} GGLINK_IE_MAP_CAUSE;


/*
 *
 */
typedef struct {
    bool             ie_present;
    short            gsn_addr_length;    // will we have an IE for GTP interface?
    unsigned char    addr_octets[MAX_GSN_ADDR_LENGTH];
} GGLINK_IE_GSN_ADDRESS;


/*
 * GGLINK_GS_SRQ_MSG
 */

typedef struct {
    T_CNI_RIL3_IE_MOBILE_ID            imsi;
} GGLINK_GS_SRQ_MSG;

/*
 * GGLINK_GS_SRS_MSG
 */

typedef struct {
    T_CNI_RIL3_IE_MOBILE_ID           imsi;
    T_SEC_IE_AUTH_LIST                triplet_list;    
} GGLINK_GS_SRS_MSG;

/*
 * GGLINK_GS_RRQ_MSG
 */

typedef struct {
    T_CNI_RIL3_IE_MOBILE_ID           imsi;
    T_CNI_RIL3_IE_ROUTING_AREA_ID     rai;
    // SGSN number
    // SGSN address
} GGLINK_GS_RRQ_MSG;

/*
 * GGLINK_GS_RCF_MSG
 */

typedef struct {
    T_CNI_RIL3_IE_MOBILE_ID            imsi;
} GGLINK_GS_RCF_MSG;

/*
 * GGLINK_GS_RRJ_MSG
 */

typedef struct {
    T_CNI_RIL3_IE_MOBILE_ID            imsi;
    T_CNI_RIL3_REJECT_CAUSE_VALUE      cause;
} GGLINK_GS_RRJ_MSG;

/*
 * GGLINK_GS_PRQ_MSG
 */

typedef struct {
    T_CNI_RIL3_IE_MOBILE_ID            imsi;

} GGLINK_GS_PRQ_MSG;

/*
 * GGLINK_GS_PRS_MSG
 */

typedef struct {
    T_CNI_RIL3_IE_MOBILE_ID            imsi;
    short                              data_size;
    char                               data[1024];
} GGLINK_GS_PRS_MSG;

/*
 * GGLINK_GS_URQ_MSG
 */

typedef struct {
    T_CNI_RIL3_IE_MOBILE_ID            imsi;

} GGLINK_GS_URQ_MSG;

/*
 * GGLINK_SRI_REQ_MSG
 */

typedef struct {
    // send routing info
    short                              gsn_seqno;
    T_CNI_RIL3_IE_MOBILE_ID            imsi;
    GGLINK_IE_GSN_ADDRESS              ggsn_addr;
} GGLINK_SRI_REQ_MSG;

/*
 * GGLINK_SRI_ACK_MSG
 */

typedef struct {
    // send routing info ack
    short                              gsn_seqno;
    T_CNI_RIL3_IE_MOBILE_ID            imsi;
    GGLINK_IE_CAUSE                    cause;
    GGLINK_IE_MAP_CAUSE                map_cause;
    GGLINK_IE_GSN_ADDRESS              sgsn_addr;
    GGLINK_IE_GSN_ADDRESS              ggsn_addr;
} GGLINK_SRI_ACK_MSG;

/*
 * GGLINK_FR_MSG
 */

typedef struct {
    // failure report
    short                              gsn_seqno;
    T_CNI_RIL3_IE_MOBILE_ID            imsi;
} GGLINK_FR_MSG;

/*
 * GGLINK_FR_ACK_MSG
 */

typedef struct {
    // failure report ack
    short                              gsn_seqno;
    T_CNI_RIL3_IE_MOBILE_ID            imsi;
    GGLINK_IE_CAUSE                    cause;
    GGLINK_IE_MAP_CAUSE                map_cause;
} GGLINK_FR_ACK_MSG;

/*
 * GGLINK_MS_GPRS_PRE_MSG
 */

typedef struct {
    // Note MS GPRS PRESENT
    T_CNI_RIL3_IE_MOBILE_ID            imsi;
    GGLINK_IE_GSN_ADDRESS              gsn_addr;
} GGLINK_MS_GPRS_PRE_MSG;

/*
 * GGLINK_MS_GPRS_PRE_ACK_MSG
 */

typedef struct {
    // Note MS GPRS PRESENT ack
    T_CNI_RIL3_IE_MOBILE_ID            imsi;
    GGLINK_IE_GSN_ADDRESS              gsn_addr;
    GGLINK_IE_CAUSE                    cause;
} GGLINK_MS_GPRS_PRE_ACK_MSG;


/*
 * Send SGSN Routing Info Request
 */

typedef struct {
    // send sgsn routing info
    short                             gsn_seqno;
    T_CNI_RIL3_IE_ROUTING_AREA_ID     rai;
} GGLINK_SSRI_REQ_MSG;


/*
 * Send SGSN Routing Info Response
 */

typedef struct {
    // send sgsn routing info response
    short                              gsn_seqno;
    GGLINK_IE_CAUSE                    cause;
    T_CNI_RIL3_IE_ROUTING_AREA_ID      rai;
    GGLINK_IE_GSN_ADDRESS              sgsn_addr;
} GGLINK_SSRI_RSP_MSG;

/*
 * The main API msg; which is overloaded with
 * all the individual msgs
 */
typedef struct {

    GGLINK_API_MSG_TYPE                msg_type;
    int                                magic;
    union {
        GGLINK_GS_SRQ_MSG            security_req;
        GGLINK_GS_SRS_MSG            security_rsp;
        GGLINK_GS_RRQ_MSG            ra_update_req;
        GGLINK_GS_RCF_MSG            ra_update_cnf;
        GGLINK_GS_RRJ_MSG            ra_update_rej;
        GGLINK_GS_PRQ_MSG            profile_req;
        GGLINK_GS_PRS_MSG            profile_rsp;
        GGLINK_GS_URQ_MSG            unregister_req;

        // for location management
        GGLINK_SRI_REQ_MSG            send_routing_info_req;
        GGLINK_SRI_ACK_MSG            send_routing_info_ack;
        GGLINK_FR_MSG                 fr_req;
        GGLINK_FR_ACK_MSG             fr_ack;
        GGLINK_MS_GPRS_PRE_MSG        ms_gprs_pre_req;
        GGLINK_MS_GPRS_PRE_ACK_MSG    ms_gprs_pre_ack;

        // for SGSN-SGSN routing Area  information
        GGLINK_SSRI_REQ_MSG           ssri_req;
        GGLINK_SSRI_RSP_MSG           ssri_rsp;   
    } msg;

} GGLINK_API_MSG;



BOOL DecodeGGLinkMessage(
    int                                    &maxlength,        // max length of input buffer
    unsigned char *                        &ptr,            // starting address of input buffer
    GGLINK_API_MSG                        &msg            // output msg data structure
    );
BOOL EncodeGGLinkMessage(
    int                                    &maxlength,        // max length of output buffer
    unsigned char *                        &ptr,            // starting address of output buffer
    GGLINK_API_MSG                        &msg            // input msg data structure
    );
/*
 * API function prototypes
 */

/*
 * This function below will be called by :-
 * On GMC : MAP interface layers
 * On GS  : GMM2
 */
BOOL gglink_api_send_msg(GGLINK_API_MSG &msg);

/* 
 * Callback function provided by :
 *  MAP IF on GMC 
 *  GMM2 on GS
 */
BOOL gglink_msg_callback(GGLINK_API_MSG msg);
BOOL gglink_gsn_msg_callback(GGLINK_API_MSG &msg);



#endif /* #ifndef _GGLINK_API_HDR_INC_ */
