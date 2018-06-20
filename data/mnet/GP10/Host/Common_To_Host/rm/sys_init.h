/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __SYS_INIT_H__
#define __SYS_INIT_H__

#include "vxWorks.h"
#include "msgQLib.h"
#include "ril3\ril3irt.h"
#include "ril3\ril3md.h"

// Declarations for RM module
#define RM_MAX_RXMSGS						100								// Max # of messages
#define RM_MAX_RXQMSG_LENGTH				sizeof(T_CNI_RIL3MD_RRM_MSG)	// Max length from LAPDm
#define RM_MAX_TXQMSG_LENGTH				sizeof(T_CNI_RIL3MD_RRM_MSG)    // Max length from LAPDm

#define RM_L3_SUBID							0      

// Prototypes used to intialize the whole system 
void 	sys_Init							(void);
void 	sys_CreateMsgQs						(void);
void	sys_StartTasks						(void);

#endif // __SYS_INIT_H__
