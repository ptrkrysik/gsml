/*
 ****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: bssgp_net.h													*
 *																						*
 *	Description			: Network handling functions for the BSS-GP layer module		*
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
#ifndef BSSGP_NET_HDR_INCLUDE
#define BSSGP_NET_HDR_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include "bssgp_prot.h"
#include "bssgp_util.h"
#include "bssgp_api.h"
#include "ril3/ie_cell_id.h"

typedef struct {

	BOOL					used;
	char					peer_ip_addr[INET_ADDR_LEN];
	T_CNI_RIL3_IE_CELL_ID	cell_id;
	UINT8					routing_area_code;
    UINT16                  leak_rate;
	BSSGP_BVCI				peer_bvci;

} BSSGP_BVCI_ENTRY;


typedef struct {

	int						socks[BSSGP_MAX_NETWORK_QOS_LEVELS];
	unsigned short			ports[BSSGP_MAX_NETWORK_QOS_LEVELS];
	unsigned short			base_port;
	BOOL					continue_processing;

	BSSGP_BVCI_ENTRY		bvci_list[BSSGP_MAX_BVCI];

	int						init_magic;

} BSSGP_NET_CONFIG;
	

/*
 * Function prototypes
 */
BOOL bssgp_net_initialize(void);
void bssgp_net_task(void);
void bssgp_net_rx_handler(int index,int rx_bytes);
BOOL bssgp_net_init_bvcis(void);
BSSGP_BVCI bssgp_net_get_bvci_by_ip(char *ip_addr);
char *bssgp_net_get_ip_by_bvci(BSSGP_BVCI bvci);
#if defined(MNET_GS)
/*
 ******************************************************************
 *  Network Functions specific to the GS Platform
 ******************************************************************
 */
BOOL bssgp_net_tx_msg(BSSGP_BVCI bvci,BSSGP_NETWORK_QOS_LEVEL qos_level,UINT8 *msg,UINT16 msg_len);

#elif defined(MNET_GP10)
/*
 ******************************************************************
 *  Network Functions specific to the GP-10 Platform
 ******************************************************************
 */
BOOL bssgp_net_tx_msg(BSSGP_NETWORK_QOS_LEVEL qos_level,UINT8 *msg,UINT16 msg_len);

#endif


#endif /* #ifndef BSSGP_NET_HDR_INCLUDE */