/*
 ****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: gslink.cpp													*
 *																						*
 *	Description			: The GPLink subsystem [Link between GP-10 & the GS]			*
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
#include <stdio.h>
#include <stdlib.h>
#include <vxworks.h>
#include <sockLib.h>
#include <inetLib.h>
#include <stdioLib.h>
#include <strLib.h>
#include <hostLib.h>
#include <ioLib.h>
#include <tasklib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <syslib.h>
#include "ril3/ie_cell_id.h"
#include "ril3/ie_location_area_id.h"
#include "ril3/ril3_gmm_msg.h"
#include "gslink.h"
#include "oam_api.h"
#include "AlarmCode.h"
#include "logging/vcmodules.h"
#include "logging/vclogging.h"
#include "bssgp/bssgp_net.h"
#include "bssgp/bssgp_api.h"
#include "gplink/gplink_prot.h"

/* common for all platforms */
#include "Os/JCModule.h"
#include "Os/JCTask.h"

/* common for GS */
#include "GP10OsTune.h"
#include "GP10MsgTypes.h"
#include "MnetModuleId.h"



#define GSLINK_RETRY_CONNECT_TIME   5

static GSLINK_CONFIGURATION     gslink_config;
static BOOL                     gslink_initialized = FALSE,gslink_continue_processing = TRUE;
static char                     gprs_server_ip_addr[64];
static BOOL                     gslink_use_non_oam_gs_addr=FALSE;
JCTask *g_gslink_task;



extern "C"
gslink_set_gs_address(char *gs_addr)
{
    strcpy(gprs_server_ip_addr,gs_addr);
    printf("Set the GS IP address to %s successfully\n",gprs_server_ip_addr);
    gslink_use_non_oam_gs_addr=TRUE;
    return (0);
}

extern "C"
gslink_show_status()
{
    if (gslink_initialized) {
        printf("GSLink has been initialized\n");
    }
    else {
        printf("GSLink has NOT been initialized\n");
        return(0);
    }

    if (gslink_config.connected) {
        printf("GSLink is connected to %s\n",gslink_config.gs_addr_str);
    }
    else {
        printf("GSLink is NOT connected \n");
    }
    if (gslink_config.registered) {
        printf("\n------------------------------------------------------------------------------\n");
        printf("GSLink is REGISTERED with GS at IP Address %s with following details:-\n",gslink_config.gs_addr_str);
        printf("Cell ID             : %d\n",gslink_config.cell_id.value);
        printf("Location Area ID    : MCC %d%d%d, MNC %d%d%x, LAC %d\n",
			gslink_config.location_area_id.mcc[0],
            gslink_config.location_area_id.mcc[1],
			gslink_config.location_area_id.mcc[2],
            gslink_config.location_area_id.mnc[0],
			gslink_config.location_area_id.mnc[1],
			gslink_config.location_area_id.mnc[2],
            gslink_config.location_area_id.lac);
        printf("Routing Area ID     : MCC %d%d%d, MNC %d%d%x, LAC %d, RAC %d\n",
            gslink_config.routing_area_id.mcc[0],
            gslink_config.routing_area_id.mcc[1],
			gslink_config.routing_area_id.mcc[2],
            gslink_config.routing_area_id.mnc[0],
			gslink_config.routing_area_id.mnc[1],
			gslink_config.routing_area_id.mnc[2],
            gslink_config.routing_area_id.lac,
			gslink_config.routing_area_id.rac);
        printf("\n------------------------------------------------------------------------------\n");
    }
    else {
        printf("GSLink is NOT REGISTERED \n");
    }

}





/*
 * gslink_initialize_client
 */
