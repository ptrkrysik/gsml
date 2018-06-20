
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: H323Task.H													*
 *																						*
 *	Description			: 																*
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
#ifndef H323TASK_HDR_INC
#define H323TASK_HDR_INC

#include <ci.h>
#include <cibuf.h>
#include "ccb.h"
#include "voip\voipapi.h"
#include "util.h"

#define LOCK_CCB_LIST();
#define LOCK_MCB_LIST();

#define UNLOCK_CCB_LIST();
#define UNLOCK_MCB_LIST();


int H323Task(void);
int InitializeH323Task(void);
void ShutdownH323Task(void);
HAPP InitializeStackInstance();


BOOL StackConfigSetQ931Parameters(HCFG hCfg,unsigned long ResponseTimeout,
                unsigned long ConnectTimeout,unsigned long SignallingPort);
BOOL StackConfigSetCountryCode(HCFG hCfg,unsigned long CountryCode);
BOOL StackConfigSetGateKeeperAddress(HCFG hCfg,unsigned char *ipAddress, unsigned long port);
void ShowStackConfigSettings(void);





#endif /* #ifndef H323TASK_HDR_INC */ 



