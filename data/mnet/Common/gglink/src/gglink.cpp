
#include <string.h>
#include "gglink/gglink_api.h"

BOOL
GGLINK_UNPACK_SHORT(
    int             &maxlength, // max length of input buffer
    unsigned char * &ptr,       // starting address of input buffer
    short           &dest
    )
{    
    if(maxlength < 2)
        return FALSE;
    dest = (ptr[0] << 8) + ptr[1]; 
    ptr+=2; 
    maxlength-=2;
    return TRUE;
}

inline BOOL
GGLINK_UNPACK_BYTE(
    int             &maxlength,  // max length of input buffer
    unsigned char * &ptr,        // starting address of input buffer
    unsigned char   &dest
    )
{
    if (maxlength < 1)
        return FALSE;

    dest = *ptr++;    
    maxlength--;

    return TRUE;
}

inline BOOL
GGLINK_UNPACK_STRING(
    int              &maxlength,   // max length of input buffer
    int              stringlength, // size of the string to be unpacked
    unsigned char *  &ptr,         // starting address of input buffer
    unsigned char *  dest
    )
{
    if(maxlength < stringlength)
        return FALSE;

    memcpy(dest, ptr, stringlength);
    ptr += stringlength;
    maxlength -= stringlength;
    return TRUE;
}


BOOL GGLINK_DECODE_IE_MAP_CAUSE(
    int              &maxlength, // max length of input buffer
    unsigned char *  &ptr,       // starting address of input 
    GGLINK_IE_MAP_CAUSE &map_cause    // output IE data structure
    )
{
    map_cause.ie_present = true;
    return GGLINK_UNPACK_BYTE(maxlength, ptr, map_cause.map_error_code);
}

BOOL GGLINK_DECODE_IE_CAUSE(
    int              &maxlength, // max length of input buffer
    unsigned char *  &ptr,       // starting address of input buffer
    GGLINK_IE_CAUSE  &cause      // output IE data structure
    )
{
    cause.ie_present = true;
    return GGLINK_UNPACK_BYTE(maxlength, ptr, cause.cause_code);
}



BOOL GGLINK_DECODE_IE_GSN_ADDRESS(
    int              &maxlength,  // max length of input buffer
    unsigned char *  &ptr,        // starting address of input buffer
    GGLINK_IE_GSN_ADDRESS &gsn_address  // output IE data structure
    )
{
    gsn_address.ie_present = true; 
    if (!GGLINK_UNPACK_SHORT(maxlength, ptr, gsn_address.gsn_addr_length))
        return FALSE;

    if(gsn_address.gsn_addr_length > 16)
        return FALSE;
    
    return GGLINK_UNPACK_STRING(maxlength, gsn_address.gsn_addr_length, ptr, gsn_address.addr_octets);
}


BOOL GGLINK_DECODE_IE_MOBILE_ID(
    int             &maxlength,    // max length of input buffer
    unsigned char * &ptr,          // starting address of input buffer
    T_CNI_RIL3_IE_MOBILE_ID &mobileId  // output IE data structure
    )
{

    int totalLength = ptr[0] + 1;

    if (maxlength < totalLength)
    {
        return FALSE;
    }

    mobileId.ie_present = true;

    // get type
    mobileId.mobileIdType = (T_CNI_RIL3_MOBILE_ID_TYPE) (ptr[1] & 0x07);
    
    if(mobileId.mobileIdType == CNI_RIL3_TMSI)
    {
        // assuming tmsi is 4 bytes
        if(totalLength != 6) return FALSE;
        mobileId.tmsi = (ptr[2] << 24) +
                        (ptr[3] << 16) +
                        (ptr[4] << 8) +
                        ptr[5]
                        ;
    }
    else
    {
        bool oddInd = (ptr[1] & 0x08)? true: false;
        
        mobileId.numDigits = (totalLength - 2) * 2;
        if(oddInd) mobileId.numDigits +=1;
        
        mobileId.digits[0] = ptr[1] >> 4;
        
        int bufferIndex, digitIndex;
        for(bufferIndex=2, digitIndex = 1; 
            bufferIndex < totalLength; 
            bufferIndex++)
        {
            mobileId.digits[digitIndex++] = 
                ptr[bufferIndex] & 0x0f;
            mobileId.digits[digitIndex++] = 
                ptr[bufferIndex] >> 4;
        }
    }

    maxlength -= totalLength;
    ptr += totalLength;

    return TRUE;
}


