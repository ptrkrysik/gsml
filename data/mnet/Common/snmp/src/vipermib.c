/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************
**                          
**  FILE NAME: vipermib.c  
**                      
**  DESCRIPTION: This file contains snmp get/set/next/test methods
**      
**  COMPONENTS:                      
**                              
**                                  
**  NOTES:                              
**                             
**  REVISION HISTORY                            
**  __________________________________________________________________
**  ----------+--------+----------------------------------------------
**  Name      |  Date  |  Reason                
**  ----------+--------+----------------------------------------------
**  Bhawani   |03/02/01| Modified original file and resturctured to use
**            |        | common vob and clean up the code. Separated 
**            |        | file I/O in a separated file.
**  Bhawani   |03/16/01| Added SnmpMibLookup routines
**  ----------+--------+----------------------------------------------
**********************************************************************
*/


/*
**********************************************************************
** Include Files. 
**********************************************************************
*/

#ifdef DEBUG
#define PARANOID    /* Full debugging */
#endif

#include <vxWorks.h>
#include <mib.h>
#include <snmpdefs.h>
#include <snmpdLib.h>
#include <ioLib.h>
#include <fioLib.h>
#include <sysLib.h>
#include <stdlib.h>
#include <stdio.h>     
#include <stdarg.h>
#include <memLib.h>    
#include <hostLib.h>   
#include <time.h>
#include <timers.h>
#include <inetLib.h>
#include <ctype.h>

#include "vipermib.h"  
#include "vipertbl.h"  
#include "oam_api.h"
#include "snmp/include/version.h"

/********************************************************************
* Defined used in this file
*********************************************************************/
#define LOCAL_MEM_LOCAL_ADRS    0x00000000  /* Base of RAM */
#define BOOT_LINE_OFFSET        0x4200
#define BOOT_LINE_ADRS  ((char *) (LOCAL_MEM_LOCAL_ADRS+BOOT_LINE_OFFSET))
#define BOOT_LINE_SIZE  255 /* use 255 bytes for bootline */
#define MAX_PATH_NAME   256

/********************************************************************
* Global variables
*********************************************************************/
char SnmpDefaultMibFileName[MAX_PATH_NAME];
int SnmpTimerRebootLongDelay           = 30;
int SnmpTimerRebootShortDelay          = 5;
int SnmpTimerRebootDelay               = 0;

int SnmpTimerMibFlushDelay             = 5;          /* default in seconds */
int SnmpTimerMibFlushMinFileIo         = 5*60;        /* 5 minutes */
int SnmpTimerMibFlushMinDelay          = 2;           /* 2 seconds */


extern char *snmp_tagName( MibTag );
extern void snmp_printPDU(SNMP_PKT_T *, char *);
extern char *snmp_printPduType(int);

int SnmpMibLoadCount    = 0;
int SnmpMibSaveCount    = 0;
int SnmpMibSetCount     = 0;
int SnmpMibGetCount     = 0;

int mibInitialized      = False;
int snmpInitialized     = False;
int SnmpPostSetEnabled  = False;

char *mibVersionId = VIPERCELL_EXTERNAL_VERSION_NO;
char *mibBuildId = VIPERCELL_INTERNAL_BUILD_ID;
char *mibDate = __DATE__;
char *mibTime = __TIME__;
char *mibCrypto = VIPERCELL_CRYPTO_INFO;

FILE    *logFS;
char    logBuf[256];


/* forward declaration */
extern void snmpLocalIoInit();
extern void snmpLocalInitCommunityNames();
extern STATUS snmp_postLoadMibHandler();



/* convert unsigned int to dotted IP string */
char* int2ip(UINT32 nIp)
{
   UINT8* pIp = (UINT8*) &nIp;
   static char pszIp[16];
   sprintf(pszIp, "%u.%u.%u.%u", pIp[0], pIp[1], pIp[2], pIp[3]);
   return pszIp;
}

void SnmpStatusShow()
{
   printf("Snmp default Mib file: '%s'\n", SnmpDefaultMibFileName);
   printf("Snmp MIB Flush Delay Timer: %d\n", SnmpTimerMibFlushDelay);
   printf("Mib File Load Count: %d\n", SnmpMibLoadCount);
   printf("Mib File Save Count: %d\n", SnmpMibSaveCount);
   printf("Mib File Set Count: %d\n", SnmpMibSetCount);
   printf("Mib File Get Count: %d\n", SnmpMibGetCount);

}

void snmp_oamCmdStrSet(char *str)
{
   snmp_setEntry(MIB_oamCommandLastError, 0, str, strlen(str));
   
}


void snmp_initializeMib()

{
   int     i, j, end, tag;
   void    **addr;
   Table   *tbl;
   
   
   if (snmpInitialized == True)    /* This code is a one-shot. */
      return;
   snmpInitialized = True;
   mibInitialized = True;

   snmp_mibTimerInit();
   
   /* This speeds up the data validation routines. By defining this
   ** Table header and associating the data type array and a sort proc
   ** with the table, we can use the table_find() routine to do a binary
   ** search of this table. Since the data validation routines expect
   ** this to be setup, do it first. */
   
   mnetMibDataTypeTable = table_create(sizeof(MnetMibDataType));
   mnetMibDataTypeTable->data = (char *)mnetMibDataType;
   i = -1;
   while (mnetMibDataType[++i].tag);
   mnetMibDataTypeTable->last = i;
   mnetMibDataTypeTable->end = i;
   
   /* Part of the MIB is a statically allocated in the MnetMib structure.
   ** This contains all of the base packages. The tables and string buffers are
   ** allocated at runtime with malloc. The following loop will allocate the
   ** dynamic part of the MIB: */
   
   i = -1;
   while ((tag = mnetMibIndex[++i].tag))
   {
      if (NULL == mnetMibIndex[i].addr)
      {
         if (MIB_TYP(tag) == MTYPE_STRING)
         {
            mnetMibIndex[i].addr = (void *)calloc(1, mnetMibIndex[i].size+1);
         }
         else
         {
            mnetMibIndex[i].addr = addr = (void *)table_create(mnetMibIndex[i].size);
            table_limit((Table *)addr, mnetMibIndex[i].limit);
            snmp_fixupTable((Table *)addr);       /* Add an index to the tables. */
         }
      }
      if (!((MIB_TYP(tag) == MTYPE_STRUCT) && (MIB_ITM(tag) == 0)))
      {
         addr = (void**)snmp_getAddress(tag);         /* Were this tag resolves to. */
         *addr = mnetMibIndex[i].addr;                /* Store the real address away. */
         DMSG("fixup mib", printf("snmp_initMib(): Fixup loading '%s' @%x = '0x%08x'.\n",
            snmp_tagName(tag), (int) addr, (int)mnetMibIndex[i].addr));
      }                                            /* Can't use snmp_setValue since */
   }                                                /* it would copy the string, not */
                                                     /* just set the address. */
   
   /* Now we can load the default values in the MIB structure. */
   /* Setup the COMPLEX number callbacks as well. */
   i = -1;
   while ((tag = mnetMibDataType[++i].tag))
   {
      if (mnetMibDataType[i].value)
      {
         if (MTYPE_IS_TABLE(MIB_TYP(tag)))
         {
            if ((tbl = snmp_getTableAddress(tag))) 
            {
               end = table_end(tbl);
               for (j=0; j < end; j++)
               {
                  snmp_setEntry(tag, j, (void *) mnetMibDataType[i].value, 4);
               }
            }
         }
         else
         {
            snmp_setEntry(tag, 0, (void *) mnetMibDataType[i].value, 4);
         }
      } /* Value not zero */
   }   
   
   /* Now that the internal structure is setup, we can load in the
   * real MIB. However, we should wait until the date has been set
   * in the startup script. Also, there is a MNET_BASE flag
   * that point to where the MIB files should be put. Thus, we set
   * a flag saying we are not intialized yet. The following routine
   * gets executed from the startup script. It loads the MIB and then
   * clears the flag allowing access.
   */
   
   mibInitialized = False;
}

