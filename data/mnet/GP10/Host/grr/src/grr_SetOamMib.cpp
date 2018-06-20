/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_SETOAMMIB_CPP__
#define __GRR_SETOAMMIB_CPP__

#include "grr\grr_head.h"

void grr_SetOamBtsAmState(AdministrativeState state)
{
  DBG_FUNC("grr_SetOamBtsAmState", GRR_LAYER_OAM);
  DBG_ENTER();

  DBG_TRACE("grr_SetOamBtsAmState: state(%d)\n", state);

  OAMgrr_BTS_AMSTATEa = state;

  if (STATUS_OK !=
      oam_setMibIntVar(MODULE_GRR,MIB_bts_administrativeState, state) )
  {
      grr_SubmitAlarm(EC_GRR_OAM_CALL_SETMIBINTVAR_FAILED);
      DBG_ERROR("grr_SetOamBtsAmState: oam_setMibIntVar failed for setting bts amState(%d)\n",
	           state);
      printf("grr_SetOamBtsAmState: oam_setMibIntVar failed for setting bts amState(%d)\n",
	           state);
      assert(0);
  }
  DBG_LEAVE();
}

void grr_SetOamTrxAmState(unsigned char trx, AdministrativeState state)
{
  MibTag tag;

  DBG_FUNC("grr_SetOamTrxAmState", GRR_LAYER_OAM);
  DBG_ENTER();

  DBG_TRACE("grr_SetOamTrxAmState: set trx(%d) amState(%d)\n", trx, state);

  //Check if the requested trx existent
  if (trx>=OAMgrr_MAX_TRXS)
  {
      DBG_WARNING("grr_SetOamTrxAmState: setting amState(%d) for invalid trx(%d)\n", state,trx);
      DBG_LEAVE();
      return;
  }
     
  OAMgrr_TRX_AMSTATEa(trx) = state;

  if (trx==0) tag = MIB_basebandAdministrativeState_0;
  else 	  tag = MIB_basebandAdministrativeState_1;

  if ( STATUS_OK!=oam_setMibIntVar(MODULE_GRR, tag, state) )
  {
      grr_SubmitAlarm(EC_GRR_OAM_CALL_SETMIBINTVAR_FAILED);
      DBG_ERROR("grr_SetOamTrxAmState: oam_setMibIntVar failed for setting trx(%d) amState(%d)\n",
              trx, state);
      printf("grr_SetOamTrxAmState: failed in calling oam_setMibIntVar for trx(%d) amState(%d)\n",
              trx, state);
      assert(0);
  }

  DBG_LEAVE();
}

void grr_SetOamTrxSlotAmState(unsigned char trx, unsigned char slot, AdministrativeState state)
{
  MibTag tag;
  STATUS sta;

  DBG_FUNC("grr_SetOamTrxSlotAmState", GRR_LAYER_OAM);
  DBG_ENTER();

  DBG_TRACE("grr_SetOamTrxSlotAmState: set trx(%d) slot(%d) amState(%d)\n", trx, slot, state);

  //Check if (trx,slot) existent
  if ( (trx>=OAMgrr_MAX_TRXS) || (slot>=8) )
  {
      DBG_WARNING("grr_SetOamTrxSlotAmState: set amState(%d) for invalid trx(%d) slot(%d)\n",
                   state, trx, slot);
      DBG_LEAVE();
      return;
  }
    
  if (trx==0) tag=MIBT_channelAdministrativeState_0;
  else 	  tag=MIBT_channelAdministrativeState_1;

  OAMgrr_CHN_AMSTATEa(trx,slot) = state;

  if ( STATUS_OK!= (sta=oam_setTblEntryField(MODULE_GRR,tag,slot,state)) )
  {
      grr_SubmitAlarm(EC_GRR_OAM_CALL_SETMIBINTVAR_FAILED);
      DBG_ERROR("grr_SetOamTrxSlotAmState: oam_setMibIntVar failed for setting trx(%d) slot(%d) amState(%d) retErr(%d)\n",
              trx, slot, state,sta);
      printf("grr_SetOamTrxSlotAmState: oam_setMibIntVar failed for setting trx(%d) slot(%d) amState(%d) retErr(%d)\n",
              trx, slot, state,sta);
      assert(0);
  }

  DBG_LEAVE();
}

