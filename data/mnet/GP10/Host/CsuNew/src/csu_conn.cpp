/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef __CSU_CONN_CPP__
#define __CSU_CONN_CPP__

#include "CsuNew\csu_head.h"


// Establish a simplex connection from src port to snk port
T_CSU_RESULT_CON csu_SimplexConnect(T_CSU_PORT_ID *src, T_CSU_PORT_ID *snk)
{
     Uint8   i, s_type, d_type;
     Uint16  s_pidx,d_pidx;
     T_CSU_RESULT ret_src, ret_snk;

     DBG_FUNC("csu_SimplexConnect", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU UNI-CONNECT REQ Info: src port(%d,%x)-->snk port(%d,%x)\n",
                     src->portType, src->portId.rtpHandler,
                     snk->portType, snk->portId.rtpHandler);

     csu_TakeSemaphore();

     // Get index of src port and snk port in their respective table
     if ( CSU_RESULT_SUCCESS !=
          csu_FindSrcPortPIdx(src, &s_type, &s_pidx) )
     {
          DBG_ERROR("CSU UNI-CONNECT REQ Error csu_FindSrcPortPIdx(): src(%d,%x)-->snk(%d,%x)\n",
                     src->portType, src->portId.rtpHandler,
                     snk->portType, snk->portId.rtpHandler);
          csu_GiveSemaphore(); 
          DBG_LEAVE()
          return CSU_RESULT_CON_SRC_PORT_NOT_READY;
     }

     if ( CSU_RESULT_SUCCESS !=
          csu_FindSnkPortPIdx(snk, &d_type, &d_pidx) )
     { 
          DBG_ERROR("CSU UNI-CONNECT REQ Error csu_FindSnkPortPIdx(): src(%d,%x)-->snk(%d,%x)\n",
                     src->portType, src->portId.rtpHandler,
                     snk->portType, snk->portId.rtpHandler);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SNK_PORT_NOT_READY;
     }

     // Check if both src and snk port are connection capable now
     if ( CSU_RESULT_SUCCESS != (ret_src=csu_CheckSrcCapable(s_type, s_pidx, d_type, d_pidx)) )
     { 
		  if (ret_src != CSU_RESULT_SRC_PORT_CONNECTED_AS_EXPECTED) 
          {
              DBG_ERROR("CSU UNI-CONNECT REQ Error(%d) csu_CheckSrcCapable(): src(%d,%x)-->snk(%d,%x)\n",
				         ret_src,
                         src->portType, src->portId.rtpHandler,
                         snk->portType, snk->portId.rtpHandler);
	          csu_GiveSemaphore();
              DBG_LEAVE()
              return CSU_RESULT_CON_SRC_PORT_NOT_READY;
		  }
     }

     if ( CSU_RESULT_SUCCESS != (ret_snk=csu_CheckSnkCapable(d_type, d_pidx, s_type, s_pidx)) )
     { 
		  if (ret_snk != CSU_RESULT_SNK_PORT_CONNECTED_AS_EXPECTED)
		  {	  
              DBG_ERROR("CSU UNI-CONNECT REQ Error(%d) csu_CheckSnkCapable(): src(%d,%x)-->snk(%d,%x)\n",
				         ret_snk,
                         src->portType, src->portId.rtpHandler,
                         snk->portType, snk->portId.rtpHandler);
	          csu_GiveSemaphore();
              DBG_LEAVE()
              return CSU_RESULT_CON_SNK_PORT_NOT_READY;
          }
     }

	 if ( (ret_src == CSU_RESULT_SRC_PORT_CONNECTED_AS_EXPECTED) &&
		(ret_snk == CSU_RESULT_SNK_PORT_CONNECTED_AS_EXPECTED)  )
	 {
	      csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SUCCESS;
	 }


     //Connect both src port and snk port of the connection
     if ( CSU_RESULT_SUCCESS != csu_ConnSrcPort(s_type,s_pidx,d_type,d_pidx,snk) )
     {
          DBG_ERROR("CSU UNI-CONNECT REQ Error csu_ConnSrcPort(): src(%d,%x)-->snk(%d,%x)\n",
                     src->portType, src->portId.rtpHandler,
                     snk->portType, snk->portId.rtpHandler);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SRC_PORT_NOT_READY;
     }

     if ( CSU_RESULT_SUCCESS != csu_ConnSnkPort(d_type,d_pidx,s_type,s_pidx,snk) )
     {
          DBG_ERROR("CSU UNI-CONNECT REQ Error csu_ConnSnkPort(): src(%d,%x)-->snk(%d,%x)\n",
                     src->portType, src->portId.rtpHandler,
                     snk->portType, snk->portId.rtpHandler);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SNK_PORT_NOT_READY;
     }

     csu_GiveSemaphore();
     DBG_LEAVE()
     return CSU_RESULT_CON_SUCCESS;
}