/* Aug 23, 2000 Modified this function to fix AR 1216 - Bhawani*/
int snmp_initialize(char    *name)
{
   char    *str;
   char    *vcBase;
   int     versionFd;
   int     snmpLocalIP;
   char    localhostname[MAXHOSTNAMELEN];
   char    versionFileName[MAXHOSTNAMELEN];
   char    verBuf[64];
   char    buildBuf[64];
   char    timeBuf[32];
   char    dateBuf[32];
   char    cryptoBuf[32];
   char    tmpBuf[128];
   char    buf[512];
   int     charsRead;
   
   gethostname(localhostname, MAXHOSTNAMELEN);
   snmpLocalIP = hostGetByName(localhostname);



    /* first initialize mib and create table etc. */
     snmp_initializeMib();

     
   /* Initialize IO Task */
   snmp_initIoTask();

   /* Initialize Local IO first */
   snmpLocalIoInit();
   
   /* check if this function is called multiple times */
   if (mibInitialized == True)
      return OK;

   snmp_log(False, "\nSnmp Initialize", "SNMP agent is booting up");
   
   /* Get the value of the MNET_BASE environment variable. */
   vcBase = getenv( "MNET_BASE" );
   if( vcBase == NULL )
   {
      snmp_log(True, "snmp_initialize", "SNMP initialization failure because environment variable \"MNET_BASE\" could not be found.");
      return ERROR;
   }
   
   /* Set the Mib file name*/
   sprintf(SnmpDefaultMibFileName, "%s/mib/mib.txt", vcBase);
   
   mibInitialized = True; /* allow MIB access in snmp_loadMib */
   
   /* Call product specific pre loadMib handler */
   snmp_initDefaultMib();
   
   /* Overwrite the default value with the one specified in the MIB file */
   if (snmp_loadMib(SnmpDefaultMibFileName) ==  Success)
   {
      snmp_log(True, "Snmp Initialize", "SNMP MIB was successfully loaded from file %s.", SnmpDefaultMibFileName);
   }
   else
   {
      snmp_saveMib(SnmpDefaultMibFileName);
      snmp_log(True, "Snmp Initialize", "SNMP mib could not be loaded from %s: %s", SnmpDefaultMibFileName, strerror(errno));
   }

   /* Access control status enabled */
    snmp_setValue(MIB_mibAccessControlFlag, True);
   
    /* Clear the OAM command last error */
    snmp_oamCmdStrSet("");
   
   /* Initialize Element IP Address */
    snmp_setValue(MIB_mnetIpAddress, snmpLocalIP);

   /* Initialize Software build */
   if ((str = (char *)snmp_getValue(MIB_mnetSoftwareBuild)))
   {
      /* Open file version.txt and try to read the version and build number */
      sprintf(versionFileName, "%s/version.txt", vcBase);
      
      printf("Version file %s ", versionFileName);
      if ((versionFd = open (versionFileName, O_RDONLY, 0)) != ERROR)
      {
         printf("will be used to set Version and Build Id\n");
         while (fioRdString(versionFd, buf, 512) != EOF)
         {
            sscanf(buf, "%s", tmpBuf);
            if (strcmp(tmpBuf, "[BUILDID]") == 0)
            {
               charsRead = fioRdString(versionFd, buildBuf, 63); 
               buildBuf[charsRead] = 0;
               mibBuildId = buildBuf;
            } 
            if (strcmp(tmpBuf, "[VERSION]") == 0)
            {
               charsRead = fioRdString(versionFd, verBuf, 63); 
               verBuf[charsRead] = 0;
               mibVersionId = verBuf;
            }
            if (strcmp(tmpBuf, "[DATE]") == 0)
            {
               charsRead = fioRdString(versionFd, dateBuf, 31); 
               dateBuf[charsRead] = 0;
               mibDate = dateBuf;
            }   
            if (strcmp(tmpBuf, "[TIME]") == 0)
            {
               charsRead = fioRdString(versionFd, timeBuf, 31); 
               timeBuf[charsRead] = 0;
               mibTime = timeBuf;
            }   
            if (strcmp(tmpBuf, "[CRYPTO]") == 0)
            {
               charsRead = fioRdString(versionFd, cryptoBuf, 31); 
               cryptoBuf[charsRead] = 0;
               mibCrypto = cryptoBuf;
            }   
            
         }
         close(versionFd);
      }
      else
      {
         printf("not found...using default Version and Build Id\n");
      }
      
      sprintf(str, "%s %s %s %s %s %s %s %s %s", 
         VERSION_PREFIX, mibVersionId, 
         DATE_AND_TIME_PREFIX, mibDate, mibTime,
         BUILD_PREFIX, mibBuildId,
		 CRYPTO_PREFIX, mibCrypto);
   }

   /* Call product specific post loadMib handler */
   snmp_postLoadMibHandler();


   /* initialize community names */
	snmpLocalInitCommunityNames();

   /* finally here we go */
   snmp_log(False, "snmp_initialize", "Snmp initialized successfully");
   SnmpPostSetEnabled = True;
   return OK;
}


/* Aug 23, 2000 modified snmp_log() to localize logFileName - Bhawani*/
void snmp_log(int flag, char *fname, char *spec, ...)
{
   char    *str;
   time_t  stamp;
   FILE    *logFS;
   va_list  arg_list;
   static char    logFileName[PATH_MAX] = "";
   static char logstr[SIZE_DisplayString+1];
   int pos;
   
   if (logFileName[0] == 0)
   {
      /* Get the value of the MNET_BASE environment variable. */
      str = getenv( "MNET_BASE" );
      if( str == NULL )
      {
         return;
      }
      /* Set the log file path */
      sprintf(logFileName, "%s/mib/snmp.log", str);
      
   }   
   
   if ((logFS = fopen(logFileName, "a")) == NULL)
   {
      sprintf(logstr, "Unable to open logfile %s: (%d) '%s'", logFileName, errno, strerror(errno));
      snmp_oamCmdStrSet(logstr);
      return;
   }
   
   va_start(arg_list, spec);  
   time(&stamp);
   str = ctime(&stamp);
   str[24] = ',';                      /* Zap the annoying newline. */
   
   pos = sprintf(logstr, "%s %s, ", str, fname);
   pos += vsprintf(logstr+pos, spec, arg_list);
   if (flag == True)
   {
      snmp_oamCmdStrSet(logstr);          /* a copy to Snmp Last error Mib variable */
   }
   fprintf(logFS, "%s\n", logstr);     /* a copy to snmp log file */
   printf("%s\n", logstr);             /* a copy to the console */  
   fclose(logFS);
   va_end(arg_list);
   return;
}

void snmp_fixupTable(Table   *tbl)
{
   int     i;
   int     *addr;
   
   if (!VALID_TABLE(tbl)) 
   {
      DMSG("fixup", printf("snmp_fixupTable passed an invalid table pointer 0x%x\n", (int)tbl));
      return;     /* We do not have a valid table header! */
   }
   DMSG("fixup", printf("snmp_fixupTable is fixing up table indicies for tbl at 0x%x\n", (int)tbl));
   
   for (i=0; i<tbl->end; i++)
   {
      addr = (int *)((int)tbl->data + (i*tbl->size));
      *addr = i;
   }
   
}

