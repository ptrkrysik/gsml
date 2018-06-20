/*********************************************************
*
*  (c) Copyright Cisco 2000
*  All Rights Reserved
*
**********************************************************/

#if (!defined(OAM_TRAP_H))
#define OAM_TRAP_H

/******************************************************************
 ** Include Files. 
 ******************************************************************/

#include <stdio.h>
#include <msgQLib.h>
#include <semLib.h>
#include <snmp/asn1conf.h>
#include "mibtags.h"
#include "oam_api.h"
#include "vipermib.h"
#include "vipertbl.h"

typedef struct ListNode_t
{
    struct ListNode_t * next;
    MibTag tag;
	MSG_Q_ID qid;      /* typedef msg_q * MSG_Q_ID */
	OCTET_T mid;       /* module id of the subscriber */
}ListNode, *ListNodePtr;


typedef struct ListHead_t
{
    ListNodePtr first;    /* first element of the list  */
	ListNodePtr last;     /* last valid element of the list */
    int nodeCount;        /* No of items */
}ListHead, *ListHeadPtr;

typedef struct MibInitIntTable_T
{
	MibTag tag;
	INT_32_T value;
}MibInitIntTable;

#ifdef  __cplusplus
extern "C" {
#endif

ListNodePtr oam_newListNode(MibTag tag, MSG_Q_ID id);
STATUS oam_insertNode(ListHeadPtr head, ListNodePtr aNode);
STATUS oam_insertNodeSortedByTag(ListHeadPtr head, ListNodePtr aNode);
STATUS oam_appendNode(ListHeadPtr head, ListNodePtr aNode);
ListNodePtr oam_getNodeByID(ListHeadPtr head, MSG_Q_ID id);
ListNodePtr oam_getFirstNodeByTag(ListHeadPtr head, INT_32_T tag);
STATUS oam_deleteNode(ListHeadPtr head, MibTag tag, OCTET_T mid);
STATUS oam_deleteNodeByModule(ListHeadPtr head, OCTET_T mid);
STATUS oam_deleteNodeByQ(ListHeadPtr head, MSG_Q_ID qid);
void oam_printList(ListHeadPtr head);
void oam_oamError( char * formatString, ...);

#ifdef  __cplusplus
}
#endif


/* function in api_init.cpp */
STATUS oam_initApi(int debugLevel);
ListHeadPtr oam_getTrapTableHead();
ListHeadPtr oam_newListHead();

/* function in mib_init.cpp */
STATUS oam_initializeMib();

void oam_lockMutex();
void oam_unlockMutex();

/* debugging and printing */
void oam_msgPrint(char *format, ...) ;

#define LOG_ERROR(XX); \
        printf("OAM API ERROR at %s:", fname); \
        oam_msgPrint XX;

#ifdef	_DEBUG
#define LOG_TRACE(XX); \
        printf("OAM API Trace at %s:", fname); \
        oam_msgPrint XX; 
#else
#define LOG_TRACE(XX);
#endif

#endif
