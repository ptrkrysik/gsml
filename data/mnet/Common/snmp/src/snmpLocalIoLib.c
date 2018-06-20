/*
************************************************************************
*
* (c) Copyright Cisco 2000
* All Rights Reserved
*_______________________________________________________________________
*                          
*  FILE NAME: sndcpLocalIo.c
*                      
*  DESCRIPTION: This file contains snmp Local I/O routines that links 
*               WindSnmp Callback to our customized get, set, test, next
*               etc method handlers.
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
* Bhawani  |01/23/01| Initial Draft
*----------+--------+---------------------------------------------------
************************************************************************
*/

#include <vxWorks.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sockLib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ioLib.h>
#include <taskLib.h>
#include <snmpdLib.h>
#include <snmpIoLib.h>
#include <view.h>
#include <snmpstat.h>
#include <auxfuncs.h>
#include <hostLib.h>
#include <inetLib.h>
#include <buildpkt.h>
#include <objectid.h>
#include <m2Lib.h>
#include <semLib.h>
#include <subInstall.h>


#include <timers.h>
#include <selectLib.h>

#include "oam_api.h"
#include "vipermib.h"
#include "vipertbl.h"

#define  SNMP_PORT          161         /* port for snmp */
#define  TRAP_PORT          162         /* port for snmp traps */

/*
************************************************************************
* External variable defined snmpIoLib.c
************************************************************************
*/

extern      int		  snmpRemoteIP;
extern      int		  snmpRemotePort;
extern      int		  snmpProxyIP;


/* Snmp Call back routines */
typedef  int (*SnmpFuncPtr) (SNMP_PKT_T *pktp);
extern      SnmpFuncPtr pSnmpPrivateRelease;
extern      SnmpFuncPtr pSnmpSetPduValidate;
extern      SnmpFuncPtr pSnmpPreSetRoutine;
extern      SnmpFuncPtr pSnmpPostSetRoutine;
extern      SnmpFuncPtr pSnmpSetFailedRoutine;

/*  SNMP PDU Processing method routines */
typedef  void (*SnmpPduProsFuncPtr) (
                     OIDC_T      lastmatch,
                     int         compc,
                     OIDC_T     *compl,
                     SNMP_PKT_T *pktp,
                     VB_T       *vbp);

extern      SnmpPduProsFuncPtr pSnmpTestMethod;
extern      SnmpPduProsFuncPtr pSnmpGetMethod ;
extern      SnmpPduProsFuncPtr pSnmpNextMethod;
extern      SnmpPduProsFuncPtr pSnmpSetMethod;

typedef void (*SnmpCallbackFuncPtr) (int snmpSocket);

extern      SnmpCallbackFuncPtr pSnmpCallbackHandler;

/* struct for community to view index mapping */
typedef struct {
    char *     Cmnty;
    int        viewIndex;
} CMNTY_TO_VIEW_T;

static CMNTY_TO_VIEW_T getCmntyViewTbl[] =	{	{NULL, 1},
												{NULL, 1},
												{"icmp", 2},
												{NULL, 0}
											};

typedef int (*SnmpIoCommunityValidateFuncPtr) (
	SNMP_PKT_T *    pPkt,               /* ptr to snmp pkt */
    SNMPADDR_T *    pRemoteAddr,        /* remote address */
    SNMPADDR_T *    pLocalAddr          /* local address */
    );

extern SnmpIoCommunityValidateFuncPtr pSnmpLocalIoCommunityValidate;

int snmpLocalIoCommunityValidate(SNMP_PKT_T *pPkt, SNMPADDR_T *pRemoteAddr,
								 SNMPADDR_T *pLocalAddr);

/* Defined other places */
extern void  snmp_initializeMib();
extern    void snmp_getMethod (
                               OIDC_T      lastmatch,
                               int         compc,
                               OIDC_T     *compl,
                               SNMP_PKT_T *pktp,
                               VB_T       *vbp);

extern  void snmp_setMethod(
                            OIDC_T      lastmatch,
                            int         compc,
                            OIDC_T     *compl,
                            SNMP_PKT_T *pktp,
                            VB_T       *vbp);

extern  void snmp_nextMethod (
                              OIDC_T      lastmatch,
                              int         compc,
                              OIDC_T     *compl,
                              SNMP_PKT_T *pktp,
                              VB_T       *vbp);


extern  void snmp_testMethod (
                              OIDC_T      lastmatch,
                              int         compc,
                              OIDC_T     *compl,
                              SNMP_PKT_T *pktp,
                              VB_T       *vbp);
                              

/* gets string dot form IP from 32 bit unsighed integer */

char * IPDotAddr( int     addr)
{
   static   char  buf[20];
   unsigned char *ip = (unsigned char *) &addr;
   
   sprintf(buf, "%d.%d.%d.%d", ip[3], ip[2], ip[1], ip[0]);
   return buf;
}



/* Snmp User Private release routine */
int SnmpPrivateRelease ( SNMP_PKT_T *pktp)
{
   return 0;
};

