
// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************
/*
 ****************************************************************************************
 *																						*
 *	File				: ViperChannel.h												*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |06/8/99  | DSN    | File created												*
 *		 |		   |		|															*
 *		 |		   |	    |															*
 ****************************************************************************************
 */
#ifndef VIPERCHANNEL_HDR_INC
#define VIPERCHANNEL_HDR_INC


#include "assert.h"
#include "vxworks.h"
#include "msgqlib.h"
#include "sockLib.h"
#include "inetLib.h"
#include "stdioLib.h"
#include "strLib.h"
#include "hostLib.h"
#include "ioLib.h"
#include "tasklib.h"
#include "time.h"
#include "string.h"

#define MAX_SIGNAL_PACKETS_IN_Q				300
#define VIPERCHANNEL_MAGIC_VALUE			1234
#define MAX_VIPERCHANNEL_PACKET_SIZE		1024
#define VIPERCHANNEL_WORK_PRIORITY    		50    /* priority of logging server's work task */
#define VIPERCHANNEL_STACK_SIZE       		100000  /* stack size of server's work task */
#define VIPERCHANNEL_PORT_NUMBER		 	7221


typedef struct {
	int				Magic;
	int				DataSize;
} ViperChannelPacketHeader_t;


typedef struct {
	struct sockaddr_in  	LocalAddress;
	int						SignalSocket;
	BOOL					Initialized;
} ViperChannelConfiguration_t;


/*
 * Function prototypes
 */ 
void InitializeViperChannel(BOOL SpawnTask);

VOID ViperChannel(void);

BOOL ViperChannelSendMessage(unsigned char *Packet, int DataSize, UINT32 RemoteAddress);

void RecvViperChannelMessage(unsigned char *Packet,int DataSize, UINT32 RemoteAddress);


#endif