// Establish a duplex connection between src port and snk port
T_CSU_RESULT_CON csu_DuplexConnect(T_CSU_PORT_ID *src, T_CSU_PORT_ID *snk)
{
     T_CSU_RESULT_CON ret;

     DBG_FUNC("csu_DuplexConnect", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU BI-CONNECT REQ Info: src port(%d,%x)<-->snk port(%d,%x)\n",
                src->portType, src->portId.rtpHandler,
                snk->portType, snk->portId.rtpHandler);

     //Connect src->snk
     if ( CSU_RESULT_CON_SUCCESS != (ret=csu_SimplexConnect(src,snk)) )
     {
          DBG_ERROR("CSU BI-CONNECT Error '%d' csu_SimplexConnect(): src(%d,%x)-->snk(%d,%x)\n", 
                     ret,
                     src->portType, src->portId.rtpHandler,
                     snk->portType, snk->portId.rtpHandler);
          csu_SimplexBreak(src,snk);
          DBG_LEAVE()
          return ret;
     }

     //Connect snk->src
     if ( CSU_RESULT_CON_SUCCESS != (ret=csu_SimplexConnect(snk,src)) )
     {
          DBG_ERROR("CSU BI-CONNECT Error '%d' csu_SimplexConnect():snk(%d,%x)-->src(%d,%x)\n",
                     ret,
                     snk->portType, snk->portId.rtpHandler,
                     src->portType, src->portId.rtpHandler);
          csu_SimplexBreak(src,snk);
          csu_SimplexBreak(snk,src);
          DBG_LEAVE()
          return ret;
     }

     DBG_LEAVE()
     return CSU_RESULT_CON_SUCCESS;
}

// Break a simplex connection from src port to snk port
T_CSU_RESULT_CON csu_SimplexBreak(T_CSU_PORT_ID *src, T_CSU_PORT_ID *snk)
{
     Uint8  	s_type, d_type;
     Uint16 	s_pidx, d_pidx;

     DBG_FUNC("csu_SimplexBreak", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU UNI-BREAK REQ Info: src port(%d,%x)-->snk port(%d,%x)\n",
                src->portType, src->portId.rtpHandler,
                snk->portType, snk->portId.rtpHandler);

     csu_TakeSemaphore();

     // Get index of src port and snk port in their respective table
     if ( CSU_RESULT_SUCCESS !=
          csu_FindSrcPortPIdx(src, &s_type, &s_pidx) )
     { 
          DBG_ERROR("CSU UNI-BREAK REQ Error csu_FindSrcPortPIdx(): src(%d,%x)-->snk(%d,%x)\n",
                     src->portType, src->portId.rtpHandler,
                     snk->portType, snk->portId.rtpHandler);

	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SRC_PORT_NOT_ENGAGED;
     } 

     if ( CSU_RESULT_SUCCESS !=
          csu_FindSnkPortPIdx(snk, &d_type, &d_pidx) )
     { 
          DBG_ERROR("CSU UNI-BREAK REQ Error csu_FindSnkPortPIdx(): src(%d,%x)-->snk(%d,%x)\n",
                     src->portType, src->portId.rtpHandler,
                     snk->portType, snk->portId.rtpHandler);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SNK_PORT_NOT_ENGAGED;
     } 

     // Check if both src and snk port are engaged 
     if ( CSU_RESULT_SUCCESS !=
          csu_CheckSrcConn(s_type, s_pidx, d_type, d_pidx) )
     { 
          DBG_TRACE("CSU UNI-BREAK REQ Error csu_CheckSrcConn(): src(%d,%x)-->snk(%d,%x)\n",
                     src->portType, src->portId.rtpHandler,
                     snk->portType, snk->portId.rtpHandler);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SRC_PORT_NOT_ENGAGED;
     }

     if ( CSU_RESULT_SUCCESS !=
          csu_CheckSnkConn(d_type, d_pidx, s_type, s_pidx) )
     { 
          DBG_ERROR("CSU UNI-BREAK REQ Error csu_CheckSnkConn(): src(%d,%x)-->snk(%d,%x)\n",
                     src->portType, src->portId.rtpHandler,
                     snk->portType, snk->portId.rtpHandler);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SNK_PORT_NOT_ENGAGED;
     }

     if ( CSU_RESULT_SUCCESS !=
	  csu_BreakSrcConn(s_type, s_pidx, d_type, d_pidx) )
     { 
          DBG_ERROR("CSU UNI-BREAK REQ Error csu_BreakSrcConn(): src(%d,%x)-->snk(%d,%x)\n",
                     src->portType, src->portId.rtpHandler,
                     snk->portType, snk->portId.rtpHandler);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SRC_PORT_NOT_ENGAGED;
     }

     if ( CSU_RESULT_SUCCESS !=
	  csu_BreakSnkConn(d_type, d_pidx, s_type, s_pidx) )
     { 
          DBG_ERROR("CSU UNI-BREAK REQ Error csu_BreakSnkConn(): src(%d,%x)-->snk(%d,%x)\n",
                     src->portType, src->portId.rtpHandler,
                     snk->portType, snk->portId.rtpHandler);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SNK_PORT_NOT_ENGAGED;
     }

     csu_GiveSemaphore();
     DBG_LEAVE()
     return CSU_RESULT_CON_SUCCESS;
}