BOOL
gslink_initialize_client()
{
    DBG_FUNC("gslink_initialize_client",GSLINK_LAYER);
    DBG_ENTER();
    BOOL status = FALSE;
    int ret_val;
    struct sockaddr_in local_addr;
    long int oam_retval;
    int keep_alive_on= 1;
    UINT8 mcc[3],mnc[3], gs_ip_octets[4];

    if (gslink_initialized) {
        printf("GPLink already initialized\n");
        DBG_LEAVE();
        return (status);
    }

    memset(&gslink_config,0,sizeof(gslink_config));
    /*
     * Retrieve all the required registration parameters from the MIB
     */
    /* GS IP address */
    oam_retval = 0;
	if (oam_getMibIntVar(MIB_gprsServerIpAddress, (long int*)&oam_retval) != STATUS_OK) {
		printf("Unable to get GS's IP Address from OAM!!\n");
        DBG_LEAVE();
        return (status);
	}
    else {
        gs_ip_octets[0] = (oam_retval >> 24) & 0x000000ff;
        gs_ip_octets[1] = (oam_retval >> 16) & 0x000000ff;
        gs_ip_octets[2] = (oam_retval >> 8) & 0x000000ff;
        gs_ip_octets[3] = (oam_retval) & 0x000000ff;

        sprintf(gslink_config.gs_addr_str,"%d.%d.%d.%d\n",gs_ip_octets[0],gs_ip_octets[1],
                gs_ip_octets[2],gs_ip_octets[3]);
        if (gslink_use_non_oam_gs_addr)
        strcpy(gslink_config.gs_addr_str,gprs_server_ip_addr);

        printf("GS's IP Address value %s\n",gslink_config.gs_addr_str);
    }




    /* Cell ID */
    oam_retval = 0;
	if (oam_getMibIntVar(MIB_btsID, (long int*)&oam_retval) != STATUS_OK) {
		printf("Unable to get GP-10 Cell ID from OAM!!\n");
        DBG_LEAVE();
        return (status);
	}
    else {
        printf("GP-10 Cell ID OAM value %ld\n",oam_retval);
        gslink_config.cell_id.value = oam_retval;
    }

    /* Mobile Country Code */
	if (oam_getMibIntVar(MIB_bts_mcc, (long int *)&oam_retval) != STATUS_OK) {
		printf("Unable to get GP-10 MCC from OAM!!\n");
        DBG_LEAVE();
        return (status);
	}
    else {
	    if (oam_getByteAryByInt(oam_retval, (char*) mcc, 3) != STATUS_OK) {
		    printf("Unable to convert GP-10 MCC from OAM!!\n");
            DBG_LEAVE();
            return (status);
        }
        else {
            printf("GP-10 MCC OAM value %d:%d:%d\n",mcc[0],mcc[1],mcc[2]);
            gslink_config.location_area_id.mcc[0] = mcc[0];
            gslink_config.location_area_id.mcc[1] = mcc[1];
            gslink_config.location_area_id.mcc[2] = mcc[2];
        }
    }
    
    /* Mobile Network Code */
	if (oam_getMnc(mnc) != STATUS_OK) {
		printf("Unable to get GP-10 MNC from OAM!!\n");
        DBG_LEAVE();
        return (status);
	}
    else {
        printf("GP-10 MNC OAM value %d:%d:%x\n",mnc[0],mnc[1],mnc[2]);
        gslink_config.location_area_id.mnc[0] = mnc[0];
        gslink_config.location_area_id.mnc[1] = mnc[1];
        gslink_config.location_area_id.mnc[2] = mnc[2];
    }


    /* Location Area Code */
    oam_retval = 0;
	if (oam_getMibIntVar(MIB_bts_lac, (long int*)&oam_retval) != STATUS_OK) {
		printf("Unable to get GP-10 LAC from OAM!!\n");
        DBG_LEAVE();
        return (status);
	}
    else {
        gslink_config.location_area_id.lac = (UINT16)oam_retval;
        printf("GP-10 LAC OAM value %ld\n",oam_retval);
    }

    /* Routing Area Code */
    oam_retval = 0;
	if (oam_getMibIntVar(MIB_gprsRac, (long int*)&oam_retval) != STATUS_OK) {
		printf("Unable to get GP-10 RAC from OAM!!\n");
        DBG_LEAVE();
        return (status);
	}
    else {
        memcpy(gslink_config.routing_area_id.mcc,gslink_config.location_area_id.mcc,3);
        memcpy(gslink_config.routing_area_id.mnc,gslink_config.location_area_id.mnc,3);
        gslink_config.routing_area_id.lac = gslink_config.location_area_id.lac;
        gslink_config.routing_area_id.rac = oam_retval;
        printf("GP-10 RAC OAM value %ld\n",gslink_config.routing_area_id.rac);
    }
    

    gslink_config.client_socket = socket (AF_INET, SOCK_STREAM, 0);

    if (gslink_config.client_socket == ERROR) {
        printf("Could not open socket for GPLink, error %d\n",errno);
        DBG_LEAVE();
        return (status);
    }

    memset(&local_addr,0,sizeof(local_addr));
    local_addr.sin_addr.s_addr  = INADDR_ANY;
    local_addr.sin_family       = AF_INET;
    local_addr.sin_port         = htons(0);

    ret_val = bind(gslink_config.client_socket,(struct sockaddr *)&local_addr,sizeof(struct sockaddr_in));
    if (ret_val == ERROR) {
        printf("Could not bind socket to GPLink TCP Port, error %d\n",errno);
        close(gslink_config.client_socket);
        DBG_LEAVE();
        return (status);
    }

    /*
     * Enable KeepAlive on this socket/connection
     */
  	setsockopt(gslink_config.client_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&keep_alive_on, sizeof(keep_alive_on));


    /*
     * Spawn client task here
     */
    gslink_initialized = TRUE;

	g_gslink_task = new JCTask("GSLink");

	ret_val = g_gslink_task->JCTaskSpawn(GSLINK_TASK_PRIORITY,
						   GSLINK_TASK_OPTION,
						   GSLINK_TASK_STACK_SIZE,
				           (FUNCPTR)gslink_client_task,
						   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						   MODULE_GSLINK, JC_CRITICAL_TASK);				
	if (ret_val == JC_ERROR){
	    printf("[GSLink] JCTaskSpawn returned error.\n");
        close(gslink_config.client_socket);
        memset(&gslink_config,0,sizeof(gslink_config));
        gslink_initialized = FALSE;
        DBG_LEAVE();
        return (status);
	}
	else {
        printf("GSLink module initialized\n");
        status = TRUE;
        gslink_initialized = TRUE;
    }


    DBG_LEAVE();
    return (status);
}



