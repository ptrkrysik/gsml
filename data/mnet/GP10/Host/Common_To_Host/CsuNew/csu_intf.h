/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __CSU_INTF_H__
#define __CSU_INTF_H__

#include "csu_const.h"
#include "csu_types.h"
#include "csu_jetrtp.h"


#define CSU_CONN_MAX		3

//Announcemnt port ID,expected to change later
typedef unsigned short T_CSU_ANN_ID;

//definition of trunk type
typedef enum {
        CSU_GSM_PORT = 10,	//GSM
        CSU_RTP_PORT = 20,    //RTP
        CSU_ANN_PORT = 30,    //announcement
        CSU_NIL_PORT = 255,
} T_CSU_PORT_TYPE;

//definition of port ID
typedef struct {
        T_CSU_PORT_TYPE         portType;
        union  {
           	     T_CNI_IRT_ID   gsmHandler;		//ID of GSM port
                 HJCRTPSESSION  rtpHandler;		//ID of RTP port
		     T_CSU_ANN_ID   annHandler; 		//ID of ANN port
        } portId;
		T_CNI_RIL3_SPEECH_VERSION speechVersion;
} T_CSU_PORT_ID;

//definition of port ID list
typedef struct {
        Uint8 		count;
        T_CSU_PORT_ID   port[CSU_CONN_MAX];
} T_CSU_PORT_LIST;

// Return codes for register/unregister functions
// related to H323/RTP tasks
typedef enum {
	  CSU_RESULT_REG_SUCCESS=0,
	  CSU_RESULT_REG_PORT_ALREADY_REGISTERED,
        CSU_RESULT_REG_PORT_REGISTER_FAILED,
	  CSU_RESULT_REG_PORT_NOT_REGISTERED,
	  CSU_RESULT_REG_PORT_UNREGISTER_PENDING,
        CSU_RESULT_REG_PORT_UNREGISTER_FAILED,
	  CSU_RESULT_REG_PORT_TYPE_UNSUPPORTED,
	  CSU_RESULT_REG_PORT_REGISTER_OVERFLOW
} T_CSU_RESULT_REG;

// Defs of annoucement Identifiers
typedef enum {
	  CSU_ANN_RINGBACK=0
} T_CSU_ANN_REG;


// Return codes for connection-establishment functions
// related to CC/connection functionality
typedef enum {
        //Common for all successful cases
	  CSU_RESULT_CON_SUCCESS=0,

        //Return codes from connection functions
	  CSU_RESULT_CON_SRC_PORT_NOT_READY,
        CSU_RESULT_CON_SNK_PORT_NOT_READY,
        CSU_RESULT_CON_SRC_PORT_OVERFLOW,
        CSU_RESULT_CON_SNK_PORT_OVERFLOW,

        //Return codes from break functions
        CSU_RESULT_CON_SRC_PORT_NOT_ENGAGED,
        CSU_RESULT_CON_SNK_PORT_NOT_ENGAGED,
          
        //Return codes from inquiry function
        CSU_RESULT_CON_FAILED

} T_CSU_RESULT_CON;

//
//CSU functional calls for use by CC/connection entity

//Request for establishing a simplex connction from source port to sink port
T_CSU_RESULT_CON csu_SimplexConnect(T_CSU_PORT_ID *source, T_CSU_PORT_ID *sink);

//Request for establish a duplex connction between source port and sink port
T_CSU_RESULT_CON csu_DuplexConnect(T_CSU_PORT_ID *source, T_CSU_PORT_ID *sink);

//Request for breaking a simplex connection from source port to sink port
T_CSU_RESULT_CON csu_SimplexBreak(T_CSU_PORT_ID *source, T_CSU_PORT_ID *sink);

//Request for breaking a duplex connection between source port and sink port
T_CSU_RESULT_CON csu_DuplexBreak(T_CSU_PORT_ID *source, T_CSU_PORT_ID *sink);

//Request for breaking all the connections Originating from the given port
T_CSU_RESULT_CON csu_OrigBreakAll(T_CSU_PORT_ID *source);

//Request for breaking all the connections terminated at the given port
T_CSU_RESULT_CON csu_TermBreakAll(T_CSU_PORT_ID *sink);

//Request for breaking all the connections either originating from or terminated
//at the given port
T_CSU_RESULT_CON csu_OrigTermBreakAll(T_CSU_PORT_ID *port);

//Return all the ports involved in connections with source as originating side. count
//containing the total number of ports involved and sink pointing to these ports.
T_CSU_RESULT_CON csu_SrcStatus(T_CSU_PORT_ID *source, T_CSU_PORT_LIST *sink);

//Return all the ports involved in connections with sink as terminated side. count
//containing the total number of ports involved and source pointing to these ports.
T_CSU_RESULT_CON csu_SnkStatus(T_CSU_PORT_ID *sink, T_CSU_PORT_LIST *source);

//Return all the ports involved in connections with port as originating or terminated sides,
//count containing the total number of ports involved and source pointing to these ports.
T_CSU_RESULT_CON csu_AllStatus(T_CSU_PORT_ID *port, T_CSU_PORT_LIST *source, T_CSU_PORT_LIST *sink);

