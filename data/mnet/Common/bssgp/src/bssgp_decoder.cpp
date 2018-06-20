/*
 ****************************************************************************************
 *                                                                                      *
 *  Copyright Cisco Systems, Inc 2000 All rights reserved                               *
 *                                                                                      *
 *  File                : bssgp_decoder.cpp                                             *
 *                                                                                      *
 *  Description         : Decoding functions for the BSS-GP layer module                *
 *                                                                                      *
 *  Author              : Dinesh Nambisan                                               *
 *                                                                                      *
 *--------------------------- Notes ----------------------------------------------------*
 *  References are to sections numbers of the BSSGP spec. (GSM 08.18) unless otherwise  *
 *  indicated                                                                           *
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description                                               *
 *--------------------------------------------------------------------------------------*
 * 00:00 |10/10/00 | DSN    | File created                                              *
 *       |         |        |                                                           *
 ****************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include "bssgp\bssgp_prot.h"
#include "bssgp\bssgp_util.h"
#include "bssgp\bssgp_decoder.h"
#include "bssgp\bssgp_ie_decoder.h"
#include "bssgp\bssgp_util.h"
#include "bssgp\bssgp_api.h"
#include "bssgp\bssgp_unitdata.h"


/*
 * MAIN TO DO: Use the incoming bool flag to see if we just need protocol decode functionality
 *  or if we need to actually process the msg
 *
 */



/*
 * bssgp_decode_msg
 *
 * Notes : entry point for packets sent through BSSGP, either from the
 *         IP network side or to the IP network side (in the latter
 *         case, it is being used more as a protocol decoder, with
 *         the boolean flag incoming set to FALSE)
 */
BOOL
bssgp_decode_msg(UINT8 *msg,UINT16 msg_len,BOOL incoming,
            BSSGP_NETWORK_QOS_LEVEL qos_level,BSSGP_BVCI bvci,BOOL *free_buff)
{
    DBG_FUNC("bssgp_decode_msg",BSSGP_LAYER);
    DBG_ENTER();
    BOOL status = FALSE;
    UINT8 *byte;
    UINT16 index = 0, pdu_len;

    byte = (UINT8*)&msg[index];

    pdu_len = msg_len - 1;


    DBG_TRACE("PDU Type : %s",bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));

    switch(*byte) {
        case BSSGP_PDU_DL_UNITDATA: {
            status = bssgp_decode_dl_unitdata(&msg[index],&pdu_len,qos_level);
            *free_buff = TRUE;
        }
        break;

        case BSSGP_PDU_UL_UNITDATA:{
            status = bssgp_decode_ul_unitdata(&msg[index],&pdu_len,qos_level,bvci);
            *free_buff = TRUE;
        }
        break;

        case BSSGP_PDU_RA_CAP:{
            status = bssgp_decode_radio_cap(&msg[index],&pdu_len,incoming);
            *free_buff = TRUE;
        }
        break;
        case BSSGP_PDU_PAGING_PS:{
            status = bssgp_decode_paging_ps(&msg[index],&pdu_len,incoming);
            *free_buff = TRUE;
        }
        break;

        case BSSGP_PDU_RA_CAP_UPDATE:{
            status = bssgp_decode_radio_cap_update(&msg[index],&pdu_len,incoming);
            *free_buff = TRUE;
        }
        break;

        case BSSGP_PDU_RA_CAP_UPDATE_ACK:{
            status = bssgp_decode_radio_cap_update_ack(&msg[index],&pdu_len,incoming);
            *free_buff = TRUE;
        }
        break;

        case BSSGP_PDU_RADIO_STATUS:{
            status = bssgp_decode_radio_status(&msg[index],&pdu_len,incoming);
            *free_buff = TRUE;
        }
        break;

        case BSSGP_PDU_SUSPEND:{
            status = bssgp_decode_suspend(&msg[index],&pdu_len,incoming);
            *free_buff = TRUE;
        }
        break;

        case BSSGP_PDU_SUSPEND_ACK:{
            status = bssgp_decode_suspend_ack(&msg[index],&pdu_len,incoming);
            *free_buff = TRUE;
        }
        break;

        case BSSGP_PDU_SUSPEND_NACK:{
            status = bssgp_decode_suspend_nack(&msg[index],&pdu_len,incoming);
            *free_buff = TRUE;
        }
        break;

        case BSSGP_PDU_RESUME:{
            status = bssgp_decode_resume(&msg[index],&pdu_len,incoming);
            *free_buff = TRUE;
        }
        break;

        case BSSGP_PDU_RESUME_ACK:{
            status = bssgp_decode_resume_ack(&msg[index],&pdu_len,incoming);
            *free_buff = TRUE;
        }
        break;

        case BSSGP_PDU_RESUME_NACK:{
            status = bssgp_decode_resume_nack(&msg[index],&pdu_len,incoming);
            *free_buff = TRUE;
        }
        break;

        case BSSGP_PDU_FLOW_CONTROL_MS:{
            status = bssgp_decode_flow_control_ms(&msg[index],&pdu_len,incoming);
        }
        break;

        case BSSGP_PDU_FLOW_CONTROL_MS_ACK:{
            status = bssgp_decode_flow_control_ms_ack(&msg[index],&pdu_len,incoming);
        }
        break;

        case BSSGP_PDU_FLUSH_LL:{
            status = bssgp_decode_flush_ll(&msg[index],&pdu_len,incoming);
        }
        break;

        case BSSGP_PDU_FLUSH_LL_ACK:{
            status = bssgp_decode_flush_ll_ack(&msg[index],&pdu_len,incoming);
        }
        break;

        case BSSGP_PDU_FLOW_CONTROL_BVC:{
            status = bssgp_decode_flow_control_bvc(&msg[index],&pdu_len,incoming);
        }
        break;

        case BSSGP_PDU_FLOW_CONTROL_BVC_ACK:{
            status = bssgp_decode_flow_control_bvc_ack(&msg[index],&pdu_len,incoming);
        }
        break;
        /*
        case BSSGP_PDU_PTM_UNITDATA:{
            status = bssgp_decode_ptm_unitdata(&msg[index],&pdu_len,incoming,qos_level);
        }
        break;
        
        case BSSGP_PDU_PAGING_CS:{
            status = bssgp_decode_paging_cs(&msg[index],&pdu_len,incoming);
        }
        break;*/

        case BSSGP_PDU_BVC_BLOCK:{
            status = bssgp_decode_block(&msg[index],&pdu_len,incoming);
        }
        break;

        case BSSGP_PDU_BVC_BLOCK_ACK:{
            status = bssgp_decode_block_ack(&msg[index],&pdu_len,incoming);
        }
        break;

        case BSSGP_PDU_RESET:{
            status = bssgp_decode_reset(&msg[index],&pdu_len,incoming);
        }
        break;

        case BSSGP_PDU_RESET_ACK:{
            status = bssgp_decode_reset_ack(&msg[index],&pdu_len,incoming);
        }
        break;

        case BSSGP_PDU_UNBLOCK:{
            status = bssgp_decode_unblock(&msg[index],&pdu_len,incoming);
        }
        break;

        case BSSGP_PDU_UNBLOCK_ACK:{
            status = bssgp_decode_unblock_ack(&msg[index],&pdu_len,incoming);
        }
        break;

        case BSSGP_PDU_LLC_DISCARDED:{
            status = bssgp_decode_llc_discarded(&msg[index],&pdu_len,incoming);
        }
        break;
        /*
        case BSSGP_PDU_SGSN_INVOKE_TRACE:{
            status = bssgp_decode_invoke_trace(&msg[index],&pdu_len,incoming);
        }
        break;*/

        case BSSGP_PDU_STATUS:{
            status = bssgp_decode_status(&msg[index],&pdu_len,incoming);
        }
        break;

        default:{
            DBG_ERROR("Unknown PDU ! Error");
        }
    } /* End of switch pdu_type */
    
    DBG_LEAVE();
    return (status);
}



