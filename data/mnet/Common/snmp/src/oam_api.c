/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************
**                          
**  FILE NAME: oam_api.c   
**                      
**  DESCRIPTION: This file contains implementation of OAM API routines
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
**  Bhawani   |05/11/99| Initial Draft
**  Bhawani   |12/29/00| resturctured to share among GP, GMS and GS   
**  Bhawani   |02/28/01| Change to use unify mib change part
**  Bhawani   |04/10/01| Added oam_getMnc routine
**  ----------+--------+----------------------------------------------
**********************************************************************
*/

/*
**********************************************************************
** Include Files. 
**********************************************************************
*/
#include "oam_api.h"
#include "vipermib.h"		/* Get the ViperCell MIB definitions.*/
#include "Table.h"
#include "vipertbl.h"		/* Thi&Sandeep */


/*
*********************************************************************
** Global Variables / Functions
*********************************************************************/

/********************************************************************
** Static Variables 
*********************************************************************/

/*
 * gets address of a single MIB tag. Index is optional 
 * parameter if the tag is table entry of filed.
*/

void *oam_getMibAddress(MibTag tag, ...)
{
   void     *retval = NULL;
   va_list  argn;
   int      idx = 0;
   int      typ = MIB_TYP(tag);
   
   
   if (True == snmp_validTag(tag))
   {
      if (MTYPE_IS_TABLE(typ))
      {
         va_start(argn, tag);  
         idx = va_arg(argn, int);
         va_end(argn);
      }
      
      retval = snmp_getAddress(tag, idx);
      if ((NULL != retval) && (typ== MTYPE_STRING || typ == MTYPE_TSTRING || typ== MTYPE_TABLE))
      {
         retval = *((void **)retval);
      }
   }
   return retval;
} 

/*
** This function stores address of the MIB variable to the out
** parameters supplied.
*/
STATUS oam_getMibVarAdr(MibTag varTag, void ** mibVarAdr)
{
   static char fname[] = "oam_getMibVarAdr";
   
   if (NULL == mibVarAdr)
   {
      LOG_ERROR(("Invalid output parameter(mibVarAdr = NULL)"));
      return STATUS_INVALID_PARAMS;
   }
   
   *mibVarAdr = oam_getMibAddress(varTag);
   
   if (*mibVarAdr == NULL)
   {
      LOG_ERROR(("Invalid Tag (varTag = %#x)", varTag));
      return STATUS_INVALID_TAG;
   } 
   return STATUS_OK;
}


/* get scalar integer variable  and IP address */
STATUS oam_getMibIntVar(MibTag varTag, INT_32_T *varPtr)
{
   INT_32_T*   adr;
   static char fname[] = "oam_getMibIntVar";

   adr = oam_getMibAddress(varTag);
   
   if (NULL == adr)
   {
      LOG_ERROR(("Invalid Tag (varTag = %#x)", varTag));
      return STATUS_INVALID_TAG;
   }
  
   if (!MTYPE_IS_INT(MIB_TYP(varTag)))
   {
      LOG_ERROR(("Invalid scalar number Tag (varTag = %#x)", varTag));
      return STATUS_INVALID_TAG;
   }
   
      
   if (NULL == varPtr)
   {
      LOG_ERROR(("Invalid output parameter (varPtr = NULL)"));
      return STATUS_INVALID_PARAMS;
   }

   *varPtr =  *adr;
   
   return STATUS_OK;   
}


/* get scalar string variable example tag: MIB_viperCellName */

STATUS oam_getMibByteAry(MibTag aryTag, OCTET_T *buffer, INT_32_T size)
{
   
   char *pStr;
   static char fname[] = "oam_getMibByteAry";
   
   pStr = oam_getMibAddress(aryTag);
   
   if (NULL == pStr)
   {
      LOG_ERROR(("Invalid Tag (aryTag = %#x)", aryTag));
      return STATUS_INVALID_TAG;
   }
   
   if (!MTYPE_IS_STR(MIB_TYP(aryTag)))
   {
      LOG_ERROR(("Invalid scalar string Tag (aryTag = %#x)", aryTag));
      return STATUS_INVALID_TAG;
   }
   
   if (NULL == buffer || size <= 0 )
   {
      LOG_ERROR(("Invalid Output parameters (buffer = %#x, size=%d)", aryTag, size));
      return STATUS_INVALID_PARAMS;
   }
   
   strncpy(buffer, pStr, size);
   return STATUS_OK;
}

/* 
** Returns the struct type of MIB variable
*/

STATUS oam_getMibStruct(MibTag structTag, OCTET_T * buffer, int size)

