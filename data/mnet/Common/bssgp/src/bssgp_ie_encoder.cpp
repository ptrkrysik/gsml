/*
 ****************************************************************************************
 *                                                                                      *
 *  Copyright Cisco Systems, Inc 2000 All rights reserved                               *
 *                                                                                      *
 *  File                : bssgp_ie_encoder.cpp                                          *
 *                                                                                      *
 *  Description         : Functions for encoding all the individual IE indicators       *
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
#include "bssgp/bssgp_prot.h"
#include "bssgp/bssgp_util.h"
#include "bssgp/bssgp_ie_encoder.h"


/*
 * bssgp_ie_alignment_octets
 * 
 *----------------------------------------------------------
 *  11.3.1 Alignment Octets
 *  
 *  The Alignment Octets are used to align a subsequent IEI
 *  onto a 32 bit boundary. 
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3-5   spare octet
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_alignment_octets(UINT8 count,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_alignment_octets",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_ALIGNMENT_OCTETS;
    index++;
    DBG_TRACE("Encoding %d Alignment Octets\n", count);

    bssgp_ie_encode_iei_length(count,&msg[index],&index);

    index+=count;

    DBG_TRACE("Alignemnt Octets IEI length %d bytes",index);
    *len += index;

    DBG_LEAVE();
    return;
}


/*
 * bssgp_ie_encode_bmax_def_ms
 *
 *----------------------------------------------------------
 *  11.3.2  Bmax default MS
 *  
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3-4   Bmax
 *  
 *  The Bmax field is coded as in section GSM 08.18 
 *  BVC Bucket Size/Bmax.
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_bmax_def_ms(UINT16 bmax,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_bmax_def_ms",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_BMAX_DEFAULT_MS;
    index++;
    DBG_TRACE("Encoding Bmax default MS IE.");

    bssgp_ie_encode_iei_length(sizeof(UINT16),&msg[index],&index);

    msg[index++] = (UINT8)(bmax >> 8);
    msg[index++] = (UINT8)(bmax);

    DBG_TRACE("Bmax default MS IE length %d bytes",index);
    *len += index;
    
    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_bss_area_ind
 *
 *----------------------------------------------------------
 *  11.3.3  BSS Area Indication
 *  
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3     BSS indicator
 *  
 *  The coding of octet 2 is a binary number indicating 
 *  the Length of the remaining element.
 *  The coding of octet 3 shall not be specified. The 
 *  recipient shall ignore the value of this octet.
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_bss_area_ind(RIL3_BSS_AREA_INDICATION bss_area,
    UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_bss_area_ind",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_BSS_AREA_IND;
    index++;
    DBG_TRACE("Encoding BSS Area Indication IE.");

    bssgp_ie_encode_iei_length(sizeof(UINT8),&msg[index],&index);

    /* Just skip the actual value as it is ignored. */
    index++;

    DBG_TRACE("BSS Area Indication IE length %d bytes",index);
    *len += index;
    
    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_bucket_leak_rate
 *
 *----------------------------------------------------------
 *  11.3.4  Bucket Leak Rate (R)
 *  This information element indicates the leak rate (R) 
 *  to be applied to a flow control bucket. 
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3     R Value (MSB)
 *  octet 4     R Value (LSB)
 *  
 *  The R field is the binary encoding of the rate 
 *  information expressed in 100 bits/sec increments, 
 *  starting from 0 x 100 bits/sec until 65535 x 100 
 *  bits/sec (6 Mbps).
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_bucket_leak_rate(UINT16 leak_rate,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_bucket_leak_rate",BSSGP_ENCODER_LAYER);
    DBG_ENTER();
                                            
    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_BUCKET_LEAK_RATE;
    index++;
    DBG_TRACE("Encoding Bucket leak rate IE.");

    bssgp_ie_encode_iei_length(sizeof(UINT16),&msg[index],&index);

    msg[index++] = (UINT8)(leak_rate >> 8);
    msg[index++] = (UINT8)(leak_rate);

    DBG_TRACE("Bucket leak rate IE length %d bytes",index);
    *len += index;
    
    DBG_LEAVE();
    return;
}




/*
 * bssgp_ie_encode_bvc_bucket_size
 *
 *----------------------------------------------------------
 *  11.3.5  BVC Bucket Size 
 *  This information element indicates the maximum bucket 
 *  size (Bmax) in octets for a BVC. 
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3     Bmax (MSB)
 *  octet 4     Bmax (LSB)
 *  
 *  The Bmax field is the binary encoding of the bucket 
 *  size information expressed in 100 octet increments, 
 *  starting from 0 x 100 octets until 65535 x 100 octets 
 *  (6 Mbytes).
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_bvc_bucket_size(UINT16 bucket_size,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_bvc_bucket_size",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_BVC_BUCKET_SIZE;
    index++;
    DBG_TRACE("Encoding BVC Bucket size IE.");

    bssgp_ie_encode_iei_length(sizeof(UINT16),&msg[index],&index);

    msg[index++] = (UINT8)(bucket_size >> 8);
    msg[index++] = (UINT8)(bucket_size);

    DBG_TRACE("BVC Bucket size IE length %d bytes",index);
    *len += index;
    
    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_bvci
 *
 *----------------------------------------------------------
 *  11.3.6  BVCI (BSSGP Virtual Connection Identifier)
 *  The BVCI identifies a BVC.  The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3-4   Unstructured value
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_bvci(RIL3_BVCI bvci,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_bvci",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 *shorty;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_BVCI;
    index++;

    bssgp_ie_encode_iei_length(sizeof(UINT16),&msg[index],&index);

    shorty = (UINT16*)&msg[index];
    /* now we get shorty ;-) */
    *shorty = htons(bvci.bvci);
    memcpy(&msg[index],shorty,sizeof(UINT16));
    index += 2;

    DBG_TRACE("BVCI %d encoded; IEI length %d bytes",bvci.bvci,index);

    *len += index;

    DBG_LEAVE();
    return;
}

