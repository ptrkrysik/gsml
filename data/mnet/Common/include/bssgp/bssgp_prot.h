/****************************************************************************************
 *                                                                                      *
 *  Copyright Cisco Systems, Inc 2000 All rights reserved                               *
 *                                                                                      *
 *  File                : bssgp_prot.h                                                  *
 *                                                                                      *
 *  Description         : Header file containing data structures & definitions for the  *
 *                        BSS-GP layer                                                  *
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

#ifndef BSSGP_PROT_HDR_INCLUDE
#define BSSGP_PROT_HDR_INCLUDE

#define BSSGP_MAGIC                     0x1234
#define BSSGP_BVCI                      unsigned short /* just an index into an array, really */
#define BSSGP_MAX_BVCI                  10
#define BSSGP_ROOT_BVCI                 0 /* for the GP-10 there is only 1 BVCI */

/*
 * The IE Identifier types
 */
typedef enum {

    BSSGP_IEI_ALIGNMENT_OCTETS      = 0x00,
    BSSGP_IEI_BMAX_DEFAULT_MS       = 0x01,
    BSSGP_IEI_BSS_AREA_IND          = 0x02,
    BSSGP_IEI_BUCKET_LEAK_RATE      = 0x03,
    BSSGP_IEI_BVCI                  = 0x04,
    BSSGP_IEI_BVC_BUCKET_SIZE       = 0x05,
    BSSGP_IEI_BVC_MEASUREMENT       = 0x06,
    BSSGP_IEI_CAUSE                 = 0x07,
    BSSGP_IEI_CELL_ID               = 0x08,
    BSSGP_IEI_CHAN_NEEDED           = 0x09,
    BSSGP_IEI_DRX_PARAMS            = 0x0a,
    BSSGP_IEI_EMLPP_PRIORITY        = 0x0b,
    BSSGP_IEI_FLUSH_ACTION          = 0x0c,
    BSSGP_IEI_IMSI                  = 0x0d,
    BSSGP_IEI_LLC_PDU               = 0x0e,
    BSSGP_IEI_LLC_FRAMES_DISCARDED  = 0x0f,
    BSSGP_IEI_LOCATION_AREA         = 0x10,
    BSSGP_IEI_MOBILE_ID             = 0x11,
    BSSGP_IEI_MS_BUCKET_SIZE        = 0x12,
    BSSGP_IEI_MS_RADIO_ACCESS_CAP   = 0x13,
    BSSGP_IEI_OMC_ID                = 0x14,
    BSSGP_IEI_PDU_IN_ERROR          = 0x15,
    BSSGP_IEI_PDU_LIFETIME          = 0x16,
    BSSGP_IEI_PRIORITY              = 0x17,
    BSSGP_IEI_QOS_PROFILE           = 0x18,
    BSSGP_IEI_RADIO_CAUSE           = 0x19,
    BSSGP_IEI_RA_CAP_UPD_CAUSE      = 0x1a,
    BSSGP_IEI_ROUTING_AREA          = 0x1b,
    BSSGP_IEI_R_DEFAULT_MS          = 0x1c,
    BSSGP_IEI_SUSPEND_REF_NUM       = 0x1d,
    BSSGP_IEI_TAG                   = 0x1e,
    BSSGP_IEI_TLLI                  = 0x1f,
    BSSGP_IEI_TMSI                  = 0x20,
    BSSGP_IEI_TRACE_REF             = 0x21,
    BSSGP_IEI_TRACE_TYPE            = 0x22,
    BSSGP_IEI_TRANSACTION_ID        = 0x23,
    BSSGP_IEI_TRIGGER_ID            = 0x24,
    BSSGP_IEI_NUM_OCTETS_AFFECTED   = 0x25,
    BSSGP_IEI_LSA_ID_LIST           = 0x26,
    BSSGP_IEI_LSA_INFORMATION       = 0x27,

    /* Cisco-EWTBU proprietary extensions */
    BSSGP_IEI_QOS_PARAMS            = 0x28,
    BSSGP_IEI_LA_REJECT_CAUSE       = 0x29,


    BSSGP_IEI_MAX_TYPES             = 0x30

} BSSGP_IEI_TYPE;

