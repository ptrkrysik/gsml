
/*
************************************************************************
*
* (c) Copyright Cisco 2000
* All Rights Reserved
*-----------------------------------------------------------------------
*                          
*  FILE NAME: snmp_fileio.c   
*                      
*  DESCRIPTION: This file contains snmp agent file I/O routines
*                
*
*      COMPONENTS:                      
*                              
*                                  
*  NOTES:                              
*                                  
*  REVISION HISTORY:                           
*----------+--------+---------------------------------------------------
* Name     |  Date  |  Reason                
*----------+--------+---------------------------------------------------
* Bhawani  |02/28/00| Initial Draft
*----------+--------+---------------------------------------------------
************************************************************************
*/

/*
************************************************************************
* Common Include files
************************************************************************
*/
#include <vxWorks.h>
#include <stdlib.h>
#include <stdio.h>            /* for printf... */
#include <string.h>

#include <ioLib.h>
#include <fioLib.h>
#include <wdLib.h>
#include <sysLib.h>
#include <taskLib.h>
#include <inetLib.h>
#include <rebootLib.h>


#include "vipermib.h"		      /* Get the MIB definitions. */
#include "vipertbl.h"		      /* Control Table definitions.	*/


/********************************************************************
* Defines
*********************************************************************/


#define FMT_SPEC_STRING    "%s.%u=\"%s\"\n"
#define FMT_SPEC_NUMBER    "%s.%u=\"%d\"\n"
#define SIZE_MibTagName    128
#define MAX_LINE           SIZE_DisplayString + SIZE_MibTagName + 5
#define MAX_PATH_NAME      256

/********************************************************************
* Global variables
*********************************************************************/

SEM_ID      SnmpSemaIo;                   /* Snmp IO task semaphore */
char        SnmpMibFileNameBuffer[SIZE_DisplayString];

/********************************************************************
* Defined in vipermib.h 
*********************************************************************/

extern int SnmpMibLoadCount;
extern int SnmpMibSaveCount;
extern int SnmpPostSetEnabled;
extern int  SnmpTimerMibFlushDelay;
extern int  SnmpTimerRebootDelay;
extern int  SnmpTimerRebootShortDelay;
extern char SnmpDefaultMibFileName[];


/********************************************************************
* Routines
*********************************************************************/

char* StringToken( char* pszThis, char* pszDelims, char** ppszNext )
{
   size_t szToken;

   if( pszThis == NULL )
      pszThis = *ppszNext;

   while( *pszThis != '\0' && 0 == ( szToken = strcspn( pszThis, pszDelims ) ) )
      pszThis++; /* skipping leading delimiters */

   switch( *pszThis )
   {
      case '\0':
         return NULL;

      case '"': /* remove this case if there's no need to treat quotation marks specially */
         return StringToken( pszThis + 1, "\"", ppszNext );

      default:
         if( *( *ppszNext = pszThis + szToken ) != '\0' )
            *(*ppszNext)++ = '\0';

         return pszThis;
   }
}

void StripEOL(char *pStr)
{
   int i;

   if (NULL != pStr)
   {
      i = strlen(pStr);
      if ((i > 0) &&  ((pStr[i-1] == '\r') || (pStr[i-1] == '\n')))
      {
         pStr[i-1] = 0;
      }

      i--;
      if ((i > 0) &&  ((pStr[i-1] == '\r') || (pStr[i-1] == '\n')))
      {
         pStr[i-1] = 0;
      }
   }
}


/* writing mib varible to the file */
Status snmp_writeString(FILE *fp, char *name, int idx, char *value)
{
   
   if ((NULL != value) && (0 != *value))
   {
      StripEOL(value);   
      fprintf (fp, FMT_SPEC_STRING, name, idx, value);
   }
   else 
   {
      fprintf (fp, FMT_SPEC_STRING, name, idx, "");
   }
   
   return Success;
}


Status snmp_writeIpAddress(FILE *fp, char *name, int idx, UINT32 ip)
{
   return snmp_writeString(fp, name, idx, int2ip(ip));
}


Status snmp_writeNumber(FILE *fp, char *name, int idx, UINT32 number)
{
   fprintf (fp, FMT_SPEC_NUMBER, name, idx, (int) number);
   return Success;
}


