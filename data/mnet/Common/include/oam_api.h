/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* Version     : 1.0                                                 */
/* Status      : Under development                                   */
/* File        : oam_api.h                                           */
/* Author(s)   : Bhawani Sapkota                                     */
/* Create Date : 07/11/99                                            */
/* Description : This file contains Oam Api prototypes               */
/*                                                                   */
/*********************************************************************/
/*  REVISION HISTORY												 */
/*___________________________________________________________________*/
/*----------+--------+-----------------------------------------------*/
/* Name     |  Date  |  Reason                                       */
/*----------+--------+-----------------------------------------------*/
/* Bhawani  |09/28/00| Initial Draft                                 */
/* Bhawani  |11/20/00| resturctured to share among GP, GMS and GS    */
/*----------+--------+-----------------------------------------------*/
/*********************************************************************/

#ifndef OAM_API_H
#define OAM_API_H

/******************************************************************
 ** Include Files. 
 ******************************************************************/

#include <msgqLib.h>
#include "MnetModuleId.h"
#include "MibTags.h"
#include "oam_trap.h"
#include "asn1conf.h"


#ifndef True
#define True 1
#endif

#ifndef False
#define False 0
#endif


/******************************************************************
 ** Trap Message Format
 ******************************************************************/

typedef enum MsgType {
    MT_TRAP = 0, /* Currently only TRAP is used */
    MT_REQUEST,
    MT_REPLY,
    MT_ACK,
    MT_NOTIFICATION
}MsgType;


typedef enum ValueType{
    VT_VALUE = 0,   /* an ordinary integer value */
    VT_ADDRESS,     /* an address, in case of structure, byte array and table */
    VT_TABLE        /* an individual item of a table */
}ValueType;



/* This structure holds the value and index number of
 * a table field.
 */

typedef struct TblField_t
{
    OCTET_T idx;
    INT_32_T value;
}TableField;


/*  The following structure used to send trap from agent to an
 *  application. If the MIB tag being trapped is a non-scalar, the
 *  application should use get API to retrieve the structure
 *  or array. See  below
 */ 

typedef struct ipc_msg_t
{
    OCTET_T     srcModuleId;  /* source module ID (SNMP_AGENT_ID) */
    OCTET_T     msgType;      /* message type (MT_TRAP) */
    MibTag      mibTag;       /* mib tag */
    OCTET_T     valType;      /* value or address */
    union {
        INT_32_T    varVal;   /* actual value */   
        void *      varAdr;   /* address of the table or array */
        TableField  varTbl;   /* table field */
    }val;
}TrapMsg;


/******************************************************************
 ** API used to revirve MIB variables
 ******************************************************************/
/* The actual value  MODULE_OAM is defined in jcc/include/sys_const.h
 * file and its current value is 7
 */
#define STATUS_OK               0x00
#define STATUS_INVALID_TAG      -1
#define STATUS_INVALID_PARAMS   -2
#define STATUS_NO_MEMORY        -3
#define STATUS_NOT_FOUND        -4
#define STATUS_MSGQ_SEND_ERROR  -5
#define STATUS_NOK              -6 


