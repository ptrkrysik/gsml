/****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: bssgp_util.h													*
 *																						*
 *	Description			: Header file containing function prototypes of the utility 	*
 *						  module within BSSGP layer										*
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

#ifndef BSSGP_UTIL_HDR_INCLUDE
#define BSSGP_UTIL_HDR_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bssgp_prot.h"
#include "bssgp_api.h"


#if defined(MNET_GS)

#define BSSGP_LAYER				GS_LOG_BSSGP
#define BSSGP_ENCODER_LAYER		GS_LOG_BSSGP_ENCODER
#define BSSGP_DECODER_LAYER		GS_LOG_BSSGP_DECODER
#define BSSGP_NET_LAYER			GS_LOG_BSSGP_NET
#define BSSGP_UNITDATA_LAYER 	GS_LOG_BSSGP_UNITDATA

#include "logmodules.h"
#include "vclogging.h"


#elif defined(MNET_GP10)


#include "logging/vcmodules.h"
#include "logging/vclogging.h"


#elif defined(WIN32)

#include "dbgout.h"
#include <winsock.h>

#define INET_ADDR_LEN		20

/* This to be defined in the driving stub */
void bssgp_api_send_ul_unitdata(TLLI tlli,T_CNI_RIL3_IE_CELL_ID cell_id,UINT8 *pdu_data,UINT16 pdu_len);



#endif

#ifndef ASSERT
#define ASSERT	assert
#endif

#define bssgp_assert_on_not_implemented		1
#define bssgp_assert_on_invalid_msg			2



typedef struct {
#if defined(MNET_GS)
	int								magic;
#elif defined(MNET_GP10)
	T_CNI_RIL3_IE_CELL_ID 			cell_id;
	T_CNI_RIL3_IE_ROUTING_AREA_ID	routing_area_id;
#endif	
} BSSGP_CONFIGURATION;

/*
 * Function prototypes
 */
char *bssgp_util_get_iei_str(BSSGP_IEI_TYPE iei_type);
char *bssgp_util_get_pdu_type_str(BSSGP_PDU_TYPE pdu_type);
char *bssgp_util_get_api_msg_str(BSSGP_API_MSG_TYPE api_msg);

char *bssgp_util_get_radio_cause_str(UINT8 radio_cause);
char *bssgp_util_get_exception_cause_str(BSSGP_EXCEPTION_CAUSE cause);
char *bssgp_util_get_ra_cap_cause_str(RA_CAP_UPD_CAUSE ra_cause);

BOOL bssgp_util_init_rx_buffs(void);
char *bssgp_util_get_rx_buff(int reqd_buff_size);
void bssgp_util_return_rx_buff(char *);
int bssgp_util_get_rx_buff_size();
#endif /* #ifndef BSSGP_UTIL_HDR_INCLUDE */