{
   int		typ = MIB_TYP(structTag);
   int		sz =  MIB_SIZ(structTag);
   OCTET_T* addr;
   static char fname[] = "oam_getMibStruct";
   
   addr = oam_getMibAddress(structTag);
   
   if (NULL == addr)
   {
      LOG_ERROR(("Invalid Tag (structTag = %#x)", structTag));
      return STATUS_INVALID_TAG;
   }
   
   if (typ != MTYPE_STRUCT )
   {
      LOG_ERROR(("Invalid struct type Tag (structTag = %#x)", structTag));
      return STATUS_INVALID_TAG;
   }
   
   
   if (buffer == NULL || size < sz)
   {
      
      LOG_ERROR(("Invalid Parameters (tag= %s, buffer = %#x, size = %d)",
         snmp_tagName(structTag), buffer, size ));
      return STATUS_INVALID_PARAMS;
   }
   
   memcpy(buffer, addr, sz);
   
   return STATUS_OK;
}


/* 
 * returns size,in bytes, of the Table entry.
 * returns STATUS_INVALID_TAG (-1) if the tag is invalid
 * tag is Table tag eg: MIB_trapFwdTable
*/

int oam_getMibTblEntSz(MibTag tag)
{   
   
   Table*   tbl;
   static char fname[] = "oam_getMibTblEntSz";
   
   if (MIB_TYP(tag) == MTYPE_TENTRY)
   {
      tag = MIB_TABLE(MIB_TBL(tag), MIB_ITM(tag));
   }
   
   if (MIB_TYP(tag) != MTYPE_TABLE )
   {
      LOG_ERROR(("Invalid Table Tag (tag = %#x)", tag));
      return STATUS_INVALID_TAG;
   }
   
   tbl = (Table *) oam_getMibAddress(tag);
   
   if (tbl == NULL)
   {
      LOG_ERROR(("Invalid Tag (tag = %#x)", tag));
      return STATUS_INVALID_TAG;
   }
   
   return table_size(tbl);
}

/* 
* returns number of rows on the table.
* returns STATUS_INVALID_TAG (-1) if the tag is invalid
* tag is Table tag eg: MIB_trapFwdTable 
*/

int oam_getMibTblEntCount(MibTag tag)
{
   Table*   tbl;
   static char fname[] = "oam_getMibTblEntCount";
   
   if (MIB_TYP(tag) == MTYPE_TENTRY)
   {
      tag = MIB_TABLE(MIB_TBL(tag), MIB_ITM(tag));
   }
   
   if (MIB_TYP(tag) != MTYPE_TABLE )
   {
      LOG_ERROR(("Invalid Table Tag (tag = %#x)", tag));
      return STATUS_INVALID_TAG;
   }
   
   tbl = (Table *) oam_getMibAddress(tag);
   
   if (tbl == NULL)
   {
      LOG_ERROR(("Invalid Tag (tag = %#x)", tag));
      return STATUS_INVALID_TAG;
   }
   return table_end(tbl);
}



/*
 * get the entire table into the burffer supplied.
 * tag is Table tag eg: MIB_trapFwdTable 
 */

STATUS oam_getMibTbl(MibTag tblTag, void *tblBuf, int bufSize)
{
   int		entCnt;
   int      entSz;
   Table*   tbl;

   static char fname[] = "oam_getMibTbl";
   
   if (MIB_TYP(tblTag) == MTYPE_TENTRY)
   {
      tblTag = MIB_TABLE(MIB_TBL(tblTag), MIB_ITM(tblTag));
   }

   if (MIB_TYP(tblTag) != MTYPE_TABLE )
   {
      LOG_ERROR(("Invalid Table Tag (tblTag = %#x)", tblTag));
      return STATUS_INVALID_TAG;
   }
   
   tbl = (Table *) oam_getMibAddress(tblTag);
   
   if (tbl == NULL)
   {
      LOG_ERROR(("Invalid Tag (tblTag = %#x)", tblTag));
      return STATUS_INVALID_TAG;
   }
      
   entCnt = table_end(tbl);
   entSz =  table_size(tbl);
   
   if (bufSize < entCnt*entSz)
   {
      LOG_ERROR(("Too small buffer(supplied size=%d, needed =%d)", 
         bufSize, entCnt*entSz));
      return STATUS_INVALID_PARAMS;
   }
   
   
   if (NULL == tblBuf)
   {
      LOG_ERROR(("Output parameter buffer is null"));
      return STATUS_INVALID_PARAMS;
   }
   
   memcpy(tblBuf, table_getRow(tbl, 0), entCnt*entSz);
   return STATUS_OK;
   
}


/* 
 * retrieves a Table Entry of the into the memory supplied 
 * Tag is Table Entry tag, eg: MIBT_trapFwdEntry
 */