/* Snmp Set PDU validate routine */
int SnmpSetPduValidate ( SNMP_PKT_T *pktp)
{
   return 0;
};


/* SNMP Pre set Routine */
int SnmpPreSetRoutine ( SNMP_PKT_T *pktp)
{
   return 0;
};


/* Snmp Set Failed Routine */
int SnmpSetFailedRoutine ( SNMP_PKT_T *pktp)
{
   return 0;
};

/* Snmp Local IO Init routine */
snmpLocalIoInit()
{
     /* Populate Call back routines */

     pSnmpPrivateRelease = SnmpPrivateRelease;
     pSnmpSetPduValidate = SnmpSetPduValidate;
     pSnmpPreSetRoutine = SnmpPreSetRoutine;
     pSnmpSetFailedRoutine = SnmpSetFailedRoutine;

     /* Populate PDU Processing routines */

     pSnmpTestMethod = snmp_testMethod;
     pSnmpGetMethod  = snmp_getMethod;
     pSnmpNextMethod = snmp_nextMethod;
     pSnmpSetMethod  = snmp_setMethod;
     
	 pSnmpLocalIoCommunityValidate = snmpLocalIoCommunityValidate;

}

void snmpLocalInitCommunityNames()
{

	/* community table for gets, NULL entry terminated */
	getCmntyViewTbl[0].Cmnty = oam_getMibAddress(MIB_readCommunity);
	getCmntyViewTbl[1].Cmnty = oam_getMibAddress(MIB_writeCommunity);

   if (strlen(getCmntyViewTbl[0].Cmnty) == 0)
   {
      strcpy(getCmntyViewTbl[0].Cmnty, "public");
   }

      if (strlen(getCmntyViewTbl[1].Cmnty) == 0)
   {
      strcpy(getCmntyViewTbl[1].Cmnty, "private");
   }
}


/*******************************************************************************
* snmpLocalIoCommunityValidate - community validation routine
* 
* This routine is used to set up the view-index field in the SNMP
* packet.  This product is shipped with defaults such that the "priv"
* community is allowed to `set' variables, and the "pub" community is
* allowed to `get' variables.
*
* The agent designer is required to write this function according to
* the design of the application.
*
* RETURNS: 0 if the community is acceptable, otherwise 1.
*/

int snmpLocalIoCommunityValidate
    (
    SNMP_PKT_T *    pPkt,               /* ptr to snmp pkt */
    SNMPADDR_T *    pRemoteAddr,        /* remote address */
    SNMPADDR_T *    pLocalAddr          /* local address */
    )
{
    int                     pktCmntyLen;          /* length of community string
                                                     in received pkt  */
    CMNTY_TO_VIEW_T *       pEntry;               /* ptr to traverse community
                                                     to view tables */

    char *                  pCmnty;               /* ptr to Cmnty  field */

    /* We don't use private memory so set this to NULL else the
     *  hook to release this is invoked if non NULL 
     */

    pPkt->user_private = (char *) 0;

    /* 
     * Install the local and remote network addresses in the packet.
     * These will be used later to respond to this pdu
     */

    (void) memcpy ((char *) &(pPkt->pkt_src), (char *) pRemoteAddr, 
                   sizeof (struct sockaddr_in));
    (void) memcpy((char *) &(pPkt->pkt_dst), (char *) pLocalAddr, 
                   sizeof (struct sockaddr_in) );
    
    pktCmntyLen = EBufferUsed (&(pPkt->community));
    
    /* select get or set cmmunity to view table as appropriate */

    pEntry = getCmntyViewTbl;


    for ( pCmnty = pEntry->Cmnty; pCmnty != NULL; 
          ++ pEntry, pCmnty = pEntry->Cmnty)
        {
        if ((strlen (pCmnty) == pktCmntyLen) &&
            (memcmp (pCmnty, pPkt->community.start_bp, pktCmntyLen) == 0))
            {
            pPkt->view_index = pEntry->viewIndex;
            return (0);
            }
        } 
        
    /* Got a bad community if were here, so increment the
     * snmpInBadCommunityNames counter
     */

    ++ snmp_stats.snmpInBadCommunityNames;
        
    /* An auth fail trap may be sent here */
    
    return (1);
}

int snmpSetCommunityNames(char *read, char *write)
{
   int retValue = STATUS_INVALID_PARAMS;
   
   if (NULL == read || NULL == write)
   {
      return STATUS_INVALID_PARAMS;
   }
   
   retValue = oam_setMibByteAry(SNMP_AGENT_MODULE_ID, MIB_readCommunity, read, strlen(read)+1);
   
   
   if (STATUS_OK == retValue)
   {
      printf ("Set read community = %s\n", read);
      retValue = oam_setMibByteAry(SNMP_AGENT_MODULE_ID, MIB_writeCommunity, write, strlen(write)+1);
   }
   
   if ( STATUS_OK == retValue)
   {
      printf ("Set write community = %s\n", write);
   }
      
   return retValue;
}
