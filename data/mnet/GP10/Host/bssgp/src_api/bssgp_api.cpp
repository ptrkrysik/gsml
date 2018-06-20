/*
 ****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: bssgp_api.cpp													*
 *																						*
 *	Description			: API handler functions for the BSS-GP layer module				*
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
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "bssgp/bssgp_prot.h"
#include "bssgp/bssgp_util.h"
#include "bssgp/bssgp_api.h"
#include "bssgp/bssgp_encoder.h"
#include "bssgp/bssgp_net.h"

/* common for all platforms */
#include "Os/JCModule.h"
#include "Os/JCTask.h"
#include "Os/JCMsgQueue.h"

/* common for GS */
#include "GP10OsTune.h"
#include "GP10MsgTypes.h"
#include "MnetModuleId.h"





/*
 * Global variables are declared here
 */
MSG_Q_ID bssgp_api_msg_q;
BOOL g_bssgp_shutdown=FALSE, g_bssgp_api_initialized = FALSE;
BSSGP_NET_CONFIG bssgp_net_config;
BSSGP_CONFIGURATION     bssgp_config;


/*
 * !!!!! FIXME : FIXME : FIXME : FIXME : FIXME : FIXME : FIXME : FIXME !!!!!
 *
 * Right now we are going to allocate memory buffers ourselves here, but
 * in a short while we should change to using ZBufs
 *
 */
#define BSSGP_UTIL_MAX_BUFFS_PER_SIZE       100
#define BSSGP_BUFF_MAGIC                    0x456

typedef struct {

    BOOL        used;
    int         magic;
    UINT8       buff[256];

} BSSGP_RX_BUFF_256;


typedef struct {

    BOOL        used;
    int         magic;
    UINT8       buff[512];

} BSSGP_RX_BUFF_512;

typedef struct {

    BOOL        used;
    int         magic;
    UINT8       buff[1500];

} BSSGP_RX_BUFF_1500;


static BSSGP_RX_BUFF_256 *rx_buffs_256[BSSGP_UTIL_MAX_BUFFS_PER_SIZE];
static BSSGP_RX_BUFF_512 *rx_buffs_512[BSSGP_UTIL_MAX_BUFFS_PER_SIZE];
static BSSGP_RX_BUFF_1500 *rx_buffs_1500[BSSGP_UTIL_MAX_BUFFS_PER_SIZE];



/*
 * bssgp_api_send_msg
 *
 * Notes : main api function for sending msgs via
 * BSSGP from GS to GP-10 and vice versa
 */
BOOL 
bssgp_api_send_msg(BSSGP_API_MSG msg)
{
	DBG_FUNC("bssgp_api_send_msg",BSSGP_LAYER);
	DBG_ENTER();
	BOOL status = FALSE;

    if (g_bssgp_api_initialized == FALSE) {
        DBG_ERROR("BSSGP API not initialized yet!!");
        DBG_LEAVE();
        return(status);
    }

	if (msg.magic != BSSGP_MAGIC) {
		DBG_ERROR("BSSGP API message invalid!");
		DBG_LEAVE();
		return (status);
	}
	
	if ((msg.msg_type < BSSGP_API_MSG_DL_UNITDATA) || (msg.msg_type >= BSSGP_API_MAX_MSGS)) {
	    DBG_TRACE("Invalid BSSGP API Message %d received",msg.msg_type);
        DBG_LEAVE();
        return (FALSE);
    }
    else {
	    DBG_TRACE("Received BSSGP API Message %s",bssgp_util_get_api_msg_str(msg.msg_type));
    }



    status = TRUE;
    /* fixme : need to check msgqsend status */
	msgQSend(bssgp_api_msg_q,(char *)&msg,sizeof(BSSGP_API_MSG),NO_WAIT,MSG_PRI_NORMAL);

	DBG_LEAVE();
	return (status);
}









/*
 * bssgp_util_init_rx_buffs
 */
