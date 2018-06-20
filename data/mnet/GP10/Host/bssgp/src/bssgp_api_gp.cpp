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
#include "bssgp/bssgp_prot.h"
#include "bssgp/bssgp_util.h"
#include "bssgp/bssgp_api.h"
#include "bssgp/bssgp_encoder.h"
#include "bssgp/bssgp_net.h"
#include "bssgp/bssgp_unitdata.h"

/* common for all platforms */
#include "Os/JCModule.h"
#include "Os/JCTask.h"
#include "Os/JCMsgQueue.h"

/* common for GS */
#include "GP10OsTune.h"
#include "GP10MsgTypes.h"
#include "MnetModuleId.h"
#include "oam_api.h"



/*
 * Definitions
 */
#define BSSGP_MAX_API_MSGS_IN_Q     100



/*
 * Global variables
 *
 */
extern MSG_Q_ID bssgp_api_msg_q;
extern BOOL g_bssgp_shutdown, g_bssgp_api_initialized;
extern BSSGP_CONFIGURATION bssgp_config;
JCTask *g_bssgp_net_task,*g_bssgp_api_task;





/*
 * Function prototypes
 */
BOOL bssgp_api_initialize(void);
void bssgp_api_handler(void);
BOOL bssgp_api_send_msg(BSSGP_API_MSG msg);
void bssgp_api_process_msg(BSSGP_API_MSG api_msg);
BOOL bssgp_gp_initialize(void);
int	SysCommand_BSSGP(T_SYS_CMD	action);






/*
 * bssgp_api_get_config
 */
