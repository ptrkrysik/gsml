/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000-2001                                     */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* Version     : 1.0                                                 */
/* Status      : Under development                                   */
/* File        : snmp_trap.h                                         */
/* Author(s)   : Bhawani Sapkota                                     */
/* Create Date : 02/05/01                                            */
/* Description : This file contains routines to send SNMP enterprise */
/*               trap to multiple destinations.                      */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY                                                 */                           
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |02/05/01| Initial Draft                                 */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/


/*******************************************************************************
** Include Files
*******************************************************************************/


#include <vxWorks.h>
#include <sys/socket.h>
#include <snmpdLib.h>
#include <snmpIoLib.h>
#include <hostLib.h>
#include <inetLib.h>

#include <symLib.h>
#include <sysSymTbl.h>
#include <loadLib.h>
#include <usrLib.h>
#include <stdio.h>
#include <taskLib.h>

#include "oam_api.h"

/* Allocation default severity of AlarmCode space per module  */
#define ALARM_CRITICAL_START_POINT  0x0001
#define ALARM_MAJOR_START_POINT     0x0100    
#define ALARM_MINOR_START_POINT     0x0800
#define ALARM_INFO_START_POINT      0x1000

#define ALARM_SEV_CRITICAL     1 
#define ALARM_SEV_MAJOR        2
#define ALARM_SEV_MINOR        3 
#define ALARM_SEV_INFORMATION  4    

#define MAX_LINE 512
#define CLEAN_MSG 40

struct alarmDetail_t
{
	int code;
	char *synopsis;
	char *detail;
	struct alarmDetail_t *next;
};

typedef  struct alarmDetail_t AlarmEntry;

AlarmEntry *AlarmListEntryHead = NULL;
/*-------------------------------------------------------------------------------*/
AlarmEntry *
ReadAlarmFile()
{
	FILE *fp;
	char buffer[MAX_LINE];
	char *subject1=NULL,*subject2=NULL;
	AlarmEntry *head=NULL, *node=NULL, *tmpNode;
	int len,i=0,j,slen;


	memset(buffer, 0, MAX_LINE);
	strcpy(buffer, getenv( "MNET_BASE" ));
	strcat(buffer,"/Alarm/AlarmCode.English");
	
	fp = fopen(buffer, "r");
	if (fp == NULL)
	{
		printf("Cannot open Alarm Code file: %s\n", buffer);
		perror("Reason:");
		return NULL;
	}
	
   while(fgets(buffer, MAX_LINE, fp) != NULL)
   {
      /* skip line starting with / or # as comments */
      if (buffer[0] =='/' || buffer[0] == '#')
         continue;
      
      tmpNode = (AlarmEntry *) malloc(sizeof(AlarmEntry));
      if (tmpNode == NULL)
      {
         printf("tmpNode Memory Allocaton Failure");
         break;
      }
		
		if (head == NULL)
		{
			head = node = tmpNode;
		} else {
			node->next = tmpNode;
			node = node->next;
		}
		
		sscanf(buffer, "%d", &node->code);

		subject1 = strstr(buffer, "|");
		subject1++; i=0;
		while(subject1[i++] != '|');

		subject2 = strstr(subject1, "|");
		subject2++;
		
		node->synopsis = malloc(i);
		memset(node->synopsis,0,i);
		if(node->synopsis == NULL)
		{
			printf("synopsis : Memory Allocaton(%d) Failure\n%s\n",i,subject1);
			break;
		}
		for(j=0;j<i - 1 ;j++)
			node->synopsis[j]=subject1[j];

		 

		/* remove the end of line */
		slen = strlen(subject2);
		node->detail = malloc(slen+1);
		memset(node->detail,0,slen+1);
		if(node->detail == NULL)
		{
			printf("detail : Memory Allocaton(%d) Failure\n%s\n",slen,subject2);
			break;
		}
		for(j=0;j<slen - 1 ;j++)
			node->detail[j]=subject2[j];
	}
	
	fclose(fp);
	if (node != NULL)
	{
		node->next = NULL;
	}
	return head;
}


char *trapAlarmDetail(int errorcode)
{

	AlarmEntry  *tmpNode;
	char *noDetail="Undifined alarm code"; 
	
	if (AlarmListEntryHead == NULL)
		AlarmListEntryHead = ReadAlarmFile();
	
	tmpNode = AlarmListEntryHead;

	while( tmpNode && tmpNode->code != errorcode)
	{
		tmpNode = tmpNode->next;
	}

	if ( tmpNode != NULL){
		 return tmpNode->synopsis;
	}
	return noDetail;
}

/*******************************************************************************
* 
* Snmp Trap Callback Var Bind Data
*/

typedef struct SnmpTrapCallbackData_
{
	MibTag		mibTag;
	int			snmpVersion;
}SnmpTrapCallbackData;

