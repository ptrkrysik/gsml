
// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : SubscGPRSInfoType.h
// Author(s)   : Kevin Lim
// Create Date : 12-12-00
// Description : message and ie interface among GUDB and other modules 
//
// *******************************************************************

#ifndef SUBSC_GPRS_INFO_TYPE_H
#define SUBSC_GPRS_INFO_TYPE_H

#include "ril3/ril3_sm_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define		MAX_NUM_SUBSC_GPRS_DATA			16		// was 50
#define		MAX_LEN_SUBSC_PDP_ADDR			16
#define		MAX_LEN_SUBSC_APN				CNI_RIL3_MAX_ACCESS_POINT_NAME_LENGTH

typedef struct {
	CNI_RIL3_QOS_DELAY_CLASS			delay_class;
	CNI_RIL3_QOS_RELIABILITY_CLASS		reliability_class;
	CNI_RIL3_QOS_PRECEDENCE_CLASS		precedence_class;
	CNI_RIL3_QOS_PEAK_THROUGHPUT		peak_throughput;
	CNI_RIL3_QOS_MEAN_THROUGHPUT		mean_throughput;
} T_SUBSC_QOS_DATA;

typedef struct {
	unsigned char						pdpContextId;
	CNI_RIL3_PDP_ADDRESS_TYPE			pdpType;
	unsigned char						pdpAddress[MAX_LEN_SUBSC_PDP_ADDR];
	T_SUBSC_QOS_DATA					qosSubscribed;
	unsigned char						vplmnAddressAllowed;
	unsigned char						apn_length;
	unsigned char						apn[MAX_LEN_SUBSC_APN];
	// extension
} T_SUBSC_GPRS_DATA;
 
typedef struct {
//	T_SUBSC_INFO_IE_TYPE				ie_type;	// only in encoded message
	short								num_data;
	T_SUBSC_GPRS_DATA					data[MAX_NUM_SUBSC_GPRS_DATA];
} T_SUBSC_IE_GPRS_DATA_INFO;	



#ifdef __cplusplus
}
#endif

#endif // SUBSC_GPRS_INFO_TYPE_H





