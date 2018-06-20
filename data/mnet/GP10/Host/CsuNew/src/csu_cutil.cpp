/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef __CSU_CUTIL_CPP__
#define __CSU_CUTIL_CPP__

#include "CsuNew\csu_head.h"

//Add a source port to a source port list
T_CSU_RESULT csu_AddToSrcPortList(T_CSU_PORT_LIST *list,Uint8 type, Uint16 pidx)
{
     DBG_FUNC("csu_AddToSrcPortList", CSU_LAYER);
     DBG_ENTER();
   
     if (list->count == (CSU_CONN_MAX-1) )
     {
         DBG_ERROR("CSU Error csu_AddToSrcPortList(): src port list constraint:count=%,size=%d\n",
                    list->count, CSU_CONN_MAX);
         DBG_LEAVE();
         return CSU_RESULT_FAILED;
     }

     switch(type)
     {
     case CSU_GSM_PORT:
          T_CNI_IRT_ID entryId;
          if (!rm_TrxSlotToEntryId(pidx,&entryId))
          {
              DBG_ERROR("CSU Error csu_AddToSrcPortList(): no entryId matched to port(%d,%x)\n",
                         type, pidx);
              DBG_LEAVE();
              return CSU_RESULT_FAILED;
          }
          list->port[list->count].portType = CSU_GSM_PORT;            
          list->port[list->count].portId.gsmHandler = entryId;
          break;
     case CSU_RTP_PORT:
          list->port[list->count].portType = CSU_RTP_PORT;            
          list->port[list->count].portId.rtpHandler = csu_RtpSrcPort[pidx].rtpHandler;
          break;
     case CSU_ANN_PORT:	 
	    list->port[list->count].portType = CSU_ANN_PORT;            
          list->port[list->count].portId.annHandler = csu_AnnSrcPort[pidx].annHandler;
          break;
     default:
          DBG_ERROR("CSU Error csu_AddToSrcPortList():invalid source type '%d'\n", type);
          DBG_LEAVE();
          return CSU_RESULT_FAILED;
     }

     list->count++;

     DBG_LEAVE();
     return CSU_RESULT_SUCCESS;
}

//Add a sink port to a sink port list
T_CSU_RESULT csu_AddToSnkPortList(T_CSU_PORT_LIST *list,Uint8 type,Uint16 pidx)
{

     DBG_FUNC("csu_AddToSnkPortList", CSU_LAYER);
     DBG_ENTER();
   
     if (list->count == (CSU_CONN_MAX-1) )
     {
         DBG_ERROR("CSU Error csu_AddToSnkPortList(): sink port list constraint:count=%,size=%d\n",
                    list->count, CSU_CONN_MAX); 
         DBG_LEAVE();
         return CSU_RESULT_FAILED;
     }

     switch(type)
     {
     case CSU_GSM_PORT:
          T_CNI_IRT_ID entryId;
          if (!rm_TrxSlotToEntryId(pidx,&entryId))
          {
              DBG_ERROR("CSU Error csu_AddToSnkPortList(): no entryId matched to port(%d,%x)\n",
                         type, pidx);
              DBG_LEAVE();
              return CSU_RESULT_FAILED;
          }
          list->port[list->count].portType = CSU_GSM_PORT;            
          list->port[list->count].portId.gsmHandler = entryId;
          break;
     case CSU_RTP_PORT:
          list->port[list->count].portType = CSU_RTP_PORT;            
          list->port[list->count].portId.rtpHandler = csu_RtpSnkPort[pidx].rtpHandler;
          break;
     //case CSU_ANN_PORT:	 
	    
     default:
         DBG_ERROR("CSU Error csu_AddToSnkPortList(): invalid sink type '%d'\n", type);
         return CSU_RESULT_FAILED;
     }

     list->count++;

     DBG_LEAVE();
     return CSU_RESULT_SUCCESS;
}

