/*
 ****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: bssgp_net.cpp													*
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
#include <stdio.h>
#include <stdlib.h>
#include "bssgp/bssgp_prot.h"
#include "bssgp/bssgp_util.h"
#include "bssgp/bssgp_api.h"
#include "bssgp/bssgp_net.h"
#include "bssgp/bssgp_decoder.h"

#if (defined(MNET_GS) || defined(MNET_GP10))
#include "os/JCTask.h"
extern JCTask *g_bssgp_net_task;
#endif
	

extern BSSGP_NET_CONFIG bssgp_net_config;


/*
 * bssgp_net_initialize
 */
BOOL
bssgp_net_initialize()
{
	DBG_FUNC("bssgp_net_initialize",BSSGP_NET_LAYER);
	DBG_ENTER();
	BOOL status = FALSE;
	int count, ret_status, reverse_count;
	struct sockaddr_in local_addr;

	memset(&bssgp_net_config,0,sizeof(BSSGP_NET_CONFIG));

	/* We need to get this from the MIB */
	bssgp_net_config.base_port = 24240;

	for(count=0;count<BSSGP_MAX_NETWORK_QOS_LEVELS;count++) {
		bssgp_net_config.ports[count] = bssgp_net_config.base_port + count;
	}

	for(count=0;count<BSSGP_MAX_NETWORK_QOS_LEVELS;count++) {
		bssgp_net_config.socks[count] = socket(AF_INET,SOCK_DGRAM,0);
		if (bssgp_net_config.socks[count] == ERROR) {
			DBG_ERROR("Unable to create socket # %d for BSSGP; error %d\n",count,errno);
			for(reverse_count=count-1;reverse_count>=0;reverse_count--) {
#ifdef WIN32
				closesocket(bssgp_net_config.socks[reverse_count]);
#elif defined(__VXWORKS__)
				close(bssgp_net_config.socks[reverse_count]);
#endif
			}
			DBG_LEAVE();
			return (status);
		}
		/* now bind it to the specified port */
		memset(&local_addr,0,sizeof(local_addr));
		local_addr.sin_family		= AF_INET;
		local_addr.sin_port			= htons(bssgp_net_config.ports[count]);
#ifdef WIN32
		local_addr.sin_addr.S_un.S_addr			= INADDR_ANY;
#elif defined(__VXWORKS__)
		local_addr.sin_addr.s_addr		            = INADDR_ANY;
#endif
		ret_status = bind(bssgp_net_config.socks[count],(struct sockaddr *)&local_addr,sizeof(local_addr));
		if (ret_status == ERROR) {
			DBG_ERROR("Unable to bind socket # %d=%d to port %d; error %d\n",
					bssgp_net_config.socks[count],count,bssgp_net_config.ports[count],errno);
			for(reverse_count=count;reverse_count>=0;reverse_count--) {
#ifdef WIN32
				closesocket(bssgp_net_config.socks[reverse_count]);
#elif defined(__VXWORKS__)
				close(bssgp_net_config.socks[reverse_count]);
#endif
			}
			DBG_LEAVE();
			return (status);
		}
	}

	if (bssgp_net_init_bvcis()) {
		status = TRUE;
		DBG_TRACE("Network initialized for BSSGP successfully\n");
        bssgp_net_config.init_magic = BSSGP_MAGIC;
        bssgp_net_config.continue_processing = TRUE;
	}
	else {
		DBG_ERROR("Unable to initialize BVCIs for BSSGP!\n");
		for(reverse_count=BSSGP_MAX_NETWORK_QOS_LEVELS;reverse_count>=0;reverse_count--) {
#ifdef WIN32
			closesocket(bssgp_net_config.socks[reverse_count]);
#elif defined(__VXWORKS__)
			close(bssgp_net_config.socks[reverse_count]);
#endif
		}
	}

	DBG_LEAVE();
	return (status);
}



/*
 * bssgp_net_task
 */