//
//CSU functional calls for use by H323/RTP task
//
//Register port as src port in future connection
T_CSU_RESULT_REG csu_RegSrcPort(T_CSU_PORT_ID *port);

//Register port as snk port in future connection
T_CSU_RESULT_REG csu_RegSnkPort(T_CSU_PORT_ID *port);

//Register port as both src and snk ports in future connection
T_CSU_RESULT_REG csu_RegSrcSnkPort(T_CSU_PORT_ID *port);

//Unregister port's src port registration 
T_CSU_RESULT_REG csu_UnregSrcPort(T_CSU_PORT_ID *port);

//Unregister port's snk port registration
T_CSU_RESULT_REG csu_UnregSnkPort(T_CSU_PORT_ID *port);

//Unregister port's both src and snk ports registration
T_CSU_RESULT_REG csu_UnregSrcSnkPort(T_CSU_PORT_ID *port);

//CSU eventhandler provided for H323/RTP task's calling
//void H323RTPEventHandler(HRTPSESSION hrtp, H323CALL hCall, T_CNI_IRT_ID entryId);
RTP_READ_STATUS RTPEventHandler( HJCRTPSESSION RtpHandle, VOIP_CALL_HANDLE VoipCallHandle, TXN_ID TxnId);
void RTPFastReadHandler(HJCRTPSESSION RtpHandle,unsigned char *newPacket,
                              int packetLength,rtpParam RtpParams,VOIP_CALL_HANDLE CallHandle,T_CNI_IRT_ID MobileHandle);

//
//CSU functional call provided for L1PROXY's calling
//
Int32 procUlTchFrame(Uint8 *buf);

//
//CSU functional call provided to RootTask
//
void csu_TblInitAll(void); 

//
//CSU functional calls which might be invoked from OAM
//

//Show GSM-orig connections
void csu_ShowGsmSrcTable(void);

//Show RTP-orig connections
void csu_ShowRtpSrcTable(void);

//Show ANN-orig connections
void csu_ShowAnnSrcTable(void);

//Show all-orig connections
void csu_ShowAllSrcTable(void);

//Show GSM-term connections
void csu_ShowGsmSnkTable(void);

//Show RTP-term connections
void csu_ShowRtpSnkTable(void);

//Show all-term connections
void csu_ShowAllSnkTable(void);

//Show all orig/term connections
void csu_ShowAllTable(void);

//Clear all orig/term connections
void csu_ClearAllTable(void);

//Manually register given port as src port
void csu_ManRegSrcPort(Uint8 type, Uint32 handler);

//Manually register given port as snk port
void csu_ManRegSnkPort(Uint8 type, Uint32 handler);

//Manually register given port as both src and snk port
void csu_ManRegSrcSnkPort(Uint8 type, Uint32 handler);

//Manually connect from src port to snk port uni-directionally
void csu_ManUniConnect(Uint8 s_type, Uint32 s_handler, Uint8 d_type, Uint32 d_handler);

//Manually connect src port and snk port bi-directionally
void csu_ManBiConnect(Uint8 s_type, Uint32 s_handler, Uint8 d_type, Uint32 d_handler);

//Manually break connection from src port to snk port uni-directionally
void csu_ManUniBreak(Uint8 s_type, Uint32 s_handler, Uint8 d_type, Uint32 d_handler);

//Manually break connection between src port and snk port bi-directionally
void csu_ManBiBreak(Uint8 s_type, Uint32 s_handler, Uint8 d_type, Uint32 d_handler);

//Manually break all connections originating from given port
void  csu_ManOrigBreakAll(Uint8 type, Uint32 handler);

//Manually break all connections terminated at given port
void  csu_ManTermBreakAll(Uint8 type, Uint32 handler);

//Manually break all connections either originating from or terminated at given port
void  csu_ManOrigTermBreakAll(Uint8 type, Uint32 handler);

//Start record speech to a given snk port identified by (type, handler)
void csu_StartRecordSpeech(Uint8 type, Uint32 handler);

//Stop record speech to a given snk port identified by (type, handler)
void csu_StopRecordSpeech(Uint8 type, Uint32 handler);

//Start play speech to a given snk port identified by (type, handler)
void csu_StartPlaySpeech(Uint8 type, Uint32 handler);

//Stop play speech to a given snk port identified by (type, handler)
void csu_StopPlaySpeech(Uint8 type, Uint32 handler);

//Start loopback voice from a given src port to itself
void csu_StartLoopback(Uint8 type, Uint32 handler);

//Stop loopback voice from a given src port to itself
void csu_StopLoopback(Uint8 type, Uint32 handler);

//Write speech recorded to a file 
void  csu_WriteSpeechToFile(void);

//Start showing occurrence of writing to a given sink port
void csu_ShowWriteToSnk(Uint8 type, Uint32 handler);

//Clear showing occurrence of writing to a given sink port
void csu_ClrShowWriteToSnk(Uint8 type, Uint32 handler);

//Start showing occurrence of receiving to a given source port
void csu_ShowReceiveFromSrc(Uint8 type, Uint32 handler);

//Clear showing occurrence of receiving to a given source port
void csu_ClrShowReceiveFromSrc(Uint8 type, Uint32 handler);

#endif /* __CSU_INTF_H__ */
