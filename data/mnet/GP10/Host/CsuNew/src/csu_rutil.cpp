/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef  __CSU_RUTIL_CPP__
#define  __CSU_RUTIL_CPP__

#include "CsuNew\csu_head.h"

//Update src port in src port table as registered 
T_CSU_RESULT csu_DoSrcPortReg(T_CSU_SRC_PORT *src)
{
     Uint8 i;

     DBG_FUNC("csu_DoSrcPortReg", CSU_LAYER);
     DBG_ENTER();

     if (src->state != CSU_RESSTATE_NULL)
     {
         DBG_TRACE("src port already registered (%d,%x)\n",
                    src->type, src->rtpHandler);
         return CSU_RESULT_SUCCESS;
     } else
     {
         // Not registered yet, do it now
	 src->state = CSU_RESSTATE_FREE;
         src->resvd = CSU_RESSTATE_IGNORE;
	 src->count = 0;
       src->test  = CSU_TEST_FUNC_NULL;
              
         for (i=0;i<CSU_PEER_PORT_MAX;i++)
         {
              src->peer[i].type = CSU_NIL_PORT;
              src->peer[i].pidx = CSU_PEER_PORT_NULL;
		  src->peer[i].ppos = 0;
         }
     
         DBG_TRACE("Registering src port (%d,%x) successful\n", 
                    src->type, src->rtpHandler);

         return CSU_RESULT_SUCCESS;
     }
}

//Update snk port in snk port table as registered 
T_CSU_RESULT csu_DoSnkPortReg(T_CSU_SNK_PORT *snk)
{
     Uint8 i;

     DBG_FUNC("csu_DoSnkPortReg", CSU_LAYER);
     DBG_ENTER();

     if (snk->state != CSU_RESSTATE_NULL)
     {
         DBG_TRACE("snk port already registered (%d,%x)\n",
                    snk->type, snk->rtpHandler);
         return CSU_RESULT_SUCCESS;
     } else
     {
         // Not registered yet, do it now
	 snk->state = CSU_RESSTATE_FREE;
         snk->resvd = CSU_RESSTATE_IGNORE;
	 snk->count = 0;
       snk->test  = CSU_TEST_FUNC_NULL;
              
         for (i=0;i<CSU_PEER_PORT_MAX;i++)
         {
              snk->peer[i].type = CSU_NIL_PORT;
              snk->peer[i].pidx = CSU_PEER_PORT_NULL;
         }
     
         DBG_TRACE("Registering snk port (%d,%x) successful\n", 
                    snk->type, snk->rtpHandler);

         return CSU_RESULT_SUCCESS;
     }
}


T_CSU_RESULT csu_DoSrcPortUnreg(T_CSU_SRC_PORT *src)
{
     DBG_FUNC("csu_DoSrcPortPort", CSU_LAYER);
     DBG_ENTER();
     
     if (src->state == CSU_RESSTATE_NULL)
     {
         DBG_TRACE("src port (%d,%x) actually not registered\n",
                    src->type, src->rtpHandler);
         return CSU_RESULT_SUCCESS;
     }
     else if (src->state == CSU_RESSTATE_USED)
     {
         // Currently engaged in calls, record and unreg later
         src->resvd = CSU_RESSTATE_NULL;
	 DBG_TRACE("Unregister src port (%d,%x) pending on call\n", 
		    src->type, src->rtpHandler);
         return CSU_RESULT_SUCCESS;
     } else
     {
	 // Port free,unregister it now
	 src->state = CSU_RESSTATE_NULL;
         src->resvd = CSU_RESSTATE_IGNORE;
	 src->count = 0;
       src->test  = CSU_TEST_FUNC_NULL;
             
         for (int i=0;i<CSU_PEER_PORT_MAX;i++)
         {
              src->peer[i].type = CSU_NIL_PORT;
              src->peer[i].pidx = CSU_PEER_PORT_NULL;
		  src->peer[i].ppos = 0;
         }

	 DBG_TRACE("Unregister src port:type=%d,handler=%x successful\n", 
		    src->type,src->rtpHandler);

         return CSU_RESULT_SUCCESS;
     } 
}


T_CSU_RESULT csu_DoSnkPortUnreg(T_CSU_SNK_PORT *snk)
{
     DBG_FUNC("csu_DoSnkPortPort", CSU_LAYER);
     DBG_ENTER();
     
     if (snk->state == CSU_RESSTATE_NULL)
     {
         DBG_TRACE("snk port (%d,%x) actually not registered\n",
                    snk->type, snk->rtpHandler);
         return CSU_RESULT_SUCCESS;
     }
     else if (snk->state == CSU_RESSTATE_USED)
     {
         // Currently engaged in calls, record and unreg later
         snk->resvd = CSU_RESSTATE_NULL;
	 DBG_TRACE("Unregister snk port (%d,%x) pending on call\n", 
		    snk->type, snk->rtpHandler);
         return CSU_RESULT_SUCCESS;
     } else
     {
	 // Port free,unregister it now
	 snk->state = CSU_RESSTATE_NULL;
         snk->resvd = CSU_RESSTATE_IGNORE;
	 snk->count = 0;
       snk->test  = CSU_TEST_FUNC_NULL;

             
         for (int i=0;i<CSU_PEER_PORT_MAX;i++)
         {
              snk->peer[i].type = CSU_NIL_PORT;
              snk->peer[i].pidx = CSU_PEER_PORT_NULL;
         }

	 DBG_TRACE("Unregister snk port:type=%d,handler=%x successful\n", 
		    snk->type,snk->rtpHandler);

         return CSU_RESULT_SUCCESS;
     } 
}

#endif /*__CSU_RUTIL_CPP__*/
