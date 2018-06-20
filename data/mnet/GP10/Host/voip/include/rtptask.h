
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: RtpTask.h														*
 *																						*
 *	Description			: Hdr file containing data structures etc for file RtpTask.cpp	*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |03/24/99 | DSN    | File created												*
 *		 |		   |		|															*
 *		 |		   |	    |															*
 ****************************************************************************************
 */
#ifndef RTPTASK_HDR_INC
#define RTPTASK_HDR_INC

#include <vxworks.h>
#include <stdlib.h>
#include <string.h>

#include <tickLib.h>
#include <drv\timer\timerDev.h>
#include <taskLib.h>

#include "voip\jetrtp.h"



typedef enum
{
   /* ?? -> RTP */
   RtpInputMessageTypeTaskShutdown,

}RtpInputMessageType;

typedef struct
{
   RtpInputMessageType Type;
/*
   union
   {
   };
*/
}RtpInputMessage_t;



/*
 * Common function prototypes
 */
void SignalRtpTaskShutdown(void);
int RtpTask(void);
void RtpEventHandler(HJCRTPSESSION RtpHandle,void *);
void RtpInputMessageHandler(RtpInputMessage_t* pMsg );
void ShutdownRtpTask( void );


#endif