// T_AUTH_TRIPLET is defined in the C only header file and used in embedded C only struct,
// so can't pass by reference
BOOL GGLINK_DECODE_AUTH_TRIPLET(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    T_AUTH_TRIPLET   *triplet      // output IE data structure
    )
{
    int i;
    int tripletlength = SEC_RAND_LEN + SEC_SRES_LEN + SEC_KC_LEN;

    if(maxlength < tripletlength) return FALSE;
    for(i=0;i<SEC_RAND_LEN;i++) triplet->RAND[i] = *ptr++;
    for(i=0;i<SEC_SRES_LEN;i++) triplet->SRES[i] = *ptr++;
    for(i=0;i<SEC_KC_LEN;i++) triplet->Kc[i] = *ptr++;
    maxlength -= tripletlength;

    return TRUE;    
}

BOOL GGLINK_DECODE_IE_AUTH_LIST(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    T_SEC_IE_AUTH_LIST &auth_set        // output IE data structure
    )
{
    int i, num;

    auth_set.num_set = num = *ptr++;
    maxlength--;

    if(num > SEC_MAX_TRIPLET){
        printf("Err. auth triplet list number too large : %d\n", num);
        return FALSE;
    }

    for(i=0; i<num; i++){
        if (!GGLINK_DECODE_AUTH_TRIPLET(maxlength, ptr, &auth_set.triplet[i]))
            return FALSE;
    }

    // sanity check
    if (maxlength < 0) {
        printf("GGLINK_DECODE_IE_AUTH_LIST buffer length too short \n");
        return FALSE;
    }
        
    return TRUE;
}

BOOL 
GGLINK_DECODE_IE_ROUTING_AREA_ID(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    T_CNI_RIL3_IE_ROUTING_AREA_ID  &ie    // output IE data structure
    )
{
    if(maxlength<6) return FALSE;

    // IE-specific processing
    ie.ie_present = true;

    ie.mcc[0] =  (*ptr) & 0x0F;                // MCC 1
    ie.mcc[1] = ((*ptr) & 0xF0) >> 4;        // MCC 2
    ptr++;
    ie.mcc[2] =  (*ptr) & 0x0F;                // MCC 3
    ie.mnc[2] = ((*ptr) & 0xF0) >> 4;        // MNC 3
    ptr++;
    ie.mnc[0] =  (*ptr) & 0x0F;                // MNC 1
    ie.mnc[1] = ((*ptr) & 0xF0) >> 4;        // MNC 2
    ptr++;
    ie.lac = *ptr++;
    ie.lac = (ie.lac << 8) + *ptr++;
    ie.rac = *ptr++;

    maxlength-=6;

    return TRUE;
}

BOOL
GGLINK_DECODE_API_MSG_GS_SRQ(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_GS_SRQ;

    return GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.security_req.imsi);
}


BOOL
GGLINK_DECODE_API_MSG_GS_SRS(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_GS_SRS;

    if (!GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.security_rsp.imsi))
        return FALSE;

    return GGLINK_DECODE_IE_AUTH_LIST(maxlength, ptr, msg.msg.security_rsp.triplet_list);
}

BOOL
GGLINK_DECODE_API_MSG_GS_RRQ(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_GS_RRQ;

    if (!GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.ra_update_req.imsi))
        return FALSE;

    return GGLINK_DECODE_IE_ROUTING_AREA_ID(maxlength, ptr, msg.msg.ra_update_req.rai);
}

BOOL
GGLINK_DECODE_API_MSG_GS_RCF(
    int               &maxlength,  // max length of input buffer
    unsigned char *   &ptr,        // starting address of input buffer
    GGLINK_API_MSG    &msg         // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_GS_RCF;

    return GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.ra_update_cnf.imsi);

}

BOOL
GGLINK_DECODE_API_MSG_GS_RRJ(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_GS_RRJ;

    if (!GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.ra_update_rej.imsi))
        return FALSE;

    short rejectCause;
    if (!GGLINK_UNPACK_SHORT(maxlength, ptr, rejectCause))
        return FALSE;

    msg.msg.ra_update_rej.cause = (T_CNI_RIL3_REJECT_CAUSE_VALUE)rejectCause;
    return TRUE;
}

BOOL
GGLINK_DECODE_API_MSG_GS_PRQ(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_GS_PRQ;

    return GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.profile_req.imsi);

}