//Find src port's type and pidx appearing in its peer snk port's peer[]
T_CSU_RESULT csu_FindSrcPortPIdx(T_CSU_PORT_ID *src, Uint8 *type, Uint16 *pidx)
{    
     Uint8  i,j;
     Uint16 gsmHandler;

     DBG_FUNC("csu_FindSrcPortPIdx", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU Info csu_FindSrcPortPIdx(): src (%d,%x)\n", src->portType,
               src->portId.rtpHandler);
     switch(src->portType)
     {
     case CSU_GSM_PORT:
          //Converting entryId to gsmHandler
          if (!rm_EntryIdToTrxSlot(src->portId.gsmHandler, &gsmHandler))
          {
              DBG_ERROR("CSU Error csu_FindSrcPortPIdx(): failed in converting entryId %d to gsmHandler=%x\n",
                         src->portId.gsmHandler, gsmHandler);
              DBG_LEAVE();
              return CSU_RESULT_FAILED;
          }

          //DBG_TRACE("entryId=%d, trx/slot=%x\n",src->portId.gsmHandler, gsmHandler);

          for (i=0;i<CSU_GSM_SRC_TRX_MAX ;i++)
          for (j=0;j<CSU_GSM_SRC_SLOT_MAX;j++)
          {
	       if (csu_GsmSrcPort[i][j].state != CSU_RESSTATE_NULL &&
                 csu_GsmSrcPort[i][j].gsmHandler == gsmHandler)
           
               {
                   *type = CSU_GSM_PORT;        
                   *pidx = (Uint16)(i<<8|j);
                   DBG_LEAVE();
                   return CSU_RESULT_SUCCESS;
	       }
	    }
          break;

     case CSU_RTP_PORT:
          for (i=0;i<=CSU_RTP_SRC_PORT_MAX;i++)
          {
 	       if (csu_RtpSrcPort[i].state != CSU_RESSTATE_NULL &&
                   csu_RtpSrcPort[i].rtpHandler == src->portId.rtpHandler)
               {
                   *type = CSU_RTP_PORT;        
		       *pidx=i;
                   return CSU_RESULT_SUCCESS;
	       }
	  }
	  break;

     case CSU_ANN_PORT:
	   for (i=0;i<=CSU_ANN_SRC_PORT_MAX;i++)
          {
 	       if (csu_AnnSrcPort[i].state != CSU_RESSTATE_NULL &&
                   csu_AnnSrcPort[i].annHandler == src->portId.annHandler)

               {
                   *type = CSU_ANN_PORT;        
		       *pidx=i;
                   return CSU_RESULT_SUCCESS;
	       }
	  }
	  break;

     default:
          DBG_ERROR("CSU Error csu_FindSrcPortPIdx(): invalid src port type:%d\n",
                     src->portType, src->portId.rtpHandler);
          return CSU_RESULT_FAILED;
     }
    
     return CSU_RESULT_FAILED;
}

//Find snk port's type and pidx appearing in its peer snk port's peer[]
T_CSU_RESULT csu_FindSnkPortPIdx(T_CSU_PORT_ID *snk, Uint8 *type, Uint16 *pidx)
{    
     Uint8   i,j;
     Uint16  gsmHandler;

     DBG_FUNC("csu_FindSnkPortPIdx", CSU_LAYER);
     DBG_ENTER();
    
     DBG_TRACE("CSU Info csu_FindSnkPortPIdx(): snk (%d,%x)\n", snk->portType,
               snk->portId.rtpHandler);

     switch(snk->portType)
     {
     case CSU_GSM_PORT:
          //Converting entryId to gsmHandler
          if (!rm_EntryIdToTrxSlot(snk->portId.gsmHandler, &gsmHandler))
          {
              DBG_ERROR("CSU Error csu_FindSnkPortPIdx(): failed in converting entryId %d to gsmHandler=%x\n",
                         snk->portId.gsmHandler, gsmHandler);
              return CSU_RESULT_FAILED;
          }

          for (i=0;i<CSU_GSM_SNK_TRX_MAX ;i++)
          for (j=0;j<CSU_GSM_SNK_SLOT_MAX;j++)
          {
	       if (csu_GsmSnkPort[i][j].state != CSU_RESSTATE_NULL &&
                   csu_GsmSnkPort[i][j].gsmHandler == gsmHandler)
           
               {
                   *type = CSU_GSM_PORT;        
                   *pidx = (Uint16)(i<<8|j);
                   return CSU_RESULT_SUCCESS;
	       }
	  }
          break;

     case CSU_RTP_PORT:
          for (i=0;i<=CSU_RTP_SNK_PORT_MAX;i++)
          {
 	       if (csu_RtpSnkPort[i].state != CSU_RESSTATE_NULL &&
                   csu_RtpSnkPort[i].rtpHandler == snk->portId.rtpHandler)
               {
                   *type = CSU_RTP_PORT;        
		       *pidx=i;
                   DBG_LEAVE();
                   return CSU_RESULT_SUCCESS;
	       }
	  }
	  break;

     //case CSU_ANN_PORT:
	
     default:
          DBG_ERROR("CSU Error csu_FindSnkPortPIdx():invalid snk port type:%d\n",
                     snk->portType, snk->portId.rtpHandler);
          DBG_LEAVE();
          return CSU_RESULT_FAILED;
     }
    
     DBG_LEAVE();
     return CSU_RESULT_FAILED;
}

