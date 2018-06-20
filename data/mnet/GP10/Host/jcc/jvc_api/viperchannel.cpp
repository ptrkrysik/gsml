// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************
/*																						*
 *	File				: ViperChannel.cpp												*
 *																						*
 *	Description			: The library functions for Inter-ViperCell signalling transport*
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

/*
 * VxWorks and ANSI C Headers
 */

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

#include "Os/JCTask.h"
#include "Os/JCModule.h"
#include "GP10OsTune.h"
#include "JCC/viperchannel.h"
#include "MnetModuleId.h"

JCTask ViperChannelTaskObj("ViperChannel");


ViperChannelConfiguration_t		ViperChannelConfig;
static ipAddrBuff[INET_ADDR_LEN];

/*
 * SysCommand_VChan
 */
int SysCommand_VChan(T_SYS_CMD action)
{
	int tid;
	switch(action){
		case SYS_SHUTDOWN:
			printf("[ViperChannel] Received system shutdown notification\n");
			break;
		case SYS_START:
			printf("[ViperChannel] Received task start notification\n");
			InitializeViperChannel(TRUE);
			break;
		case SYS_REBOOT:
			printf("[ViperChannel] Reboot ready.\n");
			break;

		default:
			printf("[ViperChannel] Unknown system command received\n");
	}
	return 0;
}

/*
 * InitializeViperChannel
 */
void InitializeViperChannel(BOOL SpawnTask)
{
	memset(&ViperChannelConfig,0,sizeof(ViperChannelConfiguration_t));

	ViperChannelConfig.Initialized = FALSE;

	
	/* create client's socket */
  
	if ((ViperChannelConfig.SignalSocket = socket (AF_INET, SOCK_DGRAM, 0)) == ERROR)
	{  
		perror ("socket");
		printf("Could not create UDP socket !!\n");
		return;
	}  
  	else 
	{
		int AddressSize=0;
		AddressSize = sizeof(ViperChannelConfig.LocalAddress);

		memset(&ViperChannelConfig.LocalAddress,0,AddressSize);
		ViperChannelConfig.LocalAddress.sin_family = AF_INET;
		ViperChannelConfig.LocalAddress.sin_port = htons (VIPERCHANNEL_PORT_NUMBER);
		ViperChannelConfig.LocalAddress.sin_addr.s_addr = INADDR_ANY;

		if (bind (ViperChannelConfig.SignalSocket,(struct sockaddr *)&ViperChannelConfig.LocalAddress,AddressSize) == ERROR) {
			perror("bind");
			printf("Could not bind ViperChannel to local port!!\n");
			return;
		}
		else {
			ViperChannelConfig.Initialized = TRUE;
			if (SpawnTask) 
			{
				if(ViperChannelTaskObj.JCTaskSpawn(
							VPCHAN_TASK_PRIORITY,
							VPCHAN_TASK_OPTION,
							VPCHAN_TASK_STACK_SIZE,
							(FUNCPTR)ViperChannel,
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
							MODULE_EXT_VC, JC_CRITICAL_TASK) == ERROR)				
				{  
					printf("Unable to spawn ViperChannel task!!\n");
					return;
			}
		}


		}


	   
	}  
}



/*
 * ViperChannel
 */
VOID ViperChannel()
{
	struct sockaddr_in	FromAddress;
	int AddrSize;
	static unsigned char Buffer[8000];
	int Status;

	if (!ViperChannelConfig.Initialized) {
		printf("Unable to start ViperChannel task since it is not initialized properly\n");
		return;
	}

	ViperChannelTaskObj.JCTaskEnterLoop();
	while(TRUE) {
		AddrSize = sizeof(FromAddress);
		memset(&FromAddress,0,AddrSize);
		memset(Buffer,0,8000);
		Status = recvfrom
	    (
	    	ViperChannelConfig.SignalSocket,
	    	(char *)Buffer,
	    	8000,
	    	0,
	    	(struct sockaddr *)&FromAddress,
	    	&AddrSize
	    );
		if (Status == ERROR) {
			printf("Error receiving packet on ViperChannel!!\n");
			return;
		}
		else {
			ViperChannelPacketHeader_t *PacketHeader;
			PacketHeader = (ViperChannelPacketHeader_t *)Buffer;
			if ( (PacketHeader->Magic != VIPERCHANNEL_MAGIC_VALUE) ||
				 (Status < (PacketHeader->DataSize+ sizeof(ViperChannelPacketHeader_t))) )
			 {
				printf("Invalid packet received on ViperChannel");
			}
			else {

#ifdef DEBUG
				inet_ntoa_b(FromAddress.sin_addr,ipAddrBuff);
				printf("Received ViperChannel message from ip %s of length %d bytes\n",ipAddrBuff,PacketHeader->DataSize);
#endif
					RecvViperChannelMessage(&Buffer[sizeof(ViperChannelPacketHeader_t)],
											PacketHeader->DataSize,(UINT32)FromAddress.sin_addr.s_addr);
			}

		}
	}
    ViperChannelTaskObj.JCTaskNormExit();
}





/*
 * ViperChannelSendMessage
 */
BOOL
ViperChannelSendMessage(unsigned char *Packet, int DataSize, UINT32 RemoteAddress)
{
	int Status;
	struct sockaddr_in DestAddress;
	static char Buffer[8000];
	ViperChannelPacketHeader_t *PacketHeader;
	int AddrSize,TotalLen;
	BOOL RetStatus;

	memset(Buffer,0,8000);

	PacketHeader = (ViperChannelPacketHeader_t *)Buffer;
	PacketHeader->Magic = VIPERCHANNEL_MAGIC_VALUE;
	PacketHeader->DataSize = DataSize;

	AddrSize = sizeof(struct sockaddr_in);

	memset(&DestAddress,0,AddrSize);

	DestAddress.sin_family = AF_INET;
	DestAddress.sin_port = htons(VIPERCHANNEL_PORT_NUMBER);
	DestAddress.sin_addr.s_addr = RemoteAddress;

	memcpy(&Buffer[sizeof(ViperChannelPacketHeader_t)],Packet,DataSize);

	TotalLen = sizeof(ViperChannelPacketHeader_t) + DataSize;
#ifdef DEBUG
	inet_ntoa_b(DestAddress.sin_addr,ipAddrBuff);
	printf("Sending ViperChannel message to ip %s of length %d bytes\n",ipAddrBuff,PacketHeader->DataSize);
#endif

	Status = sendto(ViperChannelConfig.SignalSocket,(char *)PacketHeader,TotalLen,0,(struct sockaddr *)&DestAddress,AddrSize);
	if (Status != ERROR) {
		RetStatus = TRUE;
	}
	else {
		RetStatus =  FALSE;
	}
	return(RetStatus);
}