/*
 * bssgp_ie_encode_bvc_measurement
 *
 *----------------------------------------------------------
 *  11.3.7  BVC Measurement
 *  This information element describes average queuing 
 *  delay for a BVC. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3,4   Delay Value (in centi-seconds)
 *  
 *  The Delay Value field is coded as a 16-bit integer 
 *  value in units of centi-seconds (one hundredth of a 
 *  second). This coding provides a range of over 10 minutes 
 *  in increments of 10 msec. As a special case, the 
 *  hexadecimal value 0xFFFF (decimal 65535) shall be 
 *  interpreted as "infinite delay".
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_bvc_measurement(RIL3_BVC_MEASUREMENT bvc_meas,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_bvc_measurement",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_BVC_MEASUREMENT;
    index++;
    DBG_TRACE("Encoding BVC measurement IE.");

    bssgp_ie_encode_iei_length(sizeof(UINT16),&msg[index],&index);

    msg[index++] = (UINT8)(bvc_meas.bvc_meas >> 8);
    msg[index++] = (UINT8)(bvc_meas.bvc_meas);

    DBG_TRACE("BVC measurement IE length %d bytes",index);
    *len += index;
    
    DBG_LEAVE();
    return;
}


/*
 * bssgp_ie_encode_cause
 *
 *----------------------------------------------------------
 *  11.3.8  Cause
 *  The Cause information element indicates the reason for 
 *  an exception condition. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3     Cause value
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_exception_cause(RIL3_BSSGP_CAUSE cause,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_cause",BSSGP_ENCODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_CAUSE;
    index++;

    bssgp_ie_encode_iei_length(sizeof(UINT8),&msg[index],&index);

    byte = &msg[index];
    *byte = (UINT8)cause.cause;
    index++;

    DBG_TRACE("Cause value %s encoded; IEI length %d bytes",
        bssgp_util_get_exception_cause_str((BSSGP_EXCEPTION_CAUSE)cause.cause),
        index);

    *len += index;

    DBG_LEAVE();
    return;
}

/*
 * bssgp_ie_encode_cell_id
 *
 *----------------------------------------------------------
 *  11.3.9  Cell Identifier
 *  This information element uniquely identifies one cell. 
 *  The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octets 3-8  Octets 3 to 8 contain the value part 
 *              (starting with octet 2) of the Routing Area 
 *              Identification IE defined in GSM 04.08 
 *              [11], not including GSM 04.08 IEI
 *  octets 9-10 Octets 9 and 10 contain the value part 
 *              (starting with octet 2) of the Cell 
 *              Identity IE defined in GSM 04.08 [11], 
 *              not including GSM 04.08 IEI 
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_cell_id(RIL3_IE_CELL_IDENTIFIER cell_id,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_cell_id",BSSGP_ENCODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0, *shorty, lac;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_CELL_ID;
    index++;

    bssgp_ie_encode_iei_length(8,&msg[index],&index);

    /*
     * From 04.08:
     *
     * The routing area identification information element is 
     * coded as shown in figure 10.5.130/GSM 04.08 and 
     * table 10.5.148/GSM 04.08.
     *  8   7   6   5   4   3   2   1   
     *  ------------------------------
     *  * MCC digit 2 *  MCC digit 1 *  octet 2
     *  ------------------------------
     *  * MNC digit 3 *  MCC digit 3 *  octet 3
     *  ------------------------------
     *  * MNC digit 2 *  MNC digit 1 *  octet 4
     *  ------------------------------
     *  *           LAC              *  octet 5
     *  ------------------------------
     *  *           LAC cont'd       *  octet 6
     *  ------------------------------
     *  *           RAC              *  octet 7
     *  ------------------------------
     * 
     * 
     */ 

    msg[index] = (cell_id.routing_area.mcc[1] << 4) | cell_id.routing_area.mcc[0] ;
    index++;
    msg[index] = (cell_id.routing_area.mnc[2] << 4) | cell_id.routing_area.mcc[2];
    index++;
    msg[index] = (cell_id.routing_area.mnc[1] << 4) | cell_id.routing_area.mnc[0];
    index++;

    lac = htons(cell_id.routing_area.lac);

    memcpy(&msg[index],&lac,sizeof(UINT16));
    index += sizeof(UINT16);

    msg[index] = cell_id.routing_area.rac;
    index++;

    shorty = (UINT16*)&msg[index];
    *shorty = (UINT16)htons(cell_id.cell_id.value);
    index += sizeof(UINT16);

    DBG_TRACE("Cell Identifier :- MCC : %d%d%d ; MNC : %d%d(%x) ; LAC : %d ; RAC : %d\n"
              "\tCell Identity : %d",
        cell_id.routing_area.mcc[0],cell_id.routing_area.mcc[1],cell_id.routing_area.mcc[2],
        cell_id.routing_area.mnc[0],cell_id.routing_area.mnc[1],cell_id.routing_area.mnc[2],
        cell_id.routing_area.lac,cell_id.routing_area.rac,cell_id.cell_id.value);

    DBG_TRACE("Cell Identifier IE length %d bytes", index);

    *len += index;


    DBG_LEAVE();
    return;
}


/*
 * bssgp_ie_encode_channel_needed
 *
 *----------------------------------------------------------
 *  11.3.10 Channel needed
 *  This information element is coded as defined in 
 *  GSM 09.18. It is relevant to circuit-switched paging 
 *  requests. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3     Rest of element coded as the value part of 
 *              the Channel Needed PDU defined in GSM 09.18, 
 *              not including GSM 09.18 IEI and GSM 09.18 
 *              length indicator
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_channel_needed(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_channel_needed",BSSGP_ENCODER_LAYER);
    DBG_ENTER();
    /* FIXME */

    DBG_LEAVE();
    return;
}

/*
 * bssgp_ie_encode_drx_params
 *
 *----------------------------------------------------------
 *  11.3.11 DRX Parameters
 *  This information element contains MS specific DRX 
 *  information. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3-n   Rest of element coded as the value part 
 *              defined in GSM 04.08, not including GSM 
 *              04.08 IEI and GSM 04.08 octet length 
 *              indicator
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_drx_params(T_CNI_RIL3_IE_DRX_PARAMETER drx_params,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_drx_params",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_DRX_PARAMS;
    index++;

    bssgp_ie_encode_iei_length((sizeof(UINT8)*2),&msg[index],&index);

    /*
     *
     * The value part of a DRX parameter information element 
     * is coded as shown in table 10.5.139/GSM 04.08.
     *  ------------------------------
     *  8   7   6   5 | 4  | 3  2   1   
     *  ------------------------------
     *      SPLIT PG CYCLE CODE             octet 2
     *  ------------------------------
     *  0   0   0   0 | SPL| non-DRXtimer   octet 3
     *  ------------------------------
     * 
     */

    byte    = &msg[index];
    *byte   = drx_params.split_pg_cycle_code;
    index++;


    byte    = &msg[index];
    if (drx_params.split_on_CCCH) {
        *byte   = 0x80 | drx_params.non_drx_timer;
        DBG_TRACE("Encoding DRX params :- Split PG cycle code %d ; Split on CCCH ; Non-DRX timer %d",
            drx_params.split_pg_cycle_code,drx_params.non_drx_timer);
    }
    else {
        *byte   = drx_params.non_drx_timer;
        DBG_TRACE("Encoding DRX params :- Split PG cycle code %d ; Non-DRX timer %d",
            drx_params.split_pg_cycle_code,drx_params.non_drx_timer);
    }
    index++;

    *len += index;
    DBG_TRACE("DRX Params IEI length %d bytes",index);  
    DBG_LEAVE();
    return;
}

/*
 * bssgp_ie_encode_emlpp_priority
 *
 *----------------------------------------------------------
 *  11.3.12 eMLPP-Priority
 *  This element indicates the eMLPP-Priority of a PDU. 
 *  The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3     Rest of element coded as the value part of 
 *              the eMLPP-Priority IE defined in GSM 08.08, 
 *              not including GSM 08.08 IEI and GSM 08.08 
 *              length indicator
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_emlpp_priority(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_emlpp_priority",BSSGP_ENCODER_LAYER);
    DBG_ENTER();
    /* FIXME */

    DBG_LEAVE();
    return;
}