STATUS oam_getMibTblEntry(MibTag tblEntryTag, int idx, void *buffer, int bufSize)
{
   char*    etyAddr;
   int      entSz = MIB_SIZ(tblEntryTag);
   int		typ =   MIB_TYP(tblEntryTag);
   
   static char fname[] = "oam_getMibTblEntry";
   
   etyAddr = oam_getMibAddress(tblEntryTag, idx);
   
   if (etyAddr == NULL)
   {
      LOG_ERROR(("Invalid Tag (tblEntryTag = %#x)", tblEntryTag));
      return STATUS_INVALID_TAG;
   }
   
   if (typ != MTYPE_TENTRY)
   {
      LOG_ERROR(("Invalid Table Entry Tag (tag = %#x)", tblEntryTag));
      return STATUS_INVALID_TAG;
   }

 
   if (bufSize < entSz)
   {
      LOG_ERROR(("The buffer not enough to fit the entry(supplied=%d, needed =%d)",
         bufSize, entSz));
      return STATUS_INVALID_PARAMS;
   }
   
   
   if (NULL == buffer)
   {
      LOG_ERROR(("Output parameter buffer is null"));
      return STATUS_INVALID_PARAMS;
   }
   
   memcpy(buffer, etyAddr, entSz);
   return STATUS_OK;
   
}


/* 
 * retrieves an individual field of integer type from 
 * the given row of a table.
 * Tag is Table Entry tag, eg: MIBT_trapPort, MIBT_trapIpAddress
 */

STATUS oam_getMibTblNumberField(MibTag colTag, int idx, INT_32_T *value)
{
   INT_32_T*    addr; /* Raju: fixing a bug */
   int		typ =   MIB_TYP(colTag);   
   static char fname[] = "oam_getMibTblNumberField";
   
   addr = (INT_32_T *)oam_getMibAddress(colTag, idx);
   
   if (NULL == addr)
   {
      LOG_ERROR(("Invalid Tag (colTag = %#x)", colTag));
      return STATUS_INVALID_TAG;
   }
   
   if (!(MTYPE_IS_TINT(typ)))
   {
      LOG_ERROR(("Invalid Table Number Item tag (tag = %#x)", colTag));
      return STATUS_INVALID_TAG;
   }
   

   if (NULL == value)
   {
      LOG_ERROR(("Invalid output parameter value=NULL"));
      return STATUS_INVALID_PARAMS;
   }
   
   *value = *addr;
   return STATUS_OK;
   
}


/* 
 * retrieves an individual field of string type from 
 * the given row of a table.
 * Tag is Table Entry tag, eg: MIBT_trapPort, MIBT_trapIpAddress
 */

STATUS oam_getMibTblStringField(MibTag tblStrTag, int idx, OCTET_T *buffer, INT_32_T size)
{
   char     *str;
   int		typ =   MIB_TYP(tblStrTag);   
   static char fname[] = "oam_getMibTblStringField";
   
   str = oam_getMibAddress(tblStrTag, idx);
   
   if (NULL == str)
   {
      LOG_ERROR(("Invalid Tag (tblStrTag = %#x)", tblStrTag));
      return STATUS_INVALID_TAG;
   }
   
   if (!(MTYPE_IS_TSTR(typ)))
   {
      LOG_ERROR(("Invalid table string tag(tblStrTag = %#x)", tblStrTag));
      return STATUS_INVALID_TAG;
   }
   
   if (NULL == buffer || size <= 0 )
   {
      LOG_ERROR(("Invalid output parameters (buffer = %#x, size=%d)", buffer, size));
      return STATUS_INVALID_PARAMS;
   }
   
   strncpy(buffer, str, size);
   return STATUS_OK;
}

#ifdef MNET_GP10

/* convert int to three digit mnc */
STATUS oam_getMncFromInt(int nMnc, UINT8 *pszMnc)
{

   int i = 0;
   if ( nMnc >= 100)
   {
      pszMnc[i++] = nMnc/100;
      pszMnc[i++] = (nMnc%100)/10;
      pszMnc[i++] = nMnc%10;
   }
   else 
   {
      pszMnc[i++] = nMnc/10;
      pszMnc[i++] = nMnc%10;
      pszMnc[i++] = 0x0F;
   }
   return STATUS_OK;

}

/* returns 3 bytes of MNC as an arrary */
STATUS oam_getMnc(UINT8 *pszMnc)
{
   INT_32_T mnc;
   int i;
   static char fname[] = "oam_getMnc";

   if (STATUS_OK != oam_getMibIntVar(MIB_bts_mnc, &mnc))
   {
      LOG_ERROR(("Not available(MIB_bts_mnc = %#x)", MIB_bts_mnc));
      return STATUS_NOT_FOUND;
   }
   return oam_getMncFromInt(mnc, pszMnc);
}


STATUS	oam_getMccMncByteAryByInt(INT_32_T intVal, char * byteAry)
{
	byteAry[2] = (UINT8) (intVal & 0xF);
	intVal >>= 4;
	byteAry[1] = (UINT8) (intVal & 0xF);
	intVal >>= 4;
	byteAry[0] = (UINT8) (intVal & 0xF);

   return STATUS_OK;
}


#endif // MNET_GP10

