/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef  __CSU_REG_CPP__
#define  __CSU_REG_CPP__

#include "CsuNew\csu_head.h"

//Register port as src port	
T_CSU_RESULT_REG csu_RegSrcPort(T_CSU_PORT_ID *port)
{
     Uint8  i,j,trx,slot;
     Uint16 gsmHandler;
     T_CSU_SRC_PORT *src;

     DBG_FUNC("csu_RegSrcPort", CSU_LAYER);
     DBG_ENTER();
 
     switch(port->portType)
     {
     case CSU_GSM_PORT:
          //Registering a GSM port
          if (!rm_EntryIdToTrxSlot(port->portId.gsmHandler,&gsmHandler))
          {
              DBG_TRACE("FAILED in converting src entryId: entryId=%d,handler=%x\n",
                         port->portId.gsmHandler, gsmHandler);
              return CSU_RESULT_REG_PORT_NOT_REGISTERED;
          }

          trx = (Uint8)(gsmHandler>>8); slot = (Uint8)(gsmHandler);
          src = &csu_GsmSrcPort[trx][slot];
          src->gsmHandler = gsmHandler;
      
          //Do registration if proper
          if ( CSU_RESULT_SUCCESS != csu_DoSrcPortReg(src) )
          {
		   DBG_TRACE("FAILED in register GSM src port:entryId=%d, handler=%x\n",
                         port->portId.gsmHandler, gsmHandler);
               return CSU_RESULT_REG_PORT_REGISTER_FAILED;
          }
          break;
           
     case CSU_RTP_PORT:
          // Check if it is already registered
          for (i=0;i<CSU_RTP_SRC_PORT_MAX;i++)
          {
              if (csu_RtpSrcPort[i].state != CSU_RESSTATE_NULL &&
                  csu_RtpSrcPort[i].rtpHandler==port->portId.rtpHandler)
              {
                  DBG_TRACE("src RTP port already registered:handler=%x, state:%d\n",
		             port->portId.rtpHandler,csu_RtpSrcPort[i].state );
			  
		  return CSU_RESULT_REG_PORT_ALREADY_REGISTERED;
	      }
 	  }

          // Not registered yet, do it now.
          for (i=0;i<CSU_RTP_SRC_PORT_MAX;i++)
          {
	       if (csu_RtpSrcPort[i].state == CSU_RESSTATE_NULL)
             {
                 src = &csu_RtpSrcPort[i];
                 src->rtpHandler = port->portId.rtpHandler;
    
                 //Do registration here
                 if ( CSU_RESULT_SUCCESS != csu_DoSrcPortReg(src) )
                 {
		          DBG_TRACE("FAILED in register RTP src port:handler=%x\n",
                                 port->portId.rtpHandler);
                      return CSU_RESULT_REG_PORT_REGISTER_FAILED;
                 }
                 break;
		 }
          }

          //No more room left in src table to register the src port
          if (i>=CSU_RTP_SRC_PORT_MAX)
          {
              DBG_TRACE("RTP src port table constraint:size=%d\n",CSU_RTP_SRC_PORT_MAX);
              return CSU_RESULT_REG_PORT_REGISTER_OVERFLOW;
          }
          break;

     case CSU_ANN_PORT:
	    for (i=0;i<CSU_ANN_SRC_PORT_MAX;i++)
          {
	       if (csu_AnnSrcPort[i].state == CSU_RESSTATE_NULL)
             {
                 src = &csu_AnnSrcPort[i];
                 src->annHandler = port->portId.annHandler;
    
                 //Do registration here
                 if ( CSU_RESULT_SUCCESS != csu_DoSrcPortReg(src) )
                 {
		          DBG_TRACE("FAILED in register ANN src port:handler=%x\n",
                                 port->portId.annHandler);
                      return CSU_RESULT_REG_PORT_REGISTER_FAILED;
                 }
                 break;
		 }
          }

          //No more room left in src table to register the src port
          if (i>=CSU_ANN_SRC_PORT_MAX)
          {
              DBG_TRACE("ANN src port table constraint:size=%d\n",CSU_ANN_SRC_PORT_MAX);
              return CSU_RESULT_REG_PORT_REGISTER_OVERFLOW;
          }
          break;

     default:
          DBG_TRACE("Registering an invalid src portType:%d\n", port->portType);
          return CSU_RESULT_REG_PORT_TYPE_UNSUPPORTED;
     }
      
     return CSU_RESULT_REG_SUCCESS;
}     