// Break a duplex connection between src port and snk port
T_CSU_RESULT_CON csu_DuplexBreak(T_CSU_PORT_ID *src, T_CSU_PORT_ID *snk)
{
     T_CSU_RESULT_CON ret;

     DBG_FUNC("csu_DuplexBreak", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU BI-BREAK REQ Info: src port(%d,%x)<-->snk port(%d,%x)\n",
                src->portType, src->portId.rtpHandler,
                snk->portType, snk->portId.rtpHandler);


     // Break src->snk connection
     if ( CSU_RESULT_CON_SUCCESS != (ret=csu_SimplexBreak(src,snk)) )
     {
          DBG_ERROR("CSU BI-BREAK Error '%d' csu_SimplexBreak(): src(%d,%x)-->snk(%d,%x)\n",
                     ret,
                     src->portType, src->portId.rtpHandler,
                     snk->portType, snk->portId.rtpHandler);
          DBG_LEAVE()
          return ret;
     }

     //Break snk->src connection
     if ( CSU_RESULT_CON_SUCCESS != (ret=csu_SimplexBreak(snk,src)) )
     {
          DBG_TRACE("CSU BI-BREAK Error '%d' csu_SimplexBreak(): snk(%d,%x)-->src(%d,%x)\n",
                     ret,
                     snk->portType, snk->portId.rtpHandler,
                     src->portType, src->portId.rtpHandler);
          DBG_LEAVE()
          return ret;
     }

     DBG_LEAVE()
     return CSU_RESULT_CON_SUCCESS;
}