#ifdef MNET_GMC
/* 
*/

STATUS oam_getMncFromInt(int nMnc, UINT8 *pszMnc)
{
	pszMnc[2] = (UINT8) (nMnc & 0xF);
	nMnc = nMnc >> 4;
	pszMnc[1] = (UINT8) (nMnc & 0xF);
	nMnc = nMnc >> 4;
	pszMnc[0] = (UINT8) (nMnc & 0xF);

   return STATUS_OK;

}

STATUS oam_getMccFromInt(int nMcc, UINT8 *pszMcc)
{

	pszMcc[2] = (UINT8) (nMcc & 0xF);
	nMcc = nMcc >> 4;
	pszMcc[1] = (UINT8) (nMcc & 0xF);
	nMcc = nMcc >> 4;
	pszMcc[0] = (UINT8) (nMcc & 0xF);

   return STATUS_OK;

}


STATUS oam_getMnc(UINT8 *pszMnc)
{
   INT_32_T mnc;
   static char fname[] = "oam_getMnc";


   if (STATUS_OK != oam_getMibIntVar(MIB_mnetMNC, &mnc))
   {
      LOG_ERROR(("Not available(MIB_mnetMNC = %#x)", MIB_mnetMNC));
      return STATUS_NOT_FOUND;
   }
   return oam_getMncFromInt(mnc, pszMnc);
}

STATUS oam_getMcc(UINT8 *pszMcc)
{
   INT_32_T mcc;
     static char fname[] = "oam_getMnc";


   if (STATUS_OK != oam_getMibIntVar(MIB_mnetMCC, &mcc))
   {
      LOG_ERROR(("Not available(MIB_mnetMCC = %#x)", MIB_mnetMCC));
      return STATUS_NOT_FOUND;
   }
   return oam_getMccFromInt(mcc, pszMcc);
}

void testAdjMNC(int idx)
{
	UINT8 buf[3];

	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;

	printf("testAdjMNC: before buf = %d,%d,%d\n",buf[0],buf[1],buf[2]);

	oam_getAdjacentLocationAreaMNC(buf, idx);

	printf("testAdjMNC: after buf = %d,%d,%d\n",buf[0],buf[1],buf[2]);
}


STATUS oam_getAdjacentLocationAreaMNC(UINT8 *pszMnc, int idx)
{
   INT_32_T mnc;
     static char fname[] = "oam_getMnc";


   if (STATUS_OK != oam_getMibTblNumberField(MIBT_adjacentLocationAreaMNC, idx, &mnc))
   {
      LOG_ERROR(("Not available(MIBT_adjacentLocationAreaMNC = %#x)", MIBT_adjacentLocationAreaMNC));
      return STATUS_NOT_FOUND;
   }
  
   return oam_getMncFromInt(mnc, pszMnc);
}


STATUS oam_getAdjacentLocationAreaMCC(UINT8 *pszMcc, int idx)
{
   INT_32_T mcc;
     static char fname[] = "oam_getMnc";


   if (STATUS_OK != oam_getMibTblNumberField(MIBT_adjacentLocationAreaMCC, idx, &mcc))
   {
      LOG_ERROR(("Not available(MIBT_adjacentLocationAreaMCC = %#x)", MIBT_adjacentLocationAreaMCC));
      return STATUS_NOT_FOUND;
   }
   return oam_getMccFromInt(mcc, pszMcc);
}


STATUS msrn_addTblEntry(char *msrnValue)
{
	int i, msrnRowUsed;
	Boolean availSlotFound = FALSE;

	if (strlen(msrnValue) > SIZE_E164)	/* make sure msrn string does not exceed E164 size */
		return STATUS_INVALID_PARAMS;

	for (i = 0; i < MsrnTableLimit && !availSlotFound; i++)
	{
		if (oam_getMibTblNumberField(MIBT_msrnRowUsed, i, (INT_32_T *)&msrnRowUsed) != STATUS_OK)
			return STATUS_INVALID_PARAMS;
		if (msrnRowUsed == 0)
		{
			oam_setTblEntryField(SNMP_AGENT_MODULE_ID, MIBT_msrnRowUsed, i, 1);
			oam_setTblEntryStrField(SNMP_AGENT_MODULE_ID, MIBT_msrn, i, msrnValue, strlen(msrnValue)+1);
			availSlotFound = TRUE;
		}
	}
	if (!availSlotFound)
		return STATUS_NOT_FOUND;
	return STATUS_OK;
}