//Check if src port is connection capable
T_CSU_RESULT csu_CheckSrcCapable( 
             Uint8 s_type, Uint16 s_pidx, Uint8 d_type, Uint16 d_pidx)
{
     Uint8          i,trx,slot;
     T_CSU_SRC_PORT *src;
 
     DBG_FUNC("csu_CheckSrcCapable", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU Info csu_CheckSrcCapable(): srcIdx(%d, %x)-->snkIdx(%d,%x)\n",
                s_type, s_pidx, d_type, d_pidx);

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
          DBG_ERROR("CSU Error csu_CheckSrcCapable(): invalid src port type: %d\n", s_type);
          DBG_LEAVE();
          return CSU_RESULT_FAILED;
     }

     //Check if src port has gone beyond its connection capacity
     if (src->state == CSU_RESSTATE_USED && src->count >= src->limit)
     {
         DBG_ERROR("CSU Error csu_CheckSrcCapable(): Src conn overflow: srcIdx(%d,%x),count=%d,limit=%d\n",
                    s_type, src->rtpHandler, src->count, src->limit);
         DBG_LEAVE();
         return CSU_RESULT_FAILED;
     }

     //Check if src port is already engaged as expected
     if (src->state == CSU_RESSTATE_USED )
     {
         for (i=0;i<CSU_PEER_PORT_MAX;i++)
         {
	      if (src->peer[i].type == d_type &&
                  src->peer[i].pidx == d_pidx )
              {
                  DBG_TRACE("CSU INFO csu_CheckSrcCapable(): SrcIdx(%d,%x) already engaged as expected\n",
                             s_type, src->rtpHandler);
                  DBG_LEAVE();
                  return CSU_RESULT_SRC_PORT_CONNECTED_AS_EXPECTED;
              }
         }
     }

     DBG_LEAVE();
     return CSU_RESULT_SUCCESS;
}

//Check if snk port is connection capable
T_CSU_RESULT csu_CheckSnkCapable( 
             Uint8 d_type, Uint16 d_pidx, Uint8 s_type, Uint16 s_pidx)
{
     Uint8          i,trx,slot;
     T_CSU_SNK_PORT *snk;
 
     DBG_FUNC("csu_CheckSnkCapable", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU Info csu_CheckSnkCapable(): srcIdx(%d, %x)-->snkIdx(%d,%x)\n",
                s_type, s_pidx, d_type, d_pidx);

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
          DBG_ERROR("CSU Error csu_CheckSnkCapable(): invalid sink port type: %d\n", d_type);
          DBG_LEAVE();
          return CSU_RESULT_FAILED;
     }

     //Check if snk port has gone beyond its connection capacity
     if (snk->state == CSU_RESSTATE_USED && snk->count > snk->limit)
     {
         DBG_ERROR("CSU Error csu_CheckSnkCapable(): Snk conn overflow: snkIdx(%d,%x),count=%d,limit=%d\n",
                    d_type, snk->rtpHandler, snk->count, snk->limit);
         return CSU_RESULT_FAILED;
     }

     //Check if snk port is already engaged as expected
     if (snk->state == CSU_RESSTATE_USED )
     {
         for (i=0;i<CSU_PEER_PORT_MAX;i++)
         {
	      if (snk->peer[i].type == s_type &&
                  snk->peer[i].pidx == s_pidx )
              {
                  DBG_TRACE("CSU INFO csu_CheckSnkCapable(): SnkIdx(%d,%x) already engaged as expected\n",
                             d_type, snk->rtpHandler);
                  DBG_LEAVE();
                  return CSU_RESULT_SNK_PORT_CONNECTED_AS_EXPECTED;
              }
         }
     }
          
     DBG_LEAVE();
     return CSU_RESULT_SUCCESS;
}