/*
 * Information Element Format
 */
typedef enum {
    BSSGP_FORMAT_V                  = 0x0,
    BSSGP_FORMAT_TLV                = 0x1
} BSSGP_IEI_FORMAT;

/*
 * PDU Types
 */
typedef enum {
    
    BSSGP_PDU_DL_UNITDATA           = 0x00,
    BSSGP_PDU_UL_UNITDATA           = 0x01,
    BSSGP_PDU_RA_CAP                = 0x02,
    BSSGP_PDU_PTM_UNITDATA          = 0x03,

    BSSGP_PDU_PAGING_PS             = 0x06,
    BSSGP_PDU_PAGING_CS             = 0x07,
    BSSGP_PDU_RA_CAP_UPDATE         = 0x08,
    BSSGP_PDU_RA_CAP_UPDATE_ACK     = 0x09,
    BSSGP_PDU_RADIO_STATUS          = 0x0a,
    BSSGP_PDU_SUSPEND               = 0x0b,
    BSSGP_PDU_SUSPEND_ACK           = 0x0c,
    BSSGP_PDU_SUSPEND_NACK          = 0x0d,
    BSSGP_PDU_RESUME                = 0x0e,
    BSSGP_PDU_RESUME_ACK            = 0x0f,
    BSSGP_PDU_RESUME_NACK           = 0x10,

    BSSGP_PDU_BVC_BLOCK             = 0x20,
    BSSGP_PDU_BVC_BLOCK_ACK         = 0x21,
    BSSGP_PDU_RESET                 = 0x22,
    BSSGP_PDU_RESET_ACK             = 0x23,
    BSSGP_PDU_UNBLOCK               = 0x24,
    BSSGP_PDU_UNBLOCK_ACK           = 0x25,
    BSSGP_PDU_FLOW_CONTROL_BVC      = 0x26,
    BSSGP_PDU_FLOW_CONTROL_BVC_ACK  = 0x27,
    BSSGP_PDU_FLOW_CONTROL_MS       = 0x28,
    BSSGP_PDU_FLOW_CONTROL_MS_ACK   = 0x29,
    BSSGP_PDU_FLUSH_LL              = 0x2a,
    BSSGP_PDU_FLUSH_LL_ACK          = 0x2b,
    BSSGP_PDU_LLC_DISCARDED         = 0x2c,

    BSSGP_PDU_LOCATION_UPDATE       = 0x30,
    BSSGP_PDU_LOCATION_UPDATE_ACK   = 0x31,
    BSSGP_PDU_LOCATION_UPDATE_NACK  = 0x32,
    BSSGP_PDU_ROUTING_AREA_UPDATE   = 0x33,

    BSSGP_PDU_SGSN_INVOKE_TRACE     = 0x40,
    BSSGP_PDU_STATUS                = 0x42,
    BSSGP_PDU_MAX_TYPES             = 0x43

} BSSGP_PDU_TYPE;


/*
 * The QOS levels on the
 * network side
 */
typedef enum {

    BSSGP_NETWORK_QOS_LEVEL_1   = 0,
    BSSGP_NETWORK_QOS_LEVEL_2   = 1,
    BSSGP_NETWORK_QOS_LEVEL_3   = 2,
    BSSGP_NETWORK_QOS_LEVEL_4   = 3,
    BSSGP_MAX_NETWORK_QOS_LEVELS = 4

} BSSGP_NETWORK_QOS_LEVEL;


/*
 * Radio-Access-Capability-Update Cause
 */
typedef enum {
    
    RA_CAP_UPD_CAUSE_IE_PRESENT         = 0x00,
    RA_CAP_UPD_CAUSE_TLLI_UNKNOWN       = 0x01,
    RA_CAP_UPD_CAUSE_NO_RA_CAP          = 0x02

} RA_CAP_UPD_CAUSE;


/*
 * Flush Action
 */
typedef enum {
    
    BSSGP_LLC_PDU_DELETED               = 0x00,
    BSSGP_LLC_PDU_TRANSFERRED           = 0x01

} BSSGP_FLUSH_ACTION;





#endif /* #ifndef BSSGP_PROT_HDR_INCLUDE */