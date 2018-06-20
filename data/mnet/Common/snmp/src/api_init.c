#include <stdlib.h>
#include "oam_trap.h"

static ListHeadPtr OamTrapTableHead = NULL;
static SEM_ID OamApiMutex = NULL;

int  oamApiDebugLevel= 0;

STATUS oam_initApi(int debugLevel)
{
	if (OamTrapTableHead == NULL)
	{
		OamTrapTableHead = oam_newListHead();
		/* create a binary semaphore that is initially full
		 * Tasks block on semaphore wait in priority order
		 */
#if (defined(VXWORKS))
		OamApiMutex = semBCreate(SEM_Q_PRIORITY, SEM_FULL);
#endif

	}
	oamApiDebugLevel = debugLevel;

	return STATUS_OK;
}


ListHeadPtr oam_getTrapTableHead()
{
	if ( OamTrapTableHead == NULL)
		oam_initApi(0);
	return OamTrapTableHead;
}


STATUS aom_cleanOamApi()
{
	if (OamTrapTableHead != NULL)
	{
		ListNodePtr list = OamTrapTableHead->first;
		ListNodePtr node;
		while(list != NULL) 
		{
			node = list;
			list =list->next;
			free(node);
		}
		free(OamTrapTableHead);
#if (defined(VXWORKS))
		semDelete(OamApiMutex);
#endif
	}
	return STATUS_OK;
}

ListHeadPtr oam_newListHead() 
{
	ListHeadPtr hp = (ListHeadPtr) malloc(sizeof(ListHead));
	if (hp != NULL)
	{
		hp->first = NULL;
		hp->last = NULL;
		hp->nodeCount = 0;
	}
	return hp;
}

void oam_lockMutex()
{
#if (defined(VXWORKS))
	if (OamApiMutex != NULL)
		semTake(OamApiMutex, WAIT_FOREVER);
#endif
}

void oam_unlockMutex()
{
#if (defined(VXWORKS))
	if (OamApiMutex != NULL)
		semGive(OamApiMutex);
#endif
}