/*
 * bssgp_ie_encode_flush_action
 *
 *----------------------------------------------------------
 *  11.3.13 Flush Action
 *  The Flush action information element indicates to the 
 *  SGSN the action taken by the BSS in response to the 
 *  flush request. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3     Action value
 *  
 *  Table 11.16: Action coding 
 *  Action value    semantics of coding
 *      
 *  Hexadecimal 
 *      
 *  x00  LLC-PDU(s) deleted
 *  x01  LLC-PDU(s) transferred
 *      
 *      All values not explicitly shown are reserved for 
 *      future use 
 *  
 *----------------------------------------------------------
 */
void
bssgp_ie_encode_flush_action(BSSGP_FLUSH_ACTION flush_action,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_flush_action",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_FLUSH_ACTION;
    index++;
    DBG_TRACE("Encoding Flush action IE.");

    bssgp_ie_encode_iei_length(sizeof(UINT8),&msg[index],&index);

    msg[index++] = (UINT8) flush_action;

    DBG_TRACE("Flush action IE length %d bytes",index);
    *len += index;

    DBG_LEAVE();
    return;
}

/*
 * bssgp_ie_encode_imsi
 *
 *----------------------------------------------------------
 *  11.3.14 IMSI
 *  This information element contains the International 
 *  Mobile Subscriber Identity. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator = length of remaining stuff
 *  octet 3-n   Octets 3-n contain an IMSI coded as the 
 *              value part (starting with octet 3) of the 
 *              Mobile Identity IE defined in GSM 04.08, 
 *              not including GSM 04.08 IEI and GSM 04.08 
 *              length indicator
 *  
 *----------------------------------------------------------
 */
void
bssgp_ie_encode_imsi(T_CNI_RIL3_IE_MOBILE_ID imsi,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_ie_encode_imsi",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    div_t val;
    int count;
    UINT16 index = 0;
    char disp_buff[128], temp_buff[10];
    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_IMSI;
    index++;

    val = div(imsi.numDigits,2);
    
    bssgp_ie_encode_iei_length((val.quot+1),&msg[index],&index);
    /*
     * Refer to GSM 04.08; the IEI has the following 
     * structure; but this octet would be the first
     * for us
     * +-----------------------------------------------*
     * *                       *odd/ *                 *
     * *   Identity digit 1    *even * Type of identity* octet 3
     * *                       *indic*                 *
     * +-----------------------+-----------------------*
     */

    byte = &msg[index];
    if (val.rem) {
        /* Odd num of digits */
        *byte = (0x08 | CNI_RIL3_IMSI);
    }
    else {
        /* Even num of digits */
        *byte = CNI_RIL3_IMSI;
    }
    memset(disp_buff,0,128);
    sprintf(disp_buff,"%d",imsi.digits[0]);
    /* Now encode the first digit */
    *byte |= (imsi.digits[0] << 4) ;
    index++;
    for(count=1;count<=(imsi.numDigits-2);count=count+2) {
        byte = &msg[index];
        *byte = (imsi.digits[count]) | (imsi.digits[count+1] << 4);
        index++;
        memset(temp_buff,0,10);
        sprintf(temp_buff,"%d%d",imsi.digits[count],imsi.digits[count+1]);
        strcat(disp_buff,temp_buff);
    }
    if (count < imsi.numDigits) {
        /* one digit left over */
        byte = &msg[index];
        *byte = imsi.digits[count];
        memset(temp_buff,0,10);
        sprintf(temp_buff,"%d",imsi.digits[count]);
        strcat(disp_buff,temp_buff);
        index++;
    }

    *msg_len += index;
    
    DBG_TRACE("Encoded IMSI : %s; IEI length %d bytes",disp_buff,index);

    DBG_LEAVE();
    return;
}


/*
 * bssgp_ie_encode_llc_pdu
 * 
 *----------------------------------------------------------
 *  11.3.15 LLC-PDU
 *  This information element contains an LLC-PDU. The 
 *  element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet  3    LLC-PDU (first part)
 *  octet n     LLC-PDU (last part)
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_llc_pdu(UINT8 *llc_pdu, UINT16 pdu_len, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_llc_pdu",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];
    index++;

    *byte = (UINT8)BSSGP_IEI_LLC_PDU;

    bssgp_ie_encode_iei_length(pdu_len,&msg[index],&index);


    memcpy(&msg[index],llc_pdu,pdu_len);


    index += pdu_len;

    *len += index;

    DBG_TRACE("Encoded LLC PDU of size %d; IEI length %d",pdu_len,index);

    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_llc_frames_discarded
 *
 *----------------------------------------------------------
 *  11.3.16 LLC Frames Discarded
 *  This element describes the number of LLC frames that 
 *  have been discarded inside a BSS. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3     Number of frames discarded (in hex)
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_llc_frames_discarded(UINT8 num_frames,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_llc_frames_discarded",BSSGP_ENCODER_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_LLC_FRAMES_DISCARDED;
    index++;
    DBG_TRACE("Encoding llc frames discarded IE.");

    bssgp_ie_encode_iei_length(sizeof(UINT8),&msg[index],&index);

    msg[index++] = num_frames;

    DBG_TRACE("llc frames discarded IE length %d bytes",index);
    *len += index;

    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_location_area
 *
 *----------------------------------------------------------
 *  11.3.17 Location Area
 *  This element uniquely identifies one Location Area. 
 *  The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octets 3-7  Octets 3 to 7 contain the value part 
 *              (starting with octet 2) of the Location Area 
 *              Identification IE defined in GSM 04.08 [11], 
 *              not including GSM 04.08 IEI
 *  
 *  The coding of octet 2 is a binary number indicating the 
 *  Length of the remaining element.
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_location_area(T_CNI_RIL3_IE_LOCATION_AREA_ID location_area,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_location_area",BSSGP_ENCODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 lac, index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_LOCATION_AREA;
    index++;

    
    bssgp_ie_encode_iei_length((3+2+sizeof(UINT16)),&msg[index],&index);

    /*
     * From 04.08:
     * The Location Area Identification is a type 3 information element 
     * ith 6 octets length.
     *    8     7     6     5     4     3     2     1
     * +-----------------------------------------------*
     * *                       *                       *
     * *      MCC digit 2      *      MCC digit 1      * octet 2
     * +-----------------------+-----------------------*
     * *                       *                       *
     * *      MNC digit 3      *      MCC digit 3      * octet 3
     * +-----------------------+-----------------------*
     * *                       *                       *
     * *      MNC digit 2      *      MNC digit 1      * octet 4
     * +-----------------------------------------------*
     * *                                               *
     * *                      LAC                      * octet 5
     * +-----------------------------------------------*
     * *                                               *
     * *                  LAC (continued)              * octet 6
     * +-----------------------------------------------+
     * 
     */ 

    msg[index] = (location_area.mcc[1] << 4) | location_area.mcc[0] ;
    index++;
    msg[index] =  (location_area.mnc[2] << 4) |  location_area.mcc[2];
    index++;
    msg[index] = (location_area.mnc[1] << 4) | location_area.mnc[0];
    index++;

    DBG_TRACE("Location Area :- MCC : %d%d%d ; MNC : %d%d(%x) ; LAC : %d",
        location_area.mcc[0],location_area.mcc[1],location_area.mcc[2],
        location_area.mnc[0],location_area.mnc[1],location_area.mnc[2],
        location_area.lac);

    lac = htons(location_area.lac);

    memcpy(&msg[index],&lac,sizeof(UINT16));
    index += sizeof(UINT16);

    *len += index;
    DBG_TRACE("Location area IEI length %d bytes",index);
    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_lsa_identifier_list
 *
 *----------------------------------------------------------
 *  11.3.18 LSA Identifier List
 *  This information element uniquely identifies LSAs. The 
 *  element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3-?   Rest of element coded as in GSM 08.08, not 
 *              including GSM 08.08 IEI and GSM 08.08 length 
 *              indicator
 *  
 *----------------------------------------------------------
 * 
 */
