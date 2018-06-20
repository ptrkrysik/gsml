/*
 ****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: bssgp_encoder.cpp												*
 *																						*
 *	Description			: Encoding functions for the BSS-GP layer module				*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *--------------------------- Notes ----------------------------------------------------*
 *	References are to sections numbers of the BSSGP spec. (GSM 08.18) unless otherwise  *
 *  indicated																			*
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
#include "bssgp/bssgp_ie_encoder.h"

extern BSSGP_CONFIGURATION bssgp_config;

/*
 * bssgp_encode_msg
 *
 */
BOOL
bssgp_encode_msg(BSSGP_API_MSG *api_msg,UINT8 *msg,UINT16 *msg_len,
                BSSGP_NETWORK_QOS_LEVEL *qos_reqd, BOOL *tx_msg)
{
	DBG_FUNC("bssgp_encode_msg",BSSGP_LAYER);
	DBG_ENTER();
	BOOL status = FALSE;

	switch(api_msg->msg_type) {
		/* Downlink Unitdata */
		case BSSGP_API_MSG_DL_UNITDATA:{
			status = bssgp_encode_dl_unitdata(api_msg->msg.dl_unitdata,msg,msg_len,qos_reqd,
			                tx_msg);
		}
		break;	
		case BSSGP_API_MSG_UL_UNITDATA:{
			status = bssgp_encode_ul_unitdata(api_msg->msg.ul_unitdata,msg,msg_len,qos_reqd,tx_msg);
		}
		break;	
		case BSSGP_API_MSG_PAGING_PS:{
			status = bssgp_encode_paging_ps(api_msg->msg.paging_ps,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_API_MSG_RADIO_STATUS:{
			status = bssgp_encode_radio_status(api_msg->msg.radio_status,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_API_MSG_SUSPEND:{
			status = bssgp_encode_suspend(api_msg->msg.suspend,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_API_MSG_SUSPEND_ACK:{
			status = bssgp_encode_suspend_ack(api_msg->msg.suspend_ack,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_API_MSG_SUSPEND_NACK:{
			status = bssgp_encode_suspend_nack(api_msg->msg.suspend_nack,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_API_MSG_RESUME:{
			status = bssgp_encode_resume(api_msg->msg.resume,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_API_MSG_RESUME_ACK:{
			status = bssgp_encode_resume_ack(api_msg->msg.resume_ack,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_API_MSG_RESUME_NACK:{
			status = bssgp_encode_resume_nack(api_msg->msg.resume_nack,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_API_MSG_LOCATION_UPDATE:{
			status = bssgp_encode_location_update(api_msg->msg.location_update,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_API_MSG_LOCATION_UPDATE_ACK:{
			status = bssgp_encode_location_update_ack(api_msg->msg.location_update_ack,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_API_MSG_LOCATION_UPDATE_NACK:{
			status = bssgp_encode_location_update_nack(api_msg->msg.location_update_nack,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_API_MSG_RADIO_CAP:{
			status = bssgp_encode_radio_cap(api_msg->msg.radio_cap,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_API_MSG_RADIO_CAP_UPDATE:{
			status = bssgp_encode_radio_cap_update(api_msg->msg.radio_cap_update,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_API_MSG_RADIO_CAP_UPDATE_ACK:{
			status = bssgp_encode_radio_cap_update_ack(api_msg->msg.radio_cap_update_ack,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_BVC_BLOCK:{
			status = bssgp_encode_bvc_block(api_msg->msg.bvc_block,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_BVC_BLOCK_ACK:{
			status = bssgp_encode_bvc_block_ack(api_msg->msg.bvc_block_ack,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_RESET:{
			status = bssgp_encode_bvc_reset(api_msg->msg.bvc_reset,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_RESET_ACK:{
			status = bssgp_encode_bvc_reset_ack(api_msg->msg.bvc_reset_ack,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_UNBLOCK:{
			status = bssgp_encode_bvc_unblock(api_msg->msg.bvc_unblock,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_UNBLOCK_ACK:{
			status = bssgp_encode_bvc_unblock_ack(api_msg->msg.bvc_unblock_ack,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_FLOW_CONTROL_BVC:{
			status = bssgp_encode_flow_control_bvc(api_msg->msg.flow_control_bvc,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_FLOW_CONTROL_BVC_ACK:{
			status = bssgp_encode_flow_control_bvc_ack(api_msg->msg.flow_control_bvc_ack,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_FLOW_CONTROL_MS:{
			status = bssgp_encode_flow_control_ms(api_msg->msg.flow_control_ms,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_FLOW_CONTROL_MS_ACK:{
			status = bssgp_encode_flow_control_ms_ack(api_msg->msg.flow_control_ms_ack,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_FLUSH_LL:{
			status = bssgp_encode_flush_ll(api_msg->msg.flush_ll,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_FLUSH_LL_ACK:{
			status = bssgp_encode_flush_ll_ack(api_msg->msg.flush_ll_ack,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_LLC_DISCARDED:{
			status = bssgp_encode_llc_discarded(api_msg->msg.llc_discarded,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
		case BSSGP_PDU_STATUS:{
			status = bssgp_encode_bvc_status(api_msg->msg.bvc_status,msg,msg_len);
            *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
            *tx_msg = TRUE;
		}
		break;	
        
		default:{
            DBG_WARNING("BSSGP: No encoder for msg type %#x\n",api_msg->msg_type);
            DBG_LEAVE();
            return (FALSE);
		}
		break;	
	}

	DBG_LEAVE();
	return (status);
}





/*
 * bssgp_encode_dl_unitdata
 */
BOOL
bssgp_encode_dl_unitdata(BSSGP_DL_UNITDATA_MSG dl_unitdata_msg ,UINT8 *msg,
            UINT16 *len, BSSGP_NETWORK_QOS_LEVEL *qos_reqd, BOOL *tx_msg)
{
	DBG_FUNC("bssgp_encode_dl_unitdata",BSSGP_LAYER);
	DBG_ENTER();

	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_DL_UNITDATA;

	index++;

	DBG_TRACE("Encoding DL-Unitdata message");

	/* Encode the individual IE indicators */
	/* tlli */
    if (dl_unitdata_msg.tlli.ie_present) {
	    bssgp_ie_encode_tlli(dl_unitdata_msg.tlli,&msg[index],&index, BSSGP_FORMAT_V);
    }
    else
    {
        DBG_ERROR("BSSGP: TLLI(current) is manadatory in DL-UNITDATA\n");
        DBG_LEAVE();
        return (FALSE);
    }
    
    if (dl_unitdata_msg.qos_profile.ie_present) {
        bssgp_ie_encode_qos_profile(dl_unitdata_msg.qos_profile,&msg[index],&index, BSSGP_FORMAT_V);
    }
    else
    {
        DBG_ERROR("BSSGP: QOS Profile is manadatory in DL-UNITDATA\n");
        DBG_LEAVE();
        return (FALSE);
    }

    if (dl_unitdata_msg.qos_params.ie_present) {
        bssgp_ie_encode_qos_params(dl_unitdata_msg.qos_params,&msg[index],&index);
    }
    else
    {
        DBG_ERROR("BSSGP: QOS Params is manadatory in DL-UNITDATA\n");
        DBG_LEAVE();
        return (FALSE);
    }

    if (dl_unitdata_msg.pdu_lifetime.ie_present) {
        bssgp_ie_encode_pdu_lifetime(dl_unitdata_msg.pdu_lifetime,&msg[index],&index);
    }
    else
    {    
        DBG_ERROR("BSSGP: PDU Lifetim is manadatory in DL-UNITDATA\n");
        DBG_LEAVE();
        return (FALSE);
    }
    
    if (dl_unitdata_msg.radio_access_cap.ie_present) {
        bssgp_ie_encode_ms_ra_cap(dl_unitdata_msg.radio_access_cap,&msg[index],&index);
    }

    if (dl_unitdata_msg.priority.ie_present) {
        bssgp_ie_encode_priority(dl_unitdata_msg.priority,&msg[index],&index);
    }
    
    if (dl_unitdata_msg.drx_params.ie_present) {
        bssgp_ie_encode_drx_params(dl_unitdata_msg.drx_params,&msg[index],&index);
    }

    if (dl_unitdata_msg.imsi.ie_present) {
        bssgp_ie_encode_imsi(dl_unitdata_msg.imsi,&msg[index],&index);
    }

    if (dl_unitdata_msg.tlli_old.ie_present) {
        bssgp_ie_encode_tlli(dl_unitdata_msg.tlli_old,&msg[index],&index, BSSGP_FORMAT_TLV);
    }

    if (dl_unitdata_msg.lsa_info.ie_present) {
        /* align on a 4 byte boundary */
        if (index % 4)
        {
            bssgp_ie_encode_alignment_octets(4 - (index % 4),&msg[index],&index);
        }
        bssgp_ie_encode_lsa_info(dl_unitdata_msg.lsa_info,&msg[index],&index);
    }

    bssgp_ie_encode_llc_pdu((UINT8*)dl_unitdata_msg.pdu.data,dl_unitdata_msg.pdu.data_len,&msg[index],&index);
    bssgp_util_return_rx_buff((char*)dl_unitdata_msg.pdu.data);
    
    if (dl_unitdata_msg.qos_profile.t_bit == SDU_CONTAINS_SIGNALLING) {
        *qos_reqd = BSSGP_NETWORK_QOS_LEVEL_1;
    }
    else {
        *qos_reqd = (BSSGP_NETWORK_QOS_LEVEL)dl_unitdata_msg.qos_profile.precedence_class;
    }

    *len += index;

    /* FIXME : Check here if the Qs for this TLLI is backed up, otherwise out it goes */
    *tx_msg = TRUE;

	DBG_LEAVE();
	return (TRUE);
}


/*
 * bssgp_encode_ul_unitdata
 */
BOOL
bssgp_encode_ul_unitdata(BSSGP_UL_UNITDATA_MSG ul_unitdata_msg,UINT8 *msg,UINT16 *len, 
    BSSGP_NETWORK_QOS_LEVEL *qos_reqd, BOOL *tx_msg)
{
	DBG_FUNC("bssgp_encode_ul_unitdata",BSSGP_LAYER);
	DBG_ENTER();

	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_UL_UNITDATA;

	index++;

	DBG_TRACE("Encoding UL-Unitdata message");

	/* Encode the individual IE indicators */
    if (ul_unitdata_msg.tlli.ie_present) {
	    bssgp_ie_encode_tlli(ul_unitdata_msg.tlli,&msg[index],&index, BSSGP_FORMAT_V);
    }
    else
    {
        DBG_ERROR("BSSGP: TLLI is manadatory in UL-UNITDATA\n");
        DBG_LEAVE();
        return (FALSE);
    }
    
    if (ul_unitdata_msg.qos_profile.ie_present) {
        bssgp_ie_encode_qos_profile(ul_unitdata_msg.qos_profile,&msg[index],&index, BSSGP_FORMAT_V);
    }
    else
    {
        DBG_ERROR("BSSGP: QOS Profile is manadatory in UL-UNITDATA\n");
        DBG_LEAVE();
        return (FALSE);
    }
    
#if defined(MNET_GP10)
        ul_unitdata_msg.cell_id.cell_id = bssgp_config.cell_id;
        ul_unitdata_msg.cell_id.routing_area = bssgp_config.routing_area_id;
        ul_unitdata_msg.cell_id.ie_present = TRUE;
#endif
    if (ul_unitdata_msg.cell_id.ie_present) {
        bssgp_ie_encode_cell_id(ul_unitdata_msg.cell_id,&msg[index],&index);
    }
    else
    {
        DBG_ERROR("BSSGP: Cell Identifier is manadatory in UL-UNITDATA\n");
        DBG_LEAVE();
        return (FALSE);
    }

    if (ul_unitdata_msg.lsa_id_list.ie_present) {
        /* align on a 4 byte boundary */
        if (index % 4)
        {
            bssgp_ie_encode_alignment_octets(4 - (index % 4),&msg[index],&index);
        }
        bssgp_ie_encode_lsa_identifier_list(ul_unitdata_msg.lsa_id_list,&msg[index],&index);
    }

    bssgp_ie_encode_llc_pdu((UINT8*)ul_unitdata_msg.pdu.data,ul_unitdata_msg.pdu.data_len,&msg[index],&index);

    /* Indicate the QoS level */
    *qos_reqd   = (BSSGP_NETWORK_QOS_LEVEL)ul_unitdata_msg.qos_profile.precedence_class;


    /* For Uplink we dont need to Q anything */
    *tx_msg = TRUE;

    *len += index;

	DBG_LEAVE();
	return (TRUE);
}


/*
 * bssgp_encode_paging
 *
 *  Paging PDU defined in 08.18:-
 *
 *	|-------------------------|
 *  |    PDU Type             |
 *	|    IMSI                 |
 *	|    DRX Params           |
 *	|    BVCI                 |
 *	|    Location Area        |
 *  |    Routing Area         |
 *  |    BSS Area Indication  |
 *  |    QoS Profile          |
 *  |    P-TMSI               |
 *  |-------------------------|
 *
 *  From GS --> GP-10
 */
BOOL
bssgp_encode_paging_ps(BSSGP_PAGING_PS_MSG paging_msg ,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_paging_ps",BSSGP_LAYER);
	DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_PAGING_PS;

	index++;

	DBG_TRACE("Encoding Packet-Switched Paging message");

	/* Encode the individual IE indicators */
	/* imsi */
    if (paging_msg.imsi.ie_present) {
	    bssgp_ie_encode_imsi(paging_msg.imsi,&msg[index],&index);
    }
    else {
        DBG_ERROR("Mandatory element IMSI not specified for paging-ps msg!");
        DBG_LEAVE();
        return (FALSE);
    }

	/* drx params */
    if (paging_msg.drx_params.ie_present)
	    bssgp_ie_encode_drx_params(paging_msg.drx_params,&msg[index],&index);

	/* bvci */
    if (paging_msg.bvci.ie_present)
	    bssgp_ie_encode_bvci(paging_msg.bvci,&msg[index],&index);

	/* location area */
    if (paging_msg.location_area.ie_present)
	    bssgp_ie_encode_location_area(paging_msg.location_area,&msg[index],&index);

	/* routing area */
    if (paging_msg.routing_area.ie_present)
	    bssgp_ie_encode_routing_area(paging_msg.routing_area,&msg[index],&index);

    /* bss area indication */
    if (paging_msg.bss_area.ie_present)
        bssgp_ie_encode_bss_area_ind(paging_msg.bss_area,&msg[index],&index);
        
	/* qos profile */
    if (paging_msg.qos_profile.ie_present)
    {
	    bssgp_ie_encode_qos_profile(paging_msg.qos_profile,&msg[index],&index, BSSGP_FORMAT_TLV);
    }
    else
    {
        DBG_ERROR("BSSGP: QOS Profile is manadatory in PAGING PS\n");
        DBG_LEAVE();
        return (FALSE);
    }

	/* tmsi */
    if (paging_msg.tmsi.ie_present)
	    bssgp_ie_encode_tmsi(paging_msg.tmsi,&msg[index],&index);


	*msg_len += index;
	DBG_TRACE("Paging-PS message %d bytes length",index);

	DBG_LEAVE();
	return (TRUE);
}




/*
 * bssgp_encode_radio_status
 *
 *  Radio Status PDU defined in 08.18:-
 *
 *  |--------------|
 *  | PDU type	   |
 *  | TLLI         |
 *  | TMSI         |
 *  | IMSI         |
 *  | Radio Cause  |
 *  |--------------|
 * 
 * From GP-10 to GS
 */
BOOL
bssgp_encode_radio_status(BSSGP_RADIO_STATUS_MSG radio_status_msg ,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_radio_status",BSSGP_LAYER);
	DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_RADIO_STATUS;

	index++;

	DBG_TRACE("Encoding Radio Status message");

	/* Encode the individual IE indicators */
	/* tlli */
    if (radio_status_msg.tlli.ie_present)
	    bssgp_ie_encode_tlli(radio_status_msg.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);
	/* tmsi */
    if (radio_status_msg.tmsi.ie_present)
	    bssgp_ie_encode_tmsi(radio_status_msg.tmsi,&msg[index],&index);
	/* imsi */
    if (radio_status_msg.imsi.ie_present)
	    bssgp_ie_encode_imsi(radio_status_msg.imsi,&msg[index],&index);
	/* cause */
    bssgp_ie_encode_radio_cause(radio_status_msg.cause,&msg[index],&index);

	*msg_len += index;

	DBG_TRACE("Radio Status message %d bytes length",index);

	DBG_LEAVE();
	return (TRUE);
}


/*
 * bssgp_encode_suspend
 *
 *  Suspend PDU defined in 08.18:-
 *
 *	|-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    Routing Area         |
 *  |-------------------------|
 * 
 * From GP-10 to GS
 */
BOOL
bssgp_encode_suspend(BSSGP_SUSPEND_MSG suspend_msg,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_suspend",BSSGP_LAYER);
	DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_SUSPEND;

	index++;

	DBG_TRACE("Encoding Suspend message");

	/* Encode the individual IE indicators */
	/* tlli */
	bssgp_ie_encode_tlli(suspend_msg.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);
	/* routing area */
	bssgp_ie_encode_routing_area(suspend_msg.routing_area,&msg[index],&index);

	*msg_len = index;

	DBG_TRACE("Suspend message %d bytes length",index);

	DBG_LEAVE();
	return (TRUE);
}


/*
 * bssgp_encode_suspend_ack
 *
 *  Suspend Ack PDU defined in 08.18:-
 *
 *	|-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    Routing Area         |
 *  |    Suspend Ref Num.     |
 *  |    BVCI (prop. ext.)    |
 *  |-------------------------|
 *
 *  From GS --> GP-10
 */
BOOL
bssgp_encode_suspend_ack(BSSGP_SUSPEND_ACK_MSG suspend_ack_msg ,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_suspend_ack",BSSGP_LAYER);
	DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_SUSPEND;

	index++;

	DBG_TRACE("Encoding Suspend-Ack message");

	/* Encode the individual IE indicators */
	/* tlli */
	bssgp_ie_encode_tlli(suspend_ack_msg.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);
	/* routing area */
	bssgp_ie_encode_routing_area(suspend_ack_msg.routing_area,&msg[index],&index);
	/* suspend ref num */
	bssgp_ie_encode_suspend_ref_num(suspend_ack_msg.suspend_ref_num,&msg[index],&index);

	*msg_len += index;
	DBG_TRACE("Suspend-Ack message %d bytes length",index);

	DBG_LEAVE();
	return (TRUE);
}


/*
 * bssgp_encode_suspend_nack
 *
 *  Suspend NAck PDU defined in 08.18:-
 *
 *	|-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    Routing Area         |
 *  |    Exception cause      |
 *  |    BVCI (prop. ext.)    |
 *  |-------------------------|
 *
 *  From GS --> GP-10
 */
BOOL
bssgp_encode_suspend_nack(BSSGP_SUSPEND_NACK_MSG suspend_nack_msg ,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_suspend_nack",BSSGP_LAYER);
	DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_SUSPEND;


	DBG_TRACE("Encoding Suspend-Nack message");

	/* Encode the individual IE indicators */
	/* tlli */
	bssgp_ie_encode_tlli(suspend_nack_msg.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);
	/* routing area */
	bssgp_ie_encode_routing_area(suspend_nack_msg.routing_area,&msg[index],&index);
	/* exception cause */
    if (suspend_nack_msg.exception_cause.ie_present)
	    bssgp_ie_encode_exception_cause(suspend_nack_msg.exception_cause,&msg[index],&index);

	*msg_len += index;
	DBG_TRACE("Suspend-NAck message %d bytes length",index);

	DBG_LEAVE();
	return (TRUE);
}




/*
 * bssgp_encode_resume
 *
 *  Resume PDU defined in 08.18:-
 *
 *	|-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    Routing Area         |
 *  |    Suspend ref num.     |
 *  |-------------------------|
 * 
 * From GP-10 to GS
 */
BOOL
bssgp_encode_resume(BSSGP_RESUME_MSG resume_msg ,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_resume",BSSGP_LAYER);
	DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_RESUME;


	DBG_TRACE("Encoding Resume message");

	/* Encode the individual IE indicators */
	/* tlli */
	bssgp_ie_encode_tlli(resume_msg.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);
	/* routing area */
	bssgp_ie_encode_routing_area(resume_msg.routing_area,&msg[index],&index);
	/* suspend ref num */
	bssgp_ie_encode_suspend_ref_num(resume_msg.suspend_ref_num,&msg[index],&index);

	*msg_len += index;
	DBG_TRACE("Resume message %d bytes length",index);


	DBG_LEAVE();
	return (TRUE);
}


/*
 * bssgp_encode_resume_ack
 *
 *  Resume-Ack PDU defined in 08.18:-
 *
 *	|-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    Routing Area         |
 *  |    BVCI (prop. ext.)    |
 *  |-------------------------|
 *
 *  From GS --> GP-10
 */
BOOL
bssgp_encode_resume_ack(BSSGP_RESUME_ACK_MSG resume_ack_msg ,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_resume_ack",BSSGP_LAYER);
	DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_RESUME_ACK;


	DBG_TRACE("Encoding Resume-Ack message");

	/* Encode the individual IE indicators */
	/* tlli */
	bssgp_ie_encode_tlli(resume_ack_msg.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);
	/* routing area */
	bssgp_ie_encode_routing_area(resume_ack_msg.routing_area,&msg[index],&index);

	*msg_len += index;
	DBG_TRACE("Resume-Ack message %d bytes length",index);

	DBG_LEAVE();
	return (TRUE);
}


/*
 * bssgp_encode_resume_nack
 *
 *  Resume-NAck PDU defined in 08.18:-
 *
 *	|-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    Routing Area         |
 *  |    Exception cause      |
 *  |-------------------------|
 *
 *  From GS --> GP-10
 */
BOOL
bssgp_encode_resume_nack(BSSGP_RESUME_NACK_MSG resume_nack_msg ,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_resume_nack",BSSGP_LAYER);
	DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_RESUME_NACK;


	DBG_TRACE("Encoding Resume-NAck message");

	/* Encode the individual IE indicators */
	/* tlli */
	bssgp_ie_encode_tlli(resume_nack_msg.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);
	/* routing area */
	bssgp_ie_encode_routing_area(resume_nack_msg.routing_area,&msg[index],&index);
	/* exception cause */
    if (resume_nack_msg.exception_cause.ie_present)
	    bssgp_ie_encode_exception_cause(resume_nack_msg.exception_cause,&msg[index],&index);

	*msg_len += index;
	DBG_TRACE("Resume-NAck message %d bytes length",index);

	DBG_LEAVE();
	return (TRUE);
}




/*
 * bssgp_encode_location_update
 *
 *  Location-Update PDU defined by 
 *  Cisco-EWTBU (proprietary)
 *
 *	|-------------------------|
 *  |    PDU Type             |
 *  |    IMSI                 |
 *  |    BVCI                 |
 *  |    Location Area        |
 *  |    Routing Area         |
 *  |-------------------------|
 *
 *  From GS --> GP-10
 */
BOOL
bssgp_encode_location_update(BSSGP_LOCATION_UPDATE_MSG location_update_msg ,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_location_update",BSSGP_LAYER);
	DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_LOCATION_UPDATE;


	DBG_TRACE("Encoding Location-Update message");


	/* Encode the individual IE indicators */
	/* imsi */
	bssgp_ie_encode_imsi(location_update_msg.imsi,&msg[index],&index);

	/* bvci */
	bssgp_ie_encode_bvci(location_update_msg.bvci,&msg[index],&index);

	/* location area */
	bssgp_ie_encode_location_area(location_update_msg.location_area,&msg[index],&index);

	/* routing area */
	bssgp_ie_encode_routing_area(location_update_msg.routing_area,&msg[index],&index);


	*msg_len += index;
	DBG_TRACE("Location-Update message %d bytes length",index);

	DBG_LEAVE();
	return (TRUE);
}



/*
 * bssgp_encode_location_update_ack
 *
 *  Location-Update-Ack PDU defined by 
 *  Cisco-EWTBU (proprietary)
 *
 *	|-------------------------|
 *  |    PDU Type             |
 *  |    IMSI                 |
 *  |    Location Area        |
 *  |-------------------------|
 * 
 * From GP-10 to GS
 */
BOOL
bssgp_encode_location_update_ack(BSSGP_LOCATION_UPDATE_ACK_MSG location_update_ack ,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_location_update_ack",BSSGP_LAYER);
	DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_LOCATION_UPDATE_ACK;

	DBG_TRACE("Encoding Location-Update-Ack message");


	/* Encode the individual IE indicators */
	/* imsi */
	bssgp_ie_encode_imsi(location_update_ack.imsi,&msg[index],&index);

	/* location area */
	bssgp_ie_encode_location_area(location_update_ack.location_area,&msg[index],&index);


	*msg_len += index;
	DBG_TRACE("Location-Update-Ack message %d bytes length",index);

	DBG_LEAVE();
	return (TRUE);
}



/*
 * bssgp_encode_location_update_nack
 *
 *  Location-Update-NAck PDU defined by 
 *  Cisco-EWTBU (proprietary)
 *
 *	|-------------------------|
 *  |    PDU Type             |
 *  |    IMSI                 |
 *  |    Location Area        |
 *  |    Reject cause         |
 *  |-------------------------|
 * 
 * From GP-10 to GS
 */
BOOL
bssgp_encode_location_update_nack(BSSGP_LOCATION_UPDATE_NACK_MSG location_update_nack ,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_location_update_nack",BSSGP_LAYER);
	DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_LOCATION_UPDATE_NACK;

	DBG_TRACE("Encoding Location-Update-NAck message");


	/* Encode the individual IE indicators */
	/* imsi */
	bssgp_ie_encode_imsi(location_update_nack.imsi,&msg[index],&index);

	/* location area */
	bssgp_ie_encode_location_area(location_update_nack.location_area,&msg[index],&index);

	/* reject cause */
	bssgp_ie_encode_reject_cause(location_update_nack.reject_cause,&msg[index],&index);

	*msg_len += index;
	DBG_TRACE("Location-Update-NAck message %d bytes length",index);

	DBG_LEAVE();
	return (TRUE);
}




/*
 * bssgp_encode_radio_cap
 *
 *  Radio-Access-Capability PDU defined in
 *  08.18
 *
 *	|-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    MS Radio Access Cap  |
 *  |    BVCI (prop. ext.)    |
 *  |-------------------------|
 * 
 * From GS to GP-10
 */
BOOL
bssgp_encode_radio_cap(BSSGP_RADIO_CAP_MSG radio_cap ,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_radio_cap",BSSGP_LAYER);
	DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_RA_CAP;
    index++;

	DBG_TRACE("Encoding Radio-Access-Capability message");


	/* Encode the individual IE indicators */
	/* tlli */
	bssgp_ie_encode_tlli(radio_cap.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);

	/* ra-cap */
	bssgp_ie_encode_ms_ra_cap(radio_cap.ra_cap,&msg[index],&index);

	*msg_len += index;
	DBG_TRACE("Radio-Access-Capability message %d bytes length",index);

    DBG_LEAVE();
    return(TRUE);
}


/*
 * bssgp_encode_radio_cap_update
 *
 *  Radio-Access-Capability-Update PDU defined in
 *  08.18
 *
 *	|-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    bvci
                   |
 *  |-------------------------|
 * 
 * From GP-10 to GS
 */
BOOL
bssgp_encode_radio_cap_update(BSSGP_RADIO_CAP_UPDATE_MSG radio_cap_update ,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_radio_cap_update",BSSGP_LAYER);
	DBG_ENTER();

	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_RA_CAP_UPDATE;

	DBG_TRACE("Encoding Radio-Access-Capability-Update message");

	/* Encode the individual IE indicators */
	/* tlli */
	bssgp_ie_encode_tlli(radio_cap_update.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);

	/* tag */
	bssgp_ie_encode_tag(radio_cap_update.tag,&msg[index],&index);

	*msg_len += index;
	DBG_TRACE("Radio-Access-Capability-Update message %d bytes length",index);

    DBG_LEAVE();
    return(TRUE);
}


/*
 * bssgp_encode_radio_cap_update_ack
 *
 *  Radio-Access-Capability-Update-ACK PDU defined in
 *  08.18
 *
 *	|-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    Tag                  |
 *  |    IMSI                 |
 *  |    RA-Cap-Upd-Cause     |
 *  |    MS Radio Access Cap  |
 *  |    BVCI (prop. ext.)    |
 *  |-------------------------|
 * 
 * From GP-10 to GS
 */
BOOL
bssgp_encode_radio_cap_update_ack(BSSGP_RADIO_CAP_UPDATE_ACK_MSG radio_cap_update_ack ,UINT8 *msg,UINT16 *msg_len)
{
	DBG_FUNC("bssgp_encode_radio_cap_update_ack",BSSGP_LAYER);
	DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_RA_CAP_UPDATE_ACK;

	DBG_TRACE("Encoding Radio-Access-Capability-Update-Ack message");
	/* Encode the individual IE indicators */
	/* tlli */
	bssgp_ie_encode_tlli(radio_cap_update_ack.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);

	/* tag */
	bssgp_ie_encode_tag(radio_cap_update_ack.tag,&msg[index],&index);

	/* imsi */
    if (radio_cap_update_ack.imsi.ie_present)
	bssgp_ie_encode_imsi(radio_cap_update_ack.imsi,&msg[index],&index);

	/* RA-Cap-UPD-CAUSE */
    bssgp_ie_encode_ra_cap_update_cause(radio_cap_update_ack.cause,&msg[index],&index);

    /* RA-Cap */
    if (radio_cap_update_ack.ra_cap.ie_present) {
        bssgp_ie_encode_ms_ra_cap(radio_cap_update_ack.ra_cap,&msg[index],&index);
    }
	*msg_len += index;
	DBG_TRACE("Radio-Access-Capability-Update-Ack message %d bytes length",index);

    DBG_LEAVE();
    return(TRUE);
}






/*
 * bssgp_encode_flow_control_ms
 */
BOOL
bssgp_encode_flow_control_ms(BSSGP_FLOW_CONTROL_MS_MSG flow_control_ms,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_flow_control_ms",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_FLOW_CONTROL_MS;

	DBG_TRACE("Encoding Flow-Control-MS message");
	/* Encode the individual IE indicators */
	/* tlli */
    if (flow_control_ms.tlli.ie_present) 
    {
	    bssgp_ie_encode_tlli(flow_control_ms.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);
    }
    else {
        DBG_ERROR("Mandatory element TLLI not specified for flow-control-ms msg!");
        DBG_LEAVE();
        return (FALSE);
    }

	/* tag */
	bssgp_ie_encode_tag(flow_control_ms.tag,&msg[index],&index);

	/* MS Bucket Size */
	bssgp_ie_encode_ms_bucket_size(flow_control_ms.ms_bucket_size,&msg[index],&index);

	/* Bucket Leak Rate */
	bssgp_ie_encode_bucket_leak_rate(flow_control_ms.bucket_leak_rate,&msg[index],&index);

	*msg_len += index;
	DBG_TRACE("Flow-Control-MS message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_encode_flow_control_ms_ack
 */
BOOL
bssgp_encode_flow_control_ms_ack(BSSGP_FLOW_CONTROL_MS_ACK_MSG flow_control_ms_ack,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_flow_control_ms_ack",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_FLOW_CONTROL_MS_ACK;

	DBG_TRACE("Encoding Flow-Control-MS-ACK message");
	/* Encode the individual IE indicators */
	/* tlli */
    if (flow_control_ms_ack.tlli.ie_present) 
    {
	    bssgp_ie_encode_tlli(flow_control_ms_ack.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);
    }
    else {
        DBG_ERROR("Mandatory element TLLI not specified for flow-control-ms-ack msg!");
        DBG_LEAVE();
        return (FALSE);
    }

	/* tag */
	bssgp_ie_encode_tag(flow_control_ms_ack.tag,&msg[index],&index);

	*msg_len += index;
	DBG_TRACE("Flow-Control-MS-ACK message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_encode_flush_ll
 */
BOOL
bssgp_encode_flush_ll(BSSGP_FLUSH_LL_MSG flush_ll,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_flush_ll",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_FLUSH_LL;

	DBG_TRACE("Encoding Flush-LL message");
	/* Encode the individual IE indicators */
	/* tlli */
    if (flush_ll.tlli.ie_present) 
    {
	    bssgp_ie_encode_tlli(flush_ll.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);
    }
    else {
        DBG_ERROR("Mandatory element TLLI not specified for flush-ll msg!");
        DBG_LEAVE();
        return (FALSE);
    }

	/* bvci (old) */
    if (flush_ll.bvci_old.ie_present)
    {
	    bssgp_ie_encode_bvci(flush_ll.bvci_old,&msg[index],&index);
    }
    else {
        DBG_ERROR("Mandatory element BVCI (old) not specified for flush-ll msg!");
        DBG_LEAVE();
        return (FALSE);
    }

	/* bvci (new) */
    if (flush_ll.bvci_new.ie_present)
    {
	    bssgp_ie_encode_bvci(flush_ll.bvci_new,&msg[index],&index);
    }

	*msg_len += index;
	DBG_TRACE("Encoding Flush-LL message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_encode_flush_ll_ack
 */
BOOL
bssgp_encode_flush_ll_ack(BSSGP_FLUSH_LL_ACK_MSG flush_ll_ack,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_flush_ll_ack",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_FLUSH_LL_ACK;

	DBG_TRACE("Encoding Flush-LL-ACK message");
	/* tlli */
    if (flush_ll_ack.tlli.ie_present) 
    {
	    bssgp_ie_encode_tlli(flush_ll_ack.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);
    }
    else {
        DBG_ERROR("Mandatory element TLLI not specified for flush-ll-ack msg!");
        DBG_LEAVE();
        return (FALSE);
    }

    /* flush action */
    bssgp_ie_encode_flush_action(flush_ll_ack.flush_action,&msg[index],&index);
    
	/* bvci (new) */
    if (flush_ll_ack.bvci_new.ie_present)
    {
	    bssgp_ie_encode_bvci(flush_ll_ack.bvci_new,&msg[index],&index);
    }

	*msg_len += index;
	DBG_TRACE("Flush-LL-ACK message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);

}

/*
 * bssgp_encode_llc_discarded
 */
BOOL
bssgp_encode_llc_discarded(BSSGP_LLC_DISCARDED_MSG llc_discarded,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_llc_discarded",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_LLC_DISCARDED;

	DBG_TRACE("Encoding LLC-Discarded message");
	/* tlli */
    if (llc_discarded.tlli.ie_present) 
    {
	    bssgp_ie_encode_tlli(llc_discarded.tlli,&msg[index],&index, BSSGP_FORMAT_TLV);
    }
    else {
        DBG_ERROR("Mandatory element TLLI not specified for llc-discarded msg!");
        DBG_LEAVE();
        return (FALSE);
    }
    
    /* LLC frames discarded */
    bssgp_ie_encode_llc_frames_discarded(llc_discarded.num_frames,&msg[index],&index);

	/* bvci */
    if (llc_discarded.bvci.ie_present)
    {
	    bssgp_ie_encode_bvci(llc_discarded.bvci,&msg[index],&index);
    }
    else {
        DBG_ERROR("Mandatory element BVCI not specified for llc-discarded msg!");
        DBG_LEAVE();
        return (FALSE);
    }
    
    /* number of octets deleted */
    bssgp_ie_encode_num_octets_affected(llc_discarded.num_octets,&msg[index],&index);
    
	*msg_len += index;
	DBG_TRACE("LLC-Discarded message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_encode_flow_control_bvc
 */
BOOL
bssgp_encode_flow_control_bvc(BSSGP_FLOW_CONTROL_BVC_MSG flow_control_bvc, UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_flow_control_bvc",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_FLOW_CONTROL_BVC;

	DBG_TRACE("Encoding Flow-Control-BVC message");

	/* tag */
	bssgp_ie_encode_tag(flow_control_bvc.tag,&msg[index],&index);

	/* BVC Bucket Size */
	bssgp_ie_encode_bvc_bucket_size(flow_control_bvc.bvc_bucket_size,&msg[index],&index);

	/* Bucket Leak Rate */
	bssgp_ie_encode_bucket_leak_rate(flow_control_bvc.bucket_leak_rate,&msg[index],&index);

	/* Bmax default ms */
	bssgp_ie_encode_bmax_def_ms(flow_control_bvc.bmax,&msg[index],&index);

	/* R_default_MS */
	bssgp_ie_encode_r_def_ms(flow_control_bvc.r_default,&msg[index],&index);

	/* BVC measurement */
    if (flow_control_bvc.bvc_meas.ie_present)
	    bssgp_ie_encode_bvc_measurement(flow_control_bvc.bvc_meas,&msg[index],&index);
    
	*msg_len += index;
	DBG_TRACE("Flow-Control-BVC message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_encode_flow_control_bvc_ack
 */
BOOL
bssgp_encode_flow_control_bvc_ack(BSSGP_FLOW_CONTROL_BVC_ACK_MSG flow_control_bvc_ack, UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_flow_control_bvc_ack",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_FLOW_CONTROL_BVC_ACK;

	DBG_TRACE("Encoding Flow-Control-BVC-ACK message");

	/* tag */
	bssgp_ie_encode_tag(flow_control_bvc_ack.tag,&msg[index],&index);

	*msg_len += index;
	DBG_TRACE("Flow-Control-BVC-ACK message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_encode_bvc_block
 */
BOOL
bssgp_encode_bvc_block(BSSGP_BVC_BLOCK_MSG bvc_block,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_bvc_block",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_BVC_BLOCK;

	DBG_TRACE("Encoding BVC-Block message");

	/* bvci */
    if (bvc_block.bvci.ie_present)
    {
	    bssgp_ie_encode_bvci(bvc_block.bvci,&msg[index],&index);
    }
    else {
        DBG_ERROR("Mandatory element BVCI not specified for bvc-block msg!");
        DBG_LEAVE();
        return (FALSE);
    }

    /* cause */
    if (bvc_block.cause.ie_present)
    {
	    bssgp_ie_encode_exception_cause(bvc_block.cause,&msg[index],&index);
    }
    else {
        DBG_ERROR("Mandatory element BVCI not specified for bvc-block msg!");
        DBG_LEAVE();
        return (FALSE);
    }

	*msg_len += index;
	DBG_TRACE("BVC-Block message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);

}


/*
 * bssgp_encode_bvc_block_ack
 */
BOOL
bssgp_encode_bvc_block_ack(BSSGP_BVC_BLOCK_ACK_MSG bvc_block_ack,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_bvc_block_ack",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_BVC_BLOCK_ACK;

	DBG_TRACE("Encoding BVC-Block-ACK message");

	/* bvci */
    if (bvc_block_ack.bvci.ie_present)
    {
	    bssgp_ie_encode_bvci(bvc_block_ack.bvci,&msg[index],&index);
    }
    else {
        DBG_ERROR("Mandatory element BVCI not specified for bvc-block-ack msg!");
        DBG_LEAVE();
        return (FALSE);
    }

	*msg_len += index;
	DBG_TRACE("BVC-Block-ACK message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);

}


/*
 * bssgp_encode_bvc_unblock
 */
BOOL
bssgp_encode_bvc_unblock(BSSGP_BVC_UNBLOCK_MSG bvc_unblock,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_bvc_unblock",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_UNBLOCK;

	DBG_TRACE("Encoding BVC-Unblock message");

	/* bvci */
    if (bvc_unblock.bvci.ie_present)
    {
	    bssgp_ie_encode_bvci(bvc_unblock.bvci,&msg[index],&index);
    }
    else {
        DBG_ERROR("Mandatory element BVCI not specified for bvc-unblock msg!");
        DBG_LEAVE();
        return (FALSE);
    }

	*msg_len += index;
	DBG_TRACE("BVC-Unblock message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);

}


/*
 * bssgp_encode_bvc_unblock_ack
 */
BOOL
bssgp_encode_bvc_unblock_ack(BSSGP_BVC_UNBLOCK_ACK_MSG bvc_unblock_ack,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_bvc_unblock_ack",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_UNBLOCK_ACK;

	DBG_TRACE("Encoding BVC-Unblock-ACK message");

	/* bvci */
    if (bvc_unblock_ack.bvci.ie_present)
    {
	    bssgp_ie_encode_bvci(bvc_unblock_ack.bvci,&msg[index],&index);
    }
    else {
        DBG_ERROR("Mandatory element BVCI not specified for bvc-unblock-ack msg!");
        DBG_LEAVE();
        return (FALSE);
    }

	*msg_len += index;
	DBG_TRACE("BVC-Unblock-ACK message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);

}


/*
 * bssgp_encode_bvc_reset
 */
BOOL
bssgp_encode_bvc_reset(BSSGP_BVC_RESET_MSG bvc_reset,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_bvc_reset",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_RESET;

	DBG_TRACE("Encoding BVC-Reset message");

	/* bvci */
    if (bvc_reset.bvci.ie_present)
    {
	    bssgp_ie_encode_bvci(bvc_reset.bvci,&msg[index],&index);
    }
    else {
        DBG_ERROR("Mandatory element BVCI not specified for bvc-reset msg!");
        DBG_LEAVE();
        return (FALSE);
    }

    /* cause */
    if (bvc_reset.cause.ie_present)
    {
	    bssgp_ie_encode_exception_cause(bvc_reset.cause,&msg[index],&index);
    }
    else {
        DBG_ERROR("Mandatory element BVCI not specified for bvc-reset msg!");
        DBG_LEAVE();
        return (FALSE);
    }

    /* cell id */
    if (bvc_reset.cell_id.ie_present) {
        bssgp_ie_encode_cell_id(bvc_reset.cell_id,&msg[index],&index);
    }

	*msg_len += index;
	DBG_TRACE("BVC-Reset message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);

}


/*
 * bssgp_encode_bvc_reset_ack
 */
BOOL
bssgp_encode_bvc_reset_ack(BSSGP_BVC_RESET_ACK_MSG bvc_reset_ack,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_bvc_reset_ack",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_RESET_ACK;

	DBG_TRACE("Encoding BVC-Reset-Ack message");

	/* bvci */
    if (bvc_reset_ack.bvci.ie_present)
    {
	    bssgp_ie_encode_bvci(bvc_reset_ack.bvci,&msg[index],&index);
    }
    else {
        DBG_ERROR("Mandatory element BVCI not specified for bvc-reset-ack msg!");
        DBG_LEAVE();
        return (FALSE);
    }

    /* cell id */
    if (bvc_reset_ack.cell_id.ie_present) {
        bssgp_ie_encode_cell_id(bvc_reset_ack.cell_id,&msg[index],&index);
    }

	*msg_len += index;
	DBG_TRACE("BVC-Reset-Ack message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);

}


/*
 * bssgp_encode_bvc_status
 */
BOOL
bssgp_encode_bvc_status(BSSGP_BVC_STATUS_MSG bvc_status,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_encode_bvc_status",BSSGP_LAYER);
    DBG_ENTER();
	UINT8 *byte;
	UINT16 index = 0;

	byte = (UINT8*)&msg[index];
    index++;

	/* Set the PDU type */
	*byte = (UINT8)BSSGP_PDU_STATUS;

	DBG_TRACE("Encoding BVC-Status message");

    /* cause */
    if (bvc_status.cause.ie_present)
    {
	    bssgp_ie_encode_exception_cause(bvc_status.cause,&msg[index],&index);
    }
    else {
        DBG_ERROR("Mandatory element BVCI not specified for bvc-status msg!");
        DBG_LEAVE();
        return (FALSE);
    }

	/* bvci */
    if (bvc_status.bvci.ie_present)
    {
	    bssgp_ie_encode_bvci(bvc_status.bvci,&msg[index],&index);
    }
    else {
        DBG_ERROR("Mandatory element BVCI not specified for bvc-status msg!");
        DBG_LEAVE();
        return (FALSE);
    }

	*msg_len += index;
	DBG_TRACE("BVC-Status message %d bytes length",index);

    DBG_LEAVE();
    return (TRUE);

}
