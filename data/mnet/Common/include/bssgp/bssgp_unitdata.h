/****************************************************************************************
 *                                                                                      *
 *  Copyright Cisco Systems, Inc 2000 All rights reserved                               *
 *                                                                                      *
 *  File                : unitdata.h                                                    *
 *                                                                                      *
 *  Description         : Header file containing data structures & definitions for      *
 *                        handling the unitdata buffers                                 *
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

#ifndef BSSGP_UNITDATA_HDR_INCLUDE
#define BSSGP_UNITDATA_HDR_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include "bssgp_prot.h"
#include "bssgp_util.h"
#include "bssgp_api.h"



/*
 * Maximum number of pre-allocated buffers
 */
#define BSSGP_UNITDATA_MAX_BUFFERS              10240
/*
 * Maximum number of pre-allocated TLLI lists
 */
#define BSSGP_UNITDATA_MAX_TLLI_LISTS           75
/*
 * Its Magic !!!
 */
#define BSSGP_UNITDATA_MAGIC                    0x2222

/*
 * The basic structure to hold information
 * about the UNITDATA PDU. This will be in a doubly-linked
 * list on a per-QoS-level per TLLI basis
 */
typedef struct _UDT_BUFFER_{

    struct _UDT_BUFFER_     *prev;
    struct _UDT_BUFFER_     *next;
    UINT32                  magic;
    UINT32                  index;
    UINT32                  data_size;
    BSSGP_DL_UNITDATA_MSG   dl_unitdata;
    void                    *data; /* Whatever, placeholder, for now */

} BSSGP_UDT_BUFFER,*PBSSGP_UDT_BUFFER;

/*
 * the QoS queue structure; each QoS Q will hold
 * a doubly linked list of UDT buffers (above) for
 * a particular QoS level. There should be 4/four of
 * these QoS Qs per TLLI Q/List
 */
typedef struct {

    UINT32                      buff_count;
    UINT32                      rx_bytes;
    BSSGP_NETWORK_QOS_LEVEL     qos_level;
    PBSSGP_UDT_BUFFER           buff_list_head;
    PBSSGP_UDT_BUFFER           buff_list_tail;

} BSSGP_UDT_QOS_Q, *PBSSGP_UDT_QOS_Q;


/*
 * the TLLI Q structure; a doubly-linked list which
 * is allocated per TLLI, each of which holds four/4
 * QoS queues of UNITDATA PDUs on the QoS level basis
 */
typedef struct _TLLI_Q_ {

    struct _TLLI_Q_ *prev;
    struct _TLLI_Q_ *next;
    TLLI                    tlli;
    UINT32                  magic;
    UINT32                  index;
    UINT32                  buff_count;
    UINT32                  rx_bytes;
    
    BOOL                    high_watermark_indicated;
    
    BSSGP_UDT_QOS_Q         qos_queues[BSSGP_MAX_NETWORK_QOS_LEVELS];


} BSSGP_UDT_TLLI_Q,*PBSSGP_UDT_TLLI_Q;


/*
 * The basic structure to hold information
 * used to implement a PDU lifetime
 */
typedef struct _UDT_TIMEOUT_BUFFER_{

    BSSGP_UDT_BUFFER        *prev;
    BSSGP_UDT_BUFFER        *next;
    UINT32                  timeoutTicks;
    BOOL                    used;

} BSSGP_UDT_TIMEOUT_BUFFER, *PBSSGP_UDT_TIMEOUT_BUFFER;



/*
 * Wrapper for the TLLI_Q structure to
 * allocate/deallocate into a pool
 */
typedef struct {

    PBSSGP_UDT_TLLI_Q   entry;
    BOOL                used;

} BSSGP_UDT_TLLI_POOL_ENTRY, *PBSSGP_UDT_TLLI_POOL_ENTRY;


/*
 * Wrapper for the UDT_BUFFER structure to
 * allocate/deallocate into a pool
 */
typedef struct {

    PBSSGP_UDT_BUFFER   entry;
    BOOL                used;

} BSSGP_UDT_BUFF_POOL_ENTRY, *PBSSGP_UDT_BUFF_POOL_ENTRY;