static INT_32_T getAlarmSev(INT_32_T errorCode)
{
	INT_32_T alarmCode;

	alarmCode=(errorCode&0xFFFF);

	if(alarmCode >= ALARM_INFO_START_POINT) 
		return ALARM_SEV_INFORMATION;
	if(alarmCode >= ALARM_MINOR_START_POINT)
		return ALARM_SEV_MINOR;
	if(alarmCode >= ALARM_MAJOR_START_POINT)
		return ALARM_SEV_MAJOR;

	return ALARM_SEV_CRITICAL;
}

/*******************************************************************************
* 
* trapBindVals - bind values in the trap
*
* This routine binds snmpv2 format variable to Snmp packet.
*
* RETURNS: OK, or ERROR if the bind to the SNMP packet fails
*/



int snmp_trapBindVals
(
 SNMP_PKT_T *    pkt,    /* internal representation of SNMP packet */ 
 void *          vals    /* null cookie */
 )
{
	static OIDC_T   sysUpTimeOid [] = {1, 3, 6, 1, 2, 1, 1, 3, 0};
	static OIDC_T   snmpTrapOID [] =  {1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0};

	INT_32_T  errorCode;
	int i = 0;
	int mid,severity;
	char *alarmDetail;
	char *moduleName;
	char *alarmClearMsg="Module alarm has cleared.";

	extern OIDC_T   sysTrapOid [];
	extern OIDC_T   snmpTrapEventOid[];
	extern OIDC_T   errorCodeOamOid[];
	
	extern OIDC_T   trapAlarmModuleNameOid[];
	extern OIDC_T   trapAlarmCodeOid[];
	extern OIDC_T   trapAlarmDescOid[];
	
	extern int SIZE_sysTrapOid;
	extern int SIZE_snmpTrapEventOid;
	extern int SIZE_errorCodeOamOid;
	
	extern int SIZE_trapAlarmModuleNameOid;
	extern int SIZE_trapAlarmCodeOid;
	extern int SIZE_trapAlarmDescOid;
 

	SnmpTrapCallbackData *pSnmpTrapCallbackData =  ( SnmpTrapCallbackData*) vals;
	MibTag tag = pSnmpTrapCallbackData->mibTag;
	int    version = pSnmpTrapCallbackData->snmpVersion;
	
	int itm = MIB_ITM(tag);
	
	static char fname[] = "snmp_trapBindVals";
	
	LOG_TRACE(("Processing Trap Bind Vals for tag %s", snmp_tagName(tag)));
	
	/* Release the Trap data first */
	free (pSnmpTrapCallbackData);
	
	/* First get the value of the error code */
	errorCode = snmp_getValue(tag);
	moduleName=GetMnetModuleName(itm - 1);

	if(errorCode){
		alarmDetail=trapAlarmDetail(errorCode);
		severity=getAlarmSev(errorCode);
	}
	else
	{
		alarmDetail=alarmClearMsg;
		severity=ALARM_SEV_INFORMATION;
	}

	snmpTrapEventOid[SIZE_snmpTrapEventOid-1] = severity;
	errorCodeOamOid[SIZE_errorCodeOamOid-1] = itm;
   
	if(version == SNMP_VERSION_2)
	{
		/* Bind the System upTime as the first variable */
		if ((SNMP_Bind_Integer (pkt, i++, sizeof sysUpTimeOid / sizeof sysUpTimeOid [0], sysUpTimeOid, 
				((tickGet()/sysClkRateGet())*100)))  == -1)
			return (ERROR);
		
		/* Bind sys Trap OID */  /* Raju: Change sysTrapOid to snmpTrapOID to make it work with HPOV */
		if ((SNMP_Bind_Object_ID (pkt, i++, sizeof snmpTrapOID/sizeof snmpTrapOID[0], snmpTrapOID, 
				SIZE_snmpTrapEventOid, snmpTrapEventOid))  == -1)
			return (ERROR);
	}

	if (SNMP_Bind_String(pkt,i++,SIZE_trapAlarmModuleNameOid,trapAlarmModuleNameOid,VT_STRING,strlen(moduleName),moduleName,0)  == -1)
	{
		printf("SNMP_Bind_String error in moduleName(%s)\n",moduleName);
		return (ERROR); 
	}

	if ((SNMP_Bind_Integer (pkt, i++, SIZE_trapAlarmCodeOid, trapAlarmCodeOid, errorCode))  == -1)
		return (ERROR); 

	if (SNMP_Bind_String(pkt,i++,SIZE_trapAlarmDescOid,trapAlarmDescOid,VT_STRING,strlen(alarmDetail),alarmDetail,0)  == -1)
	{
		printf("SNMP_Bind_String error in alarmDetail(%s)\n",alarmDetail);
		return (ERROR); 
	}

	return (OK);
}


/*******************************************************************************
*
* TrapFwdEntry - Get the Trap table entry 
*
* RETURNS: Trap Table entry or NULL  
*/

