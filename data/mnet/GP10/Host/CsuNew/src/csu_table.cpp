/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef  __CSU_TABLE_CPP__
#define  __CSU_TABLE_CPP__

#include "CsuNew\csu_head.h"

extern DbgOutput csu_passDbg;
extern DbgOutput csu_testDbg;

//Initialize GSM src port table
void csu_TblInitGsmSrcPort(void)
{
     Uint8 i,j,k;

     DBG_FUNC("csu_TblInitGsmSrcPort", CSU_LAYER);
     DBG_ENTER();

     //Initialize GSM src trunk resource
     for (i=0;i<CSU_GSM_SRC_TRX_MAX;i++)
     for (j=0;j<CSU_GSM_SRC_SLOT_MAX;j++)
     {
	  csu_GsmSrcPort[i][j].state = CSU_RESSTATE_NULL;
        csu_GsmSrcPort[i][j].resvd = CSU_RESSTATE_IGNORE;
	  csu_GsmSrcPort[i][j].count = 0;
        csu_GsmSrcPort[i][j].nopr  = 0;
        csu_GsmSrcPort[i][j].test = CSU_TEST_FUNC_NULL;

          csu_GsmSrcPort[i][j].type  = CSU_GSM_PORT;
          csu_GsmSrcPort[i][j].limit = CSU_SRC_CONN_MAX;
          csu_GsmSrcPort[i][j].gsmHandler = (Uint16)((i<<8)|j);
          for (k=0;k<CSU_PEER_PORT_MAX;k++)
          {
	       csu_GsmSrcPort[i][j].peer[k].type = CSU_NIL_PORT;
               csu_GsmSrcPort[i][j].peer[k].pidx = CSU_PEER_PORT_NULL;
	  }
     }
}

//Initialize ANN src port table
void csu_TblInitAnnSrcPort(void)
{
     Uint8 i,k;

     DBG_FUNC("csu_TblInitAnnSrcPort", CSU_LAYER);
     DBG_ENTER();

     //Initialize ANN src trunk resource
     for (i=0;i<CSU_ANN_SRC_PORT_MAX;i++)
     {
	  csu_AnnSrcPort[i].state = CSU_RESSTATE_NULL;
        csu_AnnSrcPort[i].resvd = CSU_RESSTATE_IGNORE;
	  csu_AnnSrcPort[i].count = 0;
        csu_AnnSrcPort[i].nopr  = 0;
        csu_AnnSrcPort[i].test = CSU_TEST_FUNC_NULL;

          csu_AnnSrcPort[i].type  = CSU_ANN_PORT;
          csu_AnnSrcPort[i].limit = CSU_SRC_CONN_MAX;
          csu_AnnSrcPort[i].annHandler = (T_CSU_ANN_ID)0;
          for (k=0;k<CSU_PEER_PORT_MAX;k++)
          {
	       csu_AnnSrcPort[i].peer[k].type = CSU_NIL_PORT;
               csu_AnnSrcPort[i].peer[k].pidx = CSU_PEER_PORT_NULL;
               csu_AnnSrcPort[i].peer[k].ppos = 0;

	  }
     }
}

//Initialize RTP src port table
void csu_TblInitRtpSrcPort(void)
{
     Uint8 i,j;

     DBG_FUNC("csu_TblInitRtpSrcPort", CSU_LAYER);
     DBG_ENTER();

     //Initialize RTP src trunk resource
     for (i=0;i<CSU_RTP_SRC_PORT_MAX;i++)
     {
          csu_RtpSrcPort[i].state = CSU_RESSTATE_NULL;
          csu_RtpSrcPort[i].resvd = CSU_RESSTATE_IGNORE;
	  csu_RtpSrcPort[i].count = 0;
        csu_RtpSrcPort[i].nopr  = 0;
        csu_RtpSrcPort[i].test = CSU_TEST_FUNC_NULL;

          csu_RtpSrcPort[i].type  = CSU_RTP_PORT;
          csu_RtpSrcPort[i].limit = CSU_SRC_CONN_MAX;
          csu_RtpSrcPort[i].rtpHandler = (HRTPSESSION)0;
          for (j=0;j<CSU_PEER_PORT_MAX;j++)
          {
 	       csu_RtpSrcPort[i].peer[j].type = CSU_NIL_PORT;
               csu_RtpSrcPort[i].peer[j].pidx = CSU_PEER_PORT_NULL;
          }
     }
}

