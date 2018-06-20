#include "parser_if.h"

#include "oam_api.h"
#include "mibtags.h"
#include "MnetModuleId.h"

#define TCH_FULL      1
#define BCCH_COMBINED 5
#define USE_BCC     0

/*
 * Parse chains for transceiver configuration
 */
#define ALTERNATE       NONE
#include "../include/cfg_trx.h"
LINK_POINT(cfg_trx, ALTERNATE);
#undef  ALTERNATE



static parser_extension_request cfg_trx_init_table[] = {
  { PARSE_ADD_CFG_GP10_CELL_TRX, &pname(cfg_trx) },
  { PARSE_LIST_END, NULL }
};

/*
 * Register the parser chain
 */
void cfg_trx_parser_init (void)
{
  parser_add_command_list(cfg_trx_init_table,"config");
}

static char *admin_state[] = {
  " locked ",
  " unlocked ",
  " shutting-down "
};
void trx_nvgen ()
{
  long nvgen_int,channel;
  CellAllocationEntry cell_entry;
  CarrierFrequencyEntry freq_entry;
  ChannelEntry chan_entry;

  printf("\n\n transceiver 0");
  
  oam_getMibTblEntry(MIBT_cellAllocationEntry,0,&cell_entry,sizeof(CellAllocationEntry));
  printf("\n\t bts-cell-allocation %d",cell_entry.cellAllocation);
  
  oam_getMibTblEntry(MIBT_carrierFrequency_0_Entry,0,&freq_entry,sizeof(CarrierFrequencyEntry));
  printf("\n\t radio-carrier radio-frequency %d",freq_entry.carrierFrequency);
  
  oam_getMibIntVar(MIB_txPwrMaxReduction_0,&nvgen_int);
  printf("\n\t radio-carrier transmit-power-reduction %d",nvgen_int);
  
  oam_getMibIntVar(MIB_basebandAdministrativeState_0,&nvgen_int);
  printf("\n\t administrative-state %s",admin_state[nvgen_int]);
 
  for (channel=0;channel < 8;channel++) {
    oam_getMibTblEntry(MIBT_channel_0_Entry,channel,&chan_entry,sizeof(ChannelEntry));
    printf("\n\t channel-table %d channel-combination %s training-sequence USE-BCC administrative-state %s",channel,
	   (chan_entry.channelCombination == BCCH_COMBINED)?"BCCH-COMBINED":"TCH-FULL",
	   admin_state[chan_entry.channelAdministrativeState]);
  }
  printf("\n\n transceiver 1");
  
  oam_getMibTblEntry(MIBT_cellAllocationEntry,1,&cell_entry,sizeof(CellAllocationEntry));
  printf("\n\t bts-cell-allocation %d",cell_entry.cellAllocation);
  
  oam_getMibTblEntry(MIBT_carrierFrequency_1_Entry,0,&freq_entry,sizeof(CarrierFrequencyEntry));
  printf("\n\t radio-carrier radio-frequency %d",freq_entry.carrierFrequency);
  
  oam_getMibIntVar(MIB_txPwrMaxReduction_1,&nvgen_int);
  printf("\n\t radio-carrier transmit-power-reduction %d",nvgen_int);
  
  oam_getMibIntVar(MIB_basebandAdministrativeState_1,&nvgen_int);
  printf("\n\t administrative-state %s",admin_state[nvgen_int]);
 
  for (channel=0;channel < 8;channel++) {
    oam_getMibTblEntry(MIBT_channel_1_Entry,channel,&chan_entry,sizeof(ChannelEntry));
  printf("\n\t channel-table %d channel-combination TCH-FULL training-sequence USE-BCC adminstration-state %s",channel,admin_state[chan_entry.channelAdministrativeState]);
  }
}

/*
 * command handler for transceiver parameters
 */
void transceiver_parameters (parseinfo *csb)
{
  int ifnum;
  int intbuf[7];

  ifnum = (int) GETOBJ(udb,current1);
  
  switch (csb->which) {
  case TRX_RADIO_FREQUENCY :
    intbuf[0] = 0;
    if (csb->sense) {
      intbuf[1] = GETOBJ(int,1);
    } else {
      intbuf[1] = 0;
    }
    if (ifnum == 0) {
      oam_setMibTblEntry(CLI_MODULE_ID,MIBT_carrierFrequency_0_Entry,0,intbuf,
			 sizeof(int) *2);
    } else {
      oam_setMibTblEntry(CLI_MODULE_ID,MIBT_carrierFrequency_1_Entry,0,intbuf,
			 sizeof(int) *2);
    }
    break;

  case TRX_POWER_REDUCTION :
    if (ifnum == 0) {
      SET_INT_MIB(MIB_txPwrMaxReduction_0,GETOBJ(int,1),0,
		  "\n Configuration of transmit power reduction failed");
    } else {
      SET_INT_MIB(MIB_txPwrMaxReduction_1,GETOBJ(int,1),0,
		  "\n Configuration of transmit power reduction failed");
    }
    break;

  case TRX_CHANNEL_TABLE :
    intbuf[0] = 0;
    intbuf[1] = GETOBJ(int,1);
    intbuf[2] = GETOBJ(int,3);
    intbuf[3] = 0;
    intbuf[4] = USE_BCC;
    if (csb->sense) {
      intbuf[5] = GETOBJ(int,2);
    } else {
      intbuf[5] =  STATE_UNLOCKED;
    }
    intbuf[6] = 1;
    
    if (ifnum == 0) {
      oam_setMibTblEntry(CLI_MODULE_ID,MIBT_channel_0_Entry,GETOBJ(int,1),intbuf,
			 sizeof(int) * 7);
    } else {
      oam_setMibTblEntry(CLI_MODULE_ID,MIBT_channel_1_Entry,GETOBJ(int,1),intbuf,
			 sizeof(int) * 7);
    }
    break;

  case TRX_ADMIN_STATE :
    if (ifnum == 0) {
      SET_INT_MIB( MIB_basebandAdministrativeState_0,GETOBJ(int,1),1,
		   "\n Configuration of Administrative state failed");
    } else {
      SET_INT_MIB( MIB_basebandAdministrativeState_1,GETOBJ(int,1),1,
		   "\n Configuration of  Administrative state failed");
    }
    break;
    
  case TRX_ARFCN_NUMBER :
      intbuf[0] = 0;
      if (csb->sense) {
      intbuf[1] = GETOBJ(int,1);
      } else {
	intbuf[1] = 0;
      }
      oam_setMibTblEntry(CLI_MODULE_ID,MIBT_cellAllocationEntry,ifnum,intbuf,
			 sizeof(int) *2);
      break;
  }
}