TrapFwdEntry *snmp_trapEntryGet(int idx)
{
   TrapFwdEntry *trapEntry;
   
   trapEntry = (TrapFwdEntry *) snmp_getAddress(MIBT_trapFwdEntry, idx);
   
   if (trapEntry->trapIpAddress != NULL)
   {
      return trapEntry;
   }  
   return NULL;

}

/*******************************************************************************
* 
* snmp_trapBroadcast - send a SNMP enterprise specific to all the destination
*                      configured at trap config table. 
*
*   
* RETURNS: N/A
*/

void snmp_trapBroadcast(MibTag tag)
{
   void *                 pDestAddr[1];  /* array of pointers to hosts */ 
   struct sockaddr_in     destAddr[1];   /* inet. domain addr. struct */ 
   u_long                 snmpLocalIP;   /* trap sender's ipaddress */ 
   IMPORT int             snmpSocket;    /* socket descriptor for snmp */
   char                   localhostname[MAXHOSTNAMELEN];
   TrapFwdEntry*          pTrapEntry;
   int                    index;

   extern OIDC_T		  enterpriseOid[];
   extern int		      SIZE_enterpriseOid;
  
   SnmpTrapCallbackData   *pSnmpTrapCallbackData = NULL;
   static char fname[] = "snmp_trapBroadcast";

   
   /* agent's ipaddress converted from dot to long integer in network order */
   gethostname(localhostname, MAXHOSTNAMELEN);
   snmpLocalIP = hostGetByName(localhostname);
   
   destAddr[0].sin_family =  AF_INET;
   pDestAddr[0] =   &destAddr[0];
   
   for (index = 0; index < TrapFwdTableLimit; index++)
   {
      pTrapEntry = snmp_trapEntryGet(index);

      
      if (pTrapEntry)
      {
         LOG_TRACE(("Trap Entry type=%d, ip=%#x, port=%d, community=%s", pTrapEntry->trapType,
            pTrapEntry->trapIpAddress, pTrapEntry->trapPort, pTrapEntry->trapCommunity));
         
         destAddr[0].sin_port = htons (pTrapEntry->trapPort);
         destAddr[0].sin_addr.s_addr = pTrapEntry->trapIpAddress;
		 
		 pSnmpTrapCallbackData = (SnmpTrapCallbackData *) malloc(sizeof(SnmpTrapCallbackData));
		 pSnmpTrapCallbackData->mibTag = tag;
         
         if (pTrapEntry->trapType == VAL_trapType_snmpv1Trap)
         {
			pSnmpTrapCallbackData->snmpVersion = SNMP_VERSION_1;

			 snmpdTrapSend
				 (
				 &snmpSocket,                     /* void*     snmp agent transport endpoint      */
				 1,                               /* int       number of destinations             */
				 pDestAddr,                       /* void**    array of ptrs to destinations      */
				 NULL,                            /* void*     local address                      */
				 SNMP_VERSION_1,                  /* int       SNMP version                       */
				 pTrapEntry->trapCommunity,       /* char*     trap community string              */
				 enterpriseOid,                   /* OIDC_T*   agent object identifier            */
				 SIZE_enterpriseOid,                /* int       length of agent object identifier  */
				 &snmpLocalIP,                    /* u_long*   ip address of sender               */
				 ENTERPRISE_SPECIFIC,             /* int       trap type                          */
				 MIB_ITM(tag),                    /* int       trap specific code                 */
				 3,                               /* int       number of varbinds in packet       */
				 snmp_trapBindVals,               /* FUNCPTR   routine to bind varbinds           */
				 (void*) pSnmpTrapCallbackData    /* void*     argument to binding routine        */
				 );

         }
         else 
         {
			pSnmpTrapCallbackData->snmpVersion = SNMP_VERSION_2;
			 snmpdTrapSend
				 (
				 &snmpSocket,                     /* void*     snmp agent transport endpoint      */
				 1,                               /* int       number of destinations             */
				 pDestAddr,                       /* void**    array of ptrs to destinations      */
				 NULL,                            /* void*     local address                      */
				 SNMP_VERSION_2,                  /* int       SNMP version                       */
				 pTrapEntry->trapCommunity,       /* char*     trap community string              */
				 NULL,							  /* OIDC_T*   agent object identifier            */
				 0,								  /* int       length of agent object identifier  */
				 &snmpLocalIP,                    /* u_long*   ip address of sender               */
				 ENTERPRISE_SPECIFIC,             /* int       trap type                          */
				 0,								  /* int       trap specific code                 */
				 5,                               /* int       number of varbinds in packet       */
				 snmp_trapBindVals,               /* FUNCPTR   routine to bind varbinds           */
				 (void*) pSnmpTrapCallbackData    /* void*     argument to binding routine        */
				 );
         }
         
  
      }
   }
}