//Register port as snk port	
T_CSU_RESULT_REG csu_RegSnkPort(T_CSU_PORT_ID *port)
{
     Uint8  i,j,trx,slot;
     Uint16 gsmHandler;
     T_CSU_SNK_PORT *snk;

     DBG_FUNC("csu_RegSnkPort", CSU_LAYER);
     DBG_ENTER();
 
     switch(port->portType)
     {
     case CSU_GSM_PORT:
          //Registering a GSM port
          if (!rm_EntryIdToTrxSlot(port->portId.gsmHandler,&gsmHandler))
          {
              DBG_TRACE("FAILED in converting snk entryId: entryId=%d,handler=%x\n",
                         port->portId.gsmHandler, gsmHandler);
              return CSU_RESULT_REG_PORT_NOT_REGISTERED;
          }

          trx = (Uint8)(gsmHandler>>8); slot = (Uint8)(gsmHandler);
          snk = &csu_GsmSnkPort[trx][slot];
          snk->gsmHandler = gsmHandler;
      
          //Do registration if proper
          if ( CSU_RESULT_SUCCESS != csu_DoSnkPortReg(snk) )
          {
	       DBG_TRACE("FAILED in register GSM snk port:entryId=%d, handler=%x\n",
                         port->portId.gsmHandler, gsmHandler);
               return CSU_RESULT_REG_PORT_REGISTER_FAILED;
          }
          break;
           
     case CSU_RTP_PORT:
          // Check if it is already registered
          for (i=0;i<CSU_RTP_SNK_PORT_MAX;i++)
          {
              if (csu_RtpSnkPort[i].state != CSU_RESSTATE_NULL &&
                  csu_RtpSnkPort[i].rtpHandler==port->portId.rtpHandler)
              {
                  DBG_TRACE("snk RTP port already registered:handler=%x, state:%d\n",
		             port->portId.rtpHandler,csu_RtpSnkPort[i].state );
			  
		  return CSU_RESULT_REG_PORT_ALREADY_REGISTERED;
	      }
 	  }

          // Not registered yet, do it now.
          for (i=0;i<CSU_RTP_SNK_PORT_MAX;i++)
          {
	       if (csu_RtpSnkPort[i].state == CSU_RESSTATE_NULL)
               {
                   snk = &csu_RtpSnkPort[i];
                   snk->rtpHandler = port->portId.rtpHandler;
    
                   //Do registration here
                   if ( CSU_RESULT_SUCCESS != csu_DoSnkPortReg(snk) )
                   {
		        DBG_TRACE("FAILED in register RTP snk port:handler=%x\n",
                                   port->portId.rtpHandler);
                        return CSU_RESULT_REG_PORT_REGISTER_FAILED;
                   }
                   break;
	       }
          }

          //No more room left in snk table to register the snk port
          if (i>=CSU_RTP_SNK_PORT_MAX)
          {
              DBG_TRACE("RTP snk port table constraint:size=%d\n",CSU_RTP_SNK_PORT_MAX);
              return CSU_RESULT_REG_PORT_REGISTER_OVERFLOW;
          }
          break;

     //case CSU_ANN_PORT:
	    
     default:
          DBG_TRACE("Registering an invalid snk portType:%d\n", port->portType);
          return CSU_RESULT_REG_PORT_TYPE_UNSUPPORTED;
     }
      
     return CSU_RESULT_REG_SUCCESS;
}     

#include "CsuNew\csu_head.h"