void
bssgp_ie_encode_lsa_identifier_list(RIL3_IE_LSA_IDENTIFIER_LIST lsa_id_list,
    UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_lsa_identifier_list",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;
    INT16 num_lsa_ids;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_LSA_ID_LIST;
    index++;
    DBG_TRACE("Encoding LSA Identifier List IE\n");

    bssgp_ie_encode_iei_length(lsa_id_list.num_lsa_ids*3 + 1,&msg[index],&index);

    msg[index++] = (UINT8)(lsa_id_list.ep & 0x01);
    
    num_lsa_ids = lsa_id_list.num_lsa_ids;
    while (num_lsa_ids)
    {
        msg[index++] = (UINT8)(lsa_id_list.lsa_id[lsa_id_list.num_lsa_ids-num_lsa_ids] >> 16);
        msg[index++] = (UINT8)(lsa_id_list.lsa_id[lsa_id_list.num_lsa_ids-num_lsa_ids] >> 8);
        msg[index++] = (UINT8)(lsa_id_list.lsa_id[lsa_id_list.num_lsa_ids-num_lsa_ids]);
        num_lsa_ids--;
    }
    
    DBG_TRACE("LSA Identifier IE length %d bytes",index);
    *len += index;
    
    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_lsa_info
 * 
 *----------------------------------------------------------
 *  11.3.19 LSA Information
 *  This information element uniquely identifies LSAs, the 
 *  priority of each LSA and the access right outside these 
 *  LSAs. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3-?   Rest of element coded as in GSM 08.08, not 
 *              including GSM 08.08 IEI and GSM 08.08 
 *              length indicator
 *  
 *   
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_lsa_info(RIL3_IE_LSA_INFORMATION lsa_info,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_lsa_info",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;
    INT16 num_lsas;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_LSA_INFORMATION;
    index++;
    DBG_TRACE("Encoding LSA Information IE\n");

    bssgp_ie_encode_iei_length(lsa_info.num_lsas*4 + 1,&msg[index],&index);

    msg[index++] = (UINT8)(lsa_info.lsa_only & 0x01);
    
    num_lsas = lsa_info.num_lsas;
    while (num_lsas)
    {
        msg[index++] = (lsa_info.lsa_ids[lsa_info.num_lsas-num_lsas].act << 5)  |
                       (lsa_info.lsa_ids[lsa_info.num_lsas-num_lsas].pref << 4) |
                       (lsa_info.lsa_ids[lsa_info.num_lsas-num_lsas].priority);
        msg[index++] = (UINT8)(lsa_info.lsa_ids[lsa_info.num_lsas-num_lsas].lsa_id >> 16);
        msg[index++] = (UINT8)(lsa_info.lsa_ids[lsa_info.num_lsas-num_lsas].lsa_id >> 8);
        msg[index++] = (UINT8)(lsa_info.lsa_ids[lsa_info.num_lsas-num_lsas].lsa_id);
        num_lsas--;
    }
    
    DBG_TRACE("LSA Information IE length %d bytes",index);
    *len += index;
    
    DBG_LEAVE();
    return;
}




/*
 * bssgp_ie_encode_mobile_id
 *
 *----------------------------------------------------------
 *  11.3.20 Mobile Id
 *  The element coding is:
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3-n   Octets 3-n contain either the IMSI, IMEISV 
 *              or IMEI coded as  the value part (starting 
 *              with octet 3) of the Mobile Identity IE 
 *              defined in GSM 04.08, not including GSM 
 *              04.08 IEI and GSM 04.08 length indcator
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_mobile_id(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_mobile_id",BSSGP_ENCODER_LAYER);
    DBG_ENTER();
    /* FIXME */

    DBG_LEAVE();
    return;
}