STATUS msrn_delTblEntry(char *msrnValue)
{
	int i;
	MsrnEntry msrnEntry;
	Boolean entryFound = FALSE;
	char *pMsrn;
	
	if (strlen(msrnValue) > SIZE_E164)	/* make sure msrn string does not exceed E164 size */
		return STATUS_INVALID_PARAMS;
	
	for (i = 0; i < MsrnTableLimit && !entryFound; i++)
	{
		memset(&msrnEntry, 0, sizeof(msrnEntry));
		if (oam_getMibTblEntry(MIBT_msrnEntry, i, &msrnEntry, sizeof(msrnEntry)) != STATUS_OK)
			return STATUS_INVALID_PARAMS;
		if (msrnEntry.msrnRowUsed != 0)
		{
			pMsrn = msrnEntry.msrn;
			
			if ((NULL != pMsrn) && (strncmp(msrnValue, pMsrn, strlen(pMsrn)) == 0))
			{
				/* entry found, mark slot as available */
				oam_setTblEntryField(SNMP_AGENT_MODULE_ID, MIBT_msrnRowUsed, i, 0);
				entryFound = TRUE;
			}
		}
	}
	if (!entryFound)
		return STATUS_NOT_FOUND;
	return STATUS_OK;
}

char *msrn_getNextEntry(int *counter)
{
	int i;
	MsrnEntry msrnEntry;
	char *pMsrn;

	for (i = *counter; i < MsrnTableLimit; i++)
	{
		memset(&msrnEntry, 0, sizeof(msrnEntry));
		if (oam_getMibTblEntry(MIBT_msrnEntry, i, &msrnEntry, sizeof(msrnEntry)) != STATUS_OK)
			return NULL;
		if (msrnEntry.msrnRowUsed != 0)
		{
			*counter = i + 1;
			pMsrn = msrnEntry.msrn;
			return pMsrn;
		}
	}
	return NULL;
}

void msrn_testFunction()
{
	char buffer[256];
	char *p = NULL;
	int i = 0;
	while (p = msrn_getNextEntry(&i))
	{
		strcpy(buffer, p); /* to test strcpy to a buffer */
		printf("***** %d ***** %s\n", i, buffer);
	}
}

#endif	/* MNET_GMC */


#ifdef MNET_GS

/*
This API expects pszAPN parameter in ASCII null terminated 
string format (Ex: "gprs.cisco.com").  Stores IP Address
in pIpAddr if operation is successful. Return value 
indicates the status of the operation.

This API first looks-up in the local table. If the lookup fails
and the DNS server is configured then performs DNS lookup.

Note that a call to this routine may be blocked 
for long time (up to 10 seconds or more if the DNS server 
is not responding). Caller should be prepared for such 
long delays.
*/
STATUS oam_getGgsnIpAddress(char *pszAPN, UINT32 *pIpAddr)
{
	Table	*tbl;
	int		j,end;
	char	*apnName;
	static char fname[] = "oam_getGgsnIpAddress";

	if ((pszAPN == NULL) || (pIpAddr == NULL))
	{
		LOG_ERROR(("Invalid arguments to the funcation call"));
		return STATUS_NOK;
	}

	/* First do a look-up in the Local APN table */

	if ((tbl = snmp_getTableAddress(MIBT_apnName))) 
    {
		end = table_end(tbl);
		for (j=0; j < end; j++)
		{
			apnName =  (char *) snmp_getValue(MIBT_apnName, j);
			if (apnName != NULL)
			{
				/* Since APN name is a hostname, hence it is case insensitive */
				if (strcasecmp(apnName, pszAPN) == 0)
				{
					return oam_getMibTblNumberField(MIBT_apnGgsnIpAddress,j,(INT_32_T*)pIpAddr);
				}
			}
		}
	}

	/* Since, local lookup failed now try the DNS lookup */
	/* To be completed */

	return STATUS_NOK;
}

void debugApnLookup(char *apnName)
{
	UINT32    ipaddr;

	if (apnName == NULL)
	{
		printf("Usage: debugApnLookup apnName\n");
		return;
	}
	if (oam_getGgsnIpAddress(apnName, &ipaddr) == STATUS_OK)
	{
		printf ("GGSN IP Address for APN Name \"%s\" is %s\n",apnName,int2ip(ipaddr));
	} else
	{
		printf("APN looup failed for \"%s\".\n",apnName);
	}
}

/*
Returns name of the default APN. Willl return NULL if
no default APN name is configured by the user. Return value
is null terminated. Format for the return value is ASCII
(Ex: "gprs.cisco.com")
*/
char * oam_getDefaultAPN()
{
	char	*name;

	name = (char *) snmp_getValue(MIB_sgsnDefApnName);
	if ((name == NULL) || (strlen(name) == 0))
	{
		return (char *)0;
	}
	return name;
}

#endif /* MNET_GS */

int oam_isSnmpEnabled()
{
	INT_32_T	status;

	if (STATUS_OK != oam_getMibIntVar(MIB_snmpServiceEnabled, &status))
	{
		/* if there is any error, assume SNMP is enabled */
		return 1;
	}

	return (int) status;
}

int oam_isHttpEnabled()
{
	INT_32_T	status;

	if (STATUS_OK != oam_getMibIntVar(MIB_httpServiceEnabled, &status))
	{
		/* if there is any error, assume HTTP is enabled */
		return 1;
	}

	return (int) status;
}

