/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef __CSU_TABLE_H__
#define __CSU_TABLE_H__

#include "csu_head.h"

typedef struct {
	Uint16	pidx;
	Uint8		type;
	Uint8		ldty;       // Payload type to this peer
	Uint8		term;		// Term type of connection peer 
      Uint8  	mark;       // Indication of a speech start
      Uint8 	head;       // Head pointer of jitter buffer
	Uint8 	rear;       // Rear pointer of jitter buffer
	Uint8  	seqc;       // Out of sequence counting    
	Uint16 	seqn;       
	Uint32 	rtim;
	Uint32 	ltim;
	Uint32 	tick;       // Representing sampling rate
	Uint32 	dlNg;	      // For downlink watch purpose
	Uint32 	dlNr;	      // For uplink watch purpose
    	Uint8  	jitc;
	Uint8       start;
	Uint8       jbuf[CSU_JBUF_LEN]; 
	Uint32	ppos;   //Current pos of embedded announcement to play
	T_CNI_RIL3_SPEECH_VERSION speech_version;
} T_CSU_SRC_PEER;

typedef struct {
      Uint16      pidx;
	Uint8       type;
	Uint8		ldty;       // Payload type to this peer
	Uint8		term;		// Term type of connection peer 
      Uint8  	mark;       // Indication of a speech start
      Uint8 	head;       // Head pointer of jitter buffer
	Uint8 	rear;       // Rear pointer of jitter buffer
	Uint8  	seqc;       // Out of sequence counting    
	Uint16 	seqn;       
	Uint32 	rtim;
	Uint32 	ltim;
	Uint32 	tick;       // Representing sampling rate
	Uint32 	dlNg;	      // For downlink watch purpose
	Uint32 	dlNr;	      // For uplink watch purpose
	Uint8  	jitc;
	Uint8       start;
	Uint8       jbuf[CSU_JBUF_LEN]; 
} T_CSU_SNK_PEER;

typedef struct {
	Uint8		state;
      Uint8		resvd;
	Uint8		count;
      Uint8 	type;
      Uint8       limit;
      Uint8		test;
      Uint8		prnt;
	Uint16  	seqc;   // Out of sequence counting    
	Uint16 	seqn;       
      Uint32	wcnt;
      Uint32	ppos;   //Current pos of recorded speech to play
      Uint32	lpts;	  //loopback time stamp used in rtpParam
      Uint32	nopr;
	  Uint32	alen;
      union  {
         	      T_CNI_IRT_ID   gsmHandler;
                  HJCRTPSESSION  rtpHandler;
		      T_CSU_ANN_ID   annHandler;  //Annoucement port handler
	};
      T_CNI_IRT_ID    entryId;  //For housekeeping purpose only right now
	T_CSU_SRC_PEER  peer[CSU_SRC_CONN_MAX];
	
} T_CSU_SRC_PORT;

typedef struct {
	Uint8		state;
      Uint8		resvd;
	Uint8		count;
      Uint8 	type;
      Uint8       limit;
      Uint8		test;
      Uint8		prnt;
	Uint16  	seqc;       // Out of sequence counting    
	Uint16 	seqn;       
      Uint32	wcnt;
      Uint32	ppos;		  //Current pos of recorded speech to play
      Uint32	lpts;		  //loopback time stamp used in rtpParam
      Uint32	nopr;

      union  {
         	      T_CNI_IRT_ID   gsmHandler;
                  HJCRTPSESSION  rtpHandler;
		      T_CSU_ANN_ID   annHandler;  //Annoucement port handler
      };
      T_CNI_IRT_ID    entryId;  //For housekeeping purpose only right now
      T_CSU_SNK_PEER  peer[CSU_SNK_CONN_MAX];
	   T_CNI_RIL3_SPEECH_VERSION speech_version;

} T_CSU_SNK_PORT;

#endif /*__CSU_TABLE_CPP__*/