void grr_SetOamTrxOpState(unsigned char trx, EnableDisable state)
{
  MibTag tag;

  DBG_FUNC("grr_SetOamTrxOpState", GRR_LAYER_OAM);
  DBG_ENTER();

  DBG_TRACE("grr_SetOamTrxOpState: set trx(%d) opState(%d)\n",
             trx, state);

  //Check if the requested trx existent
  if (trx>=OAMgrr_MAX_TRXS)
  {
      DBG_WARNING("grr_SetOamTrxOpState: setting opState(%d) for invalid trx(%d)\n", state,trx);
      DBG_LEAVE();
      return;
  }

  OAMgrr_TRX_OPSTATEa(trx) = state;

  if (trx==0) tag=MIB_basebandOperationalState_0;
  else        tag=MIB_basebandOperationalState_1;

  if ( STATUS_OK!=oam_setMibIntVar(MODULE_GRR, tag, state) )
  {
      grr_SubmitAlarm(EC_GRR_OAM_CALL_SETMIBINTVAR_FAILED);
      DBG_ERROR("grr_SetOamTrxOpState: oam_setMibIntVar failed for setting trx(%d) opState(%d)\n",
  	           trx, state);
      printf("grr_SetOamTrxOpState: oam_setMibIntVar failed for setting trx(%d) opState(%d)\n",
	           trx, state);
      assert(0);
  }

  DBG_LEAVE();
}

void grr_SetOamAllTrxSlotOpState(unsigned char trx, EnableDisable state)
{
  unsigned char i;

  DBG_FUNC("grr_SetOamAllTrxSlotOpState", GRR_LAYER_OAM);
  DBG_ENTER();

  DBG_TRACE("grr_SetOamAllTrxSlotOpState: set trx(%d) all slots opState(%d)\n",
             trx, state);

  //Check if the requested trx existent
  if (trx>=OAMgrr_MAX_TRXS)
  {
      DBG_WARNING("grr_SetOamAllTrxSlotOpState: setting opState(%d) for invalid trx(%d)\n",
                   state,trx);
      DBG_LEAVE();
      return;
  }

  for (i=0;i<8;i++)		
       grr_SetOamTrxSlotOpState(trx,i,state);	

  DBG_LEAVE();

}   

void grr_SetOamTrxSlotOpState(unsigned char trx, unsigned char slot, EnableDisable state)
{
  MibTag tag;
  STATUS sta;

  DBG_FUNC("grr_SetOamTrxSlotOpState", GRR_LAYER_OAM);
  DBG_ENTER();

  DBG_TRACE("grr_SetOamTrxSlotOpState: set trx(%d) slot(%d) opState(%d)\n",
             trx, slot, state);

  OAMgrr_TRX_SLOT_OPSTATEa(trx,slot) = state;

  if (trx==0) tag = MIBT_channelOperationalState_0;
  else        tag = MIBT_channelOperationalState_1;
 
  if ( STATUS_OK!= (sta=oam_setTblEntryField(MODULE_GRR, tag, slot, state)) )
  {
      grr_SubmitAlarm(EC_GRR_OAM_CALL_SETTBLENTRYFIELD_FAILED);
      printf("grr_SetOamTrxSlotOpState: oam_setMibIntVar failed for setting trx(%d) slot(%d) opState(%d) retErr(%d)\n",
              trx, slot, state,sta);
      DBG_ERROR("grr_SetOamTrxSlotOpState: oam_setMibIntVar failed for setting trx(%d) slot(%d) opState(%d) retErr(%d)\n",
              trx, slot, state,sta);
      assert(0);
  }
  
  DBG_LEAVE();
}

void grr_SetOamTrxRcOpState(unsigned char trx, EnableDisable state)
{
  unsigned char rc;
  MibTag        tag;

  DBG_FUNC("grr_SetOamTrxRcOpState", GRR_LAYER_OAM);
  DBG_ENTER();

  rc = OAMgrr_TRX_RC(trx);

  DBG_TRACE("grr_SetOamTrxRcOpState: set trx(%d) rc(%d) opState(%d)\n",
             trx, rc, state);

  OAMgrr_RC_OPSTATEa(rc) = state;

  if (rc==0) tag=MIB_carrier_operationalState_0;
  else       tag=MIB_carrier_operationalState_1;

  if ( STATUS_OK!=oam_setMibIntVar(MODULE_GRR, tag, state) )
  {    
      grr_SubmitAlarm(EC_GRR_OAM_CALL_SETMIBINTVAR_FAILED);
      printf("grr_SetOamTrxRcOpState: oam_setMibIntVar failed for setting trx(%d) rc(%d) opState(%d)\n",
	        trx, rc, state);
      DBG_ERROR("grr_SetOamTrxRcOpState: oam_setMibIntVar failed for setting trx(%d) rc(%d) opState(%d)\n",
	        trx, rc, state);
      assert(0);
  }

  DBG_LEAVE();

}

#endif //__GRR_SETOAMMIB_CPP__