/*
 * bssgp_decode_dl_unitdata
 *
 * # DL-Unitdata is from GS --> MS in direction
 * # Decode functions are triggered on rx of data from the network side
 * # Hence this function would be called only on the GP-10 platform;
 * and we need to Q the unitdata onto the buffer Qs to be picked up by RLC
 *
 */
BOOL
bssgp_decode_dl_unitdata(UINT8 *msg,UINT16 *msg_len, BSSGP_NETWORK_QOS_LEVEL qos_level)
{
    DBG_FUNC("bssgp_decode_dl_unitdata",BSSGP_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_DL_UNITDATA_MSG dl_unitdata;



    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_DL_UNITDATA) {
        DBG_ERROR("PDU type no DL UNITDATA; %s type",bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    memset(&dl_unitdata,0,sizeof(dl_unitdata));

    DBG_TRACE("Decoding DL-Unitdata message");

    /* Decode the individual IE indicators */

    bssgp_ie_decode_tlli(&dl_unitdata.tlli,&msg[index],&index, BSSGP_FORMAT_V);

    bssgp_ie_decode_qos_profile(&dl_unitdata.qos_profile,&msg[index],&index, BSSGP_FORMAT_V);
    
    bssgp_ie_decode_qos_params(&dl_unitdata.qos_params,&msg[index],&index);
    
    bssgp_ie_decode_pdu_lifetime(&dl_unitdata.pdu_lifetime,&msg[index],&index);
    
    while(index < *msg_len) {
        byte = (UINT8*)&msg[index];
        switch(*byte) {
            case BSSGP_IEI_TLLI:{
                bssgp_ie_decode_tlli(&dl_unitdata.tlli_old,&msg[index],&index, BSSGP_FORMAT_TLV);
            }
            break;

            case BSSGP_IEI_IMSI:{
                bssgp_ie_decode_imsi(&dl_unitdata.imsi,&msg[index],&index);
            }
            break;

            case BSSGP_IEI_MS_RADIO_ACCESS_CAP:{
                bssgp_ie_decode_ms_ra_cap(&dl_unitdata.radio_access_cap,&msg[index],&index);
            }
            break;

            case BSSGP_IEI_DRX_PARAMS:{
                bssgp_ie_decode_drx_params(&dl_unitdata.drx_params,&msg[index],&index);
            }
            break;

            case BSSGP_IEI_ALIGNMENT_OCTETS:{
                bssgp_ie_decode_alignment_octets(&msg[index],&index);
            }
            break;
            
            case BSSGP_IEI_LSA_INFORMATION:{
                bssgp_ie_decode_lsa_info(&dl_unitdata.lsa_info,&msg[index],&index);
            }
            break;
            
            case BSSGP_IEI_LLC_PDU:{
#ifdef WIN32
                LLC_PDU pdu;
                pdu.data = bssgp_ie_decode_llc_pdu_ext(&pdu.data_len,&msg[index],&index);
                llcwin32_pdu_handler(pdu);
#else
                bssgp_ie_decode_llc_pdu(&dl_unitdata, &msg[index],&index);
                bssgp_udt_put_buffer_in_q(dl_unitdata.tlli.tlli,qos_level,(UINT8*)dl_unitdata.pdu.data,dl_unitdata.pdu.data_len,
                            dl_unitdata);

#endif
            }
            break;

            default: {
                DBG_ERROR("BSSGP: Invalid IEI found in DL-UNITDATA decode %#x\n", *byte);
                DBG_LEAVE();
                return (FALSE);
                /* TO DO : Report error to BSSGP peer here */
            }
            break;
        }/* end of switch */
    } /* end of while */
    
    /* TO DO : Need to check for mandatory IEs & do the needful */

    *msg_len -= index;

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_ul_unitdata
 *
 * # UL-Unitdata is from MS --> GS in direction
 * # Decode functions are triggered on rx of data from the network side
 * # Hence this function would be called only on the GS platform;
 *   and we need to pass on the unitdata to LLC (no need for Qing for Uplink)
 */
BOOL
bssgp_decode_ul_unitdata(UINT8 *msg,UINT16 *msg_len,BSSGP_NETWORK_QOS_LEVEL qos_level,BSSGP_BVCI bvci)
{
    DBG_FUNC("bssgp_decode_ul_unitdata",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    T_RIL3_IE_QOS_PROFILE_ID qos_profile;
    RIL3_IE_CELL_IDENTIFIER cell_id;
    RIL3_IE_LSA_IDENTIFIER_LIST lsa_id_list;
    LLC_PDU pdu;
    RIL3_TLLI    tlli;
    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_UL_UNITDATA) {
        DBG_ERROR("PDU type not UL UNITDATA; %s type",bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }


    DBG_TRACE("Decoding UL-Unitdata message");
    
    bssgp_ie_decode_tlli(&tlli,&msg[index],&index, BSSGP_FORMAT_V);

    bssgp_ie_decode_qos_profile(&qos_profile,&msg[index],&index, BSSGP_FORMAT_V);
    
    while(index < *msg_len) {
        byte = (UINT8*)&msg[index];
        switch(*byte) {
            case BSSGP_IEI_CELL_ID:{
                bssgp_ie_decode_cell_id(&cell_id,&msg[index],&index);
            }
            break;
            
            case BSSGP_IEI_ALIGNMENT_OCTETS:{
                bssgp_ie_decode_alignment_octets(&msg[index],&index);
            }
            break;
            
            case BSSGP_IEI_LSA_ID_LIST:{
                bssgp_ie_decode_lsa_identifier_list(&lsa_id_list,&msg[index],&index);
            }
            break;
            
            case BSSGP_IEI_LLC_PDU:{
                pdu.data = bssgp_ie_decode_llc_pdu_ext(&pdu.data_len,&msg[index],&index);
#if defined(MNET_GS)
    /* Hate to wrap an ifdef; but no other way to retain most of the common code */
                bssgp_api_send_ul_unitdata(tlli.tlli,cell_id.cell_id,cell_id.routing_area,pdu.data,pdu.data_len,bvci);
#endif
            }
            break;
            default: {
                DBG_ERROR("BSSGP: Invalid IEI found in UL-UNITDATA decode %#x\n", *byte);
                DBG_LEAVE();
                return (FALSE);
                /* TO DO : need to report error to peer BSSGP */
            }
            break;
        } /* end of switch */
    } /* end of while loop */


    /* TO DO : Need to check for the presence of mandatory IEs */

    *msg_len -= index;

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_paging
 *
 *  Paging PDU defined in 08.18:-
 *
 *  |-------------------------|
 *  |    PDU Type             |
 *  |    IMSI                 |
 *  |    DRX Params           |
 *  |    BVCI                 |
 *  |    Location Area        |
 *  |    Routing Area         |
 *  |    BSS Area Indication  |
 *  |    QoS Profile          |
 *  |    P-TMSI               |
 *  |-------------------------|
 *
 *  From GS --> GP-10
 */
BOOL
bssgp_decode_paging_ps(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_paging_ps",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_PAGING_PS_MSG *paging_msg;


    byte = (UINT8*)&msg[index];
    index++;

    /* First check the PDU type */
    if (*byte != (UINT8)BSSGP_PDU_PAGING_PS) {
        DBG_ERROR("PDU not paging-ps msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_PAGING_PS;
    paging_msg = (BSSGP_PAGING_PS_MSG*)&api_msg.msg.paging_ps;




    paging_msg->imsi.ie_present             = FALSE;
    paging_msg->location_area.ie_present    = FALSE;
    paging_msg->routing_area.ie_present     = FALSE;
    paging_msg->drx_params.ie_present       = FALSE;
    paging_msg->qos_profile.ie_present      = FALSE;
    paging_msg->tmsi.ie_present             = FALSE;
    paging_msg->bvci.ie_present             = FALSE;
    paging_msg->bss_area.ie_present         = FALSE;

    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Packet-Switched Paging message");

    while (index < *msg_len) {
        byte = (UINT8*)&msg[index];
        switch (*byte) {
        
            case BSSGP_IEI_IMSI: {
                if (bssgp_ie_decode_imsi(&paging_msg->imsi,&msg[index],&index)) {
                    paging_msg->imsi.ie_present = TRUE;
                } 
                else {
                    /* TODO : Report error to peer BSSGP here */
                }
            }
            break;

            case BSSGP_IEI_BVCI: {
                if (bssgp_ie_decode_bvci(&paging_msg->bvci,&msg[index],&index)) {
                }
                else {
                    /* TODO : Report error to peer BSSGP here */
                }
            }
            break;
        
            case BSSGP_IEI_LOCATION_AREA:{
                if (bssgp_ie_decode_location_area(&paging_msg->location_area,&msg[index],&index)) {
                    paging_msg->location_area.ie_present = TRUE;
                }
                else {
                    /* TODO : Report error to peer BSSGP here */
                }
            }
            break;

            case BSSGP_IEI_ROUTING_AREA:{
                if(bssgp_ie_decode_routing_area(&paging_msg->routing_area,&msg[index],&index)) {
                    paging_msg->routing_area.ie_present     = TRUE;
                }
                else {
                    /* TODO : Report error to peer BSSGP here */
                }
            }
            break;

            case BSSGP_IEI_BSS_AREA_IND:{
                if(bssgp_ie_decode_bss_area_ind(&paging_msg->bss_area,&msg[index],&index)) {
                    paging_msg->bss_area.ie_present     = TRUE;
                }
                else {
                    /* TODO : Report error to peer BSSGP here */
                }
            }
            break;

            case BSSGP_IEI_DRX_PARAMS:{
                if(bssgp_ie_decode_drx_params(&paging_msg->drx_params,&msg[index],&index)) {
                    paging_msg->drx_params.ie_present       = TRUE;
                }
                else {
                    /* TODO : Report error to peer BSSGP here */
                }
            }
            break;

            case BSSGP_IEI_QOS_PROFILE:{
                if(bssgp_ie_decode_qos_profile(&paging_msg->qos_profile,&msg[index],&index, BSSGP_FORMAT_TLV)) {
                    paging_msg->qos_profile.ie_present      = TRUE;
                }
                else {
                    /* TODO : Report error to peer BSSGP here */
                }
            }
            break;

            case BSSGP_IEI_TMSI:{
                if (bssgp_ie_decode_tmsi(&paging_msg->tmsi,&msg[index],&index)) {
                    paging_msg->tmsi.ie_present             = TRUE;
                }
                else {
                    /* TODO : Report error to peer BSSGP here */
                }
            }
            break;
            
            default: {
                DBG_ERROR("BSSGP: Invalid IEI found in PAGING-PS decode %#x\n", *byte);
                DBG_LEAVE();
                return (FALSE);
                /* TO DO : need to report error to peer BSSGP */
            }
            break;

        } /* end of switch */
    } /* End of while loop */

    *msg_len -= index;
    DBG_TRACE("Paging-PS message %d bytes length",index);

#if defined(MNET_GP10)
    if (!rm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GRR reports error on Packet-Switched paging msg");
    }
    else {
        DBG_TRACE("Posted packet-switched paging msg to GRR successfully");
    }

#elif defined(MNET_GS)
    DBG_ERROR("BSSGP: Unexpected PAGING PS message\n");
    DBG_LEAVE();
    return (FALSE);
#endif

    DBG_LEAVE();
    return (TRUE);
}




/*
 * bssgp_decode_radio_status
 *
 *  Radio Status PDU defined in 08.18:-
 *
 *  |--------------|
 *  | PDU type     |
 *  | TLLI         |
 *  | TMSI         |
 *  | IMSI         |
 *  | Radio Cause  |
 *  |--------------|
 * 
 * From GP-10 to GS
 */
BOOL
bssgp_decode_radio_status(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_radio_status",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_RADIO_STATUS_MSG *radio_status_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != (UINT8)BSSGP_PDU_RADIO_STATUS) {
        DBG_ERROR("PDU not Radio Status msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }


    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_RADIO_STATUS;
    radio_status_msg    = (BSSGP_RADIO_STATUS_MSG*)&api_msg.msg.radio_status;




    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Radio Status message");

    /* tlli */
    if (msg[index] == BSSGP_IEI_TLLI)
    {
        if (!bssgp_ie_decode_tlli(&radio_status_msg->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
            /* TODO : Report error to peer BSSGP here */
        }
    }
    /* tmsi */
    if (msg[index] == BSSGP_IEI_TMSI)
    {
        if (bssgp_ie_decode_tmsi(&radio_status_msg->tmsi,&msg[index],&index)) {
            /* TODO : Report error to peer BSSGP here */
        } 
    }
    
    /* imsi */
    if (msg[index] == BSSGP_IEI_IMSI)
    {
        if (bssgp_ie_decode_imsi(&radio_status_msg->imsi,&msg[index],&index)) {
            /* TODO : Report error to peer BSSGP here */
        }
    }

    /* cause */
    if (bssgp_ie_decode_radio_cause(&radio_status_msg->cause,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    DBG_TRACE("Radio Status message %d bytes length",index);


    *msg_len -= index;

#if defined(MNET_GP10)
    DBG_ERROR("BSSGP: Unexpected RADIO STATUS message\n");
    DBG_LEAVE();
    return (FALSE);
#elif defined(MNET_GS)
    if (!gmm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GMM reports error on Radio Status msg");
    }
    else {
        DBG_TRACE("Posted Radio Status msg to GMM successfully");
    }
#endif


    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_suspend
 *
 *  Suspend PDU defined in 08.18:-
 *
 *  |-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    Routing Area         |
 *  |-------------------------|
 * 
 * From GP-10 to GS
 */
BOOL
bssgp_decode_suspend(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_suspend",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_SUSPEND_MSG *suspend_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_SUSPEND) {
        DBG_ERROR("PDU not Suspend msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_SUSPEND;
    suspend_msg         = (BSSGP_SUSPEND_MSG*)&api_msg.msg.suspend;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Suspend message");

    /* tlli */
    if (!bssgp_ie_decode_tlli(&suspend_msg->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* routing area */
    if (!bssgp_ie_decode_routing_area(&suspend_msg->routing_area,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    DBG_TRACE("Suspend message %d bytes length",index);

    *msg_len -= index;

#if defined(MNET_GP10)
    DBG_ERROR("BSSGP: Unexpected SUSPEND message\n");
    DBG_LEAVE();
    return (FALSE);
#elif defined(MNET_GS)
    if (!gmm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GMM reports error on Suspend msg");
    }
    else {
        DBG_TRACE("Posted Suspend msg to GMM successfully");
    }
#endif

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_suspend_ack
 *
 *  Suspend Ack PDU defined in 08.18:-
 *
 *  |-------------------------|
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
bssgp_decode_suspend_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_suspend_ack",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_SUSPEND_ACK_MSG *suspend_ack_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Set the PDU type */
    if (*byte != (UINT8)BSSGP_PDU_SUSPEND_ACK) {
        DBG_ERROR("PDU not Suspend-Ack msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_SUSPEND_ACK;
    suspend_ack_msg     = (BSSGP_SUSPEND_ACK_MSG*)&api_msg.msg.suspend_ack;

    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Suspend-Ack message");

    /* tlli */
    if (!bssgp_ie_decode_tlli(&suspend_ack_msg->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }
    /* routing area */
    if (!bssgp_ie_decode_routing_area(&suspend_ack_msg->routing_area,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    /* suspend ref num */
    if (!bssgp_ie_decode_suspend_ref_num((UINT8*)&suspend_ack_msg->suspend_ref_num,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    *msg_len -= index;
    DBG_TRACE("Suspend-Ack message %d bytes length",index);

#if defined(MNET_GP10)
    if (!gmm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GMM reports error on Suspend-Ack msg");
    }
    else {
        DBG_TRACE("Posted Suspend-Ack msg to GMM successfully");
    }
#elif defined(MNET_GS)
    DBG_ERROR("BSSGP: Unexpected SUSPEND ACK message\n");
    DBG_LEAVE();
    return (FALSE);
#endif

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_suspend_nack
 *
 *  Suspend NAck PDU defined in 08.18:-
 *
 *  |-------------------------|
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
bssgp_decode_suspend_nack(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_suspend_nack",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_SUSPEND_NACK_MSG *suspend_nack_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != (UINT8)BSSGP_PDU_SUSPEND_NACK) {
        DBG_ERROR("PDU not Suspend-Nack msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_SUSPEND_NACK;
    suspend_nack_msg    = (BSSGP_SUSPEND_NACK_MSG*)&api_msg.msg.suspend_nack;

    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Suspend-Nack message");

    /* tlli */
    if (!bssgp_ie_decode_tlli(&suspend_nack_msg->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* routing area */
    if (!bssgp_ie_decode_routing_area(&suspend_nack_msg->routing_area,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* exception cause */
    if (msg[index] == BSSGP_IEI_CAUSE)
    {
        if (!bssgp_ie_decode_exception_cause(&suspend_nack_msg->exception_cause,&msg[index],&index)) {
            /* TODO : Report error to peer BSSGP here */
        }
    }

    *msg_len -= index;
    DBG_TRACE("Suspend-NAck message %d bytes length",index);

#if defined(MNET_GP10)
    if (!gmm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GMM reports error on Suspend-Nack msg");
    }
    else {
        DBG_TRACE("Posted Suspend-Nack msg to GMM successfully");
    }
#elif defined(MNET_GS)
    DBG_ERROR("BSSGP: Unexpected SUSPEND-NACK message\n");
    DBG_LEAVE();
    return (FALSE);
#endif


    DBG_LEAVE();
    return (TRUE);
}




/*
 * bssgp_decode_resume
 *
 *  Resume PDU defined in 08.18:-
 *
 *  |-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    Routing Area         |
 *  |    Suspend ref num.     |
 *  |-------------------------|
 * 
 * From GP-10 to GS
 */
BOOL
bssgp_decode_resume(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_resume",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_RESUME_MSG *resume_msg;


    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_RESUME) {
        DBG_ERROR("PDU not Resume msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }


    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_RESUME;
    resume_msg          = (BSSGP_RESUME_MSG*)&api_msg.msg.resume;

    DBG_TRACE("Decoding Resume message");

    /* Decode the individual IE indicators */
    /* tlli */
    if (!bssgp_ie_decode_tlli(&resume_msg->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* routing area */
    if (!bssgp_ie_decode_routing_area(&resume_msg->routing_area,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* suspend ref num */
    if (!bssgp_ie_decode_suspend_ref_num((UINT8*)&resume_msg->suspend_ref_num,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }


    *msg_len -= index;
    DBG_TRACE("Resume message %d bytes length",index);

#if defined(MNET_GP10)
    DBG_ERROR("BSSGP: Unexpected RESUME message\n");
    DBG_LEAVE();
    return (FALSE);
#elif defined(MNET_GS)
    if (!gmm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GMM reports error on Resume msg");
    }
    else {
        DBG_TRACE("Posted Resume msg to GMM successfully");
    }
#endif

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_resume_ack
 *
 *  Resume-Ack PDU defined in 08.18:-
 *
 *  |-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    Routing Area         |
 *  |    BVCI (prop. ext.)    |
 *  |-------------------------|
 *
 *  From GS --> GP-10
 */
BOOL
bssgp_decode_resume_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_resume_ack",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_RESUME_ACK_MSG *resume_ack_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_RESUME_ACK) {
        DBG_ERROR("PDU not Resume msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_RESUME_ACK;
    resume_ack_msg      = (BSSGP_RESUME_ACK_MSG*)&api_msg.msg.resume_ack;

    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Resume-Ack message");

    /* tlli */
    if (!bssgp_ie_decode_tlli(&resume_ack_msg->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* routing area */
    if (!bssgp_ie_decode_routing_area(&resume_ack_msg->routing_area,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    *msg_len -= index;
    DBG_TRACE("Resume-Ack message %d bytes length",index);

#if defined(MNET_GP10)
    DBG_ERROR("BSSGP: Unexpected RESUME-ACK message\n");
    DBG_LEAVE();
    return (FALSE);
#elif defined(MNET_GS)
    if (!gmm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GMM reports error on Resume-Ack msg");
    }
    else {
        DBG_TRACE("Posted Resume-Ack msg to GMM successfully");
    }
#endif

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_resume_nack
 *
 *  Resume-NAck PDU defined in 08.18:-
 *
 *  |-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    Routing Area         |
 *  |    Exception cause      |
 *  |-------------------------|
 *
 *  From GS --> GP-10
 */
BOOL
bssgp_decode_resume_nack(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_resume_nack",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_RESUME_NACK_MSG *resume_nack_msg;


    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_RESUME_NACK) {
        DBG_ERROR("PDU not Resume msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_RESUME_NACK;
    resume_nack_msg     = (BSSGP_RESUME_NACK_MSG*)&api_msg.msg.resume_nack;

    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Resume-NAck message");

    /* tlli */
    if (!bssgp_ie_decode_tlli(&resume_nack_msg->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* routing area */
    if (!bssgp_ie_decode_routing_area(&resume_nack_msg->routing_area,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* exception cause */
    if (msg[index] == BSSGP_IEI_CAUSE)
    {
        if (!bssgp_ie_decode_exception_cause(&resume_nack_msg->exception_cause,&msg[index],&index)) {
            /* TODO : Report error to peer BSSGP here */
        } 
    }

    *msg_len -= index;
    DBG_TRACE("Resume-NAck message %d bytes length",index);

#if defined(MNET_GP10)
    DBG_ERROR("BSSGP: Unexpected RESUME NACK message\n");
    DBG_LEAVE();
    return (FALSE);
#elif defined(MNET_GS)
    if (!gmm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GMM reports error on Resume-Nack msg");
    }
    else {
        DBG_TRACE("Posted Resume-Nack msg to GMM successfully");
    }
#endif


    DBG_LEAVE();
    return (TRUE);
}




/*
 * bssgp_decode_location_update
 *
 *  Location-Update PDU defined by 
 *  Cisco-EWTBU (proprietary)
 *
 *  |-------------------------|
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
bssgp_decode_location_update(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_location_update",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_LOCATION_UPDATE_MSG *location_update_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_LOCATION_UPDATE) {
        DBG_ERROR("PDU not Location-Update msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_LOCATION_UPDATE;
    location_update_msg = (BSSGP_LOCATION_UPDATE_MSG*)&api_msg.msg.location_update;

    /* Encode the individual IE indicators */
    DBG_TRACE("Decoding Location-Update message");

    /* imsi */
    if (!bssgp_ie_decode_imsi(&location_update_msg->imsi,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* bvci */
    if (!bssgp_ie_decode_bvci(&location_update_msg->bvci,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* location area */
    if (!bssgp_ie_decode_location_area(&location_update_msg->location_area,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }


    /* routing area */
    if (!bssgp_ie_decode_routing_area(&location_update_msg->routing_area,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    *msg_len -= index;
    DBG_TRACE("Location-Update message %d bytes length",index);

#if defined(MNET_GP10)
    if (!gmm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GMM reports error on Location Update msg");
    }
    else {
        DBG_TRACE("Posted Location Update msg to GMM successfully");
    }
#elif defined(MNET_GS)
    DBG_ERROR("BSSGP: Unexpected LOCATION UPDATE message\n");
    DBG_LEAVE();
    return (FALSE);
#endif

    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_decode_location_update_ack
 *
 *  Location-Update-Ack PDU defined by 
 *  Cisco-EWTBU (proprietary)
 *
 *  |-------------------------|
 *  |    PDU Type             |
 *  |    IMSI                 |
 *  |    Location Area        |
 *  |-------------------------|
 * 
 * From GP-10 to GS
 */
BOOL
bssgp_decode_location_update_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_location_update_ack",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_LOCATION_UPDATE_ACK_MSG *location_update_ack;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_LOCATION_UPDATE_ACK) {
        DBG_ERROR("PDU not Location-Update-Ack msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_LOCATION_UPDATE_ACK;
    location_update_ack = (BSSGP_LOCATION_UPDATE_ACK_MSG*)&api_msg.msg.location_update_ack;

    /* Encode the individual IE indicators */
    DBG_TRACE("Decoding Location-Update-Ack message");


    /* imsi */
    if (!bssgp_ie_decode_imsi(&location_update_ack->imsi,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* location area */
    if (!bssgp_ie_decode_location_area(&location_update_ack->location_area,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    *msg_len -= index;
    DBG_TRACE("Location-Update-Ack message %d bytes length",index);

#if defined(MNET_GP10)
    DBG_ERROR("BSSGP: Unexpected LOCATION UPDATE ACK message\n");
    DBG_LEAVE();
    return (FALSE);
#elif defined(MNET_GS)
    if (!gmm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GMM reports error on Location-Update-Ack msg");
    }
    else {
        DBG_TRACE("Posted Location-Update-Ack msg to GMM successfully");
    }
#endif

    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_decode_location_update_nack
 *
 *  Location-Update-NAck PDU defined by 
 *  Cisco-EWTBU (proprietary)
 *
 *  |-------------------------|
 *  |    PDU Type             |
 *  |    IMSI                 |
 *  |    Location Area        |
 *  |    Reject cause         |
 *  |-------------------------|
 * 
 * From GP-10 to GS
 */
BOOL
bssgp_decode_location_update_nack(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_location_update_nack",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_LOCATION_UPDATE_NACK_MSG *location_update_nack;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_LOCATION_UPDATE_NACK) {
        DBG_ERROR("PDU not Location-Update-NAck msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_LOCATION_UPDATE_NACK;
    location_update_nack= (BSSGP_LOCATION_UPDATE_NACK_MSG*)&api_msg.msg.location_update_nack;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Location-Update-NAck message");


    /* imsi */
    if (!bssgp_ie_decode_imsi(&location_update_nack->imsi,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* location area */
    if (!bssgp_ie_decode_location_area(&location_update_nack->location_area,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }


    /* reject cause */
    if (!bssgp_ie_decode_reject_cause(&location_update_nack->reject_cause,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }


    *msg_len -= index;
    DBG_TRACE("Location-Update-NAck message %d bytes length",index);

#if defined(MNET_GP10)
    DBG_ERROR("BSSGP: Unexpected LOCATION UPDATE NACK message\n");
    DBG_LEAVE();
    return (FALSE);
#elif defined(MNET_GS)
    if (!gmm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GMM reports error on Location-Update-Nack msg");
    }
    else {
        DBG_TRACE("Posted Location-Update-Nack msg to GMM successfully");
    }
#endif

    DBG_LEAVE();
    return (TRUE);
}




/*
 * bssgp_decode_radio_cap
 *
 *  Radio-Access-Capability PDU defined in
 *  08.18
 *
 *  |-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    MS Radio Access Cap  |
 *  |    BVCI (prop. ext.)    |
 *  |-------------------------|
 * 
 * From GS to GP-10
 */
BOOL
bssgp_decode_radio_cap(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_radio_cap",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_RADIO_CAP_MSG *radio_cap;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_RA_CAP) {
        DBG_ERROR("PDU not RA-Cap msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_RADIO_CAP;
    radio_cap           = (BSSGP_RADIO_CAP_MSG*)&api_msg.msg.radio_cap;

    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Radio-Access-Capability message");


    /* tlli */
    if (!bssgp_ie_decode_tlli(&radio_cap->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* ra-cap */
    if (!bssgp_ie_decode_ms_ra_cap(&radio_cap->ra_cap,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    *msg_len -= index;
    DBG_TRACE("Radio-Access-Capability message %d bytes length",index);

#if defined(MNET_GP10)
    if (!gmm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GMM reports error on Radio-Access-Capability msg");
    }
    else {
        DBG_TRACE("Posted Radio-Access-Capability msg to GMM successfully");
    }
#elif defined(MNET_GS)
    DBG_ERROR("BSSGP: Unexpected RA-CAPABILITY message\n");
    DBG_LEAVE();
    return (FALSE);
#endif

    DBG_LEAVE();
    return(TRUE);
}


/*
 * bssgp_decode_radio_cap_update
 *
 *  Radio-Access-Capability-Update PDU defined in
 *  08.18
 *
 *  |-------------------------|
 *  |    PDU Type             |
 *  |    TLLI                 |
 *  |    Tag                  |
 *  |-------------------------|
 * 
 * From GP-10 to GS
 */
BOOL
bssgp_decode_radio_cap_update(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_radio_cap_update",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_RADIO_CAP_UPDATE_MSG *radio_cap_update;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_RA_CAP_UPDATE) {
        DBG_ERROR("PDU not RA-Cap-Update msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_RADIO_CAP_UPDATE;
    radio_cap_update    = (BSSGP_RADIO_CAP_UPDATE_MSG*)&api_msg.msg.radio_cap_update;

    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Radio-Access-Capability-Update message");

    /* tlli */
    if (!bssgp_ie_decode_tlli(&radio_cap_update->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* tag */
    if (!bssgp_ie_decode_tag(&radio_cap_update->tag,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    *msg_len -= index;
    DBG_TRACE("Radio-Access-Capability-Update message %d bytes length",index);

#if defined(MNET_GP10)
    DBG_ERROR("BSSGP: Unexpected RA-CAPABILITY-UPDATE message\n");
    DBG_LEAVE();
    return (FALSE);
#elif defined(MNET_GS)
    if (!gmm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GMM reports error on Radio-Access-Capability-Update msg");
    }
    else {
        DBG_TRACE("Posted Radio-Access-Capability-Update msg to GMM successfully");
    }
#endif

    DBG_LEAVE();
    return(TRUE);
}


/*
 * bssgp_decode_radio_cap_update_ack
 *
 *  Radio-Access-Capability-Update-ACK PDU defined in
 *  08.18
 *
 *  |-------------------------|
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
bssgp_decode_radio_cap_update_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_radio_cap_update_ack",BSSGP_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_RADIO_CAP_UPDATE_ACK_MSG *radio_cap_update_ack;

    byte = (UINT8*)&msg[index];

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_RA_CAP_UPDATE_ACK) {
        DBG_ERROR("PDU not RA-Cap-Update-Ack msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }


    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_RADIO_CAP_UPDATE_ACK;
    radio_cap_update_ack= (BSSGP_RADIO_CAP_UPDATE_ACK_MSG*)&api_msg.msg.radio_cap_update_ack;

    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Radio-Access-Capability-Update-Ack message");

    /* tlli */
    if (!bssgp_ie_decode_tlli(&radio_cap_update_ack->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* tag */
    if (!bssgp_ie_decode_tag(&radio_cap_update_ack->tag,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }


    /* imsi */
    if (msg[index] == BSSGP_IEI_IMSI)
    {
        if (!bssgp_ie_decode_imsi(&radio_cap_update_ack->imsi,&msg[index],&index)) {
            /* TODO : Report error to peer BSSGP here */
        }
    }

    /* RA-Cap-UPD-CAUSE */
    if (!bssgp_ie_decode_ra_cap_update_cause(&radio_cap_update_ack->cause,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    if ((index < *msg_len) && (msg[index] == BSSGP_IEI_MS_RADIO_ACCESS_CAP))
    {
        if (!bssgp_ie_decode_ms_ra_cap(&radio_cap_update_ack->ra_cap,&msg[index],&index)) {
            /* TODO : Report error to peer BSSGP here */
        }  
    }
    
    *msg_len -= index;
    DBG_TRACE("Radio-Access-Capability-Update-Ack message %d bytes length",index);

#if defined(MNET_GP10)
    DBG_ERROR("BSSGP: Unexpected RA-CAPABILITY-UPDATE-ACK message\n");
    DBG_LEAVE();
    return (FALSE);
#elif defined(MNET_GS)
    if (!gmm_bssgp_msg_callback(api_msg)) {
        DBG_ERROR("GMM reports error on Radio-Access-Capability-Update-Ack msg");
    }
    else {
        DBG_TRACE("Posted Radio-Access-Capability-Update-Ack msg to GMM successfully");
    }
#endif

    DBG_LEAVE();
    return(TRUE);
}




/*
 * bssgp_decode_flow_control_ms
 */
BOOL
bssgp_decode_flow_control_ms(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_flow_control_ms",BSSGP_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_FLOW_CONTROL_MS_MSG *flow_control_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_FLOW_CONTROL_MS) {
        DBG_ERROR("PDU not Flow control MS msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_FLOW_CONTROL_MS;
    flow_control_msg    = (BSSGP_FLOW_CONTROL_MS_MSG*)&api_msg.msg.flow_control_ms;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Flow control MS message");

    /* tlli */
    if (!bssgp_ie_decode_tlli(&flow_control_msg->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* tag */
    if (!bssgp_ie_decode_tag(&flow_control_msg->tag,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    /* MS bucket size */
    if (!bssgp_ie_decode_ms_bucket_size(&flow_control_msg->ms_bucket_size,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    } 
    
    /* Bucket leak rate */
    if (!bssgp_ie_decode_bucket_leak_rate(&flow_control_msg->bucket_leak_rate,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    } 
    
    DBG_TRACE("Flow control MS message %d bytes length",index);

    *msg_len -= index;
    
    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_decode_flow_control_ms_ack
 */
BOOL
bssgp_decode_flow_control_ms_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_flow_control_ms_ack",BSSGP_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_FLOW_CONTROL_MS_ACK_MSG *flow_control_ack_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_FLOW_CONTROL_MS_ACK) {
        DBG_ERROR("PDU not Flow control MS msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_FLOW_CONTROL_MS_ACK;
    flow_control_ack_msg    = (BSSGP_FLOW_CONTROL_MS_ACK_MSG*)&api_msg.msg.flow_control_ms_ack;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Flow control MS Ack message");

    /* tlli */
    if (!bssgp_ie_decode_tlli(&flow_control_ack_msg->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* tag */
    if (!bssgp_ie_decode_tag(&flow_control_ack_msg->tag,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    DBG_TRACE("Flow control MS Ack message %d bytes length",index);

    *msg_len -= index;
    
    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_decode_flush_ll
 */
BOOL
bssgp_decode_flush_ll(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_flush_ll",BSSGP_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_FLUSH_LL_MSG *flush_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_FLUSH_LL) {
        DBG_ERROR("PDU not Flush LL msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_FLUSH_LL;
    flush_msg           = (BSSGP_FLUSH_LL_MSG*)&api_msg.msg.flush_ll;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Flush LL message");

    /* tlli */
    if (!bssgp_ie_decode_tlli(&flush_msg->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* bvci old */
    if (!bssgp_ie_decode_bvci(&flush_msg->bvci_old,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    /* bvci new */
    if (msg[index] == BSSGP_IEI_BVCI)
    {
        if (!bssgp_ie_decode_bvci(&flush_msg->bvci_new,&msg[index],&index)) {
            /* TODO : Report error to peer BSSGP here */
        } 
    }
    
    DBG_TRACE("Flush LL message %d bytes length",index);

    *msg_len -= index;

    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_decode_flush_ll_ack
 */
BOOL
bssgp_decode_flush_ll_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_flush_ll_ack",BSSGP_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_FLUSH_LL_ACK_MSG *flush_ack_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_FLUSH_LL_ACK) {
        DBG_ERROR("PDU not Flush LL Ack msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_FLUSH_LL_ACK;
    flush_ack_msg       = (BSSGP_FLUSH_LL_ACK_MSG*)&api_msg.msg.flush_ll_ack;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Flush LL Ack message");

    /* tlli */
    if (!bssgp_ie_decode_tlli(&flush_ack_msg->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* Flush action */
    if (!bssgp_ie_decode_flush_action(&flush_ack_msg->flush_action,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    /* bvci new */
    if (msg[index] == BSSGP_IEI_BVCI)
    {
        if (!bssgp_ie_decode_bvci(&flush_ack_msg->bvci_new,&msg[index],&index)) {
            /* TODO : Report error to peer BSSGP here */
        } 
    }
    
    /* Number of octets affected */
    if (!bssgp_ie_decode_num_octets_affected(&flush_ack_msg->num_octets,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    DBG_TRACE("Flush LL Ack message %d bytes length",index);

    *msg_len -= index;
    
    DBG_LEAVE();
    return (TRUE);

}


/*
 * bssgp_decode_llc_discarded
 */
BOOL
bssgp_decode_llc_discarded(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_llc_discarded",BSSGP_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_LLC_DISCARDED_MSG *llc_discarded_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_FLUSH_LL) {
        DBG_ERROR("PDU not Flush LL msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_LLC_DISCARDED;
    llc_discarded_msg   = (BSSGP_LLC_DISCARDED_MSG*)&api_msg.msg.llc_discarded;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding LLC Discarded message");

    /* tlli */
    if (!bssgp_ie_decode_tlli(&llc_discarded_msg->tlli,&msg[index],&index, BSSGP_FORMAT_TLV)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* LLC Frames Discarded */
    if (!bssgp_ie_decode_llc_frames_discarded(&llc_discarded_msg->num_frames,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* bvci */
    if (!bssgp_ie_decode_bvci(&llc_discarded_msg->bvci,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
      
    /* Number of octets affected */
    if (!bssgp_ie_decode_num_octets_affected(&llc_discarded_msg->num_octets,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
        
    DBG_TRACE("LLC Discarded message %d bytes length",index);

    *msg_len -= index;

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_flow_control_bvc
 */
BOOL
bssgp_decode_flow_control_bvc(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_flow_control_bvc",BSSGP_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_FLOW_CONTROL_BVC_MSG *flow_control_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_FLOW_CONTROL_BVC) {
        DBG_ERROR("PDU not Flow control BVC msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_FLOW_CONTROL_BVC;
    flow_control_msg    = (BSSGP_FLOW_CONTROL_BVC_MSG*)&api_msg.msg.flow_control_bvc;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Flow control BVC message");

    /* tag */
    if (!bssgp_ie_decode_tag(&flow_control_msg->tag,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    /* BVC bucket size */
    if (!bssgp_ie_decode_bvc_bucket_size(&flow_control_msg->bvc_bucket_size,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    } 
    
    /* Bucket leak rate */
    if (!bssgp_ie_decode_bucket_leak_rate(&flow_control_msg->bucket_leak_rate,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    } 
    
    /* Bmax default MS */
    if (!bssgp_ie_decode_bmax_def_ms(&flow_control_msg->bmax,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    } 
    
    /* R default MS */
    if (!bssgp_ie_decode_r_def_ms(&flow_control_msg->r_default,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    } 
    
    /* BVC measurement */
    if (msg[index] == BSSGP_IEI_BVC_MEASUREMENT)
    {
        if (!bssgp_ie_decode_bvc_measurement(&flow_control_msg->bvc_meas,&msg[index],&index)) {
            /* TODO : Report error to peer BSSGP here */
        } 
    }
    
    DBG_TRACE("Flow control BVC message %d bytes length",index);

    *msg_len -= index;
    
    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_flow_control_bvc_ack
 */
BOOL
bssgp_decode_flow_control_bvc_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_flow_control_bvc_ack",BSSGP_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_FLOW_CONTROL_BVC_ACK_MSG *flow_control_ack_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_FLOW_CONTROL_BVC_ACK) {
        DBG_ERROR("PDU not Flow control BVC Ack msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_FLOW_CONTROL_BVC_ACK;
    flow_control_ack_msg    = (BSSGP_FLOW_CONTROL_BVC_ACK_MSG*)&api_msg.msg.flow_control_bvc_ack;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding Flow control BVC Ack message");

    /* tag */
    if (!bssgp_ie_decode_tag(&flow_control_ack_msg->tag,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    DBG_TRACE("Flow control BVC Ack message %d bytes length",index);

    *msg_len -= index;
    
    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_block
 */
BOOL
bssgp_decode_block(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_block",BSSGP_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_BVC_BLOCK_MSG *block_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_BVC_BLOCK) {
        DBG_ERROR("PDU not BVC block msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_BVC_BLOCK;
    block_msg           = (BSSGP_BVC_BLOCK_MSG*)&api_msg.msg.bvc_block;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding BVC block message");

    /* bvci */
    if (!bssgp_ie_decode_bvci(&block_msg->bvci,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    /* cause */
    if (!bssgp_ie_decode_exception_cause(&block_msg->cause,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    DBG_TRACE("BVC block message %d bytes length",index);

    *msg_len -= index;
    
    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_block_ack
 */
BOOL
bssgp_decode_block_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_block_ack",BSSGP_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_BVC_BLOCK_ACK_MSG *block_ack_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_BVC_BLOCK_ACK) {
        DBG_ERROR("PDU not BVC block msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_BVC_BLOCK_ACK;
    block_ack_msg       = (BSSGP_BVC_BLOCK_ACK_MSG*)&api_msg.msg.bvc_block_ack;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding BVC block ack message");

    /* bvci */
    if (!bssgp_ie_decode_bvci(&block_ack_msg->bvci,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    DBG_TRACE("BVC block ack message %d bytes length",index);

    *msg_len -= index;
    
    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_unblock
 */
BOOL
bssgp_decode_unblock(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_unblock",BSSGP_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_BVC_UNBLOCK_MSG *unblock_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_UNBLOCK) {
        DBG_ERROR("PDU not BVC unblock msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_BVC_UNBLOCK;
    unblock_msg         = (BSSGP_BVC_UNBLOCK_MSG*)&api_msg.msg.bvc_unblock;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding BVC unblock message");

    /* bvci */
    if (!bssgp_ie_decode_bvci(&unblock_msg->bvci,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    DBG_TRACE("BVC unblock message %d bytes length",index);

    *msg_len -= index;
    
    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_unblock_ack
 */
BOOL
bssgp_decode_unblock_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_unblock_ack",BSSGP_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_BVC_UNBLOCK_ACK_MSG *unblock_ack_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_UNBLOCK_ACK) {
        DBG_ERROR("PDU not BVC unblock msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_BVC_UNBLOCK_ACK;
    unblock_ack_msg     = (BSSGP_BVC_UNBLOCK_ACK_MSG*)&api_msg.msg.bvc_unblock_ack;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding BVC unblock ack message");

    /* bvci */
    if (!bssgp_ie_decode_bvci(&unblock_ack_msg->bvci,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    DBG_TRACE("BVC unblock ack message %d bytes length",index);

    *msg_len -= index;
    
    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_reset
 */
BOOL
bssgp_decode_reset(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_reset",BSSGP_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_BVC_RESET_MSG *reset_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_RESET) {
        DBG_ERROR("PDU not BVC reset msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_BVC_RESET;
    reset_msg           = (BSSGP_BVC_RESET_MSG*)&api_msg.msg.bvc_reset;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding BVC reset message");

    /* bvci */
    if (!bssgp_ie_decode_bvci(&reset_msg->bvci,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    /* cause */
    if (!bssgp_ie_decode_exception_cause(&reset_msg->cause,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    /* Cell identifier */
    if (msg[index] == BSSGP_IEI_CELL_ID)
    {
        if (!bssgp_ie_decode_cell_id(&reset_msg->cell_id,&msg[index],&index)) {
            /* TODO : Report error to peer BSSGP here */
        }  
    }
    
    DBG_TRACE("BVC reset message %d bytes length",index);

    *msg_len -= index;
    
    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_reset_ack
 */
BOOL
bssgp_decode_reset_ack(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_reset_ack",BSSGP_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_BVC_RESET_ACK_MSG *reset_ack_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_RESET_ACK) {
        DBG_ERROR("PDU not BVC reset ack msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_BVC_RESET_ACK;
    reset_ack_msg       = (BSSGP_BVC_RESET_ACK_MSG*)&api_msg.msg.bvc_reset_ack;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding BVC reset ack message");

    /* bvci */
    if (!bssgp_ie_decode_bvci(&reset_ack_msg->bvci,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }
    
    /* Cell identifier */
    if (msg[index] == BSSGP_IEI_CELL_ID)
    {
        if (!bssgp_ie_decode_cell_id(&reset_ack_msg->cell_id,&msg[index],&index)) {
            /* TODO : Report error to peer BSSGP here */
        }  
    }
    
    DBG_TRACE("BVC reset ack message %d bytes length",index);

    *msg_len -= index;
    
    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_decode_status
 */
BOOL
bssgp_decode_status(UINT8 *msg,UINT16 *msg_len, BOOL incoming)
{
    DBG_FUNC("bssgp_decode_status",BSSGP_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0;
    BSSGP_API_MSG   api_msg;
    BSSGP_BVC_STATUS_MSG *status_msg;

    byte = (UINT8*)&msg[index];
    index++;

    /* Check the PDU type */
    if (*byte != BSSGP_PDU_STATUS) {
        DBG_ERROR("PDU not BVC status msg, type %s!",
            bssgp_util_get_pdu_type_str((BSSGP_PDU_TYPE)*byte));
        DBG_LEAVE();
        return (FALSE);
    }

    /* Clean up an API msg so we have it handy */
    memset(&api_msg,0,sizeof(api_msg));
    api_msg.magic       = BSSGP_MAGIC;
    api_msg.msg_type    = BSSGP_API_MSG_STATUS;
    status_msg          = (BSSGP_BVC_STATUS_MSG*)&api_msg.msg.bvc_reset_ack;


    /* Decode the individual IE indicators */
    DBG_TRACE("Decoding BVC status message");

    /* cause */
    if (!bssgp_ie_decode_exception_cause(&status_msg->cause,&msg[index],&index)) {
        /* TODO : Report error to peer BSSGP here */
    }

    /* bvci */
    if (msg[index] == BSSGP_IEI_BVCI)
    {
        if (!bssgp_ie_decode_bvci(&status_msg->bvci,&msg[index],&index)) {
            /* TODO : Report error to peer BSSGP here */
        }
    }
    
    DBG_TRACE("BVC status message %d bytes length",index);

    *msg_len -= index;
    
    DBG_LEAVE();
    return (TRUE);
}


