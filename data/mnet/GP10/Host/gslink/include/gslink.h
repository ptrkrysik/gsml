/*
 ****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: gslink.h  													*
 *																						*
 *	Description			: The GSLink subsystem [Link between GP-10 & the GS]			*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |01/25/01 | DSN    | File created												*
 *		 |		   |		|															*
 ****************************************************************************************
 */
#ifndef GSLINK_HDR_INCLUDE
#define GSLINK_HDR_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <vxworks.h>
#include <string.h>
#include <assert.h>
#include "ril3/ie_cell_id.h"
#include "ril3/ie_location_area_id.h"
#include "ril3/ril3_gmm_msg.h"
#include "AlarmCode.h"
#include "logging/vcmodules.h"
#include "logging/vclogging.h"
#include "gplink/gplink_prot.h"

/* common for all platforms */
#include "Os/JCModule.h"
#include "Os/JCTask.h"

/* common for GS */
#include "GP10OsTune.h"
#include "GP10MsgTypes.h"
#include "MnetModuleId.h"


#define GSLINK_MAX_IP_STRLEN        32

typedef enum {

	GSLINK_LINK_CONNECTED,
	GSLINK_LINK_DOWN

} GSLINK_LINK_EVENT;



typedef struct {
    T_CNI_RIL3_IE_CELL_ID           cell_id;
    T_CNI_RIL3_IE_LOCATION_AREA_ID  location_area_id;
    T_CNI_RIL3_IE_ROUTING_AREA_ID   routing_area_id;
    char                            gs_addr_str[GSLINK_MAX_IP_STRLEN];
    BOOL                            connected;
    BOOL                            registered;
    int                             client_socket;
} GSLINK_CONFIGURATION;

/*
 * gslink_initialize_client
 */
BOOL gslink_initialize_client(void);
void gslink_client_task(void);
BOOL gslink_register_client(void);
BOOL gslink_is_ack(UINT8 *rx_buff,int len);
void gslink_retry_connect(void);
BOOL gslink_is_socket_disconnected(int test_socket);
void gslink_set_oam_trap(GSLINK_LINK_EVENT event);
char *gslink_util_get_event_str(GPLINK_EVENT event);

#endif /* #ifndef GSLINK_HDR_INCLUDE */