int snmp_operation(int op)
{
   char    *name;
   int     status;
   
   if (op == 0)
   {
      return OK;
   }

   snmp_setValue(MIB_oamCommandExecutionStatus, VAL_oamCommandExecutionStatus_inProgress);
   
   status = VAL_oamCommandExecutionStatus_failed;   
   DMSG("oam operation", printf("snmp_operation(%d) called\n", op));
   
   switch (op)
   {
      
   case VAL_oamCommand_reloadMib:
      if (snmp_loadMib(SnmpDefaultMibFileName) == Success)
      {  
         snmp_log(False, "snmp_operation", "Reload Mib request was successful.");
         status  = VAL_oamCommandExecutionStatus_sucessful;
      }
      break;
   
   
   case VAL_oamCommand_flushMib:
      
      if (snmpMibWriteDelayTimeoutHandler((int) SnmpDefaultMibFileName) == OK)
      {
         snmp_log(False, "snmp_operation", "Flush Mib request was successful.");
         status  = VAL_oamCommandExecutionStatus_sucessful;
      }
      break;
      
      
      
   case VAL_oamCommand_loadMibFromAltFile:
      name = (char *) snmp_getValue(MIB_oamAlternateMibFileName);
      if (name)
      {
         
         if (snmp_loadMib(name) == Success)
         {
            snmp_log(False, "snmp_operation", "Load from alternate  Mib file request was successful.");
            status  = VAL_oamCommandExecutionStatus_sucessful;
         }
      }
      break;
      
   case VAL_oamCommand_saveMibToAltFile:
      name = (char *) snmp_getValue(MIB_oamAlternateMibFileName);
      if (name)
      {         
         if (snmpMibWriteDelayTimeoutHandler((int) name) == Success)
         {
            snmp_log(False, "snmp_operation", "Save Mib to alternate file request was successful.");
            status  = VAL_oamCommandExecutionStatus_inProgress;
         }
      }
      break;
      
   case  VAL_oamCommand_saveMibMinDelay:
      SnmpTimerMibFlushDelay = SnmpTimerMibFlushMinDelay;
      status  = VAL_oamCommandExecutionStatus_sucessful;
      snmp_log(False, "snmp_operation", "SNMP flush mode set to minimum.");
      break;

   case VAL_oamCommand_saveMibMinFileIo:
      SnmpTimerMibFlushDelay = SnmpTimerMibFlushMinFileIo;
      status  = VAL_oamCommandExecutionStatus_sucessful;
      snmp_log(False, "snmp_operation", "SNMP flush mode set to minimum File I/O.");      
      break;
      
      
   case    VAL_oamCommand_reboot:
      status  = VAL_oamCommandExecutionStatus_inProgress;
      snmp_reboot(SnmpTimerRebootShortDelay);     /* Give a couple of seconds so */
                                                  /* we can respond to the PDU. */
      snmp_log(False, "snmp_operation", "Network element reboot request is submitted.");      
      break;                       
      
   case    VAL_oamCommand_delayedReboot:
      status  = VAL_oamCommandExecutionStatus_inProgress;
      snmp_reboot(SnmpTimerRebootLongDelay);
      snmp_log(False, "snmp_operation", "Network element reboot in 30 second request is submitted.");      
      break;
   
   default:
      snmp_log(False, "snmp_operation", "Not recognized Oam command (%d)", op);      
      status = VAL_oamCommandExecutionStatus_failed;
   }
   
   if (op != VAL_oamCommand_none)
   {
      snmp_setValue(MIB_oamCommand, VAL_oamCommand_none);
   }
   
   snmp_setValue(MIB_oamCommandExecutionStatus, status);
   
   return status == VAL_oamCommandExecutionStatus_failed ? Failure : Success;
}


void snmp_postSet(MibTag tag)
{
   if (MIB_TBL(tag) == MIBL_mnetErrorInfoMib)
   {
      /* This is error event and need to send trap on this one */
      snmp_trapBroadcast(tag);
      return;
   }

   if (True == SnmpPostSetEnabled)
   {
      switch(tag)
      {
      case MIB_oamCommand:
         snmp_operation(snmp_getValue(MIB_oamCommand));
         break;
         
      default:
         snmp_mibTimerSet(SnmpDefaultMibFileName);
         break;
      }
   }
}


/*  int * = snmp_getAddress(tag, [idx]);
*
*   This routine is foundation of the snmp_get... and snmp_set... functions.
*   This provides the basic translation from a MIB_TAG into a pointer to the
*   data item in the MIB.
*/

void * snmp_getAddress( MibTag  tag, ...)
{
   int     typ = MIB_TYP(tag);
   int     col = MIB_SIZ(tag);
   int     idx = MIB_TBL(tag);
   int     itm = MIB_ITM(tag);
   
   int     row = 0;
   void    *addr;
   Table   *tbl;
   va_list argn;
   
   if (mibInitialized == False)
   {
      return NULL;
   }
      
   addr = (void *)((int)mnetMibIndex[idx].addr + ( itm * sizeof(int)));

   if (MTYPE_IS_TABLE(typ))
   {
      /* first get the index for the table */
      va_start(argn, tag);  
      row = va_arg(argn, int);
      va_end(argn);
      
      tbl = (Table *) *(void **)addr;

      if ((row < 0) || (row >= table_end(tbl)))
      {
         DMSG("address", printf("snmp_getAddress: Invalid row of table %s(tag = %#x, row=%d)\n",
            snmp_tagName(tag), tag, row));
         return NULL;
      }
      addr = table_getRow(tbl, row);      /* Get the address of the table entry. */
      
      if (typ != MTYPE_TENTRY)
      {
         addr = (void *)((int)addr + ( (col-1) * sizeof(int))); 
      }
   }
   DMSG("address", printf("snmp_getAddress: Address of  %s(tag=%#x,row=%d)=%#x\n",
      snmp_tagName(tag), tag, row, addr));
   
   return addr;
}



/*  snmp_getTableAddress
*
*   This is a specialized form of the getAddress function. From a MIB tag, we verify
*   this is a table element and then we return the address of the table pointer. This
*   is useful to recover items like the length of the table (ie. the maximum instance
*   of a column in the table).
*/

Table * snmp_getTableAddress(MibTag  tag)
{
   int     typ = MIB_TYP(tag);
   int     idx = MIB_TBL(tag);
   int     itm = MIB_ITM(tag);
   void    *addr;
   Table   *tbl;
   
   if (mibInitialized == False)
      return False;
   
   if (snmp_validTag(tag) == False)
   {
      DMSG("get address", printf("snmp_getTableAddress for tag 0x%x '%s' faild. Invalid tag.\n", tag, snmp_tagName(tag)));
      return Failure;
   }
   
   if (!MTYPE_IS_TABLE(typ))  /* If this is not a table, there is no address. */
      return Failure;
   
   addr = (void *)((int)mnetMibIndex[idx].addr + ( itm * sizeof(int)));
   tbl = (Table *)*((void **)addr);    /* This is a pointer to a table. It could */
   if (!VALID_TABLE(tbl))
   {
      DMSG("get address", printf("snmp_getTableAddress: Invalid Table address for %s (tag=%#x, Addr=%#x)\n",  snmp_tagName(tag), tag, tbl ));      
      return Failure;
   }
   DMSG("address table", printf("snmp_getTableAddress: Address for  table %s(tag = %#x) = %#x\n",
      snmp_tagName(tag), tag, tbl));
   return tbl;
}


/*  int = snmp_getValue (tag, [idx]);
*
*   This routine is used to retrieve a value from SNMP. It is passed the
*   MIB tag which identifies the particular MIB variable to retrieve. If
*   the MIB tag is one for a table, (MIBT_...) then one needs to specify
*   the optional index of the table element. In SNMP all table items are
*   addressed by item within the row. The index gets appended to the oid
*   for the item. Thus, val = snmp_getValue(MIB_sdcchSAPI0); returns the
*   sdcchSAPIO timer MIB value. val = snmp_getValue(MIBT_channelID_1, 5);
*   return the ChannelID for the fifth channel on transmitter 1.
*/