/*
 * bssgp_ie_encode_ms_bucket_size
 *
 *----------------------------------------------------------
 *  11.3.21 MS Bucket Size 
 *  This information element indicates an MS's bucket size 
 *  (Bmax). The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3-4   Bmax
 *  
 *  The Bmax field is coded as in section GSM 08.18 BVC Bucket 
 *  Size/Bmax.
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_ms_bucket_size(UINT16 bucket_size,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_ms_bucket_size",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_MS_BUCKET_SIZE;
    index++;
    DBG_TRACE("Encoding MS Bucket size IE.");

    bssgp_ie_encode_iei_length(sizeof(UINT16),&msg[index],&index);

    msg[index++] = (UINT8)(bucket_size >> 8);
    msg[index++] = (UINT8)(bucket_size);

    DBG_TRACE("MS Bucket size IE length %d bytes",index);
    *len += index;
    
    DBG_LEAVE();
    return;
}


/*
 * bssgp_ie_encode_ms_ra_cap
 *
 *----------------------------------------------------------
 *  11.3.22 MS Radio Access Capability
 *  This information element contains the capabilities of 
 *  the ME. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3-14  Rest of element coded as the value part 
 *              defined in GSM 04.08, not including GSM 
 *              04.08 IEI and GSM 04.08 octet length 
 *              indicator.
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_ms_ra_cap(T_CNI_RIL3_IE_MS_RADIO_ACCESS_CAPABILITY ra_cap,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_ms_ra_cap",BSSGP_ENCODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16  index = 0,count;

    byte = &msg[index];
    index++;

    *byte = (UINT8)BSSGP_IEI_MS_RADIO_ACCESS_CAP;

    
    bssgp_ie_encode_iei_length(((ra_cap.number_of_ra_capabilities*6*sizeof(UINT8))+sizeof(UINT8)),&msg[index],&index);


    /*
     *  Although the spec 04.08 is pretty clear about the IE
     *  encoding, the encoding specified is quite messy &
     *  since we would be more or less dealing with ourselves,
     *  (read proprietary..snigger..) it is well worth it to
     *  rearrange the elements to pack it nicely... DSN.
     *  So packing would be as follows:-
     *
     * |---------------------------------------------------------------|
     * |        Number of Radio Access Capabilities                    | octet 1
     * |---------------------------------------------------------------|
     *
     * After this, depending on above value there would be 5 octets per
     * radio access capability packed as follows
     *
     * |---------------------------------------------------------------|
     * |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   | 
     * |---------------------------------------------------------------|
     * |        Access Tech Type       |   RF Power Capabilities       | octet n
     * |---------------------------------------------------------------|
     * | resvd |   A5 Algorithms supported from 7 to 1                 | octet n+1
     * |---------------------------------------------------------------| 
     * |     reserved          |   es  |   ps  | vgcs  | vbs   | res   | octet n+2
     * |---------------------------------------------------------------|
     * |  pres |    reserved   |  hscd multi-slot class                | octet n+3
     * |---------------------------------------------------------------|
     * |  pres |    gprs multi-slot class              | ExCap | pres  | octet n+4
     * |---------------------------------------------------------------|
     * |  switch measure switch value  |    switch measure value       | octet n+5
     * |---------------------------------------------------------------|
     *
     */

    byte = &msg[index];
    index++;
    *byte = ra_cap.number_of_ra_capabilities;

    DBG_TRACE("Encoding %d number of Radio Access Capabilities",ra_cap.number_of_ra_capabilities);

    for(count=0;count<ra_cap.number_of_ra_capabilities;count++) {
        byte = &msg[index];
        index++;
        *byte = ra_cap.ms_ra_capability[count].rf_power_capability;
        *byte |= (ra_cap.ms_ra_capability[count].access_technology_type << 4);
        DBG_TRACE("Access Technology Type : %d, RF Power Capability %d",
            ra_cap.ms_ra_capability[count].access_technology_type,
            ra_cap.ms_ra_capability[count].rf_power_capability);

        byte = &msg[index];
        index++;
        *byte = 0;
        if (ra_cap.ms_ra_capability[count].a5_1_algorithm) {
            *byte |= 0x01;
            DBG_TRACE("Supports A5.1 algorithm");
        }
        if (ra_cap.ms_ra_capability[count].a5_2_algorithm) {
            *byte |= 0x02;
            DBG_TRACE("Supports A5.2 algorithm");
        }
        if (ra_cap.ms_ra_capability[count].a5_3_algorithm) {
            *byte |= 0x04;
            DBG_TRACE("Supports A5.3 algorithm");
        }
        if (ra_cap.ms_ra_capability[count].a5_4_algorithm) {
            *byte |= 0x08;
            DBG_TRACE("Supports A5.4 algorithm");
        }
        if (ra_cap.ms_ra_capability[count].a5_5_algorithm) {
            *byte |= 0x10;
            DBG_TRACE("Supports A5.5 algorithm");
        }
        if (ra_cap.ms_ra_capability[count].a5_6_algorithm) {
            *byte |= 0x20;
            DBG_TRACE("Supports A5.6 algorithm");
        }
        if (ra_cap.ms_ra_capability[count].a5_7_algorithm) {
            *byte |= 0x40;
            DBG_TRACE("Supports A5.7 algorithm");
        }

        byte = &msg[index];
        index++;
        *byte = 0;
        if (ra_cap.ms_ra_capability[count].es_ind) {
            *byte |= 0x10;
            DBG_TRACE("ES Indicator");
        }
        if (ra_cap.ms_ra_capability[count].ps) {
            *byte |= 0x08;
            DBG_TRACE("PS");
        }
        if (ra_cap.ms_ra_capability[count].vgcs) {
            *byte |= 0x04;
            DBG_TRACE("VGCS");
        }
        if (ra_cap.ms_ra_capability[count].vbs) {
            *byte |= 0x02;
            DBG_TRACE("VBS");
        }

        byte = &msg[index];
        index++;
        *byte = ra_cap.ms_ra_capability[count].multislot_capability.hscsd_multi_slot_class;
        *byte |= (ra_cap.ms_ra_capability[count].multislot_capability.hscsd_param_present << 7);
        if (ra_cap.ms_ra_capability[count].multislot_capability.hscsd_param_present)
            DBG_TRACE("HSCSD Multi slot class %d",ra_cap.ms_ra_capability[count].multislot_capability.hscsd_multi_slot_class);
        else    
            DBG_TRACE("HSCSD Multi slot class not present");

        byte = &msg[index];
        index++;
        *byte = (ra_cap.ms_ra_capability[count].multislot_capability.gprs_multi_slot_class << 2);
        *byte |= (ra_cap.ms_ra_capability[count].multislot_capability.gprs_params_present << 7);
        *byte |= (ra_cap.ms_ra_capability[count].multislot_capability.gprs_dynamic_allocation_capability << 1);
        *byte |= (ra_cap.ms_ra_capability[count].multislot_capability.sms_params_present);
        if (ra_cap.ms_ra_capability[count].multislot_capability.gprs_params_present)
        {
            DBG_TRACE("GPRS Multi slot class %d",ra_cap.ms_ra_capability[count].multislot_capability.gprs_multi_slot_class);
            DBG_TRACE("GPRS Ext dynamic allocation cap %d",ra_cap.ms_ra_capability[count].multislot_capability.gprs_dynamic_allocation_capability);
        }
        else
            DBG_TRACE("GPRS Multi slot class not present");
        
        byte = &msg[index];
        index++;
        *byte = ra_cap.ms_ra_capability[count].multislot_capability.switch_measure_value;
        *byte |= (ra_cap.ms_ra_capability[count].multislot_capability.switch_measure_switch_value << 4);
        if (ra_cap.ms_ra_capability[count].multislot_capability.sms_params_present)
            DBG_TRACE("Switch measure value %d, Switch measure switch value %d",
                ra_cap.ms_ra_capability[count].multislot_capability.switch_measure_value,
                ra_cap.ms_ra_capability[count].multislot_capability.switch_measure_switch_value);
        else        
            DBG_TRACE("Switch measure not present");
    }

    *len += index;

    DBG_TRACE("MS-Radio-Access-Capability IEI length %d bytes",index);

    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_omc_id
 *
 *----------------------------------------------------------
 *  11.3.23 OMC Id
 *  The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3-22  For the OMC identity, see TS 12.20
 *   
 *----------------------------------------------------------
 * 
 */