BOOL
bssgp_util_init_rx_buffs()
{
    DBG_FUNC("bssgp_util_init_rx_buffs",BSSGP_LAYER);
    DBG_ENTER();
    int count,reverse_count;


    for(count=0;count<BSSGP_UTIL_MAX_BUFFS_PER_SIZE;count++) {
    
        rx_buffs_256[count] = (BSSGP_RX_BUFF_256 *)malloc(sizeof(BSSGP_RX_BUFF_256));
        if (rx_buffs_256[count] == NULL) {
            DBG_ERROR("Unable to allocate 256 byte buffer number %d for buff pool",count);
            for(reverse_count=count-1;reverse_count>=0;reverse_count--) {
                free(rx_buffs_256[reverse_count]);
            }
            DBG_LEAVE();
            return (FALSE);
        }
        rx_buffs_256[count]->used       = FALSE;
        rx_buffs_256[count]->magic      = BSSGP_BUFF_MAGIC;

        rx_buffs_512[count] = (BSSGP_RX_BUFF_512 *)malloc(sizeof(BSSGP_RX_BUFF_512));
        if (rx_buffs_512[count] == NULL) {
            DBG_ERROR("Unable to allocate 512 byte buffer number %d for buff pool",count);
            free(rx_buffs_256[count]);
            for(reverse_count=count-1;reverse_count>=0;reverse_count--) {
                free(rx_buffs_512[reverse_count]);
                free(rx_buffs_256[reverse_count]);
            }
            DBG_LEAVE();
            return (FALSE);
        }
        rx_buffs_512[count]->used       = FALSE;
        rx_buffs_512[count]->magic      = BSSGP_BUFF_MAGIC;


        rx_buffs_1500[count] = (BSSGP_RX_BUFF_1500 *)malloc(sizeof(BSSGP_RX_BUFF_1500));
        if (rx_buffs_1500[count] == NULL) {
            DBG_ERROR("Unable to allocate 1500 byte buffer number %d for buff pool",count);
            free(rx_buffs_256[count]);
            free(rx_buffs_512[count]);
            for(reverse_count=count-1;reverse_count>=0;reverse_count--) {
                free(rx_buffs_1500[reverse_count]);
                free(rx_buffs_512[reverse_count]);
                free(rx_buffs_256[reverse_count]);
            }
            DBG_LEAVE();
            return (FALSE);
        }
        rx_buffs_1500[count]->used      = FALSE;
        rx_buffs_1500[count]->magic     = BSSGP_BUFF_MAGIC;

    
    }
    DBG_LEAVE();
    return (TRUE);
}




/*
 * bssgp_util_get_rx_buff
 */
char *
bssgp_util_get_rx_buff(int buff_size_reqd)
{
    DBG_FUNC("bssgp_util_get_rx_buff",BSSGP_LAYER);
    DBG_ENTER();
    int count;
    if (buff_size_reqd <= 256) {
        for(count=0;count<BSSGP_UTIL_MAX_BUFFS_PER_SIZE;count++) {
            if (rx_buffs_256[count]->used == FALSE) {
                rx_buffs_256[count]->used   = TRUE;
                rx_buffs_256[count]->magic  = BSSGP_BUFF_MAGIC;
                DBG_LEAVE();
                return ((char*)rx_buffs_256[count]->buff);
            }
        }
    }
    if (buff_size_reqd <= 512) {
        for(count=0;count<BSSGP_UTIL_MAX_BUFFS_PER_SIZE;count++) {
            if (rx_buffs_512[count]->used == FALSE) {
                rx_buffs_512[count]->used   = TRUE;
                rx_buffs_512[count]->magic  = BSSGP_BUFF_MAGIC;
                DBG_LEAVE();
                return ((char*)rx_buffs_512[count]->buff);
            }
        }
    }
    
    if (buff_size_reqd <= 1500) {
        for(count=0;count<BSSGP_UTIL_MAX_BUFFS_PER_SIZE;count++) {
            if (rx_buffs_1500[count]->used == FALSE) {
                rx_buffs_1500[count]->used   = TRUE;
                rx_buffs_1500[count]->magic = BSSGP_BUFF_MAGIC;
                DBG_LEAVE();
                return ((char*)rx_buffs_1500[count]->buff);
            }
        }
    }

    if (buff_size_reqd > 1500) {
        DBG_ERROR("Requesting buffer size greater than 1500; %d",buff_size_reqd);
    }
    else {
        DBG_ERROR("Exhausted all buffers in memory pool");
    }
    
    DBG_LEAVE();
    return ((char*)NULL);
}