/* read mib varialbe from the file */
Status snmp_parseLine(char *pszLine, char *pszTagName, int *pnInstance, char *pszValue)
{
   char *pToken;
   
   if (!(pszLine && pszTagName && pnInstance && pszValue))
   {
      return Failure;
   }
   /* skip comments */
   
   if (pszLine[0] == '#' || pszLine[0] == '!')
   {
      return Failure;
   }
      
   /* extract name */
   pToken = StringToken(NULL, ".\n\r", &pszLine);
   if (NULL == pToken)
   {
      return Failure;
   }

   pszTagName[SIZE_MibTagName] = 0;
   strncpy(pszTagName, pToken, SIZE_MibTagName-1);
   
   
   /* extract instance */
   pToken = StringToken(NULL, "=\n\r", &pszLine);
   if (NULL == pToken)
   {
      return Failure;
   }
   *pnInstance = atoi(pToken);
   
   /* extract the value */
   pToken = StringToken(NULL, " \t\n\r", &pszLine);

   if (pToken == NULL)
   {
      return Failure;
   }

   pszValue[SIZE_DisplayString] = 0;
   strncpy(pszValue, pToken, SIZE_DisplayString);   
   StripEOL(pszValue);   
   return Success;
}


Status snmp_saveMib(char *name)
{
   int i, j, end,  tag, cnt = 0;
   char		 tmpname[256];
   Table   *tbl;
   FILE     *fp;
   
   DMSG("ascii mib", printf("snmp_saveMib('%s') called\n", name));

   sprintf(tmpname, "%s.tmp",name);
   
   /* Try to delete the tmp file */
   unlink(tmpname);
   
   if (NULL == (fp = fopen(tmpname, "w+"))) 
   {

      snmp_log(True, "Save MiB Error","Could not open file %s for writing (%s)", tmpname, strerror(errno));
      return Failure;
   }
   SnmpMibSaveCount++;
   
   for ( i=0; tag = mnetMibDataType[i].tag; i++)
   {
      
      if (mnetMibDataType[i].flag & SAVE_ASCII_OMIT)
      {
         
         DMSG("ascii mib", printf("snmp_saveMib. Skipping item %d, '%s', typ %d\n", 
            i, snmp_tagName(tag), mnetMibDataType[i].type));
         continue;
      }
      
      DMSG("ascii mib", printf("snmp_saveMib. Procesisng item %d, '%s', typ %d\n", 
         i, snmp_tagName(tag), mnetMibDataType[i].type));
      
      switch(MIB_TYP(tag))
      {
      case MTYPE_NUMBER:
         snmp_writeNumber(fp, mnetMibDataType[i].name, 0, (INT32) snmp_getValue(tag));
         break;
         
      case MTYPE_IPADDR:
         snmp_writeIpAddress(fp, mnetMibDataType[i].name, 0, (INT32) snmp_getValue(tag));
         break;
         
      case MTYPE_STRING:
         snmp_writeString(fp, mnetMibDataType[i].name, 0, (char *) snmp_getValue(tag));
         break;
         
      case MTYPE_TNUMBER:
         
         if ((tbl = snmp_getTableAddress(tag))) 
         {
            end = table_end(tbl);
            for (j=0; j < end; j++)
            {
               snmp_writeNumber(fp, mnetMibDataType[i].name, j, (INT32) snmp_getValue(tag, j));
            }
         }
         break;
         
      case MTYPE_TIPADDR:
         
         if ((tbl = snmp_getTableAddress(tag))) 
         {
            end = table_end(tbl);
            for (j=0; j < end; j++)
            {
               snmp_writeIpAddress(fp, mnetMibDataType[i].name, j, (INT32) snmp_getValue(tag, j));
               
            }
         }
         break;
      case MTYPE_TSTRING:
         
         if ((tbl = snmp_getTableAddress(tag))) 
         {
            end = table_end(tbl);
            for (j=0; j < end; j++)
            {
               snmp_writeString(fp, mnetMibDataType[i].name, j, (char *) snmp_getValue(tag, j));
               
            }
         }
         break;

      default:
         DMSG("save mib", printf("Skipped Item %s (type= %d)", mnetMibDataType[i].name, MIB_TYP(tag)));
         break;

      } /*switch */
   } /* for */
   fclose(fp);

   /* Now remove the target file. */
   unlink(name);

   /* Try to rename the tmp file to the target file */
   if (rename(tmpname, name) == ERROR)
   {
	   if (copy(tmpname, name) == ERROR)
	   {
			snmp_log(False, "snmp_saveMib", "Error: Could not rename/copy file %s to %s: %s", tmpname, name, strerror(errno));
			return Failure;
	   }
	   unlink(tmpname);
   }

   return Success;
}