void
bssgp_ie_encode_omc_id(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_omc_id",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    /* FIXME */
    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_pdu_in_error
 * 
 *----------------------------------------------------------
 *  11.3.24 PDU In Error
 *  The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3-?   Erroneous BSSGP PDU
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_pdu_in_error(BSSGP_PDU_TYPE pdu_type,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_pdu_in_error",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    /* FIXME : TODO */
    DBG_LEAVE();
    return;
}


/*
 * bssgp_ie_encode_pdu_lifetime
 *
 *----------------------------------------------------------
 *  11.3.25 PDU Lifetime
 *  This information element describes the PDU Lifetime 
 *  for a PDU inside the BSS. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3-4   Delay Value
 *  
 *  The Delay Value field is coded as in section GSM 08.18 
 *  BVC Measurement/Delay Value.
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_pdu_lifetime(RIL3_IE_PDU_LIFETIME pdu_lifetime, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_pdu_lifetime",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_PDU_LIFETIME;
    index++;
    DBG_TRACE("Encoding PDU Lifetime IE %#x\n", pdu_lifetime.pdu_lifetime);

    bssgp_ie_encode_iei_length(sizeof(UINT16),&msg[index],&index);

    msg[index++] = (UINT8)(pdu_lifetime.pdu_lifetime >> 8);
    msg[index++] = (UINT8)(pdu_lifetime.pdu_lifetime);

    DBG_TRACE("PDU Lifetime IEI length %d bytes",index);
    *len += index;
    
    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_priority
 *
 *----------------------------------------------------------
 *  11.3.27 Priority
 *  This element indicates the priority of a PDU. The 
 *  element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3     Rest of element coded as the value part of 
 *              the Priority IE defined in GSM 08.08, not 
 *              including GSM 08.08 IEI and GSM 08.08 
 *              length indicator
 *  
 *----------------------------------------------------------
 * 
 */
void
bssgp_ie_encode_priority(RIL3_IE_PRIORITY priority,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_priority",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_PRIORITY;
    index++;
    DBG_TRACE("Encoding Priority IE %#x\n", priority.priority);

    bssgp_ie_encode_iei_length(sizeof(UINT8),&msg[index],&index);

    msg[index++] = priority.priority;

    DBG_TRACE("Priority IEI length %d bytes",index);
    *len += index;
    
    DBG_LEAVE();
    return;
}


/*
 * bssgp_ie_encode_qos_params
 * 
 *----------------------------------------------------------
 *      QoS Params (Cisco-EWTBU proprietary ext)
 *  This information element describes the QoS Params 
 *  associated with a PDU. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3     Delay Class
 *  octet 4     Reliability Class
 *  octet 5     Precedence Class
 *  octet 6     Peak Throughput
 *  octet 7     Mean Throughput
 *  
 *----------------------------------------------------------
 * 
 */
void
bssgp_ie_encode_qos_params(T_CNI_RIL3_IE_QOS qos_params, UINT8 *msg, UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_qos_params",BSSGP_ENCODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16  index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_QOS_PARAMS;
    index++;

    
    bssgp_ie_encode_iei_length((sizeof(UINT8)*5),&msg[index],&index);
    /* Delay Class */
    byte    = &msg[index];
    *byte   = (UINT8)qos_params.delay_class;
    index++;
    /* Reliability class */
    byte    = &msg[index];
    *byte   = (UINT8)qos_params.reliability_class;
    index++;
    /* Precedence class */
    byte    = &msg[index];
    *byte   = (UINT8)qos_params.precedence_class;
    index++;
    /* Peak throughput */
    byte    = &msg[index];
    *byte   = (UINT8)qos_params.peak_throughput;
    index++;
    /* Mean throughput */
    byte    = &msg[index];
    *byte   = (UINT8)qos_params.mean_throughput;
    index++;

    DBG_TRACE("Encoded QoS params; Delay %d, Reliability %d, Precedence %d, Peak throughput %d & Mean throughput %d",
        qos_params.delay_class,qos_params.reliability_class,qos_params.precedence_class,
        qos_params.peak_throughput,qos_params.mean_throughput);
    *len += index;
    DBG_LEAVE();
    return;
}


/*
 * bssgp_ie_encode_qos_profile
 * 
 *----------------------------------------------------------
 *  11.3.28 QoS Profile
 *  This information element describes the QoS Profile 
 *  associated with a PDU. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3-4   Peak bit rate provided by the network, 
 *              coded as the value part in Bucket Leak 
 *              Rate/R IE/ GSM 08.18 a)
 *  octet 5 SPARE   C/R T   A   Precedence
 *  
 *----------------------------------------------------------
 * 
 */
void
bssgp_ie_encode_qos_profile(T_RIL3_IE_QOS_PROFILE_ID qos_profile,UINT8 *msg,UINT16 *len, BSSGP_IEI_FORMAT format)
{
    DBG_FUNC("bssgp_ie_encode_qos_profile",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    DBG_TRACE("Encoding QoS Profile");
    
    if (format == BSSGP_FORMAT_TLV)
    {
        *byte = (UINT8)BSSGP_IEI_QOS_PROFILE;
        index++;
    
        bssgp_ie_encode_iei_length((sizeof(UINT16)+sizeof(UINT8)),&msg[index],&index);
    }

    msg[index] = qos_profile.bucket_leak_rate[0];
    index++;
    msg[index] = qos_profile.bucket_leak_rate[1];
    index++;

    byte = &msg[index];
    *byte = qos_profile.precedence_class;
    if (qos_profile.a_bit) {
        *byte |= 0x08;
        DBG_TRACE("Radio interface uses RLC/MAC-UNITDATA functionality");
    }
    else {
        DBG_TRACE("Radio interface uses RLC/MAC ARQ functionality");
    }
    
    if (qos_profile.t_bit) {
        *byte |= 0x10;
        DBG_TRACE("SDU contains Data");
    }
    else {
        DBG_TRACE("SDU contains Signalling (e.g. related to GMM)");
    }

    if (qos_profile.cr_bit) {
        *byte |= 0x20;
        DBG_TRACE("SDU does NOT contain a LLC ACK or SACK command/response frame type");
    }
    else {
        DBG_TRACE("SDU contains LLC ACK or SACK command/response frame type");
    }
    index++;
    *len += index;
    DBG_TRACE("QoS Profile IEI length %d bytes",index);
    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_radio_cause
 * 
 *----------------------------------------------------------
 *  11.3.29 Radio Cause
 *  This information element indicates the reason for an 
 *  exception condition on the radio interface. The element 
 *  coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3     Radio Cause value
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_radio_cause(T_RIL3_IE_RADIO_CAUSE_ID radio_cause,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_radio_cause",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_RADIO_CAUSE;
    index++;
    DBG_TRACE("Encoding Radio cause : %s",
        bssgp_util_get_radio_cause_str(radio_cause));

    bssgp_ie_encode_iei_length(sizeof(UINT8),&msg[index],&index);

    byte = &msg[index];
    *byte = (UINT8)radio_cause;
    index++;

    DBG_TRACE("Radio Cause IEI length %d bytes",index);
    *len += index;

    DBG_LEAVE();
    return;
}


/*
 * bssgp_ie_encode_ra_cap_update_cause
 *
 *----------------------------------------------------------
 *  11.3.30 RA-Cap-UPD-Cause
 *  The RA-Cap-UPD-Cause indicates the success of the 
 *  RA-CAPABILITY-UPDATE procedure or the reason of the 
 *  failure. The element coding is:
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3     RA-Cap-UPD Cause value
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_ra_cap_update_cause(RA_CAP_UPD_CAUSE ra_cause,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_ra_cap_update_cause",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_RA_CAP_UPD_CAUSE;
    index++;
    DBG_TRACE("Encoding Radio-Access-Capability-Update-Cause : %s",
        bssgp_util_get_ra_cap_cause_str(ra_cause));

    bssgp_ie_encode_iei_length(sizeof(UINT8),&msg[index],&index);

    byte = &msg[index];
    *byte = (UINT8)ra_cause;
    index++;

    DBG_TRACE("Radio-Access-Capability-Update-Cause IEI length %d bytes",index);
    *len += index;

    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_reject_cause
 * 
 *----------------------------------------------------------
 *      Reject Cause (Cisco-EWTBU proprietary ext)
 *  This information element describes the reject cause
 *  for a Location Update Nack
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3     Reject cause
 *  
 *----------------------------------------------------------
 * 
 */
void
bssgp_ie_encode_reject_cause(T_CNI_RIL3_IE_REJECT_CAUSE reject_cause, UINT8 *msg, UINT16 *len)
{
    DBG_FUNC("",BSSGP_ENCODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];
    index++;

    *byte = (UINT8)BSSGP_IEI_LA_REJECT_CAUSE;
    
    bssgp_ie_encode_iei_length(sizeof(UINT8),&msg[index],&index);

    byte = &msg[index];
    index++;
    
    *byte = (UINT8) reject_cause.causeValue;
    DBG_TRACE("Encoded Location Area Reject cause %d; IEI length %d",reject_cause.causeValue,index);
    *len += index;

    DBG_LEAVE();
}



/*
 * bssgp_ie_encode_routing_area
 *
 *----------------------------------------------------------
 *  11.3.31 Routeing Area
 *  This element uniquely identifies one routeing area. The 
 *  element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octets 3-8  Octets 3 to 8 contain the value part 
 *              (starting with octet 2) of the Routing Area 
 *              Identification IE defined in GSM 04.08 [11], 
 *              not including GSM 04.08 IEI
 *  
 *  The coding of octet 2 is a binary number indicating the 
 *  Length of the remaining element.
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_routing_area(T_CNI_RIL3_IE_ROUTING_AREA_ID routing_area,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_routing_area",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 lac, index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_ROUTING_AREA;
    index++;
    
    bssgp_ie_encode_iei_length((3+2+sizeof(UINT16)),&msg[index],&index);

    /*
     * From 04.08:
     *
     * The routing area identification information element is 
     * coded as shown in figure 10.5.130/GSM 04.08 and 
     * table 10.5.148/GSM 04.08.
     *  8   7   6   5   4   3   2   1   
     *  ------------------------------
     *  * MCC digit 2 *  MCC digit 1 *  octet 2
     *  ------------------------------
     *  * MNC digit 3 *  MCC digit 3 *  octet 3
     *  ------------------------------
     *  * MNC digit 2 *  MNC digit 1 *  octet 4
     *  ------------------------------
     *  *           LAC              *  octet 5
     *  ------------------------------
     *  *           LAC cont'd       *  octet 6
     *  ------------------------------
     *  *           RAC              *  octet 7
     *  ------------------------------
     * 
     * 
     */ 

    msg[index] = (routing_area.mcc[1] << 4) | routing_area.mcc[0] ;
    index++;
    msg[index] = (routing_area.mnc[2] << 4) | routing_area.mcc[2];
    index++;
    msg[index] = (routing_area.mnc[1] << 4) | routing_area.mnc[0];
    index++;

    DBG_TRACE("Routing Area :- MCC : %d%d%d ; MNC : %d%d(%x) ; LAC : %d ; RAC : %d",
        routing_area.mcc[0],routing_area.mcc[1],routing_area.mcc[2],
        routing_area.mnc[0],routing_area.mnc[1],routing_area.mnc[2],
        routing_area.lac,routing_area.rac);

    lac = htons(routing_area.lac);

    memcpy(&msg[index],&lac,sizeof(UINT16));
    index += sizeof(UINT16);

    msg[index] = routing_area.rac;
    index++;

    *len += index;
    DBG_TRACE("Routing Area IEI length %d bytes",index);

    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_r_def_ms
 *
 *----------------------------------------------------------
 *  11.3.32 R_default_MS
 *  This information element indicates the default bucket 
 *  leak rate (R) to be applied to a flow control bucket 
 *  for an MS. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3-4   R_default_MS value
 *  
 *  The R_default_MS field is coded as in section GSM 08.18 
 *  Bucket Leak Rate /R Value.
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_r_def_ms(UINT16 r_default,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_r_def_ms",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_R_DEFAULT_MS;
    index++;
    DBG_TRACE("Encoding R_default_MS IE.");

    bssgp_ie_encode_iei_length(sizeof(UINT16),&msg[index],&index);

    msg[index++] = (UINT8)(r_default >> 8);
    msg[index++] = (UINT8)(r_default);

    DBG_TRACE("R_default_MS IE length %d bytes",index);
    *len += index;
    
    DBG_LEAVE();
    return;
}

/*
 * bssgp_ie_encode_suspend_ref_num
 *
 *----------------------------------------------------------
 *  11.3.33 Suspend Reference Number
 *  The Suspend Reference Number  information element 
 *  contains an un-formatted reference number for each 
 *  suspend/resume transaction. The element coding is:
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3     Suspend Reference Number
 *  
 *  The Suspend Reference Number is an un-formatted 8 bit 
 *  field.
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_suspend_ref_num(UINT8 suspend_ref_num,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_suspend_ref_num",BSSGP_ENCODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_SUSPEND_REF_NUM;
    index++;
    
    bssgp_ie_encode_iei_length(sizeof(UINT8),&msg[index],&index);
    byte = &msg[index];

    *byte  = suspend_ref_num;
    index++;

    DBG_TRACE("Encoded Suspend Reference Number as %d; IEI length %d bytes",suspend_ref_num,index);
    
    *len += index;

    DBG_LEAVE();
    return;
}



/*
 * bssgp_ie_encode_tag
 *
 *----------------------------------------------------------
 *  11.3.34 Tag
 *  This information element is used to correlate request 
 *  and response PDUs. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3     Unstructured value
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_tag(UINT8 tag,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_tag",BSSGP_ENCODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_TAG;
    index++;
    
    bssgp_ie_encode_iei_length(sizeof(UINT8),&msg[index],&index);
    byte = &msg[index];

    *byte  = tag;
    index++;

    DBG_TRACE("Encoded tag as %d; IEI length %d bytes",tag,index);
    
    *len += index;
    DBG_LEAVE();
    return;
}

/*
 * bssgp_ie_encode_tlli
 *
 *----------------------------------------------------------
 *  11.3.35 Temporary logical link Identity (TLLI)
 *  The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3-6   Rest of element coded as the value part of 
 *              the TLLI information element in GSM 04.08, 
 *              not including GSM 04.08 IEI.
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_tlli(RIL3_TLLI tlli,UINT8 *msg,UINT16 *len, BSSGP_IEI_FORMAT format)
{
    DBG_FUNC("bssgp_ie_encode_tlli",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;UINT32 *shorty;

    byte = &msg[index];

    if (format == BSSGP_FORMAT_TLV)
    {
        *byte = (UINT8)BSSGP_IEI_TLLI;
        index++;

    
        bssgp_ie_encode_iei_length(sizeof(TLLI),&msg[index],&index);
    }

    shorty = (UINT32*)&msg[index];
    /* now we get shorty ;-) */
    *shorty = htonl(tlli.tlli);
    index += sizeof(UINT32);

    *len += index;
    tlli.ie_present = TRUE;
    
    DBG_TRACE("Encoded TLLI %#x IEI length %d bytes",tlli.tlli,index);
    DBG_LEAVE();
    return;
}


/*
 * bssgp_ie_encode_tmsi
 *
 *----------------------------------------------------------
 *  11.3.36 Temporary Mobile Subscriber Identity (TMSI)
 *  The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3-6   Rest of element coded as the value part of 
 *              the TMSI/P-TMSI information element in 
 *              GSM 04.08, not including GSM 04.08 IEI.
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_tmsi(T_CNI_RIL3_IE_MOBILE_ID tmsi,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_tmsi",BSSGP_ENCODER_LAYER);
    DBG_ENTER();


    UINT8 *byte;
    unsigned long tmsi_val;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_TMSI;
    index++;

    
    bssgp_ie_encode_iei_length(sizeof(unsigned long),&msg[index],&index);

    /*
     * Refer to GSM 04.08; the IEI has the following 
     * structure
     *   8     7     6     5     4     3     2     1
     * +-----------------------------------------------+
     * *                 TMSI/P-TMSI value             * octet 2
     * +-----------------------------------------------*
     * *                                               * octet 3
     * *                 TMSI/P-TMSI value (contd)     *
     * +-----------------------------------------------*
     * *                                               * octet 4
     * *                 TMSI/P-TMSI value (contd      *
     *+-----------------------------------------------*
     * *                                               * octet 5
     * *                 TMSI/P-TMSI value (contd)     *
     * +-----------------------------------------------+
     * 
     */

    byte = &msg[index];

    tmsi_val = htonl(tmsi.tmsi);


    memcpy(byte,&tmsi_val,sizeof(unsigned long));
    index += sizeof(unsigned long);

    *len += index;
    DBG_TRACE("Encoded TMSI : %ld; IEI length %d bytes",tmsi.tmsi,index);

    DBG_LEAVE();
    return;
}

/*
 * bssgp_ie_encode_trace_ref
 *
 *----------------------------------------------------------
 *  11.3.37 Trace Reference
 *  This element provides a trace reference number allocated 
 *  by the triggering entity. The element coding is:
 *
 *  octet 1 IEI
 *  octet 2,2a  Length Indicator
 *  octet 3-4   Trace Reference
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_trace_ref(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_trace_ref",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    DBG_LEAVE();
    return;
}


/*
 * bssgp_ie_encode_trace_type
 *
 *----------------------------------------------------------
 *  11.3.38 Trace Type
 *  This element provides the type of trace information to 
 *  be recorded. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3     This is coded as specified in Technical 
 *              Specification GSM 12.08.
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_trace_type(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_trace_type",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    DBG_LEAVE();
    return;
}

/*
 * bssgp_ie_encode_txn_id
 *
 *----------------------------------------------------------
 *  11.3.39 TransactionId
 *  This element indicates a particular transaction within 
 *  a trace. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3-4   Transaction Id
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_txn_id(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    DBG_LEAVE();
    return;
}


/*
 * bssgp_ie_encode_trigger_id
 *
 *----------------------------------------------------------
 *  11.3.40 Trigger Id
 *  This element provides the identity of the entity which 
 *  initiated the trace. The element coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator
 *  octet 3-22  Entity Identity ( typically an OMC identity)
 *  
 *----------------------------------------------------------
 *
 */
void
bssgp_ie_encode_trigger_id(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_trigger_id",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    DBG_LEAVE();
    return;
}

/*
 * bssgp_ie_encode_num_octets_affected
 *
 *----------------------------------------------------------
 *  11.3.41 Number of octets affected
 *  This information element indicates, for an MS, the num 
 *  of octets transferred or deleted by BSS. The element 
 *  coding is:
 *
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3-5   number of octets transferred or deleted 
 *  
 *  The number of octets transferred or deleted by the BSS 
 *  may be higher than the maximum Bmax value (6553500). 
 *  SGSN shall handle any value higher than 6553500 as the 
 *  value 6553500.
 *  
 *----------------------------------------------------------
 * */
void
bssgp_ie_encode_num_octets_affected(UINT32 num_octets,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_encode_num_octets_affected",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    byte = &msg[index];

    *byte = (UINT8)BSSGP_IEI_NUM_OCTETS_AFFECTED;
    index++;
    DBG_TRACE("Encoding Number of octets affected IE.");

    bssgp_ie_encode_iei_length(3,&msg[index],&index);

    msg[index++] = (UINT8)(num_octets >> 16);
    msg[index++] = (UINT8)(num_octets >> 8);
    msg[index++] = (UINT8)(num_octets);

    DBG_TRACE("Number of octets affected IE length %d bytes",index);
    *len += index;

    DBG_LEAVE();
    return;
}






/*
 * bssgp_ie_encode_iei_length
 *
 *----------------------------------------------------------
 *  The length indicator shall be included in all 
 *  information elements having the TLV format.
 *  Information elements may be variable in length. 
 *  The length indicator is one or two octet long, the 
 *  second octet may be absent. This field consists of the 
 *  field extension bit, 0/1 ext, and the length of the value 
 *  field which follows, expressed in octets. The field 
 *  extension bit enables extension of the length indicator 
 *  to two octets.
 *  Bit 8 of the first octet is reserved for the field 
 *  extension bit. If the field extension bit is set to 0 
 *  (zero), then the second octet of the length indicator is 
 *  present. If the field extension bit is set to 1 (one), 
 *  then the first octet is the final octet of the length 
 *  indicator.
 *  The length of the value field of the IE occupies the rest 
 *  of the bits in the length indicator.
 *             
 *              8   7   6   5   4   3   2   1
 *  octet 2     0/1 ext length
 *  octet 2a        length 
 *  
 *  Figure 10/GSM 08.16: Length indicator structure
 *  The BSS or SGSN shall not consider the presence of octet 
 *  2a in a received IE as an error when the IE is short 
 *  enough for the length to be coded in octet 2 only.
 *
 *----------------------------------------------------------
 *  
 */
void
bssgp_ie_encode_iei_length(int iei_len,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_ie_encode_iei_length",BSSGP_ENCODER_LAYER);
    DBG_ENTER();

    UINT8 *byte = msg, byte_len=0;

    if (iei_len < 128) {
        byte_len    = iei_len;
        *byte = (0x80 | byte_len);
        *msg_len += 1;
    }
    else {
        div_t val;
        val         = div(iei_len,128);
        byte_len    = val.rem;
        *byte       = (0x7f & byte_len);
        *msg_len += 1;
        byte        = &msg[1];
        *byte       = val.quot;
        *msg_len += 1;
    }

    DBG_LEAVE();
    return;
}