int snmp_getValue( MibTag  tag, ...)
{
   int       idx = 0;
   int       val = 0;
   int       value = 0;
   void     *addr;

   va_list   argn;
   
   if (MTYPE_IS_TABLE(MIB_TYP(tag)))
   {  
      va_start(argn, tag);
      idx = va_arg(argn, int);        /* Table items use an index as well. */
      va_end(argn);
   }
   

   DMSG("get", printf("snmp_getValue: %s (tag=%#x, row=%d)\n", snmp_tagName(tag), tag, idx));

   if ((addr = snmp_getAddress(tag, idx)))
   {

      if (! ( ((MIB_TYP(tag) == MTYPE_STRUCT) && (MIB_ITM(tag) == 0)) || (MIB_TYP(tag) == MTYPE_TENTRY)))
      {
         addr = (void *)*((void **)addr);    /* Value returns the contents of addr. */
      }
   } 
   else 
   {
      DMSG("get", printf("snmp_getValue: Value of %s, (tag=%#x, row=%d) = %d (0x%08x)\n", snmp_tagName(tag), tag, idx, (int)addr, (int)addr));
   }
   SnmpMibGetCount++;
   return (int) addr;
}



/*  status = snmp_getEntry(tag, [idx], &value, size);
*
*   This is similar to snmp_getValue, except that instead of returning
*   the value, it is copied into a data buffer. This can be used to copy
*   the entire contents of one of the MIB tables if desired. Like the
*   snmp_getValue routine, there is an optional index if the mib variable
*   is part of a table. (This is indicated by MIBT_...). 
*/

Status snmp_getEntry(MibTag  tag, ...)
{
   void     *addr, *buf;
   int       size, max, idx = 0;
   int       val = 0, value = 0;
   
   va_list   argn;
   va_start(argn, tag);        /* first unwind the arguments */
   
   if (MTYPE_IS_TABLE(MIB_TYP(tag)))
   {
      idx = va_arg(argn, int);        /* Table items use an index as well. */
   }
   
   buf = va_arg(argn, void *);
   size = va_arg(argn, int);
   va_end(argn);       
   
   if (NULL == buf)
   {
      return Failure;
   }
   
   addr = snmp_getAddress(tag, idx);
   
   if (NULL != addr)
   {
      switch (MIB_TYP(tag))
      {
      case MTYPE_STRING:
      case MTYPE_STRUCT:
      case MTYPE_TENTRY:
         max = MIB_SIZ(tag);
            break;
         
      case MTYPE_TSTRING:
         max = SIZE_DisplayString;
         break;
         
      case MTYPE_NUMBER:
      case MTYPE_TNUMBER:
      case MTYPE_IPADDR:
      case MTYPE_TIPADDR:
         max = 4;
         break;
      default:
         DMSG("get", printf("snmp_getEntry: Invalid tag (%s, %d)\n", snmp_tagName(tag), idx));
         return Failure;
      }
   }
   memcpy(buf, addr, min(max,size));
   return Success;
}

/*  status = snmp_setValue(tag, [idx], value);
*
*   This routine is used to set a value in the MIB structures. The value
*   that the variable is to be set to is passed as an argument. This is
*   the compliment of the snmp_getValue routine.
*/

Status snmp_setValue( MibTag  tag, ...)
{
   int         value;
   int         idx = 0;
   va_list      argn;
   
   
   va_start(argn, tag);        /* first unwind the arguments */
   
   if (MTYPE_IS_TABLE(MIB_TYP(tag)))
   {
      idx = va_arg(argn, int);        /* Table items use an index as well. */
   }
   value = va_arg(argn, int);
   va_end(argn);
   return snmp_setEntry(tag, idx, (void *) value, 0);
}