/*
 * gslink_client_task
 */
void
gslink_client_task()
{
    DBG_FUNC("gslink_client_task",GSLINK_LAYER);
    DBG_ENTER();
    int ret_status,count;
    UINT8 rx_buff[1024];
    BOOL status;
    struct sockaddr_in gs_addr;

    ret_status = ERROR;

    memset(&gs_addr,0,sizeof(gs_addr));
    gs_addr.sin_addr.s_addr  = inet_addr(gslink_config.gs_addr_str);
    gs_addr.sin_family       = AF_INET;
    gs_addr.sin_port         = htons(GPLINK_TCP_PORT);

    g_gslink_task->JCTaskEnterLoop();

    ret_status = connect(gslink_config.client_socket,(struct sockaddr *)&gs_addr,sizeof(struct sockaddr_in));
    if (ret_status == ERROR) {
        DBG_WARNING("Could not connect socket to GS GPLink Port %s:%d, error %d\n",
                gslink_config.gs_addr_str,GPLINK_TCP_PORT,errno);
        gslink_retry_connect();
    }

    gslink_config.connected = TRUE;





    while(gslink_continue_processing) {
         status = gslink_register_client();
         if (!status) {
            DBG_WARNING("Cannot register GP-10 with GS!!\n");
            if ((errno == ECONNRESET) || (errno == ENOTCONN) || (errno == ECONNABORTED)) {
                DBG_WARNING("Connection to GS lost, error %d!\n");
                gslink_config.connected = FALSE;
                gslink_set_oam_trap(GSLINK_LINK_DOWN);
                gslink_retry_connect();
                gslink_config.connected = TRUE;
            }
            else {
                DBG_WARNING("This GP-10 is not permitted for registering on this GS, please change configuration, %d!\n",errno);
                close(gslink_config.client_socket);
                memset(&gslink_config,0,sizeof(gslink_config));
                gslink_initialized = FALSE;
                DBG_LEAVE();
                return;
            }
         }
         else {
            wait_for_something:
            memset(rx_buff,0,1024);
            ret_status = recv(gslink_config.client_socket,(char*)rx_buff,1024,0);
            if (ret_status == ERROR) {
                gslink_config.connected = FALSE;
                DBG_WARNING("GSLink: recv error - errno %#x!\n", errno);
                DBG_WARNING("Connection to GS lost, error %d!\n");
                gslink_set_oam_trap(GSLINK_LINK_DOWN);
                gslink_retry_connect();
                gslink_config.connected = TRUE;
            }
            else {
                if (!gslink_is_ack(rx_buff,ret_status)) {
                    DBG_TRACE("Re-registered with GS successfully\n");
                    /* Hate to do this, but....*/
                    if (gslink_continue_processing) goto wait_for_something;
                }
                else {
                    DBG_WARNING("This GP-10 is not permitted for registering on this GS, please change configuration!\n");
                    close(gslink_config.client_socket);
                    memset(&gslink_config,0,sizeof(gslink_config));
                    gslink_initialized = FALSE;
                    DBG_LEAVE();
                    return;
                }
            }
         }
    } /* End of while loop */


    DBG_LEAVE();
    return;
}