#ifdef  __cplusplus
extern "C" {
#endif

/******************************************************************
 ** API used to retrive MIB variable address
 ******************************************************************/

/*
 * gets address of a single MIB tag. Index is optional 
 * parameter if the tag is table entry of filed.
 */

void *oam_getMibAddress(MibTag tag, ...);

/* get address of scalar mib variable */
STATUS oam_getMibVarAdr(MibTag varTag, void ** mibVarAdr);

/******************************************************************
 ** API used to retrive MIB variable address
 ******************************************************************/

/* gets value of a single MIB tag. The variable is a simple 4-byte integer */
STATUS oam_getMibIntVar(MibTag varTag, INT_32_T *varPtr);

/* get value of  MIB variable array. Number of elements to be
 * retrieved depends on the aryCount. It is also assumed that
 * the application has allocated enough memory to hold aryCount
 * elements at aryPtr.
 */


STATUS oam_getMibByteAry(MibTag aryTag, OCTET_T *byteAryPtr, INT_32_T byteCount);

/* get the entire structure.
 */
STATUS oam_getMibStruct(MibTag structTag, OCTET_T * structPtr, int structSize);

/* get value of a MIB table. Application allocates enough space to hold  entryCount
 * elements at tblPtr.
 */

/******************************************************************
 ** API used to manupulate MIB tables 
 ******************************************************************/

/* returns size,in bytes, of the table entry.
 * returns -1 if the tag is invalid 
 */
int oam_getMibTblEntSz(MibTag tblTag);

/* returns number of current entries in the table. 
 * returns -1 if the tag is invalid
 */
int oam_getMibTblEntCount(MibTag tblTag);

/* retrieves entire table into the memory supplied */
STATUS oam_getMibTbl(MibTag tblTag, void *tblBuf, int bufSize);

/* retrieves an entry of the table into the memory supplied */
 
STATUS oam_getMibTblEntry(MibTag tblTag, int idx, void *buffer, int bufSize);

/* get the address of an entry */
void *oam_getMibTblEntAdr(MibTag tblTag, int idx, int *entrySize);

/* get the address of an number and IP address type entry */
STATUS oam_getMibTblNumberField(MibTag colTag, int idx, INT_32_T *value);


/* get the address of an string type entry */
STATUS oam_getMibTblStringField(MibTag tblStrTag, int idx, OCTET_T *buffer, INT_32_T size);

#ifdef MNET_GP10

/* convert int to three digit mnc */
STATUS oam_getMncFromInt(int nMnc, UINT8 *pszMnc);

/* returns 3 bytes of MNC as an arrary */
STATUS oam_getMnc(UINT8 *pszMnc);

#endif /* MNET_GP10 */


#ifdef MNET_GMC

/* add new msrn value into next available slot in msrn table */
STATUS msrn_addTblEntry(char *msrnValue);

/* delete msrn entry from msrn table and mark slot as empty */
STATUS msrn_delTblEntry(char *msrnValue);

/* return next msrn value in table; NULL if end of table is reached */
char *msrn_getNextEntry(int *counter);

/* get MNC and fill into the specified byte array { array size must be 3 bytes } */
STATUS oam_getMnc(UINT8 *pszMnc);

/* get MCC and fill into the specified byte array { array size must be 3 bytes } */
STATUS oam_getMcc(UINT8 *pszMcc);

/* get MNC and fill into the specified byte array { array size must be 3 bytes } */
STATUS oam_getAdjacentLocationAreaMNC(UINT8 *pszMnc, int idx);

/* get MCC and fill into the specified byte array { array size must be 3 bytes } */
STATUS oam_getAdjacentLocationAreaMCC(UINT8 *pszMcc, int idx);


#endif /* MNET_GMC */


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
STATUS oam_getGgsnIpAddress(char *pszAPN, UINT32 *pIpAddr);

/*
Returns name of the default APN. Willl return NULL if
no default APN name is configured by the user. Return value
is null terminated. Format for the return value is ASCII
(Ex: "gprs.cisco.com")
*/
char * oam_getDefaultAPN();

#endif /* MNET_GS */
/******************************************************************
 ** API used to set MIB variables. 
 ******************************************************************/


/* The same set of API are used to set MIB variables by both agent
 * and application modules. Source Module ID is passed so that the
 * implementation knows the direction of message flow.
 */

/* sets a single mib variable. The variable is a  */
STATUS oam_setMibIntVar(OCTET_T srcModuleId, MibTag varTag, INT_32_T varVal);

/* sets aryCount elements of an array specified by aryTag */
STATUS oam_setMibByteAry(OCTET_T srcModuleId, MibTag aryTag, OCTET_T *ByteAryPtr, INT_32_T byteCount);

/* sets an entry of a Table, an entry is actually is row of MIB table */
STATUS oam_setMibTblEntry(OCTET_T srcModuleId, MibTag tblTag, int idx, void *tblEntBuf, int bufSize);

/* sets a field of the given entry table */
STATUS oam_setTblEntryField(OCTET_T srcModuleId, MibTag tblTag, int entNo, INT_32_T varVal);


/******************************************************************
 ** API used to subscribe  and unsubscribe trap 
 ******************************************************************/

/* 
 * Applications interested in receiving notification
 * when the value of certain managed MIB object get changed can do so
 * by calling following APIs.
 */


/* setting trap on a single MIB variable */
STATUS oam_setTrap (OCTET_T srcModuleId, MSG_Q_ID msgQid, MibTag mibTag);
STATUS setTrap (OCTET_T srcModuleId, MSG_Q_ID msgQid, MibTag mibTag);


/* setting trap on a multiple MIB variables */
STATUS oam_setMultipleTrap (OCTET_T srcModuleId, MSG_Q_ID msgQid, MibTag mibTag[], INT_32_T tagCount);

/* unsetting trap on a MIB variable */
STATUS oam_unsetTrap(OCTET_T srcModuleId, MibTag mibTag);

/* unsetting trap for all the variable for the module */
STATUS oam_unsetTrapByModule(OCTET_T srcModuleId);

/******************************************************************
 ** Misc Library
 ******************************************************************/

STATUS oam_getByteAryByInt(INT_32_T intVal, char * byteAry, int size);
STATUS oam_processTrap(OCTET_T srcMdlId, MibTag tag, OCTET_T valueType, INT_32_T newValue, int idx);


#ifdef  __cplusplus
}
#endif

/* to support older interfaces */
#define  getMibIntVar    oam_getMibIntVar
#define  getMibStruct    oam_getMibStruct
#define  getMibTbl	     oam_getMibTbl
#define  getByteAryByInt oam_getByteAryByInt


#endif /* OAM_API_H */