//Initialize GSM snk port table
void csu_TblInitGsmSnkPort(void)
{
     Uint8 i,j,k;

     DBG_FUNC("csu_TblInitGsmSnkPort", CSU_LAYER);
     DBG_ENTER();

     //Initialize GSM snk trunk resource
     for (i=0;i<CSU_GSM_SNK_TRX_MAX;i++)
     for (j=0;j<CSU_GSM_SNK_SLOT_MAX;j++)
     {
	  csu_GsmSnkPort[i][j].state = CSU_RESSTATE_NULL;
          csu_GsmSnkPort[i][j].resvd = CSU_RESSTATE_IGNORE;
	  csu_GsmSnkPort[i][j].count = 0;
        csu_GsmSnkPort[i][j].nopr  = 0;

        csu_GsmSrcPort[i][j].test = CSU_TEST_FUNC_NULL;

          csu_GsmSnkPort[i][j].type  = CSU_GSM_PORT;
          csu_GsmSnkPort[i][j].limit = 1; //TBD CSU_SNK_CONN_MAX;
          csu_GsmSnkPort[i][j].gsmHandler = (Uint16)((i<<8)|j);
          for (k=0;k<CSU_PEER_PORT_MAX;k++)
          {
 	       csu_GsmSnkPort[i][j].peer[k].type = CSU_NIL_PORT;
               csu_GsmSnkPort[i][j].peer[k].pidx = CSU_PEER_PORT_NULL;
	  }
     }
}

//Initialize RTP snk port table
void csu_TblInitRtpSnkPort(void)
{
     Uint8 i,j;

     DBG_FUNC("csu_TblInitRtpSnkPort", CSU_LAYER);
     DBG_ENTER();

     //Initialize RTP snk trunk resource
     for (i=0;i<CSU_RTP_SNK_PORT_MAX;i++)
     {
          csu_RtpSnkPort[i].state = CSU_RESSTATE_NULL;
          csu_RtpSnkPort[i].resvd = CSU_RESSTATE_IGNORE;
	  csu_RtpSnkPort[i].count = 0;
        csu_RtpSnkPort[i].nopr  = 0;
        csu_RtpSnkPort[i].test = CSU_TEST_FUNC_NULL;

          csu_RtpSnkPort[i].type  = CSU_RTP_PORT;
          csu_RtpSnkPort[i].limit = 1; //TBD CSU_SNK_CONN_MAX;
          csu_RtpSnkPort[i].rtpHandler = (HRTPSESSION)0;
          for (j=0;j<CSU_PEER_PORT_MAX;j++)
          {
 	       csu_RtpSnkPort[i].peer[j].type = CSU_NIL_PORT;
               csu_RtpSnkPort[i].peer[j].pidx = CSU_PEER_PORT_NULL;
	  }
     }
}


//Initialize the whole connection resources
void csu_TblInitAll(void)
{
     DBG_FUNC("csu_TblInitAll", CSU_LAYER);
     DBG_ENTER();

     csu_TakeSemaphore();

     csu_TblInitRtpSrcPort();
     csu_TblInitRtpSnkPort();
     csu_TblInitGsmSrcPort();
     csu_TblInitGsmSnkPort();
     csu_TblInitAnnSrcPort();
    

     csu_GiveSemaphore();

     //Init Ann resources here later
     
     // Initialize the global DbgOutput object used for ViperLog traces
     // in the csu_pass.cpp file.
     csu_passDbg.Func("csu_pass.cpp",CSU_LAYER_SPEECH);
     csu_testDbg.Func("csu_test.cpp",CSU_LAYER_DEBUG);
}

#endif /*__CSU_TABLE_CPP__*/