/*
 * The GPLink protocol has the following format:-
 * 
 *    8     7     6     5     4     3     2     1
 * +-----------------------------------------------*
 * *            Msg Length (in network order)      * octets 1,2 
 * +-----------------------+-----------------------*
 * *            GPLink message type                * octet 3
 * +-----------------------+-----------------------*
 * *        Reqd IE types, length & value          * octet 4-n
 * +-----------------------------------------------*
 * 
 *  All the messages are of fixed length. The messages are defined
 *  as follows:-
 * 
 *  Registration Request :
 *  Mandatory IEs : Cell ID, Location Area ID, Routing Area ID
 *  Optional IEs : None
 *
 *    8     7     6     5     4     3     2     1
 * +-----------------------------------------------*
 * *  Msg Length =    (in network order)           * octets 1,2 
 * +-----------------------+-----------------------*
 * *  GPLink message type = 1 (Registration Req)   * octet 3
 * +-----------------------+-----------------------*
 * *   IE Type = 1 (Cell ID)                       * octet 4  
 * +-----------------------------------------------*
 * *       Cell ID Value (in network order)        * octets 5,6
 * +-----------------------------------------------*
 * *   IE Type = 2 (Location Area ID)              * octet 7  
 * +-----------------------------------------------*
 * *    Location Area ID MCC octet 1               * octet 8
 * +-----------------------------------------------*
 * *    Location Area ID MCC octet 2               * octet 9
 * +-----------------------------------------------*
 * *    Location Area ID MCC octet 3               * octet 10
 * +-----------------------------------------------*
 * *    Location Area ID MNC octet 1               * octet 11
 * +-----------------------------------------------*
 * *    Location Area ID MNC octet 2               * octet 12
 * +-----------------------------------------------*
 * *    Location Area ID MNC octet 3               * octet 13
 * +-----------------------------------------------*
 * *    Location Area ID LAC (in network order)    * octets 14,15
 * +-----------------------------------------------*
 * *   IE Type = 3 (Routing Area ID)               * octet 16
 * +-----------------------------------------------*
 * *    Routing Area ID MCC octet 1                * octet 17
 * +-----------------------------------------------*
 * *    Routing Area ID MCC octet 2                * octet 18
 * +-----------------------------------------------*
 * *    Routing Area ID MCC octet 3                * octet 19
 * +-----------------------------------------------*
 * *    Routing Area ID MNC octet 1                * octet 20
 * +-----------------------------------------------*
 * *    Routing Area ID MNC octet 2                * octet 21
 * +-----------------------------------------------*
 * *    Routing Area ID MNC octet 3                * octet 22
 * +-----------------------------------------------*
 * *    Routing Area ID LAC (in network order)     * octets 23,24
 * +-----------------------------------------------*
 * *    Routing Area ID RAC                        * octet 25
 * +-----------------------------------------------*
 * 
 * 
 * Registration Confirm :
 * Mandatory IEs : None
 * Optional IEs : None
 * 
 * Registration Reject :
 * Mandatory IEs : Cause
 * Optional IEs : None
 * 
 * Registration Update Request :
 * Mandatory IEs : Cell ID, Location Area ID, Routing Area ID
 * Optional IEs : None
 * 
 * 
 */ 

/*
 * gslink_register_client
 */
