/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __CSU_DATA_H__
#define __CSU_DATA_H__

#include "csu_head.h"

extern T_CSU_SRC_PORT	csu_GsmSrcPort[CSU_GSM_SRC_TRX_MAX][CSU_GSM_SRC_SLOT_MAX];
extern T_CSU_SRC_PORT	csu_RtpSrcPort[CSU_RTP_SRC_PORT_MAX];
extern T_CSU_SRC_PORT	csu_AnnSrcPort[CSU_ANN_SRC_PORT_MAX];

extern T_CSU_SNK_PORT	csu_GsmSnkPort[CSU_GSM_SNK_TRX_MAX][CSU_GSM_SNK_SLOT_MAX];
extern T_CSU_SNK_PORT	csu_RtpSnkPort[CSU_RTP_SNK_PORT_MAX];


// Data definition for test function purpose
extern Uint8  csu_recdbuf[CSU_RECD_BUF_LEN];	//buffer for recording-speech-to-file purpose
extern Uint8  csu_playbuf[CSU_PLAY_BUF_LEN];	//buffer for playing-speech-from-file purpose
extern Uint32 csu_recdidx;

// Data definition of semaphore
extern SEM_ID   csu_SemaId;

// Data definition for announcement
extern Uint8  csu_annbuf_FR[CSU_ANN_SRC_PORT_MAX][CSU_PLAY_BUF_LEN]; //buffer for announcement, CSU_ANN_SRC_PORT_MAX is the same as CSU_ANN_ID_MAX
extern Uint8  csu_annbuf_EFR[CSU_ANN_SRC_PORT_MAX][CSU_PLAY_BUF_LEN]; //buffer for announcement, for support EFR codec

extern Uint32 csu_TimeStamp;

#endif /*__CSU_DATA_H__*/
