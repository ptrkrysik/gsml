/*
 ****************************************************************************************
 *                                                                                      *
 *  Copyright Cisco Systems, Inc 2000 All rights reserved                               *
 *                                                                                      *
 *  File                : bssgp_ie_decoder.cpp                                          *
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
#include <ctype.h>
#include "bssgp/bssgp_prot.h"
#include "bssgp/bssgp_util.h"
#include "bssgp/bssgp_ie_decoder.h"

void HexDumper(UINT8 *buffer, int length );

/*
 * bssgp_ie_decode_alignment_octets
 *
 *----------------------------------------------------------
 *  11.3.1  Alignment octets
 *  
 *  octet 1     IEI
 *  octet 2,2a  Length Indicator 
 *  octet 3-4   spare octet
 *  
 *  The Alignment octets are used to align a subsequent IEI
 *  onto a 32 bit boundary.
 *  
 *----------------------------------------------------------
 *
 */
BOOL
bssgp_ie_decode_alignment_octets(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_alignment_octets",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_ALIGNMENT_OCTETS) {
        DBG_ERROR("Alignment octets IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len > 3) {
        DBG_ERROR("Alignment octets IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    /* Just skip by length number of bytes */
    index += iei_len;

    DBG_TRACE("Alignment octets decoded; IEI length %d bytes",index);

    *len += index;

    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_ie_decode_bmax_def_ms
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
BOOL
bssgp_ie_decode_bmax_def_ms(UINT16 *bmax, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_bmax_def_ms",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_BMAX_DEFAULT_MS) {
        DBG_ERROR("BMAX DEFAULT MS IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT16)) {
        DBG_ERROR("BMAX DEFAULT MS IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    *bmax = (msg[index++] << 8) | msg[index++];

    DBG_TRACE("BMAX DEFAULT MS %d decoded; IEI length %d bytes",*bmax,index);

    *len += index;

    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_bss_area_ind
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
BOOL
bssgp_ie_decode_bss_area_ind(RIL3_BSS_AREA_INDICATION *bss_area, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_bss_area_ind",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_BSS_AREA_IND) {
        DBG_ERROR("BSS Area Indication IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT8)) {
        DBG_ERROR("BSS Area Indication IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    // Just skip the bss area ind value since it is ignored.
    index++;

    DBG_TRACE("BSS Area Indication decoded; IEI length %d bytes",index);

    *len += index;
    bss_area->ie_present = TRUE;
    
    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_bucket_leak_rate
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
BOOL
bssgp_ie_decode_bucket_leak_rate(UINT16 *r_value, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_bucket_leak_rate",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_BUCKET_LEAK_RATE) {
        DBG_ERROR("Bucket Leak Rate IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT16)) {
        DBG_ERROR("Bucket Leak Rate IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    *r_value = (msg[index++] << 8) | msg[index++];

    DBG_TRACE("Bucket Leak Rate %d decoded; IEI length %d bytes",*r_value,index);

    *len += index;
    
    DBG_LEAVE();
    return (TRUE);
}




/*
 * bssgp_ie_decode_bvc_bucket_size
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
BOOL
bssgp_ie_decode_bvc_bucket_size(UINT16 *bmax, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_bvc_bucket_size",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_BVC_BUCKET_SIZE) {
        DBG_ERROR("BVC Bucket Size IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT16)) {
        DBG_ERROR("BVC Bucket Size IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    *bmax = (msg[index++] << 8) | msg[index++];

    DBG_TRACE("BVC Bucket Size %d decoded; IEI length %d bytes",*bmax,index);

    *len += index;

    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_bvci
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
BOOL
bssgp_ie_decode_bvci(RIL3_BVCI *bvci,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_bvci",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 *shorty;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_BVCI) {
        DBG_ERROR("BVCI IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT16)) {
        DBG_ERROR("BVCI IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    shorty = (UINT16*)&msg[index];
    /* now we get bvci */
    bvci->bvci = ntohs(*shorty);

    index += sizeof(UINT16);

    DBG_TRACE("BVCI %d decoded; IEI length %d bytes",bvci->bvci,index);

    *len += index;
    
    bvci->ie_present = TRUE;

    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_ie_decode_bvc_measurement
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
BOOL
bssgp_ie_decode_bvc_measurement(RIL3_BVC_MEASUREMENT *delay_val, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_bvc_measurement",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_BVC_MEASUREMENT) {
        DBG_ERROR("BVC Measurement IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT16)) {
        DBG_ERROR("BVC Measurement IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    delay_val->bvc_meas = (msg[index++] << 8) | msg[index++];

    DBG_TRACE("BVC Measurement %d decoded; IEI length %d bytes",delay_val->bvc_meas,index);

    *len += index;
    delay_val->ie_present = TRUE;

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_ie_decode_exception_cause
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
BOOL
bssgp_ie_decode_exception_cause(RIL3_BSSGP_CAUSE *cause,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_exception_cause",BSSGP_DECODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_CAUSE) {
        DBG_ERROR("Exception cause IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT8)) {
        DBG_ERROR("Exception cause IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    byte = &msg[index];
    cause->cause = (BSSGP_EXCEPTION_CAUSE)*byte;
    index++;

    DBG_TRACE("Cause value %s decoded; IEI length %d bytes",
        bssgp_util_get_exception_cause_str((BSSGP_EXCEPTION_CAUSE)cause->cause),
        index);

    *len += index;
    cause->ie_present = TRUE;

    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_ie_decode_cell_id
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
BOOL
bssgp_ie_decode_cell_id(RIL3_IE_CELL_IDENTIFIER *cell_id,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_cell_id",BSSGP_DECODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0,iei_len, *shorty, lac;

    byte = &msg[index];
    index++;

    if (*byte != BSSGP_IEI_CELL_ID) {
        DBG_ERROR("Invalid Cell Identifier IEI, %d value",*byte);
        DBG_LEAVE();
        return (FALSE);
    }

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != 8) {
        DBG_ERROR("Cell Identifier IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

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
    cell_id->routing_area.mcc[0] = msg[index] & 0x0f;
    cell_id->routing_area.mcc[1] = (msg[index] & 0xf0) >> 4;
    index++;

    cell_id->routing_area.mcc[2] = msg[index] & 0x0f;
    cell_id->routing_area.mnc[2] = (msg[index] & 0xf0) >> 4;
    index++;

    cell_id->routing_area.mnc[0] = msg[index] & 0x0f;
    cell_id->routing_area.mnc[1] = (msg[index] & 0xf0) >> 4;
    index++;


    memcpy(&lac,&msg[index],sizeof(UINT16));
    cell_id->routing_area.lac = ntohs(lac);
    index += sizeof(UINT16);

    cell_id->routing_area.rac  = msg[index];
    index++;

    shorty = (UINT16*)&msg[index];
    cell_id->cell_id.value = ntohs(*shorty);
    index += sizeof(UINT16);

    DBG_TRACE("Cell Identifier IE length %d bytes",index);
    
    DBG_TRACE("Cell Identifier :- MCC : %d%d%d ; MNC : %d%d(%x) ; LAC : %d ; RAC : %d\n"
              "\tCell Identity : %d",
        cell_id->routing_area.mcc[0], cell_id->routing_area.mcc[1], cell_id->routing_area.mcc[2],
		cell_id->routing_area.mnc[0], cell_id->routing_area.mnc[1], cell_id->routing_area.mnc[2],
		cell_id->routing_area.lac,
        cell_id->routing_area.rac, 
		cell_id->cell_id.value);

    cell_id->ie_present = TRUE;

    *len += index;

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_ie_decode_channel_needed
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
BOOL
bssgp_ie_decode_channel_needed(UINT8 *val, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_channel_needed",BSSGP_DECODER_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_CHAN_NEEDED) {
        DBG_ERROR("Channel Needed IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT8)) {
        DBG_ERROR("Channel Needed IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    *val = msg[index++];

    DBG_TRACE("Channel Needed %d decoded; IEI length %d bytes",*val,index);

    *len += index;
    
    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_ie_decode_drx_params
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
BOOL
bssgp_ie_decode_drx_params(T_CNI_RIL3_IE_DRX_PARAMETER *drx_params,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_drx_params",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_DRX_PARAMS) {
        DBG_ERROR("DRX Parameters IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != (sizeof(UINT8)*2)) {
        DBG_ERROR("DRX Parameters IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

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
    drx_params->split_pg_cycle_code = *byte;
    index++;

    byte    = &msg[index];
    if (*byte & 0x80) {
        drx_params->split_on_CCCH = TRUE;
        drx_params->non_drx_timer = (T_CNI_RIL3_IE_NON_DRX_TIMER_VALUE)(*byte & 0x7f);
        DBG_TRACE("Decoded DRX params :- Split PG cycle code %d ; Split on CCCH ; Non-DRX timer %d",
            drx_params->split_pg_cycle_code,drx_params->non_drx_timer);
    }
    else {
        drx_params->non_drx_timer = (T_CNI_RIL3_IE_NON_DRX_TIMER_VALUE)*byte;
        DBG_TRACE("Decoded DRX params :- Split PG cycle code %d ; Non-DRX timer %d",
            drx_params->split_pg_cycle_code,drx_params->non_drx_timer);
    }

    index++;

    *len += index;
    drx_params->ie_present = TRUE;

    DBG_TRACE("DRX Params IEI length %d bytes",index);  
    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_ie_decode_emlpp_priority
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
BOOL
bssgp_ie_decode_emlpp_priority(UINT8 *val, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_emlpp_priority",BSSGP_DECODER_LAYER);
    DBG_ENTER();
    
    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_EMLPP_PRIORITY) {
        DBG_ERROR("eMLPP-Priority IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT8)) {
        DBG_ERROR("eMLPP-Priority IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    *val = msg[index++];

    DBG_TRACE("eMLPP-Priority %d decoded; IEI length %d bytes",*val,index);

    *len += index;
    
    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_ie_decode_flush_action
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
BOOL
bssgp_ie_decode_flush_action(BSSGP_FLUSH_ACTION *action, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_flush_action",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_FLUSH_ACTION) {
        DBG_ERROR("Flush Action IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT8)) {
        DBG_ERROR("Flush Action IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    *action = (BSSGP_FLUSH_ACTION)msg[index++];

    DBG_TRACE("Flush Action %d decoded; IEI length %d bytes",*action,index);

    *len += index;

    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_ie_decode_imsi
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
BOOL
bssgp_ie_decode_imsi(T_CNI_RIL3_IE_MOBILE_ID *imsi,UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_ie_decode_imsi",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    int count;
    UINT16 index = 0,iei_len;
    char disp_buff[128], temp_buff[10];
    bool oddDigits;

    byte = &msg[index];

    if (*byte  != BSSGP_IEI_IMSI) {
        DBG_ERROR("Invalid IMSI IEI");
        DBG_LEAVE();
        return (FALSE);
    } 
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);

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

    if ((*byte & 0x07) != CNI_RIL3_IMSI) {
        DBG_ERROR("Invalid IMSI IEI; mobile ID type %d",(*byte & 0x07));
        DBG_LEAVE();
        return (FALSE);
    }
    oddDigits = ((*byte & 0x08) >> 3);
    imsi->ie_present   = 1;
    imsi->mobileIdType = CNI_RIL3_IMSI;

    /* Now decode the first digit */
    imsi->digits[0] = (*byte & 0xf0)>> 4;
    imsi->numDigits = 1;

    index++;
    memset(disp_buff,0,128);
    memset(temp_buff,0,10);
    sprintf(disp_buff,"%d",imsi->digits[0]);

    for(count=1;count<=(iei_len*2)-(2+oddDigits);count+=2) {
        byte = &msg[index];
        imsi->digits[count] = *byte & 0x0f;
        imsi->digits[count+1] = ((*byte & 0xf0) >> 4);
        index++;
        imsi->numDigits += 2;
        memset(temp_buff,0,10);
        sprintf(temp_buff,"%d%d",imsi->digits[count],imsi->digits[count+1]);
        strcat(disp_buff,temp_buff);
    }
    if (oddDigits == 0) {
        /* one digit left over */
        byte = &msg[index];
        imsi->digits[count] = *byte & 0x0f;
        imsi->numDigits++;
        memset(temp_buff,0,10);
        sprintf(temp_buff,"%d",imsi->digits[count]);
        strcat(disp_buff,temp_buff);
        index++;
    }

    *msg_len += index;
    imsi->ie_present = TRUE;
    
    DBG_TRACE("Decoded IMSI : %s; IEI length %d bytes",disp_buff,index);
    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_ie_decode_llc_pdu
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
BOOL
bssgp_ie_decode_llc_pdu(BSSGP_DL_UNITDATA_MSG *dl_unitdata,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_llc_pdu",BSSGP_DECODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];
    index++;

    if (*byte != BSSGP_IEI_LLC_PDU) {
        DBG_ERROR("IEI not LLC PDU, %d type !!",*byte);
        DBG_LEAVE();
        return (FALSE);
    }


    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);

    dl_unitdata->pdu.data_len = iei_len;

    DBG_TRACE("Decoding LLC PDU of size %d",iei_len);

    byte = &msg[index];
    
    dl_unitdata->pdu.data = (UINT8*)bssgp_util_get_rx_buff(iei_len);

    memcpy(dl_unitdata->pdu.data,byte,iei_len);

    index += iei_len;

    *len += index;

    DBG_TRACE("Decoded LLC PDU - IEI length %d bytes",iei_len);
    HexDumper(dl_unitdata->pdu.data, iei_len);

    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_llc_pdu_ext
 */
UINT8 *
bssgp_ie_decode_llc_pdu_ext(UINT16 *pdu_len,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_llc_pdu_ext",BSSGP_DECODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];
    index++;

    if (*byte != BSSGP_IEI_LLC_PDU) {
        DBG_ERROR("IEI not LLC PDU, %d type !!",*byte);
        DBG_LEAVE();
        return (NULL);
    }


    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);

    *pdu_len = iei_len;

    DBG_TRACE("Decoding LLC PDU of size %d",iei_len);

    byte = &msg[index];

    index += *pdu_len;

    *len += index;

    DBG_LEAVE();
    return (byte);
}





/*
 * bssgp_ie_decode_llc_frames_discarded
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
BOOL
bssgp_ie_decode_llc_frames_discarded(UINT8 *frames, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_llc_frames_discarded",BSSGP_DECODER_LAYER);
    DBG_ENTER();  
    
    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_BSS_AREA_IND) {
        DBG_ERROR("LLC Frames Discarded IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT8)) {
        DBG_ERROR("LLC Frames Discarded IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    *frames = msg[index++];

    DBG_TRACE("LLC Frames Discarded %d decoded; IEI length %d bytes",*frames,index);

    *len += index;
    
    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_location_area
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
BOOL
bssgp_ie_decode_location_area(T_CNI_RIL3_IE_LOCATION_AREA_ID *location_area,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_location_area",BSSGP_DECODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 lac, index = 0, iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_LOCATION_AREA) {
        DBG_ERROR("Invalid location area IEI");
        DBG_LEAVE();
        return (FALSE);
    }

    index++;

    
    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);

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
    location_area->mcc[0] = msg[index] & 0x0f;
    location_area->mcc[1] = (msg[index] & 0xf0) >> 4;
    index++;

    location_area->mcc[2] = msg[index] & 0x0f;
    location_area->mnc[2] = (msg[index] & 0xf0) >> 4;
    index++;

    location_area->mnc[0] = msg[index] & 0x0f;
    location_area->mnc[1] = (msg[index] & 0xf0) >> 4;
    index++;

    memcpy(&lac,&msg[index],sizeof(UINT16));
    index += sizeof(UINT16);
    location_area->lac = htons(lac);
    location_area->ie_present = TRUE;

    DBG_TRACE("Location Area :- MCC : %d%d%d ; MNC : %d%d(%x) ; LAC : %d",
        location_area->mcc[0],location_area->mcc[1],location_area->mcc[2],
        location_area->mnc[0],location_area->mnc[1],location_area->mnc[2],
        location_area->lac);

    *len += index;
    DBG_TRACE("Location area IEI length %d bytes",index);
    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_lsa_identifier_list
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
BOOL
bssgp_ie_decode_lsa_identifier_list(RIL3_IE_LSA_IDENTIFIER_LIST *lsa_id_list,
    UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_lsa_identifier_list",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;
    INT16 num_lsa_ids;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_LSA_ID_LIST) {
        DBG_ERROR("LSA Identifier List IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);

    num_lsa_ids = (iei_len - 1) / 3;
    
    lsa_id_list->num_lsa_ids = num_lsa_ids;
    
    lsa_id_list->ep = msg[index++] & 0x1;
    
    while (num_lsa_ids)
    {
        lsa_id_list->lsa_id[lsa_id_list->num_lsa_ids - num_lsa_ids] = 
            (msg[index++] << 16) | (msg[index++] << 8) | msg[index++];
        num_lsa_ids--;
    }

    DBG_TRACE("LSA Identifier List; IEI length %d bytes",index);

    *len += index;
    lsa_id_list->ie_present = TRUE;
    
    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_lsa_info
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
BOOL
bssgp_ie_decode_lsa_info(RIL3_IE_LSA_INFORMATION *lsa_info, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_lsa_info",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;
    INT16 num_lsas;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_ALIGNMENT_OCTETS) {
        DBG_ERROR("LSA Information IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);

    num_lsas = (iei_len - 1) / 4;
    
    lsa_info->num_lsas = num_lsas;
    
    lsa_info->lsa_only = msg[index++] & 0x1;
    
    while (num_lsas)
    {
        lsa_info->lsa_ids[lsa_info->num_lsas - num_lsas].act = msg[index] & 0x20;
        lsa_info->lsa_ids[lsa_info->num_lsas - num_lsas].pref = msg[index] & 0x10;
        lsa_info->lsa_ids[lsa_info->num_lsas - num_lsas].priority = msg[index++] & 0x0f;
        lsa_info->lsa_ids[lsa_info->num_lsas - num_lsas].lsa_id = (msg[index++] << 16) | (msg[index++] << 8) | msg[index++];
        lsa_info->lsa_ids[lsa_info->num_lsas - num_lsas].present = TRUE;
        num_lsas--;
    }

    DBG_TRACE("LSA Information; IEI length %d bytes",index);

    *len += index;
    lsa_info->ie_present = TRUE;
    
    DBG_LEAVE();
    return (TRUE);
}




/*
 * bssgp_ie_decode_mobile_id
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
BOOL
bssgp_ie_decode_mobile_id(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_mobile_id",BSSGP_DECODER_LAYER);
    DBG_ENTER();
    /* FIXME */

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_ie_decode_ms_bucket_size
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
BOOL
bssgp_ie_decode_ms_bucket_size(UINT16 *bmax, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_ms_bucket_size",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_MS_BUCKET_SIZE) {
        DBG_ERROR("MS Bucket Size IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT16)) {
        DBG_ERROR("MS Bucket Size IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    *bmax = (msg[index++] << 8) | msg[index++];

    DBG_TRACE("MS Bucket Size %d decoded; IEI length %d bytes",*bmax,index);

    *len += index;
    
    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_ie_decode_ms_ra_cap
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
BOOL
bssgp_ie_decode_ms_ra_cap(T_CNI_RIL3_IE_MS_RADIO_ACCESS_CAPABILITY *ra_cap,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_ms_ra_cap",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16  index = 0,count, iei_len;

    byte = &msg[index];
    index++;

    if (*byte != BSSGP_IEI_MS_RADIO_ACCESS_CAP) {
        DBG_ERROR("Invalid MS-Radio-Access-Capabilities IEI");
        DBG_LEAVE();
        return (FALSE);
    }

    
    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);



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
    ra_cap->number_of_ra_capabilities  = *byte;

    if (iei_len != ((ra_cap->number_of_ra_capabilities*6*sizeof(UINT8))+sizeof(UINT8))) {
        DBG_ERROR("MS-Radio-Access-Capability IEI length mismatch with number of capabilities, len %d, num %d",
            iei_len,ra_cap->number_of_ra_capabilities);
        DBG_LEAVE();
        return (FALSE);
    }


    DBG_TRACE("Decoding %d number of Radio Access Capabilities",ra_cap->number_of_ra_capabilities);

    for(count=0;count<ra_cap->number_of_ra_capabilities;count++) {
        byte = &msg[index];
        index++;
        ra_cap->ms_ra_capability[count].rf_power_capability = (*byte & 0x07);
        ra_cap->ms_ra_capability[count].access_technology_type = 
            (T_CNI_RIL3_ACCESS_TECHNOLOGY_TYPE)((*byte &0xf0) >> 4);

        DBG_TRACE("Access Technology Type : %d, RF Power Capability %d",
            ra_cap->ms_ra_capability[count].access_technology_type,
            ra_cap->ms_ra_capability[count].rf_power_capability);

        byte = &msg[index];
        index++;
        *byte = 0;
        if (*byte & 0x01) {
            ra_cap->ms_ra_capability[count].a5_1_algorithm = TRUE;
            DBG_TRACE("Supports A5.1 algorithm");
        }
        if (*byte & 0x02) {
            ra_cap->ms_ra_capability[count].a5_2_algorithm = TRUE;
            DBG_TRACE("Supports A5.2 algorithm");
        }
        if (*byte & 0x04) {
            ra_cap->ms_ra_capability[count].a5_3_algorithm = TRUE;
            DBG_TRACE("Supports A5.3 algorithm");
        }
        if (*byte & 0x08) {
            ra_cap->ms_ra_capability[count].a5_4_algorithm = TRUE;
            DBG_TRACE("Supports A5.4 algorithm");
        }
        if (*byte & 0x10) {
            ra_cap->ms_ra_capability[count].a5_5_algorithm = TRUE;
            DBG_TRACE("Supports A5.5 algorithm");
        }
        if (*byte & 0x20) {
            ra_cap->ms_ra_capability[count].a5_6_algorithm = TRUE;            
            DBG_TRACE("Supports A5.6 algorithm");
        }
        if (*byte & 0x40) {
            ra_cap->ms_ra_capability[count].a5_7_algorithm = TRUE;
            DBG_TRACE("Supports A5.7 algorithm");
        }

        byte = &msg[index];
        index++;
        *byte = 0;
        if (*byte & 0x10) {
            ra_cap->ms_ra_capability[count].es_ind = TRUE;
            DBG_TRACE("ES Indicator");
        }
        if (*byte & 0x08) {
            ra_cap->ms_ra_capability[count].ps = TRUE;
            DBG_TRACE("PS");
        }
        if (*byte & 0x04) {
            ra_cap->ms_ra_capability[count].vgcs = TRUE;
            DBG_TRACE("VGCS");
        }
        if (*byte & 0x02) {
            ra_cap->ms_ra_capability[count].vbs = TRUE;
            DBG_TRACE("VBS");
        }

        ra_cap->ms_ra_capability[count].multislot_capability.present = TRUE;
        byte = &msg[index];
        index++;
        ra_cap->ms_ra_capability[count].multislot_capability.hscsd_multi_slot_class = (*byte & 0x1f);
        ra_cap->ms_ra_capability[count].multislot_capability.hscsd_param_present = (*byte >> 7);
        if (ra_cap->ms_ra_capability[count].multislot_capability.hscsd_param_present)
            DBG_TRACE("HSCSD Multi slot class %d",ra_cap->ms_ra_capability[count].multislot_capability.hscsd_multi_slot_class);
        else    
            DBG_TRACE("HSCSD Multi slot class not present");

        byte = &msg[index];
        index++;
        ra_cap->ms_ra_capability[count].multislot_capability.gprs_multi_slot_class = ((*byte >> 2) & 0x1f);
        ra_cap->ms_ra_capability[count].multislot_capability.gprs_dynamic_allocation_capability = ((*byte >> 1) & 0x1);
        ra_cap->ms_ra_capability[count].multislot_capability.gprs_params_present = (*byte >> 7);
        ra_cap->ms_ra_capability[count].multislot_capability.sms_params_present = (*byte & 0x1);
        if (ra_cap->ms_ra_capability[count].multislot_capability.gprs_params_present)
        {
            DBG_TRACE("GPRS Multi slot class %d",ra_cap->ms_ra_capability[count].multislot_capability.gprs_multi_slot_class);
            DBG_TRACE("GPRS Ext dynamic allocation cap %d",ra_cap->ms_ra_capability[count].multislot_capability.gprs_dynamic_allocation_capability);
        }
        else
            DBG_TRACE("GPRS Multi slot class not present");

        byte = &msg[index];
        index++;
        ra_cap->ms_ra_capability[count].multislot_capability.switch_measure_value = (*byte & 0x0f);
        ra_cap->ms_ra_capability[count].multislot_capability.switch_measure_switch_value = (*byte & 0xf0) >> 4;
        if (ra_cap->ms_ra_capability[count].multislot_capability.sms_params_present)
            DBG_TRACE("Switch measure value %d, Switch measure switch value %d",
                ra_cap->ms_ra_capability[count].multislot_capability.switch_measure_value,
                ra_cap->ms_ra_capability[count].multislot_capability.switch_measure_switch_value);
        else        
            DBG_TRACE("Switch measure not present");
    }

    ra_cap->ie_present = TRUE;
    *len += index;

    DBG_TRACE("MS-Radio-Access-Capability IEI length %d bytes",index);

    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_omc_id
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
BOOL
bssgp_ie_decode_omc_id(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_omc_id",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    /* FIXME */
    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_pdu_in_error
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
BOOL
bssgp_ie_decode_pdu_in_error(UINT8 *llc_pdu,UINT16 *pdu_len,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_pdu_in_error",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];
    index++;

    if (*byte != BSSGP_IEI_PDU_IN_ERROR) {
        DBG_ERROR("IEI not LLC PDU in Error, %d type !!",*byte);
        DBG_LEAVE();
        return (FALSE);
    }


    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);

    *pdu_len = iei_len;

    DBG_TRACE("Decoding LLC PDU in error of size %d",iei_len);

    byte = &msg[index];

    memcpy(llc_pdu,byte,iei_len);

    index += *pdu_len;

    *len += index;
    
    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_ie_decode_pdu_lifetime
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
BOOL
bssgp_ie_decode_pdu_lifetime(RIL3_IE_PDU_LIFETIME *delay, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_pdu_lifetime",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_PDU_LIFETIME) {
        DBG_ERROR("PDU Lifetime IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT16)) {
        DBG_ERROR("PDU Lifetime IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    delay->pdu_lifetime = (msg[index++] << 8) | msg[index++];

    DBG_TRACE("PDU Lifetime %d decoded; IEI length %d bytes",delay->pdu_lifetime,index);

    *len += index;
    delay->ie_present = TRUE;
    
    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_priority
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
BOOL
bssgp_ie_decode_priority(RIL3_IE_PRIORITY *priority, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_priority",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_PRIORITY) {
        DBG_ERROR("Priority IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT8)) {
        DBG_ERROR("Priority IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    priority->priority = msg[index++];

    DBG_TRACE("Priority %d decoded; IEI length %d bytes",priority->priority,index);

    *len += index;
    priority->ie_present = TRUE;
    
    DBG_LEAVE();
    return (TRUE);
}







/*
 * bssgp_ie_decode_qos_params
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
BOOL
bssgp_ie_decode_qos_params(T_CNI_RIL3_IE_QOS *qos_params, UINT8 *msg, UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_qos_params",BSSGP_DECODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0, iei_len;

    byte = &msg[index];
    index++;

    if (*byte != BSSGP_IEI_QOS_PARAMS) {
        DBG_ERROR("Invalid QoS Params IEI");
        DBG_LEAVE();
        return (FALSE);
    }

    
    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != (sizeof(UINT8)*5)) {
        DBG_ERROR("Invalid QoS Params IEI Length, %d",iei_len);
        DBG_LEAVE();
        return (FALSE);
    }

    /* Delay Class */
    byte    = &msg[index];
    qos_params->delay_class = (CNI_RIL3_QOS_DELAY_CLASS)*byte;
    index++;
    /* Reliability class */
    byte    = &msg[index];
    qos_params->reliability_class = (CNI_RIL3_QOS_RELIABILITY_CLASS)*byte;
    index++;
    /* Precedence class */
    byte    = &msg[index];
    qos_params->precedence_class = (CNI_RIL3_QOS_PRECEDENCE_CLASS)*byte;
    index++;
    /* Peak throughput */
    byte    = &msg[index];
    qos_params->peak_throughput = (CNI_RIL3_QOS_PEAK_THROUGHPUT)*byte;
    index++;
    /* Mean throughput */
    byte    = &msg[index];
    qos_params->mean_throughput = (CNI_RIL3_QOS_MEAN_THROUGHPUT)*byte;
    index++;

    qos_params->ie_present = TRUE;
    DBG_TRACE("Decoded QoS params; Delay %d, Reliability %d, Precedence %d, Peak throughput %d & Mean throughput %d",
        qos_params->delay_class,qos_params->reliability_class,qos_params->precedence_class,
        qos_params->peak_throughput,qos_params->mean_throughput);
    *len += index;
    DBG_LEAVE();
    return (FALSE);
}


/*
 * bssgp_ie_decode_qos_profile
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
BOOL
bssgp_ie_decode_qos_profile(T_RIL3_IE_QOS_PROFILE_ID *qos_profile,UINT8 *msg,UINT16 *len, BSSGP_IEI_FORMAT format)
{
    DBG_FUNC("bssgp_ie_decode_qos_profile",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0;

    if (format == BSSGP_FORMAT_TLV)
    {
        UINT16 iei_len;
        
        byte = &msg[index];
        if (*byte != BSSGP_IEI_QOS_PROFILE) {
            DBG_ERROR("Invalid QoS profile IEI");
            DBG_LEAVE();
            return (FALSE);
        }

        index++;
    
        iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    }

    qos_profile->bucket_leak_rate[0] = msg[index];
    index++;
    qos_profile->bucket_leak_rate[1] = msg[index];
    index++;


    DBG_TRACE("Bucket Leak Rate %d:%d",qos_profile->bucket_leak_rate[0],
            qos_profile->bucket_leak_rate[1]);

    byte = &msg[index];
    qos_profile->precedence_class = (CNI_RIL3_QOS_PRECEDENCE_CLASS)(*byte & 0x07);


    if (*byte & 0x08) {
        qos_profile->a_bit = RADIO_INT_USES_RLC_MAX_UDT;
        DBG_TRACE("Radio interface uses RLC/MAC-UNITDATA functionality");
    }
    else {
        qos_profile->a_bit = RADIO_INT_USES_RLC_MAC_ARQ;
        DBG_TRACE("Radio interface uses RLC/MAC ARQ functionality");
    }
    
    if (*byte & 0x10) {
        qos_profile->t_bit = SDU_CONTAINS_DATA;
        DBG_TRACE("SDU contains Data");
    }
    else {
        qos_profile->t_bit = SDU_CONTAINS_SIGNALLING;
        DBG_TRACE("SDU contains Signalling (e.g. related to GMM)");
    }

    if (*byte & 0x20) {
        qos_profile->cr_bit = SDU_LLC_ACK_SACK_ABSENT;
        DBG_TRACE("SDU does NOT contain a LLC ACK or SACK command/response frame type");
    }
    else {
        qos_profile->cr_bit = SDU_LLC_ACK_SACK_PRESENT;
        DBG_TRACE("SDU contains LLC ACK or SACK command/response frame type");
    }
    qos_profile->ie_present = TRUE;
    index++;
    *len += index;
    DBG_TRACE("QoS Profile IEI length %d bytes",index);
    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_radio_cause
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
BOOL
bssgp_ie_decode_radio_cause(T_RIL3_IE_RADIO_CAUSE_ID *radio_cause,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_radio_cause",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];
    index++;

    if (*byte != (UINT8)BSSGP_IEI_RADIO_CAUSE) {
        DBG_ERROR("Invalid Radio Cause IEI");
        DBG_LEAVE();
        return (FALSE);
    }
    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT8)) {
        DBG_ERROR("Invalid Radio Cause IEI length; %d",iei_len);
        DBG_LEAVE();
        return (FALSE);
    }
    byte = &msg[index];
    *radio_cause = (T_RIL3_IE_RADIO_CAUSE_ID)*byte;
    index++;

    DBG_TRACE("Decoded Radio cause : %s",
        bssgp_util_get_radio_cause_str(*radio_cause));


    DBG_TRACE("Radio Cause IEI length %d bytes",index);
    *len += index;

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_ie_decode_ra_cap_update_cause
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
BOOL
bssgp_ie_decode_ra_cap_update_cause(RA_CAP_UPD_CAUSE *ra_cause,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_ra_cap_update_cause",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];
    index++;

    if (*byte != BSSGP_IEI_RA_CAP_UPD_CAUSE) {
        DBG_ERROR("Error in Radio-Access-Capability-Update IEI");
        DBG_LEAVE();
        return (FALSE);
    }

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT8)) {
        DBG_ERROR("Error in Radio-Access-Capability-Update IEI length, %d",iei_len);
        DBG_LEAVE();
        return (FALSE);
    }

    byte = &msg[index];
    *ra_cause = (RA_CAP_UPD_CAUSE)*byte;
    index++;

    DBG_TRACE("Decoded Radio-Access-Capability-Update-Cause : %s",
        bssgp_util_get_ra_cap_cause_str(*ra_cause));


    DBG_TRACE("Radio-Access-Capability-Update-Cause IEI length %d bytes",index);
    *len += index;

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_ie_decode_reject_cause
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
BOOL
bssgp_ie_decode_reject_cause(T_CNI_RIL3_IE_REJECT_CAUSE *reject_cause, UINT8 *msg, UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_reject_cause",BSSGP_DECODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0, iei_len;

    byte = &msg[index];
    index++;

    if (*byte != BSSGP_IEI_LA_REJECT_CAUSE) {
        DBG_ERROR("Error in LA Reject cause IEI");
        DBG_LEAVE();
        return (FALSE);
    }

    
    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT8)) {
        DBG_ERROR("Error in LA Reject cause IEI length, %d",iei_len);
        DBG_LEAVE();
        return (FALSE);
    }

    byte = &msg[index];
    index++;
    
    reject_cause->causeValue = (T_CNI_RIL3_REJECT_CAUSE_VALUE)*byte;
    reject_cause->ie_present = TRUE;

    DBG_TRACE("Decoded Location Area Reject cause %d; IEI length %d",reject_cause->causeValue,index);
    *len += index;

    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_routing_area
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
BOOL
bssgp_ie_decode_routing_area(T_CNI_RIL3_IE_ROUTING_AREA_ID *routing_area,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_routing_area",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 lac, index = 0, iei_len;

    byte = &msg[index];
    index++;

    if (*byte != BSSGP_IEI_ROUTING_AREA) {
        DBG_ERROR("Error in Routing Area IEI");
        DBG_LEAVE();
        return (FALSE);
    }
    
    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != (3+2+sizeof(UINT16))) {
        DBG_ERROR("Error in Routing Area IEI length, %d",iei_len);
        DBG_LEAVE();
        return (FALSE);
    }

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
    routing_area->mcc[0] = msg[index] & 0x0f;
    routing_area->mcc[1] = (msg[index] & 0xf0) >> 4;
    index++;

    routing_area->mcc[2] = msg[index] & 0x0f;
    routing_area->mnc[2] = (msg[index] & 0xf0) >> 4;
    index++;

    routing_area->mnc[0] = msg[index] & 0x0f;
    routing_area->mnc[1] = (msg[index] & 0xf0) >> 4;
    index++;


    memcpy(&lac,&msg[index],sizeof(UINT16));
    routing_area->lac = ntohs(lac);
    index += sizeof(UINT16);

    routing_area->rac  = msg[index];
    index++;

    routing_area->ie_present = TRUE;

    DBG_TRACE("Routing Area :- MCC : %d%d%d ; MNC : %d%d(%x) ; LAC : %d ; RAC : %d",
        routing_area->mcc[0],routing_area->mcc[1],routing_area->mcc[2],
        routing_area->mnc[0],routing_area->mnc[1],routing_area->mnc[2],
        routing_area->lac,routing_area->rac);

    *len += index;
    DBG_TRACE("Routing Area IEI length %d bytes",index);

    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_r_def_ms
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
BOOL
bssgp_ie_decode_r_def_ms(UINT16 *r_default,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_r_def_ms",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_BMAX_DEFAULT_MS) {
        DBG_ERROR("R_Default_MS IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT16)) {
        DBG_ERROR("R_Default_MS IEI length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    *r_default = (msg[index++] << 8) | msg[index++];

    DBG_TRACE("R_Default_MS %d decoded; IEI length %d bytes",*r_default,index);

    *len += index;
    
    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_ie_decode_suspend_ref_num
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
BOOL
bssgp_ie_decode_suspend_ref_num(UINT8 *suspend_ref_num,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_suspend_ref_num",BSSGP_DECODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];
    index++;

    if (*byte != BSSGP_IEI_SUSPEND_REF_NUM) {
        DBG_ERROR("Invalid Suspend Ref number IEI");
        DBG_LEAVE();
        return (FALSE);
    }
    
    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT8)) {
        DBG_ERROR("Invalid Suspend Ref number IEI length");
        DBG_LEAVE();
        return (FALSE);
    }

    byte = &msg[index];
    index++;

    *suspend_ref_num = *byte;

    DBG_TRACE("Decoded Suspend Reference Number as %d; IEI length %d bytes",suspend_ref_num,index);
    
    *len += index;

    DBG_LEAVE();
    return (TRUE);
}



/*
 * bssgp_ie_decode_tag
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
BOOL
bssgp_ie_decode_tag(UINT8 *tag,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_tag",BSSGP_DECODER_LAYER);
    DBG_ENTER();
    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];
    index++;

    if (*byte != BSSGP_IEI_TAG) {
        DBG_ERROR("Error in TAG IEI");
        DBG_LEAVE();
        return (FALSE);
    }
    
    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(UINT8)) {
        DBG_ERROR("TAG IEI length invalid; %d value",iei_len);
        DBG_LEAVE();
        return (FALSE);
    }

    byte = &msg[index];
    index++;

    *tag = *byte;

    DBG_TRACE("Decoded tag as %d; IEI length %d bytes",*tag,index);
    
    *len += index;
    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_ie_decode_tlli
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
BOOL
bssgp_ie_decode_tlli(RIL3_TLLI *tlli,UINT8 *msg,UINT16 *len, BSSGP_IEI_FORMAT format)
{
    DBG_FUNC("bssgp_ie_decode_tlli",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT16 index = 0;
    UINT32 *shorty;

    if (format == BSSGP_FORMAT_TLV)
    {
        UINT8 *byte;
        UINT16 iei_len;
    
        byte = &msg[index];
        index++;

        if (*byte != BSSGP_IEI_TLLI) {
            DBG_ERROR("TLLI IEI invalid");
            DBG_LEAVE();
            return (FALSE);
        }

        iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
        if (iei_len != sizeof(TLLI)) {
            DBG_ERROR("TLLI IEI length invalid, %d",iei_len);
            DBG_LEAVE();
            return (FALSE);
        }
    }
    shorty= (UINT32*)&msg[index];

    /* now we get shorty ;-) */
    tlli->tlli = ntohl(*shorty);
    index += sizeof(UINT32);

    *len += index;
    tlli->ie_present = TRUE;
    
    DBG_TRACE("Decoded TLLI %#x; IEI length %d bytes",tlli->tlli,index);
    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_ie_decode_tmsi
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
BOOL
bssgp_ie_decode_tmsi(T_CNI_RIL3_IE_MOBILE_ID *tmsi,UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_tmsi",BSSGP_DECODER_LAYER);
    DBG_ENTER();


    UINT8 *byte;
    unsigned long tmsi_val;
    UINT16 index = 0,iei_len;

    byte = &msg[index];
    index++;

    if (*byte != BSSGP_IEI_TMSI) {
        DBG_ERROR("Error in TMSI IEI, invalid IEI type");
        DBG_LEAVE();
        return (FALSE);
    }

    
    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != sizeof(unsigned long)) {
        DBG_ERROR("IEI length invalid for TMSI IEI; %d",iei_len);
        DBG_LEAVE();
        return (FALSE);
    }

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
    memcpy(&tmsi_val,byte,sizeof(unsigned long));
    index += sizeof(unsigned long);

    tmsi->tmsi = ntohl(tmsi_val);


    tmsi->ie_present    = TRUE;
    tmsi->mobileIdType  = CNI_RIL3_TMSI;

    *len += index;
    DBG_TRACE("Decoded TMSI : %ld; IEI length %d bytes",tmsi->tmsi,index);

    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_ie_decode_trace_ref
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
BOOL
bssgp_ie_decode_trace_ref(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_trace_ref",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_ie_decode_trace_type
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
BOOL
bssgp_ie_decode_trace_type(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_trace_type",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_ie_decode_txn_id
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
BOOL
bssgp_ie_decode_txn_id(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    DBG_LEAVE();
    return (TRUE);
}


/*
 * bssgp_ie_decode_trigger_id
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
BOOL
bssgp_ie_decode_trigger_id(UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_trigger_id",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    DBG_LEAVE();
    return (TRUE);
}

/*
 * bssgp_ie_decode_num_octets_affected
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
BOOL
bssgp_ie_decode_num_octets_affected(UINT32 *num, UINT8 *msg,UINT16 *len)
{
    DBG_FUNC("bssgp_ie_decode_num_octets_affected",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte;
    UINT16 index = 0,iei_len;

    byte = &msg[index];

    if (*byte != (UINT8)BSSGP_IEI_NUM_OCTETS_AFFECTED) {
        DBG_ERROR("Number of octets affected IEI Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }
    index++;

    iei_len = bssgp_ie_decode_iei_length(&msg[index],&index);
    if (iei_len != 3) {
        DBG_ERROR("Number of octets affected length Invalid!");
        DBG_LEAVE();
        return (FALSE);
    }

    *num = (msg[index++] << 16) | (msg[index++] << 8) | msg[index++];

    DBG_TRACE("Number of octets affected %d decoded; IEI length %d bytes",*num,index);

    *len += index;

    DBG_LEAVE();
    return (TRUE);
}




/*
 * bssgp_ie_decode_iei_length
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
UINT16
bssgp_ie_decode_iei_length(UINT8 *msg,UINT16 *msg_len)
{
    DBG_FUNC("bssgp_ie_decode_iei_length",BSSGP_DECODER_LAYER);
    DBG_ENTER();

    UINT8 *byte = msg,  lo,hi;
    UINT16 iei_len=0;

    if (*byte & 0x80) {
        iei_len = *byte & 0x7f;
        *msg_len += 1;
        DBG_LEAVE();
        return (iei_len);
    }
    lo = (*byte & 0x7f);

    byte =(UINT8*)&msg[1];

    hi = *byte;

    iei_len = (hi * 128) + lo;

    DBG_TRACE("IEI length octet 1 %d octet 2 %d, total %d",lo,hi,iei_len);
    *msg_len += 2;

    DBG_LEAVE();
    return (iei_len);
}


void HexDumper(UINT8 *buffer, int length )
{     
    int size;     
    int i;     
    int dwPos = 0;
    static char line[255];
    int pos;

    DBG_FUNC("HexDumper", BSSGP_DECODER_LAYER);
    DBG_ENTER();

    while (length > 0) 
    { 
        //         
        // Take min of 16 or length         
        //                  
        size = min(16, length );          
        //         
        // Build text line        
        //                  
        pos = sprintf(line, "  %04X ", dwPos );          
        for (i = 0; i < size; i++) 
        {             
            pos += sprintf(line+pos, "%02X ", buffer[i] );             
        }          
        //         
        // Add spaces for short lines         
        //                  
        while (i < 16) 
        {             
            pos += sprintf(line+pos, "   " );             
            i++;         
        }          
        //         
        // Add ASCII chars         
        //                  
        for (i = 0; i < size; i++) 
        {             
            if (isprint(buffer[i])) 
            {                 
                pos += sprintf(line+pos, "%c", buffer[i] );                 
            } else {                 
                pos += sprintf(line+pos, "." );             
            }         
        }          
        
        DBG_TRACE((line));
        
        //         
        // Advance positions         
        // 
        
        length -= size;         
        dwPos += size;         
        buffer += size;     
    }
    DBG_LEAVE();
}