int oam_isFtpEnabled()
{
	INT_32_T	status;

	if (STATUS_OK != oam_getMibIntVar(MIB_ftpServiceEnabled, &status))
	{
		/* if there is any error, assume FTP is enabled */
		return 1;
	}

	return (int) status;
}

void oam_setSnmpEnabled(int status)
{
	int value = 1;

	if (status == 0)
		value = 0;

	snmp_setEntry(MIB_snmpServiceEnabled, 0, (void *) value, 4);
	agent_setSnmpEnabled(value);
}

void oam_setHttpEnabled(int status)
{
	int value = 1;

	if (status == 0)
		value = 0;

	snmp_setEntry(MIB_httpServiceEnabled, 0, (void *) value, 4);
}

void oam_setFtpEnabled(int status)
{
	int value = 1;

	if (status == 0)
		value = 0;

	snmp_setEntry(MIB_ftpServiceEnabled, 0, (void *) value, 4);
	
	/* No harm in calling ftpdDelete/ftpdInit even though
	   FTP server is not running/already running
	*/
	if (value == 0)
		ftpdDelete();
	else
		ftpdInit((FUNCPTR *)0, 0);
}

/******************************************************************
** API used to set MIB variables. 
******************************************************************/

/*
** The same set of API are used to set MIB variables by both agent
** and application modules. Source Module ID is passed so that the
** implementation knows the direction of message flow.
*/

/* sets a single mib variable.
 * tag is   */
STATUS oam_setMibIntVar(OCTET_T srcModuleId, MibTag varTag, INT_32_T varVal)
{
   static char fname[] = "oam_setMibIntVar";
   
   if (False == snmp_validTag(varTag))
   {
      LOG_ERROR(("Invalid Tag (varTag = %#x)", varTag));
      return STATUS_INVALID_TAG;
   }

   if (!MTYPE_IS_INT(MIB_TYP(varTag)))
   {
      LOG_ERROR(("Invalid Tag for integer type item (tag = %#x)", varTag));
      return STATUS_INVALID_TAG;
   }
   
   if (Success == snmp_setEntry(varTag, 0, (void *) varVal, 4))
   {
      oam_processTrap(srcModuleId, varTag, VT_VALUE, varVal, 0);
      return STATUS_OK;   
   }

   LOG_ERROR(("Invalid Parameters (varTag =%s(%#x), varVal = %d)", 
      snmp_tagName(varTag), varTag, varVal));
   
   return STATUS_NOK;
}



/* sets mib string
 * tag is a string: eg: MIB_viperCellName */
STATUS oam_setMibByteAry(OCTET_T srcModuleId, MibTag strTag, OCTET_T *byteAryPtr, INT_32_T byteCount)
{
   static char fname[] = "oam_setMibByteAry";
   
   if (False == snmp_validTag(strTag))
   {
      LOG_ERROR(("Invalid Tag (strTag = %#x)", strTag));
      return STATUS_INVALID_TAG;
   }
   
   if (!MTYPE_IS_STR(MIB_TYP(strTag)))
   {
      LOG_ERROR(("Invalid string tag (tag = %#x)", strTag));
      return STATUS_INVALID_TAG;
   }
   
   if (Success == snmp_setEntry(strTag, 0, (void *) byteAryPtr, byteCount))
   {
      oam_processTrap(srcModuleId, strTag, VT_ADDRESS, (int) oam_getMibAddress(strTag), 0);
      return STATUS_OK;   
   }    
   LOG_ERROR(("Invalid Parameters (tag =%s(%#x), byteAryPtr = %#x, byteCount=%d)", 
      snmp_tagName(strTag), strTag, byteAryPtr, byteCount));
   return STATUS_NOK;
   
}


/* 
 * sets an entry of a Table, an entry is actually is row of MIB table
 * tag is a table entry, eg: MIBT_trapFwdEntry
 */

STATUS oam_setMibTblEntry(OCTET_T srcModuleId, MibTag entryTag, int idx, void *tblEntBuf, int bufSize)
{   
   static char fname[] = "oam_getMibTblEntry";
   
   if (False == snmp_validTag(entryTag))
   {
      LOG_ERROR(("Invalid Tag (entryTag = %#x)", entryTag));
      return STATUS_INVALID_TAG;
   }
   
   if (MIB_TYP(entryTag) != MTYPE_TENTRY)
   {
      LOG_ERROR(("Invalid table entry tag (tag = %#x)", entryTag));
      return STATUS_INVALID_TAG;
   }
   
   if (Success == snmp_setEntry(entryTag, idx, tblEntBuf, bufSize))
   {
      oam_processTrap(srcModuleId, entryTag, VT_TABLE,
         (int) oam_getMibAddress(entryTag, idx), idx);
      return STATUS_OK;   
   }    
   LOG_ERROR(("Invalid Parameters (entryTag =%s(%#x), idx = %d, tblEntBuf = 0x%08p, bufSize=%d)", 
      snmp_tagName(entryTag), entryTag, idx, tblEntBuf, bufSize));
   return STATUS_NOK;
}


