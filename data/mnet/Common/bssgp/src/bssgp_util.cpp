/****************************************************************************************
 *																						*
 *	Copyright Cisco Systems, Inc 2000 All rights reserved     							*
 *																						*
 *	File				: bssgp_util.cpp												*
 *																						*
 *	Description			: Utility functions for the BSS-GP layer/module					*
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
#include <string.h>
#include "bssgp/bssgp_prot.h"
#include "bssgp/bssgp_util.h"


static char *unknown_str = "UNKNOWN TYPE";
static char *resvd_str = "Reserved";

/*
 * bssgp_util_get_iei_str
 */
char *
bssgp_util_get_iei_str(BSSGP_IEI_TYPE iei_type)
{
	static char *iei_str[] = {
			"Alignment octets",                     
			"Bmax default MS",
			"BSS Area Indication",
			"BVCI",
			"BVC Bucket Size",
			"BVC Measurement",
			"Cause",
			"Cell Identifier",
			"Channel needed",
			"DRX parameters",
			"eMLPP Priority",
			"Flush action",
			"IMSI",
			"LLC-PDU",
			"LLC Frames discarded",
			"Location area",
			"Mobile Id",
			"MS Bucket size",
			"MS Radio Access capability",
			"OMC Id",
			"PDU in error",
			"PDU Lifetime",
			"Priority",
			"QoS Profile",
			"Radio Cause",
			"RA-Cap-UPD-cause",
			"Routing area",
			"R_default_MS",
			"Suspend reference number",
			"Tag",
			"TLLI",
			"TMSI",
			"Trace reference",
			"Trace type",
			"Transaction Id",
			"Trigger Id",
			"Number of octets affected",
			"LSA Identifier list",
			"LSA Information",
            "QoS Parameters",
            "Location Area Reject Cause"
			};

	if (iei_type < BSSGP_IEI_MAX_TYPES) {
		return (iei_str[(int)iei_type]);
	}
	else {
		return (unknown_str);
	}
}



/*
 * bssgp_util_get_pdu_type_str
 */