BOOL
gslink_register_client()
{
    DBG_FUNC("gslink_register_client",GSLINK_LAYER);
    DBG_ENTER();
    BOOL status = FALSE;
    UINT8 tx_buff[1024],rx_buff[1024];
    int index=0,ret_status;
    UINT8  *ie_type,*byte_ptr;
    UINT16 *shorty, *msg_len;

    msg_len     = (UINT16*)&tx_buff[index];
    index += sizeof(UINT16);

    byte_ptr    = &tx_buff[index];
    *byte_ptr   = GPLINK_MSG_TYPE_REG_REQ;
    index++; 

    /* Set the Cell ID IE type & the value */
    ie_type     = &tx_buff[index];
    *ie_type    = GPLINK_IE_CELL_ID;
    index++;

    shorty  = (UINT16*)&tx_buff[index];
    index   += sizeof(UINT16);
    *shorty = htons(gslink_config.cell_id.value);


    /* Set the Location Area ID IE type & the value */
    ie_type     = &tx_buff[index];
    *ie_type    = GPLINK_IE_LOCATION_AREA_ID;
    index++;

    /* The MCC octets */    
    tx_buff[index++]    = gslink_config.location_area_id.mcc[0];
    tx_buff[index++]    = gslink_config.location_area_id.mcc[1];
    tx_buff[index++]    = gslink_config.location_area_id.mcc[2];

    /* The MNC octets */    
    tx_buff[index++]    = gslink_config.location_area_id.mnc[0];
    tx_buff[index++]    = gslink_config.location_area_id.mnc[1];
    tx_buff[index++]    = gslink_config.location_area_id.mnc[2];

    /* The LAC */    
    shorty = (UINT16*)&tx_buff[index];
    index += sizeof(UINT16);
    *shorty = htons(gslink_config.location_area_id.lac);


    /* Set the Routing Area ID IE type & the value */
    ie_type     = &tx_buff[index];
    *ie_type    = GPLINK_IE_ROUTING_AREA_ID;
    index++;

    /* The MCC octets */    
    tx_buff[index++]    = gslink_config.routing_area_id.mcc[0];
    tx_buff[index++]    = gslink_config.routing_area_id.mcc[1];
    tx_buff[index++]    = gslink_config.routing_area_id.mcc[2];

    /* The MNC octets */    
    tx_buff[index++]    = gslink_config.routing_area_id.mnc[0];
    tx_buff[index++]    = gslink_config.routing_area_id.mnc[1];
    tx_buff[index++]    = gslink_config.routing_area_id.mnc[2];
    
    /* The LAC */    
    shorty  = (UINT16*)&tx_buff[index];
    index   += sizeof(UINT16);
    *shorty = htons(gslink_config.routing_area_id.lac);

    /* The RAC */
    tx_buff[index] = gslink_config.routing_area_id.rac;
    index++;

    /* Now finally set the msg length */
    *msg_len = htons(index);


    ret_status = send(gslink_config.client_socket,(char*)tx_buff,index,0);
    if (ret_status == ERROR) {
        printf("Error sending message on socket %d; error %d\n",gslink_config.client_socket,errno);
        DBG_LEAVE();
        return(status);
    }
    DBG_TRACE("Sent REGISTRATION REQUEST msg of %d bytes\n",index);

    memset(rx_buff,0,1024);

    ret_status = recv(gslink_config.client_socket,(char*)rx_buff,1024,0);
    if (ret_status == ERROR) {
        printf("Error receiving message on socket %d; error %d\n",gslink_config.client_socket,errno);
        DBG_LEAVE();
        return(status);
    }

    status = gslink_is_ack(rx_buff,ret_status);

    if (status) {
        gslink_set_oam_trap(GSLINK_LINK_CONNECTED);
    }

    DBG_LEAVE();
    return(status);
}


/*
 * gslink_is_ack
 */
BOOL
gslink_is_ack(UINT8 *rx_buff,int len)
{
    DBG_FUNC("gslink_is_ack",GSLINK_LAYER);
    DBG_ENTER();
    int index = 0;
    UINT16 *shorty;


    /* Now check the ack  */
    shorty = (UINT16 *)rx_buff;
    index += 2;
    
    switch(rx_buff[index]) {
        case GPLINK_MSG_TYPE_REG_CNF: {
            DBG_TRACE("Received Registration Confirm from GS!\n");
            gslink_config.registered = TRUE;
            DBG_LEAVE();
            return(TRUE);
        }
        break;
        case GPLINK_MSG_TYPE_REG_REJ: {
            DBG_TRACE("Received Registration Reject from GS, code %d!\n");
            errno=0;
            DBG_LEAVE();
            return(FALSE);
        }
        break;
        default:{
            DBG_WARNING("Received Unknown response from GS!\n");
            errno=0;
            DBG_LEAVE();
            return(FALSE);
        }
        break;
    }
    DBG_LEAVE();
    return (FALSE);
}


                


/*
 * gslink_retry_connect
 */