/* 
** sets a individual number field of the given table
** The field is identified using the tag name, which is the column tag name
** and the row index of the table. The index start from 0.
** tag example: MIBT_trapPort, MIBT_trapIpAddress
*/

STATUS oam_setTblEntryField(OCTET_T srcModuleId, MibTag colTag, int idx, INT_32_T fieldVal)
{

   int typ = MIB_TYP(colTag);
   static char fname[] = "oam_setTblEntryField";
   
   if (False == snmp_validTag(colTag))
   {
      LOG_ERROR(("Invalid Tag (colTag = %#x)", colTag));
      return STATUS_INVALID_TAG;
   }
   
   if (!(MTYPE_IS_TINT(typ)))
   {
      LOG_ERROR(("Invalid table number tag %s(%#x)", snmp_tagName(colTag), colTag));
      return STATUS_INVALID_TAG;
   }


   if (Success == snmp_setEntry(colTag, idx, (void*) fieldVal, 4))
   {
      oam_processTrap(srcModuleId, colTag, VT_TABLE, fieldVal, idx);
      return STATUS_OK;   
   }    
   LOG_ERROR(("Invalid Parameters (colTag =%s(%#x), idx = %d, fieldVal=%d)", 
      snmp_tagName(colTag), colTag, idx, fieldVal));
   return STATUS_NOK;
   
}


/* 
** sets a individual string field of the given table
** tag is table string, which is the column tag name
** (eg MIBT_trapCommunity) and the row index of the table.
** The index start from 0.
 */

STATUS oam_setTblEntryStrField(OCTET_T srcModuleId, MibTag tstrTag, int idx, OCTET_T *tstr, INT_32_T byteCount)
{
   
   static char fname[] = "oam_setTblEntryField";
   
   if (False == snmp_validTag(tstrTag))
   {
      LOG_ERROR(("Invalid Tag (tstrTag = %#x)", tstrTag));
      return STATUS_INVALID_TAG;
   }
   
   if (MIB_TYP(tstrTag) != MTYPE_TSTRING)
   {
      LOG_ERROR(("Invalid table string tag (tag = %#x)", tstrTag));
      return STATUS_INVALID_TAG;
   }


   if (Success == snmp_setEntry(tstrTag, idx, (void*) tstr, byteCount))
   {
      oam_processTrap(srcModuleId, tstrTag, VT_TABLE,
         (int) oam_getMibAddress(tstrTag, idx), idx);
      return STATUS_OK;   
   }    
   LOG_ERROR(("Invalid Parameters (tstrTag =%s(%#x), idx = %d, tstr = %#x, byteCount=%d)",
      snmp_tagName(tstrTag), tstrTag, idx, tstr, byteCount));
   
   return STATUS_NOK;
   
}

/******************************************************************
** API used to subscribe trap 
******************************************************************/

/* 
* Applications interested in receiving notification
* when the value of certain MIB variables get changed can do so
* by calling following APIs.
*/

/* setting trap on a single MIB variable */
STATUS oam_setTrap (OCTET_T srcModuleId, MSG_Q_ID msgQid, MibTag mibTag)
{
   ListNodePtr nodePtr;
   int status = STATUS_NO_MEMORY;
   nodePtr = oam_newListNode(mibTag, msgQid);
   if (nodePtr != NULL)
   {
      nodePtr->mid = srcModuleId;
      oam_lockMutex();
      status = oam_insertNodeSortedByTag(oam_getTrapTableHead(),nodePtr);
      oam_unlockMutex();
   }
   return status;
   
}

STATUS setTrap (OCTET_T srcModuleId, MSG_Q_ID msgQid, MibTag mibTag)
{
   return oam_setTrap(srcModuleId, msgQid, mibTag);
}

/* setting trap on a multiple MIB variables */
STATUS oam_setMultipleTrap (OCTET_T srcModuleId, MSG_Q_ID msgQid, MibTag mibTag[], INT_32_T tagCount)
{
   ListNodePtr nodePtr;
   int i, status = STATUS_OK;
   oam_lockMutex();
   for (i= 0; i < tagCount && status == STATUS_OK; i++)
   {
      nodePtr = oam_newListNode(mibTag[i], msgQid);
      if (nodePtr != NULL)
      {
         nodePtr->mid = srcModuleId;
         status = oam_insertNodeSortedByTag(oam_getTrapTableHead(),nodePtr);
      } else {
         status = STATUS_NO_MEMORY;
      }
   }
   oam_unlockMutex();
   return STATUS_OK;
   
}