typedef void (*BSSGP_UDT_WATERMARK_CB)(TLLI tlli,UINT32 rx_bytes,UINT32 rx_buffs,BOOL high);




/*
 * The core UNITDATA configuration structure
 * for holding configuration information for
 * the unitdata buffer queuing mechanisms
 */
typedef struct {
                                /*
                                 * The num of UDT buffers to be pre-allocated
                                 * and placed into the pool
                                 */
    UINT32                      udt_buff_count;

                                /*
                                 * The num of TLLI Qs/Lists to be pre-allocated
                                 * and placed into the pool
                                 */
    UINT32                      tlli_list_count;

    UINT32                      tlli_high_watermark_bytes; 
                                /* 
                                 * indicates the num of bytes, which if 
                                 * exceeded, flow-control should be enabled 
                                 */

    UINT32                      tlli_low_watermark_bytes;
                                /* 
                                 * indicates the num of bytes, which if the list 
                                 * count falls below, flow-control can be 
                                 * disabled 
                                 */

    PBSSGP_UDT_TLLI_Q           tlli_q_head; /* head of the TLLI doubly-linked list */
    PBSSGP_UDT_TLLI_Q           tlli_q_tail; /* tail of the TLLI doubly-linked list */

                                /*
                                 * Pools for holding TLLI Lists and UDT Buffers
                                 *
                                 */
    BSSGP_UDT_TLLI_POOL_ENTRY   *tlli_pool;
                                
    BSSGP_UDT_BUFF_POOL_ENTRY   *udt_buff_pool;
    
    BSSGP_UDT_TIMEOUT_BUFFER    **udt_timeout_pool;

                                /*
                                 * Callback functions to be called when the high or
                                 * low watermarks are reached for a particular TLLI
                                 */
    BSSGP_UDT_WATERMARK_CB      watermark_cb;

} BSSGP_UNITDATA_CONFIG;


/*
 * Function prototypes
 */
BOOL bssgp_udt_initialize(int tlli_list_count,int bssgp_udt_buffer_count,BSSGP_UDT_WATERMARK_CB watermark_cb);
BOOL bssgp_udt_put_buffer_in_q(TLLI tlli,BSSGP_NETWORK_QOS_LEVEL qos_level,UINT8 *buff, int buff_size,BSSGP_DL_UNITDATA_MSG dl_unitdata);
UINT8 *bssgp_udt_get_buff_by_tlli(TLLI tlli,BSSGP_DL_UNITDATA_MSG*);
UINT8 *bssgp_udt_get_buff_by_qos(BSSGP_NETWORK_QOS_LEVEL qos_level,BSSGP_DL_UNITDATA_MSG*);
UINT8 *bssgp_udt_get_buff_by_tlli_qos(TLLI tlli, BSSGP_NETWORK_QOS_LEVEL qos_level,BSSGP_DL_UNITDATA_MSG*);
BOOL bssgp_udt_check_buff_by_tlli(TLLI tlli, BSSGP_DL_UNITDATA_MSG **dl_unitdata);
BOOL bssgp_udt_check_buff_by_qos(BSSGP_NETWORK_QOS_LEVEL qos_level, BSSGP_DL_UNITDATA_MSG **dl_unitdata);
void bssgp_udt_flush_tlli(TLLI tlli);
BOOL bssgp_udt_allocate_buff_pool();
BOOL bssgp_udt_free_buff_pool();
PBSSGP_UDT_BUFFER bssgp_udt_get_buff_from_pool();
void bssgp_udt_return_buff_to_pool(PBSSGP_UDT_BUFFER bssgp_udt_buff);
BOOL bssgp_udt_allocate_tlli_pool();
BOOL bssgp_udt_free_tlli_pool();
PBSSGP_UDT_TLLI_Q bssgp_udt_get_tlli_list_from_pool();
void bssgp_udt_return_tlli_list_to_pool(PBSSGP_UDT_TLLI_Q tlli_q);
void bssgp_udt_free_qos_list_buffs(PBSSGP_UDT_QOS_Q qos_q, UINT32 *bytes_freed, UINT32 *buffs_freed);

#endif /* #ifndef BSSGP_UNITDATA_HDR_INCLUDE */