/*  status = snmp_setEntry(tag, [idx], &value,  size);
*
*   This routine is used to set a value in the MIB structures. A pointer
*   to the variable and its size are passed as arguments. This is the
*   compliment of the snmp_getEntry routine and is used to set entire
*   structures of information at a time.
*/
Status
snmp_setEntry
(
     MibTag  tag,
     int     idx,
     void*   value,
     int     sz
)
{
   void*          addr;
   char           *src, **pDst;
   int            size;
   Status         status = Success;
   
   
   addr = snmp_getAddress(tag, idx);

   if (NULL == addr)
   {
      /* Verify & get addrs for memcpy. */     
      return Failure;
   }
   
 /* Single access here to prevent any race condition */
   snmp_lockMib(True);    
   switch(MIB_TYP(tag))
   {
   case MTYPE_STRING:
   case MTYPE_TSTRING:      
      pDst = (char **) addr;
      src = (char *) value;

      if (NULL == src)
      {
         sz = 0;
      }
      
      if (MIB_TYP(tag) == MTYPE_TSTRING)
      {
         size = min (sz, SIZE_DisplayString);
         if (*pDst == NULL)
         {
            *pDst = malloc(SIZE_DisplayString+1);
            (*pDst)[SIZE_DisplayString] = 0;
         }

         memset(*pDst+size, 0, SIZE_DisplayString-size);
      }
      else
      {
         if (*pDst == NULL)
         {
            /* to prevent error string problem */
            return Failure;
         }
         size = min(sz, MIB_SIZ(tag));
         memset(*pDst+size, 0, MIB_SIZ(tag)-size);
      }
      strncpy(*pDst, src, size);
      DMSG("set", printf("snmp_setEntry: Set string %s.%d = %s\n", snmp_tagName(tag), idx, *pDst));
      break;
      
   case MTYPE_NUMBER:
   case MTYPE_TNUMBER:
      *(INTEGER *)addr = (INTEGER)value;
      DMSG("set", printf("snmp_setEntry: Set number %s.%d = %d (%#x)\n", snmp_tagName(tag), idx, value, value));
      break;

   case MTYPE_IPADDR:
   case MTYPE_TIPADDR:
      *(INTEGER *)addr = (INTEGER)value;
      DMSG("set", printf("snmp_setEntry: Set IP %s.%d = %s\n", snmp_tagName(tag), idx, int2ip((UINT32)value)));
      break;

   case MTYPE_STRUCT:
   case MTYPE_TENTRY:
      size = MIB_SIZ(tag);
      memcpy(addr, value, size);      /* rather than *(void **)addr = (void *)value; */
      DMSG("set", printf("snmp_setEntry struct %s.%d\n", snmp_tagName(tag), idx));
      break;
      
   default:
      DMSG("set", printf("snmp_setEntry: Invalid Tag type (%s, %#x, %d)\n", snmp_tagName(tag), tag, (MIB_TYP(tag))));
      status = Failure;
      break;
   }
   snmp_lockMib(False);
   
   if (status == Success)
   {
      snmp_postSet(tag);
   }
   SnmpMibSetCount++;
   return status;
}

 
 /*  snmp_findTag()      Qsort style function to find a particular tag.
 *
 *   This function is used as a qsort process to sort tables based on the tags.
 *   It is assumed that the first entry in the table is the tag to look for.
 *   This can be used for the table_find process as well if we emulate the
 *   the qsort style of argument passing, that is a pointer to the argument
 *   is passed.
 */
 
 int snmp_findTag( const void *arg1, const void *arg2)
 {
    
    return (*(int*)arg1-*(int*)arg2);
 }
 
  
 SEM_ID mibLock = 0;

 Status snmp_lockMib(int  lock)
 {
    
    if (!mibLock)
    {
       mibLock = semBCreate(SEM_Q_PRIORITY, SEM_FULL);
    }
    
    if (lock)
    {
       semTake(mibLock, WAIT_FOREVER);
    }
    else
    {
       semGive(mibLock);
    }
    return Success;
 }
 
 /*  Status = snmp_validTag(tag);
 *
 *   This routine is used to validate the mib tags. We look at each field
 *   make sure that it is within range. This will insure that the address
 *   that is calculated is contained within the actual MIB structures.
 */ 
 
 Status snmp_validTag(MibTag  tag)
 {
    int  typ = MIB_TYP(tag);
    int  siz = MIB_SIZ(tag);
    int  tbl = MIB_TBL(tag);
    int  itm = MIB_ITM(tag);
    int  i;
    static int strStartIdx = 0;
    

    DMSG("validate", printf("snmp_validTag: Tag %s(tag=%#x,typ=%d,sz=%d,tbl=%d,itm=%d)\n",
       snmp_tagName(tag), tag, typ, siz, tbl, itm));

    
    if ((tbl < 0) || (tbl > MTBL_last))
    {
       return False;
    }
    
    if (itm > mnetMibIndex[tbl].size/4)
    {
       return False;
    }

    if (strStartIdx == 0)
    {
       strStartIdx = MTBL_last;
       tag = mnetMibIndex[++strStartIdx].tag;
       
       while (tag && (MIB_TYP(tag) !=  MTYPE_STRING))
       {
          tag = mnetMibIndex[++strStartIdx].tag;
       }
    }
        
    switch (typ)
    {
    case MTYPE_STRUCT:
       if (itm ||  (siz != mnetMibIndex[tbl].size))
       {
          return False;
       }
       break;
       
    case MTYPE_NUMBER:
    case MTYPE_IPADDR:
       if (siz != 4)
       {
          return False;
       }
       break;
    
    case MTYPE_STRING:
       for ( i = strStartIdx; mnetMibIndex[i].tag != 0; i++)
       {
          if (tag == mnetMibIndex[i].tag)
          {
             return True;
          }
       }
       return False;
       

    case MTYPE_TENTRY:
    case MTYPE_TNUMBER:
    case MTYPE_TIPADDR:
    case MTYPE_TSTRING:
       tag = MIB_TABLE(tbl, itm);  /* create table tag first */
    case MTYPE_TABLE:

       for ( i = MTBL_last+1; i < strStartIdx; i++)
       {
          if ( tag == mnetMibIndex[i].tag)
          {
             return True;
          }
       }
       return False;
       
    default:
       return False;
    }
    return True;
 }
 
 void snmp_getPdu(SNMP_PKT_T *pktp, VB_T *vbp, int value)
 {
    int     size;
    char*   str;
    
    switch (VB_TYPE(vbp))
    {
    case MTYPE_STRING:
    case MTYPE_TSTRING:
       str = (char *) value;
       if (str == NULL)
       { 
          size = 0;
       }
       else 
       {
          size = min (strlen(str), SIZE_DisplayString);
       }
       getproc_got_string(pktp, vbp, size , str, 0, VT_STRING);
       break;
       
    case MTYPE_IPADDR:
    case MTYPE_TIPADDR:
       getproc_got_ip_address(pktp, vbp, value);
       break;
       
    case MTYPE_NUMBER:
    case MTYPE_TNUMBER:
       getproc_got_int32(pktp, vbp, value);
       break;
       
    default:
       getproc_error(pktp, vbp, INCONSISTENT_NAME);
       DMSG("get method", printf("snmp_getPdu: 6: Bad tag!!! Setting getproc_error(..., INCONSISTENT_NAME)\n"));
       return;
       
    } /* switch */
    
    getproc_good(pktp, vbp);
    DMSG("get method", printf("snmp_getPdu: 7: setting getproc_good(...)\n"));
    
 }


 /*  SNMP PDU Processing method routines.
 *
 *   These routines are used by the SNMP agent to retrieve values from the
 *   MIB. The started from the -skel output from MIBCOMP. The are used to
 *   implement the Get/Set processing of the PDU's. There are 4 basic method
 *   routines that we implement.
 *
 *   snmp_getMethod()    Used to get an item from the MIB
 *   snmp_setMethod()    USed to set an item in the MIB
 *   snmp_testMethod()   Used to test the value of a MIB variable.
 *   snmp_nextMethod()   Used to get the next item from the MIB
 */
 
 
 
 void snmp_getMethod (
    OIDC_T      lastmatch,
    int         compc,
    OIDC_T     *compl,
    SNMP_PKT_T *pktp,
    VB_T       *vbp)

 {
    int     tag, ttag, value, idx = 0;
    Table   *tbl;
        
    /* The get method routine is called to retrieve the current item. The
    * processing here is very similar to the next method, except that we
    * don't need to set the next instance information.
    */
    
    /* Although I have never seen it happen, the varbind list may have
    * multiple data items to process. Do them all.
    */

    for (snmpdGroupByGetprocAndInstance(pktp, vbp, compc, compl);
    vbp; vbp = vbp->vb_link) {
       tag = VB_TAG(vbp);
       DMSG("get method", printf("snmp_getMethod: Processing varbind for item %s\n", snmp_tagName(tag)));
       
       if (mibInitialized == False)
       {
          getproc_error(pktp, vbp, RESOURCE_UNAVAILABLE);
          DMSG("get method", printf("snmp_getMethod: MIB not initialized!!! Setting getproc_error(..., RESOURCE_UNAVAILABLE)\n"));
          continue;
       }
       if (compc && MTYPE_IS_SCALAR(VB_TYPE(vbp)))
       {
          /* Scalar */

          if (*compl)
          {               /* Scalars can only be single instances. */
             getproc_nosuchins(pktp, vbp);
             DMSG("get method", printf("snmp_getMethod: 2: setting nextproc_nosuchins(...)\n"));
             continue;                   /* are in the MIB. */
          }
       }
       else if (compc)
       {             /* Tabular entry ? */
          idx = *compl;
          ttag = MIB_TABLE(VB_TBL(vbp), VB_ITM(vbp));  /* Get the table pointer. */
          tbl = (Table *)snmp_getValue(ttag);     /* So that we can tell if */
          if (!VALID_TABLE(tbl))
          {
             DMSG("get method", printf("WARNING!! Invalid table pointer!!\n"));
             getproc_nosuchins(pktp, vbp);       /* nope. */
             DMSG("get method", printf("snmp_getMethod: 3: Setting getproc_nosuchins(...)\n"));
             continue;
          }
          
          if (idx >= table_end(tbl)) 
          {     /* Is entry there? */
             DMSG("get method", printf("snmp_getMethod: Instance %d too big. Table has %d entries.\n", idx, table_end(tbl)));
             getproc_nosuchins(pktp, vbp);       /* nope. */
             DMSG("get method", printf("snmp_getMethod: 4: Setting getproc_nosuchins(...)\n"));
             continue;
          }
       }
       else 
       {     /* Scalar entry with compc = 0 */
          getproc_nosuchins(pktp, vbp);
          DMSG("get method", printf("snmp_getMethod: 5: Setting getproc_nosuchins(...)\n"));
          continue;
       }
       
       if (snmp_validTag(tag) == False) 
       {
          getproc_error(pktp, vbp, NO_SUCH_NAME);
          DMSG("get method", printf("snmp_getMethod: 6: Bad tag!!! Setting getproc_error(..., NON_SUCH_NAME)\n"));
          continue;
       }
       value = snmp_getValue(tag, idx);
       DMSG("get method", printf("snmp_getMethod: Value of %s(tag=%#x,row=%d)=%d (%#x)\n", snmp_tagName(tag), tag, idx, value, value));
       snmp_getPdu(pktp, vbp, value);       
    }
 }
  
 void snmp_setMethod(
    OIDC_T        lastmatch,
    int           compc,
    OIDC_T*       compl,
    SNMP_PKT_T*   pktp,
    VB_T*         vbp)
 {
    int           tag, ttag, value, old, idx = 0;
    Table         *tbl;
    EBUFFER_T     *ebuf;
    char           *str;
    ValueType      typ           = VT_VALUE;
    static int*    pAccessFlag   = NULL;
    
    if (pAccessFlag == NULL)
    {
       if (NULL==(pAccessFlag = snmp_getAddress(MIB_mibAccessControlFlag)))
          return;
    }
    
    /* The set method routine is called to store a value for the current item.
    * The processing here is very similar to the get method, except that we
    * store data rather than retrieve it.
    */
    
    /* Although I have never seen it happen, the varbind list may have
    * multiple data items to process. Do them all.
    */

    for (snmpdGroupByGetprocAndInstance(pktp, vbp, compc, compl);  vbp; vbp = vbp->vb_link) 
    {
       tag = VB_TAG(vbp);
       DMSG("set method", printf("snmp_setMethod: %s(tag=%#x)\n", snmp_tagName(tag), tag));
       
       if (mibInitialized == False)
       {
          setproc_error(pktp, vbp, RESOURCE_UNAVAILABLE);
          DMSG("set method", printf("snmp_setMethod: 1a: MIB not initialized!!! Setting setproc_error(..., RESOURCE_UNAVAILABLE)\n"));
          continue;
          /* Note: This is temporary. Later we will add a value to the OAM table. */
       } 
       
       if ((*pAccessFlag == VAL_mibAccessControlFlag_false) && (tag != MIB_mibAccessControlFlag))
       {
          setproc_error(pktp, vbp, NO_ACCESS);
          DMSG("set method", printf("snmp_setMethod: MIB not enabled!!! Setting setproc_error(..., NO_ACCESS)\n"));
          continue;
       } 
       
       if (compc && MTYPE_IS_SCALAR(VB_TYPE(vbp))) 
       {
          if (*compl) {               /* Scalar entry ? */
             setproc_error(pktp, vbp, NO_SUCH_NAME); /* Yes, only single instances */
             DMSG("set method", printf("snmp_setMethod: 2: setting settproc_error(..., NO_SUCH_NAME)\n"));
             continue;                   /* are in the MIB. */
          }
       }
       else if (compc) 
       {             /* Tabular entry ? */
          idx = *compl;
          ttag = MIB_TABLE(VB_TBL(vbp), VB_ITM(vbp));  /* Set the table pointer. */
          tbl = (Table *)snmp_getValue(ttag);     /* So that we can tell if */
          
          if (!VALID_TABLE(tbl))
          {
             DMSG("set method", printf("WARNING!! Invalid table pointer!!\n"));
             setproc_error(pktp, vbp, INCONSISTENT_NAME);     /* nope. */
             DMSG("set method", printf("snmp_setMethod: 3a: Setting setproc_error(..., INCONSISTENT_NAME)\n"));
             continue;
          }

          if (idx >= table_end(tbl))
          {         /* Is entry there? */
             DMSG("set method", printf("snmp_setMethod: 3b: Instance %d too big. Table has %d entries.\n", idx, table_end(tbl)));
             setproc_error(pktp, vbp, NO_SUCH_NAME);     /* nope. */
             DMSG("set method", printf("snmp_setMethod: 4: Setting setproc_error(..., NO_SUCH_NAME)\n"));
             continue;
          }

       }
       else
       {                /* Scalar entry with compc = 0 */
          setproc_error(pktp, vbp, NO_SUCH_NAME);
          DMSG("set method", printf("snmp_setMethod: 5: Setting setproc_error(..., NO_SUCH_NAME)\n"));
          continue;
       }

       if (snmp_validTag(tag) == False) {
          setproc_error(pktp, vbp, NO_SUCH_NAME);
          DMSG("set method", printf("snmp_setMethod: 6: Bad tag!!! Setting setproc_error(..., NON_SUCH_NAME)\n"));
          continue;
       }
       
       switch (VB_TYPE(vbp)) 
       {
       case MTYPE_TNUMBER:
       case MTYPE_TIPADDR:

          typ = VT_TABLE;

       case MTYPE_NUMBER:
       case MTYPE_IPADDR:
  
 
          old = snmp_getValue(tag, idx);
          value = (int) VB_GET_INT32(vbp);
          
          if (value == old) 
          {    /* Same as old value, say set was good, but bypass actual saving */
             DMSG("set method", printf("snmp_setMethod: 6a: Suppressing set for '%s'. Old value = %d, New value = %d\n",
                snmp_tagName(tag), old, value));
             setproc_good(pktp, vbp);
             continue;
          }
          else
          {
             DMSG("set method", printf("snmp_setMethod: 7: Setting number %s(%d) = %d (%#x)\n", snmp_tagName(tag), idx, value, value));
             
             snmp_setEntry(tag, idx, (void *) value, 4);
             setproc_good(pktp, vbp);
          }
          break;

       case MTYPE_STRING:
       case MTYPE_TSTRING:

          ebuf = VB_GET_STRING(vbp);
          str = (char *) EBufferStart(ebuf);
          snmp_setEntry(tag, idx, str, EBufferUsed(ebuf));
          typ = VT_ADDRESS;       /* Setup for the processTrap call below */
          value = (int)snmp_getValue(tag, idx);
          DMSG("set method", printf("snmp_setMethod: 8a: Setting string %s(%d) ='%s'\n", snmp_tagName(tag), idx, (char *)value));
         break;
       
       
       default:    /* Only structural elements are left.. The manager can't write them. */
          setproc_error(pktp, vbp, NOT_WRITABLE);
          DMSG("set method", printf("snmp_setMethod: 10: Bad tag!!! Setting setproc_error(..., NOT_WRITABLE)\n"));
          continue;
          
       } /* Switch */
       DMSG("set method", printf("snmp_setMethod: 11: setting setproc_good(...)\n"));
       setproc_good(pktp, vbp);
       
       DMSG("set method", printf("snmp_setMethod: 12: calling processTrap(%d, %s, %d, %d)\n",
          SNMP_AGENT_MODULE_ID, snmp_tagName(tag), typ, value));
       oam_processTrap(SNMP_AGENT_MODULE_ID, tag, typ, value, idx);
    } /* for */
}