/*
 * bssgp_util_return_rx_buff
 */
void 
bssgp_util_return_rx_buff(char *buff)
{
    int negative_offset;
    BSSGP_RX_BUFF_256 *rx_buff;
    negative_offset = offsetof(BSSGP_RX_BUFF_256,buff);
    rx_buff = (BSSGP_RX_BUFF_256*)&buff[-negative_offset];
    if (rx_buff->magic == BSSGP_BUFF_MAGIC) {
        rx_buff->used = FALSE;
    }
    else {
        printf("Buffer mgmt error, ptr buff %x, rx_buff %x!",buff,rx_buff);
    }
}


/*
 * bssgp_util_get_api_msg_str
 */
char *
bssgp_util_get_api_msg_str(BSSGP_API_MSG_TYPE api_msg)
{
	static char *api_msg_str[] = {
		"Downlink Unitdata",
		"Uplink Unitdata",
		"Paging",
		"Radio Status",
		"Suspend",
		"Suspend Ack",
		"Suspend Nack",
		"Resume",
		"Resume Ack",
		"Resume Nack",
		"Location Update",
		"Location Update Ack",
		"Location Update Nack"
	};
	if ((api_msg >= 0) && (api_msg < BSSGP_API_MAX_MSGS)) {
		return (api_msg_str[api_msg]);
	}
	else {
		return ("Unknown Message");
	}

}


/*
 * bssgp_net_get_bvci_by_ip
 */
BSSGP_BVCI
bssgp_net_get_bvci_by_ip(char *ip_addr)
{
	DBG_FUNC("bssgp_net_get_bvci_by_ip",BSSGP_NET_LAYER);
	DBG_ENTER();
	BSSGP_BVCI bvci = BSSGP_MAX_BVCI;
	unsigned short count;

	for (count=0;count<BSSGP_MAX_BVCI;count++) {
		if (bssgp_net_config.bvci_list[count].used == TRUE) {
            if (strcmp(bssgp_net_config.bvci_list[count].peer_ip_addr,ip_addr)==0) {
			    bvci = count;
			    DBG_LEAVE();
			    return(bvci);
            }
		}
        else continue;
	}
	DBG_ERROR("Could not find BVCI for IP address %s",ip_addr);
	DBG_LEAVE();
	return (bvci);
}

/*
 * bssgp_net_add_bvci
 */
BSSGP_BVCI
bssgp_net_add_bvci(char *peer_ip_addr, T_CNI_RIL3_IE_CELL_ID cell_id,UINT8 rac)
{
	DBG_FUNC("bssgp_net_add_bvci",BSSGP_NET_LAYER);
	DBG_ENTER();
	BSSGP_BVCI bvci = BSSGP_MAX_BVCI;
    strcpy(bssgp_net_config.bvci_list[BSSGP_ROOT_BVCI].peer_ip_addr,peer_ip_addr);
	bssgp_net_config.bvci_list[BSSGP_ROOT_BVCI].used = TRUE;
    bssgp_net_config.bvci_list[BSSGP_ROOT_BVCI].cell_id.ie_present    = TRUE;
    bssgp_net_config.bvci_list[BSSGP_ROOT_BVCI].cell_id.value         = cell_id.value;
    bssgp_net_config.bvci_list[BSSGP_ROOT_BVCI].routing_area_code     = rac;

	DBG_TRACE("Added Root BVCI : IP %s, Cell-ID %d, RAC %d ",
	    bssgp_net_config.bvci_list[BSSGP_ROOT_BVCI].peer_ip_addr,
        bssgp_net_config.bvci_list[BSSGP_ROOT_BVCI].cell_id.value,
        bssgp_net_config.bvci_list[BSSGP_ROOT_BVCI].routing_area_code
	    );


	DBG_LEAVE();
	return (bvci);
}



/*
 * bssgp_net_get_ip_by_bvci
 */
char *
bssgp_net_get_ip_by_bvci(BSSGP_BVCI bvci)
{
    DBG_FUNC("bssgp_net_get_ip_by_bvci",BSSGP_NET_LAYER);
    DBG_ENTER();
    DBG_ERROR("BSSGP : bssgp_net_get_ip_by_bvci not implemented!\n");
    DBG_LEAVE();
    return(NULL);
}