BOOL
bssgp_api_get_config()
{
    DBG_FUNC("bssgp_api_get_config",BSSGP_LAYER);
    DBG_ENTER();
    long int oam_retval;
    UINT8 mcc[3],mnc[3],rac;
    UINT16 lac;


    /* Cell ID */
    oam_retval = 0;
	if (oam_getMibIntVar(MIB_btsID, (long int*)&oam_retval) != STATUS_OK) {
		printf("Unable to get GP-10 Cell ID from OAM!!\n");
        DBG_LEAVE();
        return (FALSE);
	}
    else {
        DBG_TRACE("GP-10 Cell ID OAM value %ld\n",oam_retval);
        bssgp_config.cell_id.ie_present = TRUE;
        bssgp_config.cell_id.value      = oam_retval;
    }


    /* Retrieve the RAI */

    bssgp_config.routing_area_id.ie_present = TRUE;

    /* Mobile Country Code */
	if (oam_getMibIntVar(MIB_bts_mcc, (long int *)&oam_retval) != STATUS_OK) {
		printf("Unable to get GP-10 MCC from OAM!!\n");
        DBG_LEAVE();
        return (FALSE);
	}
    else {

	    if (oam_getByteAryByInt(oam_retval, (char*)mcc, 3) != STATUS_OK) {
        }
        else {
            DBG_TRACE("GP-10 MCC OAM value %d:%d:%d\n",mcc[0],mcc[1],mcc[2]);
            bssgp_config.routing_area_id.mcc[0] = mcc[0];
            bssgp_config.routing_area_id.mcc[1] = mcc[1];
            bssgp_config.routing_area_id.mcc[2] = mcc[2];
        }
    }

    /* Mobile Network Code */	// kevinlim 05/11/01
	if (oam_getMnc(mnc) != STATUS_OK) {
		printf("Unable to get GP-10 MNC from OAM!!\n");
        DBG_LEAVE();
        return (FALSE);
	}
    else {
        bssgp_config.routing_area_id.mnc[0] = mnc[0];
        bssgp_config.routing_area_id.mnc[1] = mnc[1];
        bssgp_config.routing_area_id.mnc[2] = mnc[2];
        DBG_TRACE("GP-10 MNC OAM value %d:%d:%x\n",mnc[0],mnc[1],mnc[2]);
    }


    /* Location Area Code */
    oam_retval = 0;
	if (oam_getMibIntVar(MIB_bts_lac, (long int*)&oam_retval) != STATUS_OK) {
		printf("Unable to get GP-10 LAC from OAM!!\n");
        DBG_LEAVE();
        return (FALSE);
	}
    else {
        bssgp_config.routing_area_id.lac = (UINT16)oam_retval;
        DBG_TRACE("GP-10 LAC OAM value %ld\n",oam_retval);
    }

    /* Routing Area Code */
    oam_retval = 0;
	if (oam_getMibIntVar(MIB_gprsRac, (long int*)&oam_retval) != STATUS_OK) {
		printf("Unable to get GP-10 RAC from OAM!!\n");
        DBG_LEAVE();
        return (FALSE);
	}
    else {
        bssgp_config.routing_area_id.rac = oam_retval;
        DBG_TRACE("GP-10 RAC OAM value %ld\n",bssgp_config.routing_area_id.rac);
    }

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_api_initialize
 */
BOOL
bssgp_api_initialize()
{
    DBG_FUNC("bssgp_api_initialize",BSSGP_LAYER);
    DBG_ENTER();

	bssgp_api_msg_q = msgQCreate(BSSGP_MAX_API_MSGS_IN_Q,sizeof(BSSGP_API_MSG),MSG_Q_FIFO);
    if (bssgp_api_msg_q == NULL) {
		DBG_ERROR("Unable to create message queue for BSSGP !!\n");
        DBG_LEAVE();
	    return (FALSE);
	}
    g_bssgp_api_initialized = TRUE;

    memset(&bssgp_config,0,sizeof(bssgp_config));

    if (!bssgp_api_get_config()) {
        DBG_ERROR("Unable to get configuration values for BSSGP!\n");
        DBG_LEAVE();
        return (FALSE);
    }

    DBG_TRACE("BSSGP API interface initialized successfully\n");
    DBG_LEAVE();
    return (TRUE);
}


/* 
 * bssgp_api_handler
 */
void
bssgp_api_handler()
{
    DBG_FUNC("bssgp_api_handler",BSSGP_LAYER);
    DBG_ENTER();
    BSSGP_API_MSG msg;

    if (g_bssgp_api_initialized == FALSE) {
        DBG_ERROR("BSSGP API not initialized yet!!\n");
        DBG_LEAVE();
        return;
    }

    memset(&msg,0,sizeof(BSSGP_API_MSG));

    g_bssgp_api_task->JCTaskEnterLoop();
    while(!g_bssgp_shutdown) {

		if (msgQReceive(bssgp_api_msg_q,(char *)&msg,sizeof(BSSGP_API_MSG),WAIT_FOREVER)!=ERROR) {
            bssgp_api_process_msg(msg);
        }
        else {
            DBG_ERROR("Error receiving API msg on BSSGP API msg Q;errno %d\n",errno);
        }
    }
    g_bssgp_api_task->JCTaskNormExit();
    DBG_LEAVE();
    return;
}







/*
 * bssgp_api_process_msg
 */
void
bssgp_api_process_msg(BSSGP_API_MSG api_msg)
{
    DBG_FUNC("bssgp_api_process_msg",BSSGP_LAYER);
    DBG_ENTER();

    BOOL tx_msg = TRUE;
    BSSGP_NETWORK_QOS_LEVEL qos_reqd;
    UINT8 *tx_buff;
    UINT16 buff_len;
    int buff_size_reqd=0;

    buff_len = 0;
    /*
     * These buffer sizes required are ballpark estimates ;-)
     */
    if (api_msg.msg_type == BSSGP_API_MSG_UL_UNITDATA) {
        buff_size_reqd  = (api_msg.msg.ul_unitdata.pdu.data_len + 100);
    }
    else {
        buff_size_reqd  = 250;
    }

    tx_buff = (UINT8*)bssgp_util_get_rx_buff(buff_size_reqd);
    
    if (!bssgp_encode_msg(&api_msg,tx_buff,&buff_len,&qos_reqd,&tx_msg)) {
        DBG_ERROR("Error encoding msg type %s\n",bssgp_util_get_api_msg_str(api_msg.msg_type));
        DBG_LEAVE();
        return;

    }

    if (tx_msg) {
        bssgp_net_tx_msg(qos_reqd,tx_buff,buff_len);
        bssgp_util_return_rx_buff((char*)tx_buff);
    }
    if (api_msg.msg_type == BSSGP_API_MSG_UL_UNITDATA) {
        bssgp_util_return_rx_buff((char*)api_msg.msg.ul_unitdata.pdu.data);
    }
    

    DBG_LEAVE();
}













/*
 * bssgp_gp_initialize
 */
BOOL
bssgp_gp_initialize()
{
    BOOL status = FALSE;
	JC_STATUS	ret_status;

    status = bssgp_api_initialize();
    if (!status) {
        printf("Cannot initialize BSSGP API interface\n");
        return (status);
    }

    status = bssgp_net_initialize();
    if (!status) {
        printf("Cannot initialize BSSGP Network interface\n");
        return (status);
    }


    status = bssgp_util_init_rx_buffs();
    if (!status) {
        printf("Cannot initialize BSSGP Network interface\n");
        return (status);
    }
    
    /* FIXME : Got to get UDT buffer configuration variables from the MIB */
    status = bssgp_udt_initialize(BSSGP_UNITDATA_MAX_TLLI_LISTS,1000,NULL);
    if (!status) {
        printf("Cannot initialize BSSGP Unitdata buffers\n");
        return (status);
    }

	/* create task instance */
	g_bssgp_net_task = new JCTask("BSSGP_NET");

	ret_status = g_bssgp_net_task->JCTaskSpawn(BSSGP_TASK_PRIORITY,
						   BSSGP_TASK_OPTION,
						   BSSGP_TASK_STACK_SIZE,
				           (FUNCPTR)bssgp_net_task,
						   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						   MODULE_BSSGP, JC_CRITICAL_TASK);				
	if(ret_status == JC_ERROR){
	    printf("[BSSGP] JCTaskSpawn returned error.\n");
	}
	else {
	    printf("[BSSGP] Net task spawned (tid=0x%x) \n", g_bssgp_net_task->GetTaskId());
	    g_bssgp_api_task = new JCTask("BSSGP_API");
	    ret_status = g_bssgp_api_task->JCTaskSpawn(BSSGP_TASK_PRIORITY,
						       BSSGP_TASK_OPTION,
						       BSSGP_TASK_STACK_SIZE,
				               (FUNCPTR)bssgp_api_handler,
						       0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						       MODULE_BSSGP, JC_CRITICAL_TASK);				
	    if(ret_status == JC_ERROR){
	        printf("[BSSGP] JCTaskSpawn returned error.\n");
	    }
	    else {
	        printf("[BSSGP] API task spawned (tid=0x%x) \n", g_bssgp_net_task->GetTaskId());
            status = TRUE;
	    }
	}

    return (status);
}
















/*
 * SysCommand_BSSGP
 */
int	
SysCommand_BSSGP(T_SYS_CMD	action)
{
	int tid;

	switch(action){
		
		case SYS_SHUTDOWN:
			printf("[BSSGP] Received system shutdown notification\n");
			break;
		
		case SYS_START: {
			printf("[BSSGP] Received task start notification\n");
			if (!bssgp_gp_initialize()) {
               printf("Unable to initialize BSSGP task\n");
            } 
            else {
               printf("Initialized BSSGP task successfully\n");
            }
        }
		break;

		case SYS_REBOOT:
			printf("[BSSGP] Reboot ready.\n");
			break;

		default:
			printf("[BSSGP] Unknown system command received\n");
	}
	return 0;
}