void snmp_nextMethod (
                      OIDC_T      lastmatch,
                      int         compc,
                      OIDC_T     *compl,
                      SNMP_PKT_T *pktp,
                      VB_T       *vbp)
{
   OIDC_T  next = 0;
   int     tag, ttag, value, idx = 0;
   Table   *tbl;
   

   /* The next method routine is called to retrieve the current item
   * and to set inforamation necessary to this routine the next time.
   * We are passed the OID of the last thing that was recovered. We
   * step to the next instance of the variable and return it.
   */
   /* Although I have never seen it happen, the varbind list may have
   * multiple data items to gather. Process all of them.
   */
   for (snmpdGroupByGetprocAndInstance(pktp, vbp, compc, compl); vbp; vbp = vbp->vb_link)
   {
      tag = VB_TAG(vbp);
      DMSG("next method", printf("snmp_nextMethod: Processing varbind for item %s\n", snmp_tagName(tag)));
      
      
      if (mibInitialized == False)
      {
         nextproc_error(pktp, vbp, RESOURCE_UNAVAILABLE);
         DMSG("next method", printf("snmp_getMethod: 1: MIB not initialized!!! Setting getproc_error(..., RESOURCE_UNAVAILABLE)\n"));
         continue;
      }
      if (compc)
      {
         if (MTYPE_IS_SCALAR(VB_TYPE(vbp)))
         {
            /* compc != 0 and scalar */
            /* If this is a scalar type, then */
            /* there are no OID.n type of entries */
            nextproc_no_next(pktp, vbp);     
            DMSG("next method", printf("snmp_nextMethod: 1: setting nextproc_no_next(...)\n"));
            continue; 
         }
         else
         {
            /* compc != 0 and tabular */
            
            idx = *compl+1;             /* Step to the next entry. */
table_entry:        
            ttag = MIB_TABLE(VB_TBL(vbp), VB_ITM(vbp));  /* Get the table pointer. */
            tbl = (Table *)snmp_getValue(ttag);          /* So that we can tell if */
            DMSG("next method", printf("snmp_getValue returned 0x%x\n",(int)tbl));
            if (!VALID_TABLE(tbl))
            {
               DMSG("next method", printf("WARNING!! Invalid table pointer\n"));
            }
            else 
            {
               DMSG("next method", printf("Found valid table pointer\n"));
            }
            
            if (idx >= table_end(tbl))
            {     /* Is this item there? */
               DMSG("next method", printf("snmp_nextMethod: 2: Instance %d too big. Table has %d entries.\n", idx, table_end(tbl)));
               nextproc_no_next(pktp, vbp);
               DMSG("next method", printf("snmp_nextMethod: 3: setting nextproc_no_next(...)\n"));
               continue;
            }
            next = idx;
            nextproc_next_instance(pktp, vbp, 1, &next);
            DMSG("next method", printf("snmp_nextMethod: 4: Setting nextproc_next_instance(..., %d)\n", (int)next));
         }
      }
      else
      {
         if (MTYPE_IS_TABLE(VB_TYPE(vbp)))
         { 
            /* compc == 0, tabular entry */
            DMSG("next method", printf("snmp_nextMethod: 5: Tabular entry with compc == 0. Processing....\n"));
            idx = 0;
            goto table_entry;    /* Yes, handle it. */
         }
         else
         {
            /* compc == 0, scalar entry */
            next = 0;         
            nextproc_next_instance(pktp, vbp, 1, &next);
            DMSG("next method", printf("snmp_nextMethod: 6: setting nextproc_next_instance(..., %d)\n", (int)next));
         }
      }

      if (snmp_validTag(tag) == False)
      {
         nextproc_error(pktp, vbp, NO_SUCH_NAME);
         DMSG("next method", printf("snmp_nextMethod: 7: Bad tag!!! Setting nextproc_error(..., NO_SUCH_NAME)\n"));
      }
      else
      {
         value = snmp_getValue(tag, idx);
         DMSG("next method", printf("snmp_nextMethod: 8: Getting value for tag '0x%08x' '%s'=%d\n", tag, snmp_tagName(tag), value));
         snmp_getPdu(pktp, vbp, value);       
      }
   } /* for */
}