BOOL
GGLINK_DECODE_API_MSG_GS_PRS(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_GS_PRS;

    if (!GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.profile_rsp.imsi))
        return FALSE;

    if (!GGLINK_UNPACK_SHORT(maxlength, ptr, msg.msg.profile_rsp.data_size))
        return FALSE;

    if(msg.msg.profile_rsp.data_size > 1024) {
        printf("GGLINK_DECODE_API_MSG_GS_PRS data length too long", msg.msg.profile_rsp.data_size);
        return FALSE;
    }

    return GGLINK_UNPACK_STRING(maxlength, msg.msg.profile_rsp.data_size, ptr, (unsigned char *)msg.msg.profile_rsp.data);

}

BOOL
GGLINK_DECODE_API_MSG_GS_URQ(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_GS_URQ;

    return GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.unregister_req.imsi);

}

BOOL
GGLINK_DECODE_API_MSG_SRI_REQ(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_SRI_REQ;

    if(!GGLINK_UNPACK_SHORT(maxlength, ptr, msg.msg.send_routing_info_req.gsn_seqno))
        return FALSE;

    if(!GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.send_routing_info_req.imsi))
        return FALSE;

    return GGLINK_DECODE_IE_GSN_ADDRESS(maxlength, ptr, msg.msg.send_routing_info_ack.ggsn_addr);
}

BOOL
GGLINK_DECODE_API_MSG_SRI_ACK(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_SRI_ACK;

    if (!GGLINK_UNPACK_SHORT(maxlength, ptr, msg.msg.send_routing_info_req.gsn_seqno))
        return FALSE;

    if (!GGLINK_DECODE_IE_CAUSE(maxlength, ptr, msg.msg.send_routing_info_ack.cause))
        return FALSE;

    if (!GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.send_routing_info_ack.imsi))
        return FALSE;
            
    if (msg.msg.send_routing_info_ack.cause.cause_code == GGLINK_CAUSE_REQ_ACCEPTED) {
        if(!GGLINK_DECODE_IE_GSN_ADDRESS(maxlength, ptr, msg.msg.send_routing_info_ack.sgsn_addr))
            return FALSE;

        return GGLINK_DECODE_IE_GSN_ADDRESS(maxlength, ptr, msg.msg.send_routing_info_ack.ggsn_addr);
    } else {
        return GGLINK_DECODE_IE_MAP_CAUSE(maxlength, ptr, msg.msg.send_routing_info_ack.map_cause);
    }
}

BOOL
GGLINK_DECODE_API_MSG_FR(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_FR;

    
    if (!GGLINK_UNPACK_SHORT(maxlength, ptr, msg.msg.fr_req.gsn_seqno))
        return FALSE;

    return GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.fr_req.imsi);

}

BOOL
GGLINK_DECODE_API_MSG_FR_ACK(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_FR_ACK;

    
    if (!GGLINK_UNPACK_SHORT(maxlength, ptr, msg.msg.fr_ack.gsn_seqno))
        return FALSE;

    if(! GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.fr_ack.imsi))
        return FALSE;

    
    if(! GGLINK_DECODE_IE_CAUSE(maxlength, ptr, msg.msg.fr_ack.cause))
        return FALSE;

    msg.msg.fr_ack.map_cause.ie_present = false;
    if(msg.msg.fr_ack.cause.cause_code != GGLINK_CAUSE_REQ_ACCEPTED)
        return GGLINK_DECODE_IE_MAP_CAUSE(maxlength, ptr, msg.msg.fr_ack.map_cause);
    
    return TRUE;
}

BOOL
GGLINK_DECODE_API_MSG_GPRS_PRE(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_GPRS_PRE;

    if (!GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.ms_gprs_pre_req.imsi))
        return FALSE;

    return GGLINK_DECODE_IE_GSN_ADDRESS(maxlength, ptr, msg.msg.ms_gprs_pre_req.gsn_addr);

}

BOOL
GGLINK_DECODE_API_MSG_GPRS_PRE_ACK(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_GPRS_PRE_ACK;


    if (!GGLINK_DECODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.ms_gprs_pre_ack.imsi))
        return FALSE;
    
    return GGLINK_DECODE_IE_GSN_ADDRESS(maxlength, ptr, msg.msg.ms_gprs_pre_ack.gsn_addr);

}

BOOL
GGLINK_DECODE_API_MSG_SSRI_REQ(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_SSRI_REQ;


    if (!GGLINK_UNPACK_SHORT(maxlength, ptr, msg.msg.ssri_req.gsn_seqno))
        return FALSE;

    return GGLINK_DECODE_IE_ROUTING_AREA_ID(maxlength, ptr, msg.msg.ssri_req.rai);
    
}

