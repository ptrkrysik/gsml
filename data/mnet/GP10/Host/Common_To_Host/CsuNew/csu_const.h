    /*
    *******************************************************************
    **
    ** (c) Copyright Cisco 2000
    ** All Rights Reserved
    **
    ******************************************************************
    */
    #ifndef __CSU_CONST_H__
    #define __CSU_CONST_H__
    
    //General definition of boolean return codes
    typedef enum {
    	  CSU_RESULT_FAILED=0,
    	  CSU_RESULT_SUCCESS,
            CSU_RESULT_NOT_RESSTATE_USED,
            CSU_RESULT_RTP_HANDLER_NOT_FOUND,
            CSU_RESULT_INVALID_PORT_TYPE,
			CSU_RESULT_SRC_PORT_CONNECTED_AS_EXPECTED,
		    CSU_RESULT_SNK_PORT_CONNECTED_AS_EXPECTED
    } T_CSU_RESULT;
    
    //State definition of either src or snk port
    typedef enum {
    	  CSU_RESSTATE_NULL=0,
    	  CSU_RESSTATE_FREE,
    	  CSU_RESSTATE_USED,
            CSU_RESSTATE_IGNORE,
    } T_CSU_RESSTATE;
    
    //define _EMBEDDED_FILE , which is in "/ata/vipercall/scripts"
    static char * EMBEDDED_FILE[]={
  	  "/speech.gsm",
  	  "/emf30.gsm"
    };
  
    //Size definintion of one announcement period
    #define ANN_PERIOD	33 //33 buffer
  
    //1500=6000/4 ie 6+ seconds elapsed
    #define CSU_TRAFFIC_FLOW_6_SEC_COUNT  500 
    
    //Size definition of connections per port
    #define CSU_SRC_CONN_MAX		3
    #define CSU_SNK_CONN_MAX		3
    
    //Size definition of source ports
    #define CSU_GSM_SRC_TRX_MAX		2
    #define CSU_GSM_SRC_SLOT_MAX	8
    #define CSU_RTP_SRC_PORT_MAX      	45
    //Add definition for ANN here
    #define CSU_ANN_SRC_PORT_MAX      	2
    
    //Size definition of sink ports 
    #define CSU_GSM_SNK_TRX_MAX		2
    #define CSU_GSM_SNK_SLOT_MAX		8
    #define CSU_RTP_SNK_PORT_MAX		45
    //Add definition for ANN here
    
    //Size definition of peer ports of either src or snk ports
    #define CSU_PEER_PORT_MAX		3    //N in 1->N conn.
    #define CSU_PEER_PORT_NULL	  	0xFF  //no peer exists
    
    //Interface definition between CSU and DSP
    #define CSU_GSM_SPEECH_MSG_MD	      0x04
    #define CSU_GSM_SPEECH_DLMSG_MSB    0x30
    #define CSU_GSM_SPEECH_DLMSG_LSB    0x01
    #define CSU_GSM_SPEECH_ULMSG_MSB    0x30
    #define CSU_GSM_SPEECH_ULMSG_LSB    0x00
    #define CSU_GSM_SPEECH_MSG_LEN      45
    #define CSU_GSM_SPEECH_DAT_POS	12    //Speech start 
    #define CSU_GSM_SPEECH_DAT_LEN	33    //Speech length
    #define CSU_GSM_SPEECH_MSG_TRX_POS  7
    #define CSU_GSM_SPEECH_MSG_SLOT_POS 8
    #define CSU_GSM_BM_ACCH_CBITS		8
    
    //Defintion of RTP GSM speech frame description
    //#define CSU_RTP_SPEECH_DAT_PAYLOAD	  0x60  //Used for GSM
    #define CSU_RTP_SPEECH_DAT_PAYLOAD	3     //Used for CSCO test use
    #define CSU_RTP_SPEECH_DAT_PAYLOADefr 20
    #define CSU_RTP_SPEECH_DAT_LENefr     32  //In fact, it's 31
    #define CSU_RTP_SPEECH_DAT_PTSBYTE	12    //Speech start
    #define CSU_RTP_SPEECH_DAT_RAT	160   //Sampling rate
    #define CSU_RTP_SPEECH_DAT_POS	12    //Speech start 
    #define CSU_RTP_SPEECH_DAT_LEN	33    //Speech length
    #define CSU_RTP_SPEECH_MSG_LEN      45
    #define CSU_RTP_SPEECH_MSG_LENefr      44 //for efr
  
     //Definition of REC speech framce description
    #define CSU_REC_SPEECH_DAT_LEN	33    //!!! =  RTP/GSM speech data len
    
    //Definition of jitter buffer control parameters
    #define CSU_JBUF_LEN			330   //330 bytes for jitter buffering
    
    //Definition of relevant max capacity
    #define CSU_MAX_SPEECH_MSG_LEN      45
    #define CSU_MAX_SPEECH_DAT_LEN      33
    
    //Definition of test control selections 
    #define CSU_RECD_BUF_LEN				27000	 //800 speech frames,16s
    #define CSU_PLAY_BUF_LEN				27000	 //800 speech frames,16s
    #define CSU_TEST_FUNC_NULL				0
    #define CSU_TEST_PLAY_SPEECH_FROM_FILE_TO_SNK	1
    #define CSU_TEST_RECORD_SRC_SPEECH_TO_FILE	2
    #define CSU_TEST_LOOPBACK_SPEECH_TO_SRC_SELF  	3
    #define CSU_TEST_PRINT_SNK_HANDLER_USED		4
    #define CSU_TEST_PRINT_SRC_HANDLER_USED		5
    #endif /* __CSU_CONST_H__ */
    