char * snmp_tagName(MibTag tag)
{
   static char buf[20];
   
   MnetMibDataType    *dtype;
   
   if (!(dtype = (MnetMibDataType *)table_find(mnetMibDataTypeTable, snmp_findTag, (void *)&tag))) {
      sprintf(buf, "Tag '0x%08x'", tag);
      return buf;
   }
   return dtype->name;
   
}

int snmp_nameToTag(char *name)
{
   char    c, buf[80], tag1[80], tag2[80];
   int     i, tag;
   
   /* Be a little clever about the name that is given. Copy it to the
   * the data buffer and put it in the form that we can use. We change
   * any - characters to the _ used in the MIB tag name. If the name
   * does not start with MIB_, we prepend it. We use strcasecmp to do
   * a case independent compare. Try to be as friendly as possible. */
   
   i = -1;
   while ((++i < 80) && (c = *name++))
   {
      if (c == '-') c = '_';  /* Convert the - to _. */
      buf[i] = c;             /* Store the character away. */
   }
   buf[i] = 0;                /* Terminate the name. */
   
   if (!strncmp(buf, "MIB", 3)) 
   {                          /* already have MIB_ in the */
      strcpy(tag1, buf);      /* name ? If so, assume the */
      tag2[0] = 0;            /* user knew what they were */
   }
   else
   {                                   /* typing. If not, they get */
      sprintf(tag1, "MIB_%s", buf);    /* an error message.*/
      sprintf(tag2, "MIBT_%s", buf);   /* 2 possibilities. */
   }
   
   DMSG("get Item", printf("snmp_nameToTag: Serching tag names: %s and %s\n", tag1, tag2));
   i = -1;
   while (mnetMibDataType[++i].tag)
   {
      if (!strcasecmp(tag1, mnetMibDataType[i].name) ||
         (*tag2 && !strcasecmp(tag2, mnetMibDataType[i].name)))
         break;
   }
   tag = mnetMibDataType[i].tag;
   return tag;   
}


int SnmpMibGet(char   *name, ...)
{
   int     tag, value, idx = 0;
   MibTableInfo    *mib;
   va_list argn;
   

   if (tag = snmp_nameToTag(name))
   {
      if (MTYPE_IS_TABLE(MIB_TYP(tag)))
      {
         va_start(argn, name);           /* Entry or an element out of the entry.  */
         idx = va_arg(argn, int);
         va_end(argn);
      }

      value = snmp_getValue(tag, idx);
      switch (MIB_TYP(tag)) 
      {
      case MTYPE_STRUCT:
         mib = (MibTableInfo *)value;
         printf("%s = (0x%08x). Size=%d, dirty=%s, lock=%s, set=%s, get=%s\n",
            snmp_tagName(tag), value, MIB_SIZ(tag), mib->info.dirty?"True":"False", mib->info.lock ? "True":"False",
            mib->info.getTrap?"True":"False", mib->info.setTrap?"True":"False");
         break;

      case MTYPE_NUMBER:
      case MTYPE_TNUMBER:
         printf("%s.%d = \"%d\" (0x%08x)\n", snmp_tagName(tag), idx, value, value);
         break;

      case MTYPE_STRING:
      case MTYPE_TSTRING:
         printf("%s.%d = \"%s\"\n", snmp_tagName(tag), idx, value, (char *)value);
         break;

      case MTYPE_IPADDR:
      case MTYPE_TIPADDR:
         printf("%s.%d = \"%s\" (%#x)\n", snmp_tagName(tag), idx, int2ip(value), value);
         break;

      case MTYPE_TENTRY:
         printf("%s.%d = (0x%08x). Table entry is %d bytes long\n", snmp_tagName(tag), idx, value, MIB_SIZ(tag));
         break;

      default:
         printf("Unknown Mib Type=%d\n", MIB_TYP(tag));

      } /* switch */
   }
   else 
   {
      printf("Unable to locate MIB tag from supplied mib string '%s'\n", name);
   }
   return 0;
}

int SnmpMibSet(char *name, ...)
{
   int     tag, value, idx = 0;
   char* str;
   Status status = Failure;
   
   va_list argn;   
   va_start(argn, name);           /* Entry or an element out of the entry.  */
      
   if (tag = snmp_nameToTag(name))
   {
      if (MTYPE_IS_TABLE(MIB_TYP(tag)))
      {
         idx = va_arg(argn, int);
      }
      
      switch (MIB_TYP(tag)) 
      {
         
      case MTYPE_NUMBER:
      case MTYPE_TNUMBER:
         value = va_arg(argn, int);
         status = snmp_setEntry(tag, idx, (void *) value, 4);
         break;
         
         
      case MTYPE_STRING:
      case MTYPE_TSTRING:
         str = va_arg(argn, char *);
         status = snmp_setEntry(tag, idx, str, strlen(str));         
         break;
         
      case MTYPE_IPADDR:
      case MTYPE_TIPADDR:
         str = va_arg(argn, char *);
         status = snmp_setEntry(tag, idx, (void *) inet_addr(str), 4);
         break;
         
      default:
         printf("Unknown or not allowed Mib Type=%d\n", MIB_TYP(tag));
         
      } /* switch */
   }
   else 
   {
      printf("Unable to locate MIB tag from supplied mib string '%s'\n", name);
   }
   
   if (Success == status)
   {
      printf("Snmp Set was successful\n");
   }

   va_end(argn);
   return 0;
}