Status snmp_loadMib( char  *name)
{
   FILE    *fs;
   char    buf[MAX_LINE];
   UINT32  ip;
   
   int     i, idx, tag;
   int line = 0;
   
   char pszTagName[SIZE_MibTagName+1];
   char pszValue[SIZE_DisplayString+1];

   if (NULL == (fs = fopen(name, "r")))
   {
      snmp_log(True, "Load Mib Error", "Could not open file %s for reading: %s.", name, strerror(errno));
      return Failure;
   }
   
   snmp_log(False, "snmp_loadMib", "Loading mib from file %s", name);
   SnmpMibLoadCount++;

   while (NULL != fgets(buf, MAX_LINE, fs)) 
   {
      line++;
      if (Failure == snmp_parseLine(buf, pszTagName, &idx, pszValue))
      {
         continue;
      }
       
      /* Search the tagname */
      for ( i=0; mnetMibDataType[i].tag; i++)
      {
         if (!strcmp(mnetMibDataType[i].name, pszTagName))
         {
            break;
         }
      }
      
      tag = mnetMibDataType[i].tag;

      if (0== tag)
      {
         snmp_log(True, "Load Mib Error", "%s(%d):  %s is not a valid tag.", name, line, pszTagName);
         continue;
      }

      switch(MIB_TYP(tag))
      {
      case MTYPE_NUMBER:
      case MTYPE_TNUMBER:
         snmp_setEntry(tag, idx, (void *) atoi(pszValue), 4);
         break;
         
      case MTYPE_IPADDR:
      case MTYPE_TIPADDR:
         if (ERROR != (ip = inet_addr(pszValue)))
         {
            snmp_setEntry(tag, idx, (void *) ip, 4);
         }
         else
         {
            /* in case the ip addres is saved as plain integer */
            snmp_setEntry(tag, idx, (void *) atoi(pszValue), 4);
         }

         break; 
         
      case MTYPE_STRING:
      case MTYPE_TSTRING:
         snmp_setEntry(tag, idx, (void *) pszValue, strlen(pszValue));
         break;
         
      default:
         snmp_log(True, "Load Mib Warning", "%s(%d):  tag %s (type=%d) is not not loadable tag.", name, line, pszTagName, MIB_TYP(tag));
         break;
      } /* switch */
   } /* fgets */
   fclose(fs);

    /* Set SNMP Agent's status */
   agent_setSnmpEnabled(oam_isSnmpEnabled());

   /* update FTP Server status */
   oam_setFtpEnabled(oam_isFtpEnabled());

   return Success;
}


/********************************************************************
* snmp Mib Save Timer
*********************************************************************/
WDOG_ID SnmpWdTimer;
BOOL bIsMibSaveTimerActive = FALSE;


void snmp_mibTimerInit()
{
   SnmpWdTimer = wdCreate();
}

void snmp_mibTimerDelete()
{
   wdDelete(SnmpWdTimer);
}

int snmpMibWriteDelayTimeoutHandler(int arg)
{
   char *name = (char *) arg;
   
   /*memset(SnmpMibFileNameBuffer, 0, SIZE_DisplayString); */
   strncpy(SnmpMibFileNameBuffer, name, SIZE_DisplayString-1);
   SnmpMibFileNameBuffer[SIZE_DisplayString-1] = 0;
   semGive (SnmpSemaIo);
   bIsMibSaveTimerActive = FALSE;
   return Success;
};

void snmp_mibTimerSet(char* arg)
{
   if (TRUE == bIsMibSaveTimerActive)
   {
      wdCancel(SnmpWdTimer);
   }
   wdStart(SnmpWdTimer, SnmpTimerMibFlushDelay*sysClkRateGet(), snmpMibWriteDelayTimeoutHandler, (int) arg);
   bIsMibSaveTimerActive = TRUE; 
}

int snmp_reboot(int delay)
{
   SnmpPostSetEnabled = False;
   
   if (TRUE == bIsMibSaveTimerActive)
   {
      wdCancel(SnmpWdTimer);
      if (delay < SnmpTimerRebootShortDelay)
      {
         delay = SnmpTimerRebootShortDelay;
      }
      SnmpTimerRebootDelay = delay*sysClkRateGet();
      snmpMibWriteDelayTimeoutHandler((int) SnmpDefaultMibFileName);
   }
   return OK;
}

int snmp_ioThread(int arg)
{
   do
   {
      semTake (SnmpSemaIo, WAIT_FOREVER);    
      snmp_saveMib(SnmpMibFileNameBuffer);
   }while (SnmpTimerRebootDelay <= 0);
   
   taskDelay(SnmpTimerRebootDelay);
   reboot(0);
   return OK;
}

void snmp_initIoTask()
{

   SnmpSemaIo = semBCreate (SEM_Q_FIFO, SEM_EMPTY);
   taskSpawn("tSnmpIo", 100, 0, 40000, (FUNCPTR) snmp_ioThread,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