//Break all connection originating from source port
T_CSU_RESULT_CON csu_OrigBreakAll(T_CSU_PORT_ID *source)
{
     Uint8	     d_there,trx,slot;
     Uint8           s_type, d_type;
     Uint16 	     s_pidx, d_pidx;
     T_CSU_SRC_PORT  *src;


     DBG_FUNC("csu_OrigBreakAll", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU ORIG-BREAK REQ Info: src port(%d,%x)\n",
                source->portType, source->portId.rtpHandler);

     csu_TakeSemaphore();

     // Get index of src port in its connection table
     if ( CSU_RESULT_SUCCESS !=
          csu_FindSrcPortPIdx(source, &s_type, &s_pidx) )
     {
          DBG_ERROR("CSU ORIG-BREAK Error csu_FindSrcPortPIdx(): src port (%d,%08x)\n",
                     source->portType, source->portId.rtpHandler);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SRC_PORT_NOT_ENGAGED;
     }

     // Find the port's corresponding item in src table
     switch(s_type)
     {
     case CSU_GSM_PORT:
          trx  = (Uint8)(s_pidx>>8);
          slot = (Uint8)(s_pidx);          
          src  = &csu_GsmSrcPort[trx][slot];
          break;
     case CSU_RTP_PORT:
          src  = &csu_RtpSrcPort[s_pidx];
          break;
     case CSU_ANN_PORT:
	    src  = &csu_AnnSrcPort[s_pidx];
          break;
     default:
          DBG_ERROR("CSU ORIG-BREAK Error: invalid source port type: %d\n", s_type);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_FAILED;
     }

     DBG_TRACE("CSU ORIG-BREAK Info: src port (%d,%x): s_type=%d, s_pidx=%x\n", source->portType,
                source->portId.rtpHandler, s_type, s_pidx);

     // Check if src port engaged in any connection or not
     if ( src->state != CSU_RESSTATE_USED || src->count==0)
     {
          DBG_TRACE("CSU ORIG-BREAK INFO src port (%d,%x) not engaged in any connection\n",
                     s_type, src->gsmHandler);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SUCCESS;
     }

     // Break all connections originating from the src port
     for (int i=0;i<CSU_PEER_PORT_MAX;i++)
     {
          if ( src->peer[i].type != CSU_NIL_PORT &&
               src->peer[i].pidx != CSU_PEER_PORT_NULL )
          {
//DBG_TRACE("src port:state=%d, count=%d, d_type=%d, d_pidx=%x\n",
//src->state, src->count, src->peer[i].type, src->peer[i].pidx);

               // Check existence of src->this snk
               d_there = 1;
               if ( CSU_RESULT_SUCCESS !=
                    csu_CheckSnkConn(src->peer[i].type, src->peer[i].pidx, s_type, s_pidx) )
               {
                    DBG_ERROR("CSU ORIG-BREAK Error CheckSnkConn():i=%d, src(%d,%x)-->snk(%d,idx:%x)\n",
                               i,
                               s_type, src->rtpHandler, 
                               src->peer[i].type, src->peer[i].pidx);
  		        d_there = 0;	                    

               }
              
               // Break src's engagement no matter whether snk of src->connection is there
               Uint8  t_type; //for temporarily keeping a value 
               Uint16 t_pidx; //for temporarily keeping a value 
               t_type = src->peer[i].type;
               t_pidx = src->peer[i].pidx;

               if ( CSU_RESULT_SUCCESS !=
 	              csu_BreakSrcConn(s_type, s_pidx, src->peer[i].type,
                                     src->peer[i].pidx) )
               {
                    DBG_ERROR("CSU ORIG-BREAK Error csu_BreakSrcConn(): i=%d, src(%d,%08x)-->snk(%d,idx:%x)\n",
                               source->portType, source->portId.rtpHandler, 
                               src->peer[i].type,src->peer[i].pidx);
               }

               // Break snk's engagement if snk now engaged in the src->this connection
               if (d_there)
               {
                  if ( CSU_RESULT_SUCCESS !=
 	               csu_BreakSnkConn(t_type, t_pidx, s_type, s_pidx) )
                  {
                       DBG_ERROR("CSU ORIG-BREAK Error csu_BreakSnkConn():src(%d,%x)-->snk(%d,idx:%x) snk break failed\n",
                                  s_type, src->rtpHandler, t_type,t_pidx);
                  }
               } 
          }                    
     }

     csu_GiveSemaphore();
     DBG_LEAVE()
     return CSU_RESULT_CON_SUCCESS;
}