#define SIZE_MibTagName    128
int SnmpMibLookup(char *name)
{
   char     buf[SIZE_MibTagName+1];
   char     pszTagName[SIZE_MibTagName+1];
   int     i, j, k;
   char c;

   if (NULL == name)
   {
      return ERROR;
   }

   k = min (SIZE_MibTagName, strlen(name));
   
   for (i=0; i <k; i++)
   {
      if (c == '-')
      {
         c = '_';           
      }
      buf[i] = toupper(name[i]); 
   }
   buf[k] = 0; 

   DMSG("lookup", printf("SnmpMibLookup: Case insensetive search \'%s\'\n", buf));

   i = 0;
   while (mnetMibDataType[i].tag)
   {
      k = min(SIZE_MibTagName, strlen(mnetMibDataType[i].name));

      for (j=0; j< k; j++)
      {
         pszTagName[j] = toupper(mnetMibDataType[i].name[j]);
      }
      pszTagName[k] = 0;

      if (strstr(pszTagName, buf))
      {
         printf("%s ( %#x = %d )\n", mnetMibDataType[i].name,  mnetMibDataType[i].tag,  mnetMibDataType[i].tag);
      }
      i++;
   }
   return 0;
}




#ifdef DEBUG
void
snmp_printPDU(
              SNMP_PKT_T *pktp,
              char       *where)
{
   VB_T    *vbp;
   MIBLEAF_T   *ml;    
   int     i;
   
#ifdef  LONG_PDU_PRINTOUT
   printf("PDU Type='%s', length=%d, view=%d, flags='%x', phase='%x', max pkt=%d\n", snmp_printPduType(pktp->pdu_type),
      pktp->pdu_length, pktp->mib_view, pktp->flags, pktp->phase, pktp->maxpkt);
   printf("    cookie='%x', version=%d, view idx=%d, ident=%d, request id=%d\n", pktp->async_cookie,
      pktp->snmp_version, pktp->view_index, pktp->lcl_ident, pktp->pdu.std_pdu.request_id);
   printf("    error status=%d, error index=%d, max reps=%d, non reps=%d\n",
      pktp->pdu.std_pdu.error_status, pktp->pdu.std_pdu.error_index,
      pktp->pdu.std_pdu.max_reps, pktp->pdu.std_pdu.non_reps);
   printf("    Std VBL_T: length=%d, count=%d, VBL_S='%x'\n", pktp->pdu.std_pdu.std_vbl.vbl_length,
      pktp->pdu.std_pdu.std_vbl.vbl_count, pktp->pdu.std_pdu.std_vbl.vblp);
   vbp = pktp->pdu.std_pdu.std_vbl.vblist;
   for (; vbp; vbp = vbp->vb_link) {
      printf("               objid='%x', num components=%d, component list=[", vbp->vb_obj_id,
         vbp->vb_obj_id.num_components);
      i = -1;
      while (++i < vbp->vb_obj_id.num_components)
         printf(i?", %d":"%d", vbp->vb_obj_id.component_list[i]);
      printf("]\n");
      
      printf("               seq size=%d, data flags n type='%x', flags='%x', data length=%d\n",
         vbp->vb_obj_id, vbp->vb_seq_size, vbp->vb_data_flags_n_type, vbp->vb_flags, vbp->vb_data_length);
      ml = vbp->vb_ml.ml_leaf;
      printf("               Mib Leaf for %s, view mask='%x', write mask='%x':\n", snmp_tagName((int)ml->user_cookie),
         ml->view_mask, ml->write_mask);
      printf("                       expected tag=%d, access type=%d, cookie='%x', locator=%d\n",
         ml->expected_tag, ml->access_type, ml->user_cookie, ml->locator);
      printf("                       testproc='%x', getproc='%x', setproc='%x', nextproc='%x'\n",
         ml->testproc, ml->getproc, ml->setproc, ml->nextproc);
   }
   printf("    Saved VBL_T: length=%d, count=%d, VBL_S='%x'\n", pktp->pdu.std_pdu.saved_vbl.vbl_length,
      pktp->pdu.std_pdu.saved_vbl.vbl_count, pktp->pdu.std_pdu.saved_vbl.vblp);
   vbp = pktp->pdu.std_pdu.saved_vbl.vblist;
   for (; vbp; vbp = vbp->vb_link) {
      printf("               objid='%x' seq size=%d, data flags n type='%x', flags='%x', data length=%d\n",
         vbp->vb_obj_id, vbp->vb_seq_size, vbp->vb_data_flags_n_type, vbp->vb_flags, vbp->vb_data_length);
      ml = vbp->vb_ml.ml_leaf;
      printf("               Mib Leaf for %s, view mask='%x', write mask='%x':\n", snmp_tagName(ml->user_cookie),
         ml->view_mask, ml->write_mask);
      printf("                       expected tag=%d, access type=%d, cookie='%x', locator=%d\n",
         ml->expected_tag, ml->access_type, ml->user_cookie, ml->locator);
      printf("                       testproc='%x', getproc='%x', setproc='%x', nextproc='%x'\n",
         ml->testproc, ml->getproc, ml->setproc, ml->nextproc);
   }
#else
   printf("%s: PDU Type='%s', length=%d, request id=%d, VBL_T length=%d, count=%d\n", where,
      snmp_printPduType(pktp->pdu_type), pktp->pdu_length, (int)pktp->pdu.std_pdu.request_id,
      pktp->pdu.std_pdu.std_vbl.vbl_length, pktp->pdu.std_pdu.std_vbl.vbl_count);
   vbp = pktp->pdu.std_pdu.std_vbl.vblist;
   if (pktp->pdu_length > 512 || pktp->pdu.std_pdu.std_vbl.vbl_count > 10)
      return;
   else for (; vbp; vbp = vbp->vb_link) {
      ml = vbp->vb_ml.ml_leaf;
      printf("    VB_T tag='0x%08x' %s, oid=", (int)ml->user_cookie,  snmp_tagName((int)ml->user_cookie));
      if (vbp->vb_obj_id.num_components < 20) {
         i = -1;
         while (++i < vbp->vb_obj_id.num_components)
            printf(i?".%d":"%d", vbp->vb_obj_id.component_list[i]);
         printf("\n");
      } else printf("invalid OID contains %d components\n", vbp->vb_obj_id.num_components);
   }
#endif
}

char *
snmp_printPduType(op)
{
   static char buf[20];
   
   switch (op) {
   case    GET_REQUEST_PDU:    return "GET_REQUEST";
   case    GET_NEXT_REQUEST_PDU:   return "GET_NEXT_REQUEST";
   case    GET_RESPONSE_PDU:   return "GET_RESPONSE";
   case    SET_REQUEST_PDU:    return "SET_REQUEST";
   case    TRAP_PDU:       return "TRAP";
   case    GET_BULK_REQUEST_PDU:   return "GET_BULK_REQUEST";
   case    INFORM_REQUEST_PDU: return "INFORM_REQUEST";
   case    TRAP2_PDU:      return "TRAP2";
   case    REPORT_PDU:     return "REPORT";
   default:            sprintf(buf, "%d", op);
      return buf;
   }
}

#endif
