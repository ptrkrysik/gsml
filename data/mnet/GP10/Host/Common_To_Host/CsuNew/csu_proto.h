/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __CSU_PROTO_H__
#define __CSU_PROTO_H__

#include "csu_head.h"

// Functions handling voice passing
T_CSU_RESULT csu_TblGetSnk(T_CSU_PORT_ID *src,Uint8 *test,Uint8 *count,T_CSU_SRC_PEER **peer);
T_CSU_RESULT csu_PassVoice(T_CSU_SRC_PEER *peer, Uint8 *buf, Uint16 seqNumber, Uint8 cd);
T_CSU_RESULT csu_TblGetRtpParam(T_CSU_SRC_PEER *peer,rtpParam *param, Uint8 cd);

// Callback function provided by L1PROXY to CSU
void sendDsp( unsigned char *buffer, int len );

// Functions defined in RM for conversion between entryId and gsmHandler
Bool rm_EntryIdToTrxSlot(T_CNI_IRT_ID entryId, Uint16 *gsmHandler);
Bool rm_TrxSlotToEntryId(Uint16 gsmHandler, T_CNI_IRT_ID *entryId);
Bool rm_TrxIsNotBlocked(Uint8 trx);
Bool rm_CodecMatch(Uint16 gsmHandler, Uint8 codec);

// Functions initializing connection tables
void csu_TblInitGsmSrcPort(void);
void csu_TblInitRtpSrcPort(void);
void csu_TblInitGsmSnkPort(void);
void csu_TblInitRtpSnkPort(void);
void csu_TblInitAnnSrcPort(void);
void csu_TblInitAll(void);

// Functions handling connection managment
T_CSU_RESULT csu_AddToSrcPortList(T_CSU_PORT_LIST *src, Uint8 type, Uint16 pidx);
T_CSU_RESULT csu_AddToSnkPortList(T_CSU_PORT_LIST *snk, Uint8 type, Uint16 pidx);
T_CSU_RESULT csu_FindSrcPortPIdx(T_CSU_PORT_ID *src, Uint8 *type, Uint16 *pidx);
T_CSU_RESULT csu_FindSnkPortPIdx(T_CSU_PORT_ID *snk, Uint8 *type, Uint16 *pidx);
T_CSU_RESULT csu_CheckSrcCapable(Uint8 s_type,Uint16 s_pidx, Uint8 d_type, Uint16 d_pidx);
T_CSU_RESULT csu_CheckSnkCapable(Uint8 d_type,Uint16 d_pidx, Uint8 s_type, Uint16 s_pidx);
T_CSU_RESULT csu_ConnSrcPort(Uint8 s_type, Uint16 s_pidx, Uint8 d_type, Uint16 d_pidx, T_CSU_PORT_ID *port_id);
T_CSU_RESULT csu_ConnSnkPort(Uint8 d_type, Uint16 d_pidx, Uint8 s_type, Uint16 s_pidx, T_CSU_PORT_ID *port_id);
T_CSU_RESULT csu_CheckSrcConn(Uint8 s_type, Uint16 s_pidx, Uint8 d_type, Uint16 d_pidx);
T_CSU_RESULT csu_CheckSnkConn(Uint8 d_type, Uint16 d_pidx, Uint8 s_type, Uint16 s_pidx);
T_CSU_RESULT csu_BreakSrcConn(Uint8 s_type, Uint16 s_pidx, Uint8 d_type, Uint16 d_pidx);
T_CSU_RESULT csu_BreakSnkConn(Uint8 d_type, Uint16 d_pidx, Uint8 s_type, Uint16 s_pidx);

// Functions handling port registration
T_CSU_RESULT csu_DoSrcPortReg(T_CSU_SRC_PORT *src);
T_CSU_RESULT csu_DoSnkPortReg(T_CSU_SNK_PORT *snk);
T_CSU_RESULT csu_DoSrcPortUnreg(T_CSU_SRC_PORT *src);
T_CSU_RESULT csu_DoSnkPortUnreg(T_CSU_SNK_PORT *snk);
T_CSU_RESULT_REG csu_RegAllGsmSrcPort(void);
T_CSU_RESULT_REG csu_RegAllGsmSnkPort(void);
T_CSU_RESULT_REG csu_RegAllAnnSrcPort(void);


// Functions handling connection tables
void csu_TblInitGsmSrcPort(void);
void csu_TblInitRtpSrcPort(void);
void csu_TblInitGsmSnkPort(void);
void csu_TblInitRtpSnkPort(void);
void csu_TblInitAnnSrcPort(void);

// Functions interfacing with old-csu environment for new csu moudule test purpose
void  csu_Init(void);
Int32 setRtpTchConnection( Uint8 trx, Uint8 slot, HRTPSESSION rtpHandler );
Int32 clrConnectionByTrxSlot( Uint8 trx, Uint8 slot );
void  csu_ReadSpeechFromFile(int);
void  csu_RecdSpeechToBuffer(Uint8 *buf);
void  csu_LoopbackSpeechToSrcSelf(T_CSU_PORT_ID *src, Uint8 *buf);

// Functions handling semaphore
void csu_CreateSemaphore(void);
void csu_TakeSemaphore(void);
void csu_GiveSemaphore(void);

#endif /*__CSU_PROTO_H__*/