BOOL
GGLINK_DECODE_API_MSG_SSRI_RSP(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    msg.msg_type = GGLINK_API_MSG_SSRI_RSP;


    if (!GGLINK_UNPACK_SHORT(maxlength, ptr, msg.msg.ssri_rsp.gsn_seqno))
        return FALSE;

    if (!GGLINK_DECODE_IE_CAUSE(maxlength, ptr, msg.msg.ssri_rsp.cause))
        return FALSE;

    if (!GGLINK_DECODE_IE_ROUTING_AREA_ID(maxlength, ptr, msg.msg.ssri_rsp.rai))
        return FALSE;

    return GGLINK_DECODE_IE_GSN_ADDRESS(maxlength, ptr, msg.msg.ssri_rsp.sgsn_addr);

}

/*
 * DecodeGGLinkMessage
 */
BOOL 
DecodeGGLinkMessage(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{
    short msgType, magic;
    if (!GGLINK_UNPACK_SHORT(maxlength, ptr, msgType))
        return FALSE;
    
    // decode magic field
    if(!GGLINK_UNPACK_SHORT(maxlength, ptr, magic))
        return FALSE;
    msg.magic = magic;
    
    switch(msgType) /* message type */
    {
        case GGLINK_API_MSG_GS_SRQ:
            return GGLINK_DECODE_API_MSG_GS_SRQ(maxlength, ptr, msg);
        case GGLINK_API_MSG_GS_SRS:
            return GGLINK_DECODE_API_MSG_GS_SRS(maxlength, ptr, msg);
        case GGLINK_API_MSG_GS_RRQ:
            return GGLINK_DECODE_API_MSG_GS_RRQ(maxlength, ptr, msg);
        case GGLINK_API_MSG_GS_RCF:
            return GGLINK_DECODE_API_MSG_GS_RCF(maxlength, ptr, msg);
        case GGLINK_API_MSG_GS_RRJ:
            return GGLINK_DECODE_API_MSG_GS_RRJ(maxlength, ptr, msg);
        case GGLINK_API_MSG_GS_PRQ:
            return GGLINK_DECODE_API_MSG_GS_PRQ(maxlength, ptr, msg);
        case GGLINK_API_MSG_GS_PRS:
            return GGLINK_DECODE_API_MSG_GS_PRS(maxlength, ptr, msg);
        case GGLINK_API_MSG_GS_URQ:
            return GGLINK_DECODE_API_MSG_GS_URQ(maxlength, ptr, msg);
        case GGLINK_API_MSG_SRI_REQ:
            return GGLINK_DECODE_API_MSG_SRI_REQ(maxlength, ptr, msg);
        case GGLINK_API_MSG_SRI_ACK:
            return GGLINK_DECODE_API_MSG_SRI_ACK(maxlength, ptr, msg);
        case GGLINK_API_MSG_FR:
            return GGLINK_DECODE_API_MSG_FR(maxlength, ptr, msg);
        case GGLINK_API_MSG_FR_ACK:
            return GGLINK_DECODE_API_MSG_FR_ACK(maxlength, ptr, msg);
        case GGLINK_API_MSG_GPRS_PRE:
            return GGLINK_DECODE_API_MSG_GPRS_PRE(maxlength, ptr, msg);
        case GGLINK_API_MSG_GPRS_PRE_ACK:
            return GGLINK_DECODE_API_MSG_GPRS_PRE_ACK(maxlength, ptr, msg);
        case GGLINK_API_MSG_SSRI_REQ:
            return GGLINK_DECODE_API_MSG_SSRI_REQ(maxlength, ptr, msg);
        case GGLINK_API_MSG_SSRI_RSP:
            return GGLINK_DECODE_API_MSG_SSRI_RSP(maxlength, ptr, msg);
        case GGLINK_API_MSG_ECHO_REQ:
    		msg.msg_type = GGLINK_API_MSG_ECHO_REQ;
			return TRUE;
        case GGLINK_API_MSG_ECHO_RSP:
    		msg.msg_type = GGLINK_API_MSG_ECHO_RSP;
            return TRUE;
  
        default:
         printf( "Decode error: Non supported message for gglink %d\n",msgType);
         return FALSE;
   }

   return TRUE;
}

/*************************************************************************************************************/
inline BOOL
GGLINK_PACK_SHORT(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    short            value
    )
{   
    if(maxlength < 2)
        return FALSE;
    *ptr++ = value>>8; 
    *ptr++ = (unsigned char) value;
    
    maxlength-=2;

    return TRUE;
}

inline BOOL
GGLINK_PACK_BYTE(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    unsigned char    value
    )
{
    if (maxlength < 1)
        return FALSE;

    *ptr++ = value;
    maxlength--;

    return TRUE;
}

inline BOOL
GGLINK_PACK_STRING(
    int              &maxlength,   // max length of output buffer
    int              stringlength, // size of the string to be packed
    unsigned char *  &ptr,         // starting address of output buffer
    unsigned char *  src
    )
{
    if(maxlength < stringlength)
        return FALSE;

    memcpy(ptr, src, stringlength);
    ptr += stringlength;
    maxlength -= stringlength;

    return TRUE;
}

BOOL GGLINK_ENCODE_IE_CAUSE(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_IE_CAUSE  &cause        // input IE data structure
    )
{
    if(!cause.ie_present) return FALSE;
    return GGLINK_PACK_BYTE(maxlength, ptr, cause.cause_code);
}

BOOL GGLINK_ENCODE_IE_MAP_CAUSE(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_IE_MAP_CAUSE  &map_cause        // input IE data structure
    )
{
    if(!map_cause.ie_present) return FALSE;
    return GGLINK_PACK_BYTE(maxlength, ptr, map_cause.map_error_code);
    
}



BOOL GGLINK_ENCODE_IE_GSN_ADDRESS(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_IE_GSN_ADDRESS &gsn_address    // input IE data structure
    )
{

    if (!GGLINK_PACK_SHORT(maxlength, ptr, gsn_address.gsn_addr_length))
        return FALSE;

    if(gsn_address.gsn_addr_length > 16 || 
        gsn_address.gsn_addr_length < 0)
        return FALSE;
    
    return GGLINK_PACK_STRING(maxlength, gsn_address.gsn_addr_length, ptr, gsn_address.addr_octets);
}


BOOL
GGLINK_ENCODE_IE_MOBILE_ID(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    T_CNI_RIL3_IE_MOBILE_ID  &mobileId   // input IE data structure
    )
{

    if(mobileId.ie_present == FALSE)
        return FALSE;

    ptr[1] = mobileId.mobileIdType;
    
    if(mobileId.mobileIdType == CNI_RIL3_TMSI)
    {
        ptr[0] = 5;
        ptr[1] |= 0xf0;
        ptr[2] = (unsigned char) (mobileId.tmsi >> 24);
        ptr[3] = (unsigned char) (mobileId.tmsi >> 16);
        ptr[4] = (unsigned char) (mobileId.tmsi >> 8);
        ptr[5] = (unsigned char) (mobileId.tmsi);
        maxlength -= 6;
        ptr += 6;
    } else
    {
        
        unsigned char odd = (mobileId.numDigits % 2)? 0x08: 0x00;
        
        if (mobileId.numDigits <0 || mobileId.numDigits>CNI_RIL3_MAX_ID_DIGITS)
            return FALSE;

        int ieLength = 2 + (mobileId.numDigits/2);
        ptr[0] = ieLength - 1;
        ptr[1] |= ((mobileId.digits[0] << 4) | odd);

        maxlength -=ieLength;
        
        if(mobileId.numDigits < 2) 
        {
            ptr +=ieLength;
            return TRUE;
        }
        
        int bufferIndex = 2;
        int loopCount = (mobileId.numDigits/2) - 1 ;
        int digit;
        int i;
        for(i=0, digit=1; i < loopCount; i++)
        {
            ptr[bufferIndex] = mobileId.digits[digit++];
            ptr[bufferIndex++] |= mobileId.digits[digit++] << 4;
        }
        
        if(odd)    
        {
            ptr[bufferIndex] = mobileId.digits[digit++];
            ptr[bufferIndex] |= mobileId.digits[digit++] << 4;
        } 
        else // even digits
        {
            // encode last digit
            ptr[bufferIndex] = mobileId.digits[digit] | 0xf0;
        }

        ptr += ieLength;
    }        
        
    return TRUE;
}


bool GGLINK_ENCODE_AUTH_TRIPLET(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    T_AUTH_TRIPLET   *triplet      // input IE data structure
    )
{
    
    int tripletlength = SEC_RAND_LEN + SEC_SRES_LEN + SEC_KC_LEN;
    if (maxlength < tripletlength)
        return FALSE;

    if (!GGLINK_PACK_STRING(maxlength, SEC_RAND_LEN, ptr, triplet->RAND))
        return FALSE;


    if (!GGLINK_PACK_STRING(maxlength, SEC_SRES_LEN, ptr, triplet->SRES))
        return FALSE;


    if (!GGLINK_PACK_STRING(maxlength, SEC_KC_LEN, ptr, triplet->Kc))
        return FALSE;

    return TRUE;
}

BOOL GGLINK_ENCODE_IE_AUTH_LIST(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    T_SEC_IE_AUTH_LIST &auth_set   // input IE data structure
    )
{
    
    if(auth_set.num_set > SEC_MAX_TRIPLET){
        printf("GGLINK_ENCODE_IE_AUTH_LIST Err. auth triplet list number too large : %d\n", auth_set.num_set);
        return FALSE;
    }

    if (!GGLINK_PACK_BYTE(maxlength, ptr, (unsigned char) auth_set.num_set))
        return FALSE;

    int i;
    for(i=0; i < auth_set.num_set; i++){
        if (!GGLINK_ENCODE_AUTH_TRIPLET(maxlength, ptr, &auth_set.triplet[i]))
            return FALSE;
    }
    return TRUE;
}

BOOL GGLINK_ENCODE_IE_ROUTING_AREA_ID(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    T_CNI_RIL3_IE_ROUTING_AREA_ID  &ie  // input IE data structure
    )
{
    if( maxlength < 6 ) return FALSE;    // make sure we have enough bufferd for LV field

    *ptr++ = (ie.mcc[1] << 4) | (ie.mcc[0] & 0x0f);
    *ptr++ = (ie.mnc[2] << 4) | (ie.mcc[2] & 0x0f);
    *ptr++ = (ie.mnc[1] << 4) | (ie.mnc[0] & 0x0f);
    *ptr++ = ie.lac >> 8;
    *ptr++ = (unsigned char) ie.lac;
    *ptr++ = ie.rac;

    maxlength -= 6;

    return TRUE;
}

BOOL
GGLINK_ENCODE_API_MSG_GS_SRQ(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{

    return GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.security_req.imsi) ;
    
}


BOOL
GGLINK_ENCODE_API_MSG_GS_SRS(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{

    if (! GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.security_rsp.imsi))
        return FALSE;

    return GGLINK_ENCODE_IE_AUTH_LIST(maxlength, ptr, msg.msg.security_rsp.triplet_list);
}

BOOL
GGLINK_ENCODE_API_MSG_GS_RRQ(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{
    if (!GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.ra_update_req.imsi))
        return FALSE;

    return GGLINK_ENCODE_IE_ROUTING_AREA_ID(maxlength, ptr, msg.msg.ra_update_req.rai);
}

BOOL
GGLINK_ENCODE_API_MSG_GS_RCF(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{
    return GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.ra_update_cnf.imsi);

}

BOOL
GGLINK_ENCODE_API_MSG_GS_RRJ(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{
    if (!GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.ra_update_rej.imsi))
        return FALSE;

    return GGLINK_PACK_SHORT(maxlength, ptr, msg.msg.ra_update_rej.cause);

}

BOOL
GGLINK_ENCODE_API_MSG_GS_PRQ(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{

    return GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.profile_req.imsi);

}

BOOL
GGLINK_ENCODE_API_MSG_GS_PRS(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{
    if(msg.msg.profile_rsp.data_size > 1024) {
        printf("GGLINK_ENCODE_API_MSG_GS_PRS data length too long", msg.msg.profile_rsp.data_size);
        return FALSE;
    }

    if (!GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.profile_rsp.imsi))
        return FALSE;

    if (!GGLINK_PACK_SHORT(maxlength, ptr, msg.msg.profile_rsp.data_size))
        return FALSE;

    return GGLINK_PACK_STRING(maxlength, msg.msg.profile_rsp.data_size, ptr, 
        (unsigned char *)msg.msg.profile_rsp.data);

}

BOOL
GGLINK_ENCODE_API_MSG_GS_URQ(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{

    return GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.unregister_req.imsi);

}

BOOL
GGLINK_ENCODE_API_MSG_SRI_REQ(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{

    if(!GGLINK_PACK_SHORT(maxlength, ptr, msg.msg.send_routing_info_req.gsn_seqno))
        return FALSE;

    if(!GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.send_routing_info_req.imsi))
        return FALSE;

    return GGLINK_ENCODE_IE_GSN_ADDRESS(maxlength, ptr, msg.msg.send_routing_info_req.ggsn_addr);
}

BOOL
GGLINK_ENCODE_API_MSG_SRI_ACK(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{

    if (!GGLINK_PACK_SHORT(maxlength, ptr, msg.msg.send_routing_info_req.gsn_seqno))
        return FALSE;

    if (!GGLINK_ENCODE_IE_CAUSE(maxlength, ptr, msg.msg.send_routing_info_ack.cause))
        return FALSE;
    
    if (!GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.send_routing_info_ack.imsi))
        return FALSE;

    if (msg.msg.send_routing_info_ack.cause.cause_code == GGLINK_CAUSE_REQ_ACCEPTED) {
        if(!GGLINK_ENCODE_IE_GSN_ADDRESS(maxlength, ptr, msg.msg.send_routing_info_ack.sgsn_addr))
            return FALSE;
        return GGLINK_ENCODE_IE_GSN_ADDRESS(maxlength, ptr, msg.msg.send_routing_info_ack.ggsn_addr);
    } else {
        return GGLINK_ENCODE_IE_MAP_CAUSE(maxlength, ptr, msg.msg.send_routing_info_ack.map_cause);
    }

}

BOOL
GGLINK_ENCODE_API_MSG_FR(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{

    if (!GGLINK_PACK_SHORT(maxlength, ptr, msg.msg.fr_req.gsn_seqno))
        return FALSE;

    return GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.fr_req.imsi);

}

BOOL
GGLINK_ENCODE_API_MSG_FR_ACK(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{

    
    if (!GGLINK_PACK_SHORT(maxlength, ptr, msg.msg.fr_ack.gsn_seqno))
        return FALSE;

    if(! GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.fr_ack.imsi))
        return FALSE;

    if(! GGLINK_ENCODE_IE_CAUSE(maxlength, ptr, msg.msg.fr_ack.cause))
        return FALSE;

    if(msg.msg.fr_ack.cause.cause_code != GGLINK_CAUSE_REQ_ACCEPTED)
        return GGLINK_ENCODE_IE_MAP_CAUSE(maxlength, ptr, msg.msg.fr_ack.map_cause);

    if(msg.msg.fr_ack.map_cause.ie_present)
    {
        printf("GGLINK_ENCODE_API_MSG_FR_ACK:Map cause should not be included if cause code is REQ ACCPETED\n");
    }

    return TRUE;

}

BOOL
GGLINK_ENCODE_API_MSG_GPRS_PRE(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{

    if (!GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.ms_gprs_pre_req.imsi))
        return FALSE;

    return GGLINK_ENCODE_IE_GSN_ADDRESS(maxlength, ptr, msg.msg.ms_gprs_pre_req.gsn_addr);

}

BOOL
GGLINK_ENCODE_API_MSG_GPRS_PRE_ACK(
    int              &maxlength,   // max length of output buffer
    unsigned char *  &ptr,         // starting address of output buffer
    GGLINK_API_MSG   &msg          // input msg data structure
    )
{

    if (!GGLINK_ENCODE_IE_MOBILE_ID(maxlength, ptr, msg.msg.ms_gprs_pre_ack.imsi))
        return FALSE;

    if (!GGLINK_ENCODE_IE_GSN_ADDRESS(maxlength, ptr, msg.msg.ms_gprs_pre_ack.gsn_addr))
        return FALSE;
    
    return GGLINK_ENCODE_IE_CAUSE(maxlength, ptr, msg.msg.ms_gprs_pre_ack.cause);
}

BOOL
GGLINK_ENCODE_API_MSG_SSRI_REQ(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{

    if (!GGLINK_PACK_SHORT(maxlength, ptr, msg.msg.ssri_req.gsn_seqno))
        return FALSE;

    return GGLINK_ENCODE_IE_ROUTING_AREA_ID(maxlength, ptr, msg.msg.ssri_req.rai);
    
}

BOOL
GGLINK_ENCODE_API_MSG_SSRI_RSP(
    int              &maxlength,   // max length of input buffer
    unsigned char *  &ptr,         // starting address of input buffer
    GGLINK_API_MSG   &msg          // output msg data structure
    )
{

    if (!GGLINK_PACK_SHORT(maxlength, ptr, msg.msg.ssri_rsp.gsn_seqno))
        return FALSE;

    if (!GGLINK_ENCODE_IE_CAUSE(maxlength, ptr, msg.msg.ssri_rsp.cause))
        return FALSE;

    if (!GGLINK_ENCODE_IE_ROUTING_AREA_ID(maxlength, ptr, msg.msg.ssri_rsp.rai))
        return FALSE;

    return GGLINK_ENCODE_IE_GSN_ADDRESS(maxlength, ptr, msg.msg.ssri_rsp.sgsn_addr);

}
/*
 * EncodeGGLinkMessage
 */
BOOL 
EncodeGGLinkMessage(int &nDataSize, unsigned char * &ptr, GGLINK_API_MSG &msg)
{
    BOOL retVal;
    int initialLength;
    unsigned char *pLength = ptr;
    ptr +=2;
    nDataSize-=2;    // 2 bytes for length

    initialLength = nDataSize;

    // encode the message type    
    if(!GGLINK_PACK_SHORT(nDataSize, ptr, msg.msg_type))
        return FALSE;
        // encode magic field
    if(!GGLINK_PACK_SHORT(nDataSize, ptr, msg.magic))
        return FALSE;

    switch(msg.msg_type) /* message type */
    {
        case GGLINK_API_MSG_GS_SRQ:
            retVal = GGLINK_ENCODE_API_MSG_GS_SRQ(nDataSize, ptr, msg);    
            break;
        case GGLINK_API_MSG_GS_SRS:
            retVal = GGLINK_ENCODE_API_MSG_GS_SRS(nDataSize, ptr, msg);    
            break;
        case GGLINK_API_MSG_GS_RRQ:
            retVal = GGLINK_ENCODE_API_MSG_GS_RRQ(nDataSize, ptr, msg);    
            break;
        case GGLINK_API_MSG_GS_RCF:
            retVal = GGLINK_ENCODE_API_MSG_GS_RCF(nDataSize, ptr, msg);    
            break;
        case GGLINK_API_MSG_GS_RRJ:
            retVal = GGLINK_ENCODE_API_MSG_GS_RRJ(nDataSize, ptr, msg);    
            break;
        case GGLINK_API_MSG_GS_PRQ:
            retVal = GGLINK_ENCODE_API_MSG_GS_PRQ(nDataSize, ptr, msg);    
            break;
        case GGLINK_API_MSG_GS_PRS:
            retVal = GGLINK_ENCODE_API_MSG_GS_PRS(nDataSize, ptr, msg);    
            break;
        case GGLINK_API_MSG_GS_URQ:
            retVal = GGLINK_ENCODE_API_MSG_GS_URQ(nDataSize, ptr, msg);    
            break;
        case GGLINK_API_MSG_SRI_REQ:
            retVal = GGLINK_ENCODE_API_MSG_SRI_REQ(nDataSize, ptr, msg); 
            break;
        case GGLINK_API_MSG_SRI_ACK:
            retVal = GGLINK_ENCODE_API_MSG_SRI_ACK(nDataSize, ptr, msg); 
            break;
        case GGLINK_API_MSG_FR:
            retVal = GGLINK_ENCODE_API_MSG_FR(nDataSize, ptr, msg);    
            break;
        case GGLINK_API_MSG_FR_ACK:
            retVal = GGLINK_ENCODE_API_MSG_FR_ACK(nDataSize, ptr, msg);    
            break;
        case GGLINK_API_MSG_GPRS_PRE:
            retVal = GGLINK_ENCODE_API_MSG_GPRS_PRE(nDataSize, ptr, msg);    
            break;
        case GGLINK_API_MSG_GPRS_PRE_ACK:
            retVal = GGLINK_ENCODE_API_MSG_GPRS_PRE_ACK(nDataSize, ptr, msg);    
            break;
        case GGLINK_API_MSG_SSRI_REQ:
            retVal = GGLINK_ENCODE_API_MSG_SSRI_REQ(nDataSize, ptr, msg);
            break;
        case GGLINK_API_MSG_SSRI_RSP:
            retVal = GGLINK_ENCODE_API_MSG_SSRI_RSP(nDataSize, ptr, msg);
            break;
        case GGLINK_API_MSG_ECHO_REQ:
        case GGLINK_API_MSG_ECHO_RSP:
            retVal = TRUE; 
            break;
  
        default:
         printf( "Encode error: Non supported message for gglink %d\n",msg.msg_type);
         return FALSE;
    }
    
    if(retVal)
    {
        int lengthLength = 2;
        short encodedLength = initialLength- nDataSize; 
        retVal = GGLINK_PACK_SHORT(lengthLength, pLength, encodedLength);
        nDataSize = encodedLength + 2;
    }

   return retVal;
}
 
