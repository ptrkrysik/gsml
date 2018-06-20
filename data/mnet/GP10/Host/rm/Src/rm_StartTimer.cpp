/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_STARTTIMER_CPP__
#define __RM_STARTTIMER_CPP__

#include "rm\rm_head.h"

void rm_StartTimer( rm_IrtEntryId_t entryId, u8 timer )
{
   STATUS 		ret;
   WDOG_ID		id;
   int 			delay;
   int		param;
   u8			*pState, ignore;
   rm_PhyChanDesc_t 	*pPhyChan;

   //Monitoring entrance to a function
   //RDEBUG__(("ENTER-rm_StartTimer: entry ID=%d, timer ID=%d\n",
   //		  entryId, timer));
   
   if ( rm_SmCtxt[entryId].smType!=RM_MAIN_SM && timer!=RM_TWDOG )
   {
        IDEBUG__(("ERROR@rm_StartTimer:smType=%d, timer=%d\n", 
                   rm_SmCtxt[entryId].smType,timer));
        return;
   }

   pPhyChan = rm_SmCtxt[entryId].pPhyChan;
	
   ignore = RM_FALSE;

   switch (timer)
   {
   case RM_T3101:
	id     = pPhyChan->T3101.id;
	delay  = pPhyChan->T3101.time;
	pState = &(pPhyChan->T3101.state);
      param  = (entryId<<8)|RM_T3101;
	break;

   case RM_T3111:
	id     = pPhyChan->T3111.id;
	delay  = pPhyChan->T3111.time;
	pState = &(pPhyChan->T3111.state);
      param  = (entryId<<8)|RM_T3111;
	break;

   case RM_T3109:
	id     = pPhyChan->T3109.id;
	delay  = pPhyChan->T3109.time;
	pState = &(pPhyChan->T3109.state);
      param  = (entryId<<8)|RM_T3109;
	break;

   case RM_T3L01:
	id     = pPhyChan->T3L01.id;
	delay  = pPhyChan->T3L01.time;
	pState = &(pPhyChan->T3L01.state);
      param  = (entryId<<8)|RM_T3L01;
	break;

   case RM_T3L02:
	id     = pPhyChan->T3L02.id;
	delay  = pPhyChan->T3L02.time;
	pState = &(pPhyChan->T3L02.state);
      param  = (entryId<<8)|RM_T3L02;
	break;

   case RM_T3105:
	id     = pPhyChan->T3105.id;
	delay  = pPhyChan->T3105.time;
	pState = &(pPhyChan->T3105.state);
      param  = (entryId<<8)|RM_T3105;
	break;

   case RM_TWDOG:
	id     = rm_TrxMgmt[entryId].TWDOG.id;
	delay  = rm_TrxMgmt[entryId].TWDOG.time;
	pState = &(rm_TrxMgmt[entryId].TWDOG.state);
      param  = (entryId<<8)|RM_TWDOG;
	break;

   default:
	IDEBUG__(("ERROR@rm_StartTimer: unsupported timer:%d\n",timer ));
      ignore = RM_TRUE;
	break;
   }

   if ( !ignore )
   {
	  ret = wdStart ( id, delay, (FUNCPTR)rm_TimerExpired, param );
        if ( ret==ERROR )
  	       EDEBUG__(("ERROR@rm_StartTimer: wdStart errno:%d\n", errno));
        else
             *pState = RM_TRUE;
   }     

} //End of rm_StartTimer()

#endif //__RM_STARTTIMER_CPP__