void
bssgp_net_task()
{
	DBG_FUNC("bssgp_net_task",BSSGP_NET_LAYER);
	DBG_ENTER();
	fd_set read_set,select_set;
	int count, select_status,no_rx_counter,ret_status;
    unsigned long rx_bytes;
	BOOL satisfied = FALSE;

	if (bssgp_net_config.init_magic != BSSGP_MAGIC) {
		DBG_ERROR("Cannot initialize BSSGP Net task without initializing module first, value %d!!\n",
		    bssgp_net_config.init_magic);
		DBG_LEAVE();
		return;
	}

	FD_ZERO(&read_set);

	for(count=0;count<BSSGP_MAX_NETWORK_QOS_LEVELS;count++) {
		FD_SET(bssgp_net_config.socks[count],&read_set);
	}

#if (defined(MNET_GS) || defined(MNET_GP10))
    g_bssgp_net_task->JCTaskEnterLoop();
#endif    
    while(bssgp_net_config.continue_processing) {
        memcpy(&select_set,&read_set,sizeof(read_set));
        if ((select_status = select(FD_SETSIZE, &select_set, 0,0,NULL)) == ERROR) {
            DBG_ERROR("Error in select; %d!!\n",errno);
        }
        else {
                /*
                 * Has data arrived on any socket ?
				 * we need to check by QoS Level
                 */
				no_rx_counter = 0;
                count = 0;
				satisfied = FALSE;
				while(!satisfied) {
					rx_bytes = 0;
#ifdef WIN32
					ret_status = ioctlsocket(bssgp_net_config.socks[count],FIONREAD,&rx_bytes);
#elif defined(__VXWORKS__)
					ret_status = ioctl(bssgp_net_config.socks[count],FIONREAD,(int)&rx_bytes);
#endif
					if ((ret_status != ERROR) && (rx_bytes > 0)) {
						bssgp_net_rx_handler(count,rx_bytes);
						count=0;
						no_rx_counter = 0;
					}
					else {
						count++;
						no_rx_counter++;
					}
					if (count == BSSGP_MAX_NETWORK_QOS_LEVELS) count = 0;
					if (no_rx_counter == BSSGP_MAX_NETWORK_QOS_LEVELS) satisfied = TRUE;
				}
		}
	}
#if (defined(MNET_GS) || defined(MNET_GP10))
    g_bssgp_net_task->JCTaskNormExit();
#endif    
	DBG_LEAVE();
	return;
}





/*
 * bssgp_net_rx_handler
 */
void
bssgp_net_rx_handler(int index,int rx_bytes)
{
	DBG_FUNC("bssgp_net_rx_handler",BSSGP_NET_LAYER);
	DBG_ENTER();
	UINT16 msg_len=0;
	UINT8 *rx_buff;
	struct sockaddr_in remote_addr;
	int conn_socket,addr_size,ret_status;
	BSSGP_BVCI	bvci;
	char peer_ip_addr[INET_ADDR_LEN];
    BOOL free_buff = TRUE;

    DBG_TRACE("QoS Level %d msg received\n",index);


	rx_buff = (UINT8*)bssgp_util_get_rx_buff(rx_bytes);
    
    if (!rx_buff)
    {
        DBG_ERROR("BSSGP: No receive buffer available for %d byte packet!\n", rx_bytes);
        DBG_LEAVE();
        return;
    }

	conn_socket = bssgp_net_config.socks[index];
	addr_size = sizeof(remote_addr);

	memset(&remote_addr,0,addr_size);
	memset(peer_ip_addr,0,INET_ADDR_LEN);

	ret_status = recvfrom(conn_socket,(char*)rx_buff,rx_bytes,0,
	                (struct sockaddr*)&remote_addr,&addr_size);
	if (ret_status == ERROR) {
		/* might want to send back msg to the peer 
		 * reporting error
		 */
		DBG_ERROR("Error receiving PDU; %d!\n",errno);
		bssgp_util_return_rx_buff((char*)rx_buff);
	}
	else {
#ifdef WIN32
        strcpy(peer_ip_addr,inet_ntoa(remote_addr.sin_addr));
#elif defined(__VXWORKS__)
		inet_ntoa_b(remote_addr.sin_addr,peer_ip_addr);
#endif
        msg_len = ret_status;
        DBG_TRACE("Received %d bytes of data from %s\n",msg_len,peer_ip_addr);
		bvci = bssgp_net_get_bvci_by_ip(peer_ip_addr);
        if (bvci == BSSGP_MAX_BVCI) {
            DBG_ERROR("Received msg of %d bytes from %s, but BVCI not up yet, so discarding msg\n",
                msg_len,peer_ip_addr);
		    bssgp_util_return_rx_buff((char*)rx_buff);
            DBG_LEAVE();
            return;
        }

		if (!bssgp_decode_msg(rx_buff,msg_len,TRUE,(BSSGP_NETWORK_QOS_LEVEL)index,bvci,&free_buff)) {
			/* might want to send back msg to the peer 
			 * reporting error
			 */
			DBG_ERROR("Error in processing received PDU from BVCI %d!\n",bvci);
			bssgp_util_return_rx_buff((char*)rx_buff);
		}
        else {
            if (free_buff) {
			    bssgp_util_return_rx_buff((char*)rx_buff);
            }
        }
	}
		
	DBG_LEAVE();
	return;
}



/*
 * bssgp_net_init_bvcis
 */
BOOL
bssgp_net_init_bvcis()
{
	DBG_FUNC("bssgp_net_init_bvcis",BSSGP_NET_LAYER);
	DBG_ENTER();
	BOOL status = FALSE;

#if defined (MNET_GS)
	
	status = TRUE;	
#elif defined (MNET_GP10)
	/* get the address of the GS from the MIB */
	status = TRUE;
#endif

	DBG_LEAVE();
	return(status);
}

#if defined(MNET_GS)

/*
 ******************************************************************
 *  Network Functions specific to the GS Platform
 ******************************************************************
 */