//Finish src port of a connection 
T_CSU_RESULT csu_ConnSrcPort(Uint8 s_type, Uint16 s_pidx, Uint8 d_type, Uint16 d_pidx,T_CSU_PORT_ID *port_id)
{
     Uint8          i;
     T_CSU_SRC_PORT *src;
     T_CSU_RESULT   ret;

     DBG_FUNC("csu_ConnSrcPort", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU Info csu_ConnSrcPort(): srcIdx(%d, %x)-->snkIdx(%d,%x)\n",
               s_type, s_pidx, d_type, d_pidx);

     switch(s_type)
     {
     case CSU_GSM_PORT:
          src = &csu_GsmSrcPort[((Uint8)(s_pidx>>8))][((Uint8)(s_pidx))];
          break;
     case CSU_RTP_PORT:
          src = &csu_RtpSrcPort[s_pidx];
          break;
     case CSU_ANN_PORT:
	    src = &csu_AnnSrcPort[s_pidx];
          break;
     default:
          DBG_ERROR("CSU Error csu_ConnSrcPort(): invalid source port type '%d'\n", s_type);
          DBG_LEAVE();
          return CSU_RESULT_FAILED;
     }

     //Update src port as connected
     for (i=0;i<CSU_PEER_PORT_MAX;i++)
     {
         if (src->peer[i].type==CSU_NIL_PORT)
         {
             src->state = CSU_RESSTATE_USED;
             src->count++;
             src->test = CSU_TEST_FUNC_NULL;
             src->peer[i].type = d_type;
             src->peer[i].pidx = d_pidx;
		 src->peer[i].ppos = 0;
		 src->peer[i].speech_version = port_id->speechVersion;

//DECIDEPT:
		 if (d_type==CSU_RTP_PORT)
		     src->peer[i].ldty=(Uint8)VoipCallGetRtpPayloadType(csu_RtpSnkPort[d_pidx].rtpHandler);
//TESTADDED
		 src->seqn = 0;
             src->seqc = 0;
             break;
         }
     }
     
     //The following for double check, should not happen in the design 
     if (i>=CSU_PEER_PORT_MAX)
     { 
         DBG_ERROR("CSU Error csu_ConnSrcPort(): Src(%d,%x) peer[] constraint, count=%d,limit=%d,MAX=%d\n",
                    s_type, s_pidx, src->count, src->limit, CSU_PEER_PORT_MAX);
         DBG_LEAVE();
         return CSU_RESULT_FAILED;
     }

     DBG_LEAVE();
     return CSU_RESULT_SUCCESS;
}

//Finish snk port of a connection 
T_CSU_RESULT csu_ConnSnkPort(Uint8 d_type, Uint16 d_pidx, Uint8 s_type, Uint16 s_pidx, T_CSU_PORT_ID *port_id)
{
     Uint8          i;
     T_CSU_SNK_PORT *snk;
     T_CSU_RESULT   ret;

     DBG_FUNC("csu_ConnSnkPort", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU Info csu_ConnSnkPort(): srcIdx(%d, %x)-->snkIdx(%d,%x)\n",
                s_type, s_pidx, d_type, d_pidx);

     switch(d_type)
     {
     case CSU_GSM_PORT:
          snk = &csu_GsmSnkPort[((Uint8)(d_pidx>>8))][((Uint8)(d_pidx))];
          break;
     case CSU_RTP_PORT:
          snk = &csu_RtpSnkPort[d_pidx];
          break;
     //case CSU_ANN_PORT:
	   
     default:
          DBG_ERROR("CSU Error csu_ConnSnkPort(): invalid snk port type '%d'\n", d_type);
          DBG_LEAVE();
          return CSU_RESULT_FAILED;
     }

     //Update snk port as connected
     for (i=0;i<CSU_PEER_PORT_MAX;i++)
     {
         if (snk->peer[i].type==CSU_NIL_PORT)
         {
             snk->peer[i].type = s_type;
             snk->peer[i].pidx = s_pidx;
		 snk->speech_version = port_id->speechVersion;
             snk->state = CSU_RESSTATE_USED;
             snk->count++;
             snk->test = CSU_TEST_FUNC_NULL;
		 
//TESTADDED
		 snk->seqn = 0;
             snk->seqc = 0;

             break;
         }
     }

     //The following for double check, should not happen in the design 
     if (i>=CSU_PEER_PORT_MAX)
     { 
         DBG_ERROR("CSU Error csu_ConnSnkPort(): SnkIdx(%d,%x) peer[] constraint, count=%d,limit=%d,MAX=%d\n",
                    d_type, d_pidx, snk->count, snk->limit, CSU_PEER_PORT_MAX);
         DBG_LEAVE();
         return CSU_RESULT_FAILED;
     }
 
     DBG_LEAVE();
     return CSU_RESULT_SUCCESS;
}