char *
bssgp_util_get_pdu_type_str(BSSGP_PDU_TYPE pdu_type)
{
	static char pdu_type_str[128];
	switch(pdu_type) {
		case BSSGP_PDU_DL_UNITDATA:{
			strncpy(pdu_type_str,"Downlink Unit Data",128);
		}
        break;

		case BSSGP_PDU_UL_UNITDATA:{
			strncpy(pdu_type_str,"Uplink Unit Data",128);
		}
        break;

		case BSSGP_PDU_RA_CAP:{
			strncpy(pdu_type_str,"Radio Access Capability",128);
		}
        break;

		case BSSGP_PDU_PTM_UNITDATA:{
			strncpy(pdu_type_str,"PTM Unit Data",128);
		}
        break;

		case BSSGP_PDU_PAGING_PS:{
			strncpy(pdu_type_str,"Paging PS",128);
		}
        break;

		case BSSGP_PDU_PAGING_CS:{
			strncpy(pdu_type_str,"Paging CS",128);
		}
        break;

		case BSSGP_PDU_RA_CAP_UPDATE:{
			strncpy(pdu_type_str,"Radio Access Capability Update",128);
		}
        break;

		case BSSGP_PDU_RA_CAP_UPDATE_ACK:{
			strncpy(pdu_type_str,"Radio Acess Capabilty Update ACK",128);
		}
        break;

		case BSSGP_PDU_RADIO_STATUS:{
			strncpy(pdu_type_str,"Radio Status",128);
		}
        break;

		case BSSGP_PDU_SUSPEND:{
			strncpy(pdu_type_str,"Suspend",128);
		}
        break;

		case BSSGP_PDU_SUSPEND_ACK:{
			strncpy(pdu_type_str,"Suspend ACK",128);
		}
        break;

		case BSSGP_PDU_SUSPEND_NACK:{
			strncpy(pdu_type_str,"Suspend NACK",128);
		}
        break;

		case BSSGP_PDU_RESUME:{
			strncpy(pdu_type_str,"Resume",128);
		}
        break;

		case BSSGP_PDU_RESUME_ACK:{
			strncpy(pdu_type_str,"Resume ACK",128);
		}
        break;

		case BSSGP_PDU_RESUME_NACK:{
			strncpy(pdu_type_str,"Resume NACK",128);
		}
        break;

		case BSSGP_PDU_BVC_BLOCK:{
			strncpy(pdu_type_str,"BVC Block",128);
		}
        break;

		case BSSGP_PDU_BVC_BLOCK_ACK:{
			strncpy(pdu_type_str,"BVC Block ACK",128);
		}
        break;

		case BSSGP_PDU_RESET:{
			strncpy(pdu_type_str,"Reset",128);
		}
        break;

		case BSSGP_PDU_RESET_ACK:{
			strncpy(pdu_type_str,"Reset ACK",128);
		}
        break;

		case BSSGP_PDU_UNBLOCK:{
			strncpy(pdu_type_str,"Unblock",128);
		}
        break;

		case BSSGP_PDU_UNBLOCK_ACK:{
			strncpy(pdu_type_str,"Unblock ACK",128);
		}
        break;

		case BSSGP_PDU_FLOW_CONTROL_BVC:{
			strncpy(pdu_type_str,"Flow Control BVC",128);
		}
        break;

		case BSSGP_PDU_FLOW_CONTROL_BVC_ACK:{
			strncpy(pdu_type_str,"Flow Control BVC ACK",128);
		}
        break;

		case BSSGP_PDU_FLOW_CONTROL_MS:{
			strncpy(pdu_type_str,"Flow Control MS",128);
		}
        break;

		case BSSGP_PDU_FLOW_CONTROL_MS_ACK:{
			strncpy(pdu_type_str,"Flow Control MS ACK",128);
		}
        break;

		case BSSGP_PDU_FLUSH_LL:{
			strncpy(pdu_type_str,"Flush LL",128);
		}
        break;

		case BSSGP_PDU_FLUSH_LL_ACK:{
			strncpy(pdu_type_str,"Flush LL ACK",128);
		}
        break;

		case BSSGP_PDU_LLC_DISCARDED:{
			strncpy(pdu_type_str,"LLC Discarded",128);
		}
        break;

		case BSSGP_PDU_LOCATION_UPDATE:{
			strncpy(pdu_type_str,"Location Update",128);
		}
        break;

		case BSSGP_PDU_LOCATION_UPDATE_ACK:{
			strncpy(pdu_type_str,"Location Update Ack",128);
		}
        break;

		case BSSGP_PDU_LOCATION_UPDATE_NACK:{
			strncpy(pdu_type_str,"Location Update Nack",128);
		}
        break;

		case BSSGP_PDU_ROUTING_AREA_UPDATE:{
			strncpy(pdu_type_str,"Routing Area Update",128);
		}
        break;

		case BSSGP_PDU_SGSN_INVOKE_TRACE:{
			strncpy(pdu_type_str,"Invoke Trace",128);
		}
        break;

		case BSSGP_PDU_STATUS:{
			strncpy(pdu_type_str,"Status",128);
		}
        break;

		default:{
			strncpy(pdu_type_str,"Unknown PDU Type",128);
		}
        break;
	}
	return (pdu_type_str);
}






/*
 * bssgp_util_get_radio_cause_str
 */
char *
bssgp_util_get_radio_cause_str(UINT8 radio_cause)
{
    static char radio_cause_str[128];
    switch(radio_cause) {
	    case RADIO_CAUSE_MS_CONTACT_LOST: {
            strncpy(radio_cause_str,"Radio contact lost with MS",128);
        }
        break;
	    case RADIO_CAUSE_LINK_QUALITY_INSUFFICIENT: {
            strncpy(radio_cause_str,"Radio link quality insufficient to continue",128);
        }
        break;
        case RADIO_CAUSE_CELL_RESELECTION_ORDERED: {
            strncpy(radio_cause_str,"Cell reselection ordered",128);
        }
        break;
        default: {
            strncpy(radio_cause_str,"Unknown : Radio contact lost with MS",128);
        }
        break;
    }
    return (radio_cause_str);
}


/*
 * bssgp_util_get_exception_cause_str
 */