#elif defined(MNET_GP10)
/*
 ******************************************************************
 *  Network Functions specific to the GP-10 Platform
 ******************************************************************
 */

#endif





/*
 * bssgp_net_tx_msg
 */
BOOL
#if defined(MNET_GS)
bssgp_net_tx_msg(BSSGP_BVCI bvci,BSSGP_NETWORK_QOS_LEVEL qos_level,UINT8 *msg,UINT16 msg_len)
{
	char peer_addr[20];

	strcpy(peer_addr,bssgp_net_config.bvci_list[bvci].peer_ip_addr);

#elif defined(MNET_GP10)
bssgp_net_tx_msg(BSSGP_NETWORK_QOS_LEVEL qos_level,UINT8 *msg,UINT16 msg_len)
{
	char peer_addr[20];
	strcpy(peer_addr,bssgp_net_config.bvci_list[BSSGP_ROOT_BVCI].peer_ip_addr);

#elif defined(WIN32)
bssgp_net_tx_msg(BSSGP_NETWORK_QOS_LEVEL qos_level,UINT8 *msg,UINT16 msg_len)
{
	char peer_addr[20];
	strcpy(peer_addr,bssgp_net_config.bvci_list[BSSGP_ROOT_BVCI].peer_ip_addr);
	
#endif
	DBG_FUNC("bssgp_net_tx_msg",BSSGP_NET_LAYER);
	DBG_ENTER();
	struct sockaddr_in remote_addr;
	int ret_status;
	UINT16	peer_port;
	int conn_socket;
	
	peer_port = qos_level + bssgp_net_config.base_port;
	conn_socket = bssgp_net_config.socks[qos_level];

	memset(&remote_addr,0,sizeof(remote_addr));
	remote_addr.sin_family		= AF_INET;
	remote_addr.sin_port		= htons(peer_port);
	remote_addr.sin_addr.s_addr	= inet_addr(peer_addr);

	ret_status = sendto(conn_socket,(char*)msg,msg_len,0,(struct sockaddr*)&remote_addr,sizeof(remote_addr));
	if (ret_status != ERROR) {
        DBG_TRACE("Sent %d bytes of data to %s:%d on QoS level %d\n",ret_status,peer_addr,peer_port,qos_level);
		DBG_LEAVE();
		return (TRUE);
	}
	else {
		DBG_ERROR("Error sending data to %s on port %s; error %d\n",peer_addr,peer_port,errno);
		DBG_LEAVE();
		return (FALSE);
	}
}



/*
 * This function invoked from the Shell for testing only....
 *
 *
 */

extern "C" { int bssgp_net_add_test_bvci(char *peer_ip_addr, UINT16 cell_id,UINT8 rac); }

int
bssgp_net_add_test_bvci(char *peer_ip_addr, UINT16 cell_id,UINT8 rac)
{
	unsigned short count;

	for (count=0;count<BSSGP_MAX_BVCI;count++) {
		if (bssgp_net_config.bvci_list[count].used == FALSE) {
			bssgp_net_config.bvci_list[count].used = TRUE;
			strncpy(bssgp_net_config.bvci_list[count].peer_ip_addr,peer_ip_addr,20);
            bssgp_net_config.bvci_list[count].cell_id.value = cell_id;
            bssgp_net_config.bvci_list[count].routing_area_code = rac;
            printf("Added test-bvci %s:%d\n",peer_ip_addr,cell_id);
			return(0);
		}
	}
	printf("No more BVCIs in pool!\n");
	return (0);
}


extern "C" void
bssgp_net_display_bvci(BSSGP_BVCI bvci)
{
    if (bssgp_net_config.bvci_list[bvci].used) {
        printf("BVCI              : %d\n",bvci);
        printf("Peer IP Address   : %s\n",bssgp_net_config.bvci_list[bvci].peer_ip_addr);
        if (bssgp_net_config.bvci_list[bvci].cell_id.ie_present) {
            printf("Cell-ID           : %d\n",bssgp_net_config.bvci_list[bvci].cell_id.value);
            printf("Routing Area Code : %d\n",bssgp_net_config.bvci_list[bvci].routing_area_code);
        }
    }
    else {
        printf("BVCI %d NOT used\n",bvci);
    }

}


extern "C"
int bssgp_net_show_all_bvci()
{
#if defined(MNET_GS)
    int bvci_count=0,count;
    printf("***************************************************************\n");
	for (count=0;count<BSSGP_MAX_BVCI;count++) {
		if (bssgp_net_config.bvci_list[count].used == TRUE) {
            bssgp_net_display_bvci((BSSGP_BVCI)count);
            bvci_count++;
		}
	}
    printf("***************************************************************\n");
    printf("Number of BVCI    : %d\n",bvci_count);
    printf("***************************************************************\n");
#elif defined(MNET_GP10)
    printf("***************************************************************\n");
    bssgp_net_display_bvci(BSSGP_ROOT_BVCI);
    printf("***************************************************************\n");
#endif
    return (0);
}