//Break all connections terminated at given sink port
T_CSU_RESULT_CON csu_TermBreakAll(T_CSU_PORT_ID *sink)
{
     Uint8	     s_there,trx,slot;
     Uint8           s_type, d_type;
     Uint16 	     s_pidx, d_pidx;
     T_CSU_SNK_PORT  *snk;

     DBG_FUNC("csu_TermBreakAll", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU TERM-BREAK REQ Info: snk port(%d,%x)\n",
                sink->portType, sink->portId.rtpHandler);

     csu_TakeSemaphore();

     // Get index of sink port in its connection table
     if ( CSU_RESULT_SUCCESS !=
          csu_FindSnkPortPIdx(sink, &d_type, &d_pidx) )
     { 
          DBG_ERROR("CSU TERM-BREAK REQ Error csu_FindSrcPortPIdx(): snk(%d,%x)\n",
                     sink->portType, sink->portId.rtpHandler);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SNK_PORT_NOT_ENGAGED;
     }

     // Find the port's corresponding item in snk table
     switch(d_type)
     {
     case CSU_GSM_PORT:
          trx  = (Uint8)(d_pidx>>8);
          slot = (Uint8)(d_pidx);          
          snk  = &csu_GsmSnkPort[trx][slot];
          break;
     case CSU_RTP_PORT:
          snk  = &csu_RtpSnkPort[d_pidx];
          break;
    // case CSU_ANN_PORT:

     default:
          DBG_ERROR("CSU TERM-BREAK REQ Error: invalid sink port type: %d\n", d_type);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_FAILED;
     }

     // Check if snk port engaged in any connection or not
     if ( snk->state != CSU_RESSTATE_USED || snk->count==0 )
     {
          DBG_TRACE("CSU TERM-BREAK REQ INFO: snk(%d, %x) not engaged in any call\n",
                     d_type, snk->rtpHandler);
	    csu_GiveSemaphore();
          DBG_LEAVE()
          return CSU_RESULT_CON_SUCCESS;
     }

     // Break all connections terminated at the sink port
     for (int i=0;i<CSU_PEER_PORT_MAX;i++)
     {
          if ( snk->peer[i].type != CSU_NIL_PORT &&
               snk->peer[i].pidx != CSU_PEER_PORT_NULL )
          {
               // Check src's engagement of this src->snk connection
               s_there = 1;
               if ( CSU_RESULT_SUCCESS !=
                    csu_CheckSrcConn(snk->peer[i].type,snk->peer[i].pidx,d_type,d_pidx) )
               {
                    DBG_ERROR("CSU TERM-BREAK REQ Error csu_CheckSrcConn(): i=%d, src(%d,idx:%x)-->snk(%d,%x)\n",
                               i,
                               snk->peer[i].type, snk->peer[i].pidx,
                               d_type, snk->rtpHandler);
		    s_there = 0;	                    

               }
              
               // Break snk's engagement no matter whether this src is engaged or not
               Uint8  t_type;
               Uint16 t_pidx;
               t_type = snk->peer[i].type;
               t_pidx = snk->peer[i].pidx;

               if ( CSU_RESULT_SUCCESS !=
 	            csu_BreakSnkConn(d_type,d_pidx,snk->peer[i].type,snk->peer[i].pidx) )
               {
                    DBG_ERROR("CSU TERM-BREAK REQ Error csu_BreakSnkConn(): i=%d, src(%d,idx:%x)-->snk(%d,%x)\n",
                               i,
                               snk->peer[i].type, snk->peer[i].pidx,
                               d_type, snk->rtpHandler);
               }

               // Break src's engagement if this src really engaged in this connection
               if (s_there)
               {
                  if ( CSU_RESULT_SUCCESS !=
 	               csu_BreakSrcConn(t_type, t_pidx, d_type, d_pidx) )
                  {
                       DBG_ERROR("CSU TERM-BREAK REQ Error csu_BreakSrcConn(): src(%d,idx:%x)-->snk(%d,%x)\n",
                                  snk->peer[i].type, snk->peer[i].pidx,
                                  s_type, snk->rtpHandler);

                  }
               } 
          }                    
     }

     csu_GiveSemaphore();
     DBG_LEAVE()
     return CSU_RESULT_CON_SUCCESS;
}

//Break all connections terminated at given sink port
T_CSU_RESULT_CON csu_OrigTermBreakAll(T_CSU_PORT_ID *port)
{
     T_CSU_RESULT_CON ret;

     DBG_FUNC("csu_OrigTermBreakAll", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU ORIG-TERM-BREAK REQ Info: port(%d,%x)\n",
                port->portType, port->portId.rtpHandler);


     if ( CSU_RESULT_CON_SUCCESS != 
          (ret = csu_OrigBreakAll(port)) )
     {
           DBG_ERROR("CSU ORIG-TERM-BREAK REQ Error OrigBreakAll(): orig(%d, %x)\n",
                      port->portType, port->portId.rtpHandler);
           DBG_LEAVE()
           return ret;
     }

     if ( CSU_RESULT_CON_SUCCESS != 
          (ret = csu_TermBreakAll(port)) )
     {
           DBG_ERROR("CSU ORIG-TERM-BREAK REQ Error TermBreakAll(): term(%d, %x)\n",
                      port->portType, port->portId.rtpHandler);
           DBG_LEAVE()
           return ret;
     }

     DBG_LEAVE()
     return CSU_RESULT_CON_SUCCESS;
}