//Check if src knows src->snk connection
T_CSU_RESULT csu_CheckSrcConn(
             Uint8 s_type, Uint16 s_pidx, Uint8 d_type, Uint16 d_pidx)
{
     Uint8 i, trx, slot;
     T_CSU_SRC_PORT *src;
     T_CSU_RESULT ret;

     DBG_FUNC("csu_CheckSrcConn", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU Info csu_CheckSrcConn(): srcIdx(%d, %x)-->snkIdx(%d,%x)\n",
               s_type, s_pidx, d_type, d_pidx);

     //Calling function responsible for validity of passed parameters
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
          DBG_ERROR("CSU Error csu_CheckSrcConn(): invalid src port type '%d'\n", s_type);
          DBG_LEAVE();
          return CSU_RESULT_FAILED;
     }

     //Check if src port engaged or not
     if (src->state != CSU_RESSTATE_USED &&
         src->count == 0)
     {   
         DBG_ERROR("CSU Error csu_CheckSrcConn(): src port (%d,%x) not engaged\n",
                    s_type, src->rtpHandler);
         DBG_LEAVE();
         return CSU_RESULT_FAILED;
     }

     //Make sure src port enaged with snk port
     for (i=0;i<CSU_PEER_PORT_MAX;i++)
     {
	  if (src->peer[i].type == d_type &&
            src->peer[i].pidx == d_pidx )
        {
            DBG_LEAVE();
            return CSU_RESULT_SUCCESS;
        }
     }

     DBG_ERROR("CSU Error csu_CheckSrcConn(): src port (%d, %x) not engaged with snkIdx(%d,%x)\n",
                s_type, src->rtpHandler, d_type, d_pidx);
    
     DBG_LEAVE();
     return CSU_RESULT_FAILED;
}

//Check if snk knows src->snk connection
T_CSU_RESULT csu_CheckSnkConn(
             Uint8 d_type, Uint16 d_pidx, Uint8 s_type, Uint16 s_pidx)
{
     Uint8          i, trx, slot;
     T_CSU_SNK_PORT *snk;
     T_CSU_RESULT   ret;

     DBG_FUNC("csu_CheckSnkConn", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU Info csu_CheckSnkConn(): srcIdx(%d, %x)-->snkIdx(%d,%x)\n",
               s_type, s_pidx, d_type, d_pidx);

     //Calling function responsible for validity of passed parameters
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
          DBG_ERROR("CSU Error csu_CheckSnkConn(): invalid snk port type: %d\n", d_type);
          DBG_LEAVE();
          return CSU_RESULT_FAILED;
     }

     //Check if snk port engaged or not
     if (snk->state != CSU_RESSTATE_USED ||
         snk->count == 0)
     {   
         DBG_ERROR("CSU Error csu_CheckSnkConn():snk port (%d,%x) not engaged \n",
                    d_type, snk->rtpHandler);
         DBG_LEAVE();
         return CSU_RESULT_FAILED;
     }

     //Make sure snk port enaged with src port
     for (i=0;i<CSU_PEER_PORT_MAX;i++)
     {
	  if (snk->peer[i].type == s_type &&
              snk->peer[i].pidx == s_pidx )
        {
              DBG_LEAVE();
              return CSU_RESULT_SUCCESS;
        }
     }

     DBG_ERROR("CSU Error csu_CheckSnkConn(): srcIdx(%d, %x) not engaged with snk (%d,%x)\n",
                s_type, s_pidx, d_type, snk->rtpHandler);
     DBG_LEAVE();
     return CSU_RESULT_FAILED;
}

