/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef __CSU_GLOB_CPP__
#define __CSU_GLOB_CPP__

#include "CsuNew\csu_head.h"

//Declaration of trunk source ports
T_CSU_SRC_PORT	csu_GsmSrcPort[CSU_GSM_SRC_TRX_MAX][CSU_GSM_SRC_SLOT_MAX];
T_CSU_SRC_PORT	csu_RtpSrcPort[CSU_RTP_SRC_PORT_MAX];
T_CSU_SRC_PORT	csu_AnnSrcPort[CSU_ANN_SRC_PORT_MAX];

//Declaration of trunk sink ports
T_CSU_SNK_PORT	csu_GsmSnkPort[CSU_GSM_SNK_TRX_MAX][CSU_GSM_SNK_SLOT_MAX];
T_CSU_SNK_PORT	csu_RtpSnkPort[CSU_RTP_SNK_PORT_MAX];
//Declaration of a M-semaphore
SEM_ID   csu_SemaId;

// Data definition for announcement
Uint8  csu_annbuf_FR[CSU_ANN_SRC_PORT_MAX][CSU_PLAY_BUF_LEN];
Uint8  csu_annbuf_EFR[CSU_ANN_SRC_PORT_MAX][CSU_PLAY_BUF_LEN];

// Use global time stamp to provide precise time stamp for all RTP packets
Uint32 csu_TimeStamp = 0;

#endif /*__CSU_GLOB_CPP__*/