//Interogate the connection status originating from the source port
T_CSU_RESULT_CON csu_SrcStatus(T_CSU_PORT_ID *source, T_CSU_PORT_LIST *sink)
{
     Uint8	     d_there,trx,slot,s_type;
     Uint16 	     s_pidx;
     T_CSU_SRC_PORT  *src;

     DBG_FUNC("csu_SrcStatus", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("SRC-STATUS REQ: src port(%d,%x)\n",
                     source->portType, source->portId.rtpHandler);

     csu_TakeSemaphore();

     // Get index of source port in its connection table
     if ( CSU_RESULT_SUCCESS !=
          csu_FindSrcPortPIdx(source, &s_type, &s_pidx) )
     { 
	    csu_GiveSemaphore();
          return CSU_RESULT_CON_FAILED;
     }

     // Find the port's corresponding item in snk table
     switch(s_type)
     {
     case CSU_GSM_PORT:
          trx  = (Uint8)(s_pidx>>8);
          slot = (Uint8)(s_pidx);          
          src  = &csu_GsmSrcPort[trx][slot];
          break;
     case CSU_RTP_PORT:
          src  = &csu_RtpSrcPort[s_pidx];
          break;
     case CSU_ANN_PORT:
	    src  = &csu_AnnSrcPort[s_pidx];
          break;
     default:
          DBG_TRACE("ERROR: invalid source port type: %d\n", s_type);
	    csu_GiveSemaphore();
          return CSU_RESULT_CON_FAILED;
     }

     // Check if src port engaged in any connection or not
     if ( src->state != CSU_RESSTATE_USED || src->count==0 )
     {
          DBG_TRACE("source port (%d, %x) not engaged in any call\n",
                     s_type, src->rtpHandler);

          sink->count = 0;

	    csu_GiveSemaphore();
          return CSU_RESULT_CON_SUCCESS;
     }

     // Return all the sinks involved in calls originated from the source
     for (int i=0;i<CSU_PEER_PORT_MAX;i++)
     {
          if ( src->peer[i].type != CSU_NIL_PORT &&
               src->peer[i].pidx != CSU_PEER_PORT_NULL )
          {
               // Check src's engagement of this src->snk connection
               d_there = 1;
               if ( CSU_RESULT_SUCCESS !=
                    csu_CheckSnkConn(src->peer[i].type,src->peer[i].pidx,s_type,s_pidx) )
               {
                    DBG_TRACE("src (%d,%x)-->snk(%d,idx:%x) snk not engaged\n",
                               s_type, src->rtpHandler,
                               src->peer[i].type, src->peer[i].pidx);

		    d_there = 0;

                    // This sink does not make any sense, clear it now
                    src->peer[i].type = CSU_NIL_PORT;
                    src->peer[i].pidx = CSU_PEER_PORT_NULL;
                    if (--src->count == 0)
                    {
                        sink->count = 0;
		 	      csu_GiveSemaphore();
                        return CSU_RESULT_CON_SUCCESS;
                    }
               }

               // This src is currently engaged, add it into the sink list
               if (d_there)
               {
                   if (CSU_RESULT_SUCCESS != 
                       csu_AddToSnkPortList(sink,src->peer[i].type,src->peer[i].pidx) )
                   {
                       DBG_TRACE("sink port list constraint:count=%,size=%d\n",
                                  sink->count, CSU_CONN_MAX);
 			     csu_GiveSemaphore();
                       return CSU_RESULT_CON_FAILED;
		   }
	       }

          }                    
     }

     csu_GiveSemaphore();
     return CSU_RESULT_CON_SUCCESS;
}

//Interogate the connection status terminating at the sink port
T_CSU_RESULT_CON csu_SnkStatus(T_CSU_PORT_ID *sink, T_CSU_PORT_LIST *source)
{
     Uint8	     s_there,trx,slot,d_type;
     Uint16 	     d_pidx;
     T_CSU_SNK_PORT  *snk;

     DBG_FUNC("csu_SnkStatus", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("TERM-STATUS REQ: snk port(%d,%x)\n",
                     sink->portType, sink->portId.rtpHandler);

     csu_TakeSemaphore();

     // Get index of sink port in its connection table
     if ( CSU_RESULT_SUCCESS !=
          csu_FindSnkPortPIdx(sink, &d_type, &d_pidx) )
     {
	    csu_GiveSemaphore();
          return CSU_RESULT_CON_FAILED;
     }

     // Find the port's corresponding item in snk table
     switch(d_type)
     {
     case CSU_GSM_PORT:
          trx  = (Uint8)(d_pidx>>8);
          slot = (Uint8)(d_pidx);          
          snk  = &csu_GsmSnkPort[trx][slot];
          break;
     case CSU_RTP_PORT:
          snk  = &csu_RtpSnkPort[d_pidx];
          break;
     //case CSU_ANN_PORT:
	  
     default:
          DBG_TRACE("ERROR: invalid sink port type: %d\n", d_type);
	    csu_GiveSemaphore();
          return CSU_RESULT_CON_FAILED;
     }

     // Check if snk port engaged in any connection or not
     if ( snk->state != CSU_RESSTATE_USED || snk->count==0 )
     {
          DBG_TRACE("sink port (%d, %x) not engaged in any call\n",
                     d_type, snk->rtpHandler);

          source->count = 0;
	    csu_GiveSemaphore();
          return CSU_RESULT_CON_SUCCESS;
     }

     // Return all the sources involved in calls terminated at the sink
     for (int i=0;i<CSU_PEER_PORT_MAX;i++)
     {
          if ( snk->peer[i].type != CSU_NIL_PORT &&
               snk->peer[i].pidx != CSU_PEER_PORT_NULL )
          {
               // Check src's engagement of this src->snk connection
               s_there = 1;
               if ( CSU_RESULT_SUCCESS !=
                    csu_CheckSrcConn(snk->peer[i].type,snk->peer[i].pidx,d_type,d_pidx) )
               {
                    DBG_TRACE("src (%d,idx:%x)-->snk(%d,%x) src not engaged\n",
                               snk->peer[i].type, snk->peer[i].pidx,
                               d_type, snk->rtpHandler);
		    s_there = 0;

                    // This source does not make any sense, clear it now
                    snk->peer[i].type = CSU_NIL_PORT;
                    snk->peer[i].pidx = CSU_PEER_PORT_NULL;
                    if (--snk->count == 0)
                    {
                        source->count = 0;
	 	 	      csu_GiveSemaphore();
                        return CSU_RESULT_CON_SUCCESS;
                    }
               }

               // This src is currently engaged, add it into the source list
               if (s_there)
               {
                   if (CSU_RESULT_SUCCESS != 
                       csu_AddToSrcPortList(source,snk->peer[i].type,snk->peer[i].pidx) )
                   {
                       DBG_TRACE("source port list constraint:count=%,size=%d\n",
                                  source->count, CSU_CONN_MAX);
			     csu_GiveSemaphore();
                       return CSU_RESULT_CON_FAILED;
		       }
	         }

          }                    
     }

     csu_GiveSemaphore();
     return CSU_RESULT_CON_SUCCESS;
}

//Interogate the whole connection status port is involved in
T_CSU_RESULT_CON csu_SrcSnkStatus(
                 T_CSU_PORT_ID *port, T_CSU_PORT_LIST *source, T_CSU_PORT_LIST *sink)
{
     T_CSU_RESULT_CON ret;

     DBG_FUNC("csu_SrcSnkStatus", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("ORIG-TERM STATUS REQ: port(%d,%x)\n",
                     port->portType, port->portId.rtpHandler);

     if ( CSU_RESULT_CON_SUCCESS != (ret=csu_SrcStatus(port, source)) )
     {
          DBG_TRACE("FAILED in checking source-side connection status of port(%d,%x)\n",
                     port->portType, port->portId.rtpHandler);
          return ret;
     }
       
     if ( CSU_RESULT_CON_SUCCESS != (ret=csu_SnkStatus(port, sink)) )
     {
          DBG_TRACE("FAILED in checking sink-side connection status of port(%d,%x)\n",
                     port->portType, port->portId.rtpHandler);
          return ret;
     }
     
     return CSU_RESULT_CON_SUCCESS;
}

#endif /*__CSU_CONN_CPP__*/