void
gslink_retry_connect()
{
    DBG_FUNC("gslink_retry_connect",GSLINK_LAYER);
    DBG_ENTER();
    int ret_val;
    struct sockaddr_in local_addr,gs_addr;
    BOOL connected_to_gs = FALSE;
    int keep_alive_on= 1;

    while(!connected_to_gs) {

        close(gslink_config.client_socket);

        gslink_config.client_socket = socket (AF_INET, SOCK_STREAM, 0);

        if (gslink_config.client_socket == ERROR) {
            DBG_ERROR("Could not open socket for GPLink, error %d\n",errno);
            DBG_LEAVE();
            return ;
        }

        memset(&local_addr,0,sizeof(local_addr));
        local_addr.sin_addr.s_addr  = INADDR_ANY;
        local_addr.sin_family       = AF_INET;
        local_addr.sin_port         = htons(0);

        ret_val = bind(gslink_config.client_socket,(struct sockaddr *)&local_addr,sizeof(struct sockaddr_in));
        if (ret_val == ERROR) {
            DBG_ERROR("Could not bind socket to GPLink TCP Port, error %d, GSLink cannot continue\n",errno);
            close(gslink_config.client_socket);
            DBG_LEAVE();
            return;
        }

        memset(&gs_addr,0,sizeof(gs_addr));
        gs_addr.sin_addr.s_addr  = inet_addr(gslink_config.gs_addr_str);
        gs_addr.sin_family       = AF_INET;
        gs_addr.sin_port         = htons(GPLINK_TCP_PORT);

  	    setsockopt(gslink_config.client_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&keep_alive_on, sizeof(keep_alive_on));


        DBG_TRACE("Retrying connection to GS GPLink Port %s:%d\n",gslink_config.gs_addr_str,GPLINK_TCP_PORT);

        ret_val = connect(gslink_config.client_socket,(struct sockaddr *)&gs_addr,sizeof(struct sockaddr_in));
        if (ret_val == ERROR) {
            DBG_ERROR("Could not connect socket to GS GPLink Port %s:%d, error %d; will retry in 5 seconds\n",
                    gslink_config.gs_addr_str,GPLINK_TCP_PORT,errno);
            taskDelay(sysClkRateGet()*GSLINK_RETRY_CONNECT_TIME); /* ? */
        }
        else {
            DBG_TRACE("Connected to GS GPLink Port %s:%d\n",gslink_config.gs_addr_str,GPLINK_TCP_PORT);
            connected_to_gs = TRUE;
        }

    }

    gslink_config.connected = TRUE;

    DBG_LEAVE();
}




/*
 * gslink_is_socket_disconnected
 */
BOOL 
gslink_is_socket_disconnected(int test_socket)
{
    DBG_FUNC("gslink_is_socket_disconnected",GSLINK_LAYER);
    DBG_ENTER();

    int ret_status;
    BOOL status =  FALSE;
    int bytes_available=0,ioctl_status;

    ioctl_status = ioctl(test_socket,FIONREAD,(int)&bytes_available);
    if ( (ret_status!=ERROR) && (bytes_available==0)) {
        status = TRUE;
    }
    DBG_LEAVE();
    return(status);
}



/*
 * gslink_set_oam_trap
 */
void
gslink_set_oam_trap(GSLINK_LINK_EVENT event)
{
    DBG_FUNC("gslink_set_oam_trap",GSLINK_LAYER);
    DBG_ENTER();

    switch(event) {
        case GSLINK_LINK_CONNECTED: {
            bssgp_net_add_bvci(gslink_config.gs_addr_str,gslink_config.cell_id,gslink_config.routing_area_id.rac);
        }
        break;
	    case GSLINK_LINK_DOWN: {
        }
        break;
        default: {
        }
        break;

    }
    DBG_LEAVE();
}




/*
 * SysCommand_GSLink
 */
int	
SysCommand_GSLink(T_SYS_CMD	action)
{
	int tid;

	switch(action){
		
		case SYS_SHUTDOWN:
			printf("[GSLink] Received system shutdown notification\n");
			break;
		
		case SYS_START: {
			printf("[GSLink] Received task start notification\n");
			if (!gslink_initialize_client()) {
               printf("Unable to initialize GSLink task\n");
            } 
            else {
               printf("Initialized GSLink task successfully\n");
            }
        }
		break;

		case SYS_REBOOT:
			printf("[GSLink] Reboot ready.\n");
            close(gslink_config.client_socket);
            memset(&gslink_config,0,sizeof(gslink_config));
            gslink_initialized = FALSE;
			break;

		default:
			printf("[GSLink] Unknown system command received\n");
	}
	return 0;
}



/*
 * gslink_util_get_event_str
 */
char *
gslink_util_get_event_str(GPLINK_EVENT event)
{
    static char *gslink_event_str[]={
        "GP-10 Registered",
        "GP-10 Disconnected",
        "GP-10 Updated Registration Information"
    };
    if (event < GPLINK_MAX_EVENTS) {
        return (gslink_event_str[event]);
    }
    else {
        return "Unknown Event";
    }
}