//Break source port's engagement in the connection
T_CSU_RESULT csu_BreakSrcConn(Uint8 s_type, Uint16 s_pidx, Uint8 d_type, Uint16 d_pidx)
{
     T_CSU_SRC_PORT *src;

     DBG_FUNC("csu_BreakSrcConn", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU Info csu_BreakSrcConn(): srcIdx(%d, %x)-->snkIdx(%d,%x)\n",
               s_type, s_pidx, d_type, d_pidx);

     //Find the source port
     switch(s_type)
     {
     case CSU_GSM_PORT:
          Uint8 trx, slot;
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
          DBG_ERROR("CSU Error csu_BreakSrcConn(): invalid src type in srcIdx(%d, %x)\n",
                     s_type, s_pidx);
          DBG_LEAVE();
          return CSU_RESULT_FAILED;
     }

     //Clear the source port's engagement
     for (int i=0;i<CSU_PEER_PORT_MAX;i++)
     {
          if (src->peer[i].type==d_type &&
              src->peer[i].pidx==d_pidx )
          {
	        src->peer[i].type = CSU_NIL_PORT;
              src->peer[i].pidx = CSU_PEER_PORT_NULL;
		  src->peer[i].ppos = 0;
              if (src->count==0)
              {
                  DBG_TRACE("CSU INFO csu_BreakSrcConn(): src(%d, %x) engagment count = 0\n",
                              s_type, src->rtpHandler);

                  //Check if any pending state transition
                  if ( src->resvd!=CSU_RESSTATE_IGNORE )
                  { 
                     src->state = src->resvd;
                     src->resvd = CSU_RESSTATE_IGNORE;
                  } else
                     src->state = CSU_RESSTATE_FREE;

                  break;
              }

              if ( (--src->count) == 0 )
              {
                  //Check if any pending state transition
                  if ( src->resvd!=CSU_RESSTATE_IGNORE )
                  { 
                     src->state = src->resvd;
                     src->resvd = CSU_RESSTATE_IGNORE;
                  } else
                     src->state = CSU_RESSTATE_FREE;
                  break;
              }

	    }
     }

     //Check if found the sink port of the src->snk connection   
     if (i>=CSU_PEER_PORT_MAX)
     {
         DBG_TRACE("CSU INFO csu_BreakSrcConn(): no engagment of src(%d,%x)->snkIdx(%d,%x)\n",
                      s_type, src->rtpHandler, d_type, d_pidx);
     }
  
     DBG_LEAVE();
     return CSU_RESULT_SUCCESS;
}

//Break the snk port's engagement in the connection
T_CSU_RESULT csu_BreakSnkConn(Uint8 d_type, Uint16 d_pidx, Uint8 s_type, Uint16 s_pidx)
{
     T_CSU_SNK_PORT *snk;

     DBG_FUNC("csu_BreakSnkConn", CSU_LAYER);
     DBG_ENTER();

     DBG_TRACE("CSU Info csu_BreakSnkConn(): srcIdx(%d, %x)-->snkIdx(%d,%x)\n",
               s_type, s_pidx, d_type, d_pidx);

     //Find the sink port
     switch(d_type)
     {
     case CSU_GSM_PORT:
          Uint8 trx, slot;
          trx  = (Uint8)(d_pidx>>8);
          slot = (Uint8)(d_pidx);
          snk  = &csu_GsmSnkPort[trx][slot];
          break;
     case CSU_RTP_PORT:
          snk  = &csu_RtpSnkPort[d_pidx];
          break;
     //case CSU_ANN_PORT: 
	    
     default:
          DBG_ERROR("CSU Error csu_BreakSnkConn(): invalid snk port type in snk(%d,%x)\n",
                     d_type, snk->rtpHandler);
          DBG_LEAVE();
          return CSU_RESULT_FAILED;
     }

     //Clear the source port's engagement
     for (int i=0;i<=CSU_PEER_PORT_MAX;i++)
     {
          if (snk->peer[i].type==s_type &&
              snk->peer[i].pidx==s_pidx )
          {
	        snk->peer[i].type = CSU_NIL_PORT;
              snk->peer[i].pidx = CSU_PEER_PORT_NULL;
              if (snk->count==0)
              {
                  DBG_TRACE("CSU INFO csu_BreakSnkConn(): snk(%d, %x) engagment count = 0\n",
                              d_type, snk->rtpHandler);
                  //Check if any pending state transition
                  if ( snk->resvd!=CSU_RESSTATE_IGNORE )
                  { 
                     snk->state = snk->resvd;
                     snk->resvd = CSU_RESSTATE_IGNORE;
                  } else
                     snk->state = CSU_RESSTATE_FREE;
                  break;
              }

              if ( (--snk->count) == 0 )
              {
                  //Check if any pending state transition
                  if ( snk->resvd!=CSU_RESSTATE_IGNORE )
                  { 
                     snk->state = snk->resvd;
                     snk->resvd = CSU_RESSTATE_IGNORE;
                  } else
                     snk->state = CSU_RESSTATE_FREE;
                  break;
              }
	  }
     }

     //Check if found the sink port of the snk->snk connection   
     if (i>=CSU_PEER_PORT_MAX)
     {
         DBG_TRACE("CSU TRACE csu_BreakSrcConn(): no engagment of snk (%d,%x)<-- srcIdx(%d,%x)\n",
                      d_type, snk->rtpHandler, s_type, s_pidx);
     }

     DBG_LEAVE();
     return CSU_RESULT_SUCCESS;
}

#endif /*__CSU_CUTIL_CPP__*/