/* setting trap on a multiple MIB variables */
STATUS oam_unsetTrapByModule (OCTET_T srcModuleId)
{
   int status;
   oam_lockMutex();
   status= oam_deleteNodeByModule(oam_getTrapTableHead(), srcModuleId);
   oam_unlockMutex();
   return status;
   
}


/* setting trap on a multiple MIB variables */
STATUS oam_unsetTrap(OCTET_T srcModuleId, MibTag tag)
{
   int status;
   oam_lockMutex();
   status= oam_deleteNode(oam_getTrapTableHead(), tag, srcModuleId);
   oam_unlockMutex();
   return status;
   
}


/* sends the actual message through message queue */
static STATUS oam_trapSend(MSG_Q_ID qid, char * trapMsg)
{
   
   /* send a normal priority message, blocking if queue is full */
#if (defined(VXWORKS))
   int status;
   status = msgQSend (qid, trapMsg, sizeof (TrapMsg), WAIT_FOREVER,
      MSG_PRI_NORMAL);
   if (status == ERROR) 
   {
      
   /* We need to check here the followings:
   S_objLib_OBJ_ID_LOG_ERROR 
   -  msgQId is invalid.
   S_objLib_OBJ_DELETED 
   - the message queue was deleted while waiting to a send message.
      */
      return STATUS_MSGQ_SEND_ERROR;
   }
#endif
   
   return STATUS_OK;
}

STATUS oam_processTrap(OCTET_T srcMdlId, MibTag tag, OCTET_T valueType, INT_32_T newValue, int idx)
{
   
   TrapMsg trapMsg;
   ListNodePtr nodePtr;
   
   static char fname[] ="oam_processTrap";
   int prevModuleId;
   
   LOG_TRACE(("(module=%d, tag=%s, vt=%d, value=%#x, idx=%d)", srcMdlId, snmp_tagName(tag), valueType, newValue, idx));
   
   /* Poputlate the Trap Message */
   trapMsg.srcModuleId = MODULE_OAM;  /*  always MODULE_OAM */
   trapMsg.msgType = MT_TRAP;
   trapMsg.mibTag = tag;
   trapMsg.valType = valueType;
   
   
   switch(valueType)
   {
   case VT_VALUE:
      {
         trapMsg.val.varVal = newValue;
         break;
      }
   case VT_ADDRESS:
      {
         
         trapMsg.val.varAdr = (void *) newValue;
         break;
      }
   case VT_TABLE:
      {
         trapMsg.val.varTbl.idx = idx;
         trapMsg.val.varTbl.value = newValue;
         break;
      }
   default:
      LOG_ERROR(("Invalid value type: %d", valueType));
      return STATUS_INVALID_PARAMS;
      break;
   }
   
   /* The 2nd condition should only be evaluated if the 1st is True.
   * The condition inside the while loop below shoud be safe.
   */
   prevModuleId = -1;
   nodePtr = oam_getFirstNodeByTag(oam_getTrapTableHead(), tag);
   
   while(nodePtr != NULL && MIB_TBL(nodePtr->tag) == MIB_TBL(tag))
   {
      
      if (nodePtr->mid != srcMdlId  && prevModuleId != nodePtr->mid
         && (MIB_TYP(nodePtr->tag) == MTYPE_STRUCT  || nodePtr->tag == tag)) 
      {
         
      /* 
      * We should not trap the module which sets the variable.
         */
         if (oam_trapSend(nodePtr->qid, (char *) &trapMsg) != STATUS_OK)
         {
            LOG_ERROR(("Message Queue Send failure (mid = %d, qid = %#x", nodePtr->mid, nodePtr->qid));
            perror("Reason of Failure");
            
         }
         
         prevModuleId = nodePtr->mid;
         LOG_TRACE(("Sent Trap Message on tag  %s to %d module", snmp_tagName(tag), nodePtr->mid));
         
      }
      nodePtr = nodePtr->next;
   }
   return STATUS_OK;
}

/* this function returns byte arrary from a given integer value */

STATUS	oam_getByteAryByInt(INT_32_T intVal, char * byteAry, int size)
{
   int i;
   
   if ( intVal < 0 || byteAry == NULL || size <1 )
      return STATUS_INVALID_PARAMS;
   memset(byteAry, 0, size);
   for (i = size-1; i>=0 && intVal > 0; i--)
   {
      byteAry[i] = intVal%10;
      intVal = intVal/10;
   }
   return STATUS_OK;
}

/* Test Print Routine */
void oam_printTrapList()
{
   oam_printList(oam_getTrapTableHead());
}

/* oam message printing routine */
void oam_msgPrint(char *format, ...) 
{
   va_list marker;
   va_start( marker, format );     /* Initialize variable arguments. */
   vprintf(format, marker);
   printf("\n");
   fflush(stdout);
   va_end( marker );              /* Reset variable arguments.      */
   return;
   
}
   