char *
bssgp_util_get_exception_cause_str(BSSGP_EXCEPTION_CAUSE cause)
{
    static char exception_cause_str[128];

    switch(cause) {
        case BSSGP_CAUSE_PROCESSOR_OVERLOAD: {
            strncpy(exception_cause_str,"Processor overload",128);
        }
        break;
        case BSSGP_CAUSE_EQUIPMENT_FAILURE: {
            strncpy(exception_cause_str,"Equipment failure",128);
        }
        break;
        case BSSGP_CAUSE_TRANSIT_NETWORK_FAILURE: {
            strncpy(exception_cause_str,"Transit network failure",128);
        }
        break;
        case BSSGP_CAUSE_CAPACITY_MODIFIED: {
            strncpy(exception_cause_str,"Network service modified from 0 to greater",128);
        }
        break;
        case BSSGP_CAUSE_UNKNOWN_MS: {
            strncpy(exception_cause_str,"Unknown MS",128);
        }
        break;
        case BSSGP_CAUSE_BVCI_UNKNOWN: {
            strncpy(exception_cause_str,"BVCI unknown",128);
        }
        break;
        case BSSGP_CAUSE_CELL_TRAFFIC_CONGESTION: {
            strncpy(exception_cause_str,"Cell traffic congestion",128);
        }
        break;
        case BSSGP_CAUSE_SGSN_CONGESTION: {
            strncpy(exception_cause_str,"SGSN congestion",128);
        }
        break;
        case BSSGP_CAUSE_OAM_INTERVENTION: {
            strncpy(exception_cause_str,"O & M intervention",128);
        }
        break;
        case BSSGP_CAUSE_BVCI_BLOCKED: {
            strncpy(exception_cause_str,"BVCI blocked",128);
        }
        break;
        case BSSGP_CAUSE_SEMANTICALLY_INCORRECT_PDU: {
            strncpy(exception_cause_str,"Semantically incorrect PDU",128);
        }
        break;
        case BSSGP_CAUSE_INVALID_MANDATORY_INFO: {
            strncpy(exception_cause_str,"Invalid mandatory information",128);
        }
        break;
        case BSSGP_CAUSE_MISSING_MANDATORY_IE: {
            strncpy(exception_cause_str,"Missing mandatory IE",128);
        }
        break;
        case BSSGP_CAUSE_MISSING_CONDITIONAL_IE: {
            strncpy(exception_cause_str,"Missing conditional IE",128);
        }
        break;
        case BSSGP_CAUSE_UNEXPECTED_CONDITIONAL_IE: {
            strncpy(exception_cause_str,"Unexpected conditional IE",128);
        }
        break;
        case BSSGP_CAUSE_CONDITIONAL_IE_ERROR: {
            strncpy(exception_cause_str,"Conditional IE error",128);
        }
        break;
        case BSSGP_CAUSE_PDU_NOT_COMPAT_PROTOCOL_STATE: {
            strncpy(exception_cause_str,"PDU not compatible with protocol state",128);
        }
        break;
        case BSSGP_CAUSE_PROTOCOL_ERROR_UNSPECIFIED: 
        default: {
            strncpy(exception_cause_str,"Protocol error - unspecified",128);
        }
        break;
    }
    return (exception_cause_str);
}


/*
 * bssgp_util_get_ra_cap_cause_str
 */
char *
bssgp_util_get_ra_cap_cause_str(RA_CAP_UPD_CAUSE ra_cause)
{
    static char ra_cap_cause_str[128];
    switch (ra_cause) {
        case RA_CAP_UPD_CAUSE_IE_PRESENT:{
            strncpy(ra_cap_cause_str,"OK, RA capability IE present",128);
        }
        break;
        case RA_CAP_UPD_CAUSE_TLLI_UNKNOWN:{
            strncpy(ra_cap_cause_str,"TLLI unknown in SGSN",128);
        }
        break;
        case RA_CAP_UPD_CAUSE_NO_RA_CAP:{
            strncpy(ra_cap_cause_str,"No RA capabilities available for this MS",128);
        }
        break;
        default:{
            strncpy(ra_cap_cause_str,"Unknown - TLLI unknown in SGSN",128);
        }
        break;
    }
    return (ra_cap_cause_str);
}