//Register port as both src and snk port	
T_CSU_RESULT_REG csu_RegSrcSnkPort(T_CSU_PORT_ID *port)
{
     T_CSU_RESULT_REG ret;
 
     DBG_FUNC("csu_RegSrcSnkPort", CSU_LAYER);
     DBG_ENTER();

     csu_TakeSemaphore();
  
     //Register port as src port
     if ( CSU_RESULT_REG_SUCCESS != 
          (ret=csu_RegSrcPort(port)) )
     {
          DBG_TRACE("FAILED in register port as src port:typ=%d,handler=%x\n",
                     port->portType, port->portId.rtpHandler);
          csu_GiveSemaphore();
          return ret;
     }

     //Register port as snk port
     if ( CSU_RESULT_REG_SUCCESS != 
          (ret=csu_RegSnkPort(port)) )
     {
          DBG_TRACE("FAILED in register port as snk port:typ=%d,handler=%x\n",
                     port->portType, port->portId.rtpHandler);
          csu_GiveSemaphore();

          return ret;
     }

     csu_GiveSemaphore();

     return CSU_RESULT_REG_SUCCESS;
}     

//Unregister port as src port	
T_CSU_RESULT_REG csu_UnregSrcPort(T_CSU_PORT_ID *port)
{
     Uint8  i,j,trx,slot;
     Uint16 gsmHandler;
     T_CSU_SRC_PORT *src;

     DBG_FUNC("csu_UnregSrcPort", CSU_LAYER);
     DBG_ENTER();
 
     switch(port->portType)
     {
     case CSU_GSM_PORT:
          //Unregistering a GSM port
          if (!rm_EntryIdToTrxSlot(port->portId.gsmHandler,&gsmHandler))
          {
              DBG_TRACE("FAILED in converting src entryId: entryId=%d,handler=%x\n",
                         port->portId.gsmHandler, gsmHandler);
              return CSU_RESULT_REG_PORT_NOT_REGISTERED;
          }

          // Check if it's already registered
          trx = (Uint8)(gsmHandler>>8); slot = (Uint8)(gsmHandler);

          src = &csu_GsmSrcPort[trx][slot];
          if ( CSU_RESULT_SUCCESS != csu_DoSrcPortUnreg(src) )
          {
	       DBG_TRACE("FAILED in unreg GSM src port:entryId=%d,handler=%x\n",
                          port->portId.gsmHandler,gsmHandler);
               return CSU_RESULT_REG_PORT_REGISTER_FAILED;
          }                       

          break;
           
     case CSU_RTP_PORT:
          // Check if it is already registered
          for (i=0;i<CSU_RTP_SRC_PORT_MAX;i++)
          {
              if (csu_RtpSrcPort[i].state != CSU_RESSTATE_NULL &&
                  csu_RtpSrcPort[i].rtpHandler==port->portId.rtpHandler)
              {
                  src = &csu_RtpSrcPort[i];
                  break;
              }
          } 
          
          if (i>=CSU_RTP_SRC_PORT_MAX)
          {
	       DBG_TRACE("RTP src port '%x' actually not registered\n",
                          port->portId.rtpHandler);
               return CSU_RESULT_REG_PORT_REGISTER_FAILED;
          }

          if ( CSU_RESULT_SUCCESS != csu_DoSrcPortUnreg(src) )
          {
	       DBG_TRACE("FAILED in unreg RTP src port '%x'\n",
                          port->portId.rtpHandler);
               return CSU_RESULT_REG_PORT_REGISTER_FAILED;
          } 

          break;

     case CSU_ANN_PORT:
	    // Check if it is already registered
          for (i=0;i<CSU_ANN_SRC_PORT_MAX;i++)
          {
              if (csu_AnnSrcPort[i].state != CSU_RESSTATE_NULL &&
                  csu_AnnSrcPort[i].annHandler==port->portId.annHandler)
              {
                  src = &csu_AnnSrcPort[i];
                  break;
              }
          } 
          
          if (i>=CSU_ANN_SRC_PORT_MAX)
          {
	       DBG_TRACE("ANN src port '%x' actually not registered\n",
                          port->portId.annHandler);
               return CSU_RESULT_REG_PORT_REGISTER_FAILED;
          }

          if ( CSU_RESULT_SUCCESS != csu_DoSrcPortUnreg(src) )
          {
	       DBG_TRACE("FAILED in unreg ANN src port '%x'\n",
                          port->portId.annHandler);
               return CSU_RESULT_REG_PORT_REGISTER_FAILED;
          } 

          break;

     default:
          DBG_TRACE("Unregistering an invalid src portType:%d\n",
                     port->portType);
          return CSU_RESULT_REG_PORT_TYPE_UNSUPPORTED;
     }

     return CSU_RESULT_REG_SUCCESS;
}     


//Unregister port as snk port	
T_CSU_RESULT_REG csu_UnregSnkPort(T_CSU_PORT_ID *port)
{
     Uint8          i,j,trx,slot;
     Uint16         gsmHandler;
     T_CSU_SNK_PORT *snk;

     DBG_FUNC("csu_UnregSnkPort", CSU_LAYER);
     DBG_ENTER();
 
     switch(port->portType)
     {
     case CSU_GSM_PORT:
          //Unregistering a GSM port
          if (!rm_EntryIdToTrxSlot(port->portId.gsmHandler,&gsmHandler))
          {
              DBG_TRACE("FAILED in converting snk entryId: entryId=%d,handler=%x\n",
                         port->portId.gsmHandler, gsmHandler);
              return CSU_RESULT_REG_PORT_NOT_REGISTERED;
          }

          // Check if it's already registered
          trx = (Uint8)(gsmHandler>>8); slot = (Uint8)(gsmHandler);

          snk = &csu_GsmSnkPort[trx][slot];
          if ( CSU_RESULT_SUCCESS != csu_DoSnkPortUnreg(snk) )
          {
	       DBG_TRACE("FAILED in unreg GSM snk port:entryId=%d,handler=%x\n",
                          port->portId.gsmHandler,gsmHandler);
               return CSU_RESULT_REG_PORT_REGISTER_FAILED;
          }                       

          break;
           
     case CSU_RTP_PORT:
          // Check if it is already registered
          for (i=0;i<CSU_RTP_SNK_PORT_MAX;i++)
          {
              if (csu_RtpSnkPort[i].state != CSU_RESSTATE_NULL &&
                  csu_RtpSnkPort[i].rtpHandler==port->portId.rtpHandler)
              {
                  snk = &csu_RtpSnkPort[i];
                  break;
              }
          } 
          
          if (i>=CSU_RTP_SNK_PORT_MAX)
          {
	       DBG_TRACE("RTP snk port '%x' actually not registered\n",
                          port->portId.rtpHandler);
               return CSU_RESULT_REG_PORT_REGISTER_FAILED;
          }

          if ( CSU_RESULT_SUCCESS != csu_DoSnkPortUnreg(snk) )
          {
	       DBG_TRACE("FAILED in unreg RTP snk port '%x'\n",
                          port->portId.rtpHandler);
               return CSU_RESULT_REG_PORT_REGISTER_FAILED;
          } 

          break;

     //case CSU_ANN_PORT:
	   
     default:
          DBG_TRACE("Unregistering an invalid snk portType:%d\n",
                     port->portType);
          return CSU_RESULT_REG_PORT_TYPE_UNSUPPORTED;
     }

     return CSU_RESULT_REG_SUCCESS;
}     

//Unregister port as both src and snk port	
T_CSU_RESULT_REG csu_UnregSrcSnkPort(T_CSU_PORT_ID *port)
{
     T_CSU_RESULT_REG ret;

     DBG_FUNC("csu_UnregSrcSnkPort", CSU_LAYER);
     DBG_ENTER();

     //Unregister port as source port
     if ( CSU_RESULT_REG_SUCCESS != 
          (ret=csu_UnregSrcPort(port)) )
     {
	  DBG_TRACE("FAILED in unreg port (%d,%x) as src port\n",
                     port->portType, port->portId.rtpHandler);
          return ret;
     }

     //Unregister port as sink port
     if ( CSU_RESULT_REG_SUCCESS != 
          (ret=csu_UnregSnkPort(port)) )
     {
	  DBG_TRACE("FAILED in unreg port (%d,%x) as snk port\n",
                     port->portType, port->portId.rtpHandler);
          return ret;
     }

     return CSU_RESULT_REG_SUCCESS;
}

#endif /*__CSU_REG_CPP__*/
