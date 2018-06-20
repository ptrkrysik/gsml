#include "parser_if.h"

#include "oam_api.h"
#include "mibtags.h"
#include "MnetModuleId.h"

/*
 * Parse chains for the cell subconfig mode
 */

#define ALTERNATE NONE
#include "cfg_trx_exit.h" 
LINK_TRANS (configure_config_cell_trx_extend_here, ALTERNATE);
NO_OR_DEFAULT (config_cell_trx_no, configure_config_cell_trx_extend_here,
               PRIV_OPR | PRIV_NONVGEN);
HELP (config_cell_trx, config_cell_trx_no,
      "Configure  cell transceiver configuration parameters:\n");
#undef ALTERNATE

/*
 * Parse chains for the cell subconfig mode
 */

#define ALTERNATE NONE
#include "cfg_bts_exit.h" 
LINK_TRANS (configure_config_cell_bts_extend_here, ALTERNATE);
NO_OR_DEFAULT (config_cell_bts_no, configure_config_cell_bts_extend_here,
               PRIV_OPR | PRIV_NONVGEN);
HELP (config_cell_bts, config_cell_bts_no,
      "Configure  cell transceiver configuration parameters:\n");
#undef ALTERNATE




/*
 * Parse chains for bts basic configuration
 */
#define ALTERNATE       NONE
#include "../include/cfg_bts_basic.h"
LINK_POINT(cfg_bts_bas, ALTERNATE);
#undef  ALTERNATE


static parser_extension_request cfg_bts_basic_init_table[] = {
  { PARSE_ADD_CFG_GP10_CELL_BTS, &pname(cfg_bts_bas) },
  { PARSE_LIST_END, NULL }
};

parser_mode *bts_basic;

/*
 * Register the parser chain
 */
void cfg_bts_basic_parser_init (void)
{

  parser_add_link_point(PARSE_ADD_CFG_GP10_CELL_TRX, "config-trx",
                          &pname(configure_config_cell_trx_extend_here));
  parser_add_mode("config-trx", "config-trx",
		  "Configure Cell Transceiver Configuration Parameters", TRUE, TRUE, "configure",
		  NULL, NULL, &pname(config_cell_trx),NULL);

  parser_add_link_point(PARSE_ADD_CFG_GP10_CELL_BTS, "bts-basic",
                          &pname(configure_config_cell_bts_extend_here));

  bts_basic = parser_add_mode("bts-basic", "bts-basic",
		  "Configure Cell Bts Configuration Parameters", TRUE, TRUE, "configure",
		  NULL, NULL, &pname(config_cell_bts),NULL);
  parser_add_command_list(cfg_bts_basic_init_table,"config");
}

/*
 * Mib tags for global identity options
 */
static MibTag bts_bsic_tags[] = {MIB_bts_mcc,MIB_bts_mnc,MIB_bts_lac,MIB_bts_ci};

static char *bts_gi_cli[] = {" mcc "," mnc "," lac "," ci "};

/*
 * failiure statements for  global identity options
 */

static char *bsic_set_fail[] = {
  "\n Configuration of bts mobile country code failed",
  "\n Configuration of bts mobile network code failed",
  "\n Configuration of bts location area code failed",
  "\n Configuration of bts cell identifier failed"
};

static char *bts_admin_state[] =  {
  " locked ",
  " unlocked ",
  " shutdown "
};


void bts_gi_nvgen()
{
  long nvgen_int;
  int i;
  
  for (i=0;i<4;i++) {
    oam_getMibIntVar(bts_bsic_tags[i],&nvgen_int);
    printf("\n\t cell-global-identity %s %d",bts_gi_cli[i],nvgen_int);
  }
}


/*
 * bts basic command handler
 */


void bts_basic_parameters(parseinfo *csb)
{
  int i = 0; /* used for indexing permitted ncc */
  long nvgen_int;
  int intbuf[2] = {0,1};
  int intnobuf[2] = {0,0};
  PlmnPermittedEntry plmn_entry;

  switch (csb->which) {
  case BTS_RECV_LEVEL :
    if (csb->nvgen) {
      oam_getMibIntVar(MIB_rxLevAccessMin,&nvgen_int);
      printf("\n\t %s %d",csb->nv_command,nvgen_int);
      return;
    }
    SET_INT_MIB(MIB_rxLevAccessMin,GETOBJ(int,1),10,
		"\nConfiguration of minimum receive level access failed");
    break;
  case BTS_LINK_TIMEOUT :
    if (csb->nvgen) {
      oam_getMibIntVar(MIB_radioLinkTimeout,&nvgen_int);
      printf("\n\t %s %d",csb->nv_command,nvgen_int);
      return;
    }
    SET_INT_MIB(MIB_radioLinkTimeout,GETOBJ(int,1),15,
		"\nConfiguration of radio link timeout failed");
    break;
  case BTS_PER_NCC_ALL :
    if (csb->nvgen) {
      return;
    }
    if (csb->sense) {
      for (i=0;i < 8;i++) {
	oam_setMibTblEntry(CLI_MODULE_ID,MIBT_plmnPermittedEntry,i,intbuf,
			   sizeof(int) *2);
      } 
    } else {
      for (i=0;i < 8;i++) {
	oam_setMibTblEntry(CLI_MODULE_ID,MIBT_plmnPermittedEntry,i,intnobuf,
			   sizeof(int) *2);
      }
    }
    break;
  case BTS_PER_NCC :
    if (csb->nvgen) {
      for (i=0;i < 8; i++) {
	oam_getMibTblEntry(MIBT_plmnPermittedEntry,i,&plmn_entry,sizeof(PlmnPermittedEntry));
	if (plmn_entry.plmnPermitted)
	  printf("\n\t plmn-permitted-ncc %d",i);
	else
	  printf("\n\t no plmn-permitted-ncc %d",i);
      }
      return;
    }
    if (csb->sense) {
      oam_setMibTblEntry(CLI_MODULE_ID,MIBT_plmnPermittedEntry,GETOBJ(int,1),intbuf,
			 sizeof(int) *2);
    } else {
      oam_setMibTblEntry(CLI_MODULE_ID,MIBT_plmnPermittedEntry,GETOBJ(int,1),intnobuf,
			 sizeof(int) *2);
    }
    break;
  case BTS_PHY_VALUE :
    if (csb->nvgen) {
      oam_getMibIntVar(MIB_ny1,&nvgen_int);
      printf("\n\t %s %d",csb->nv_command,nvgen_int);
      return;
    }
    SET_INT_MIB(MIB_ny1,GETOBJ(int,1),3,
		"\nConfiguration num of physical repetition failed");
    break;
  case BTS_RES_VALUE :
    if (csb->nvgen) {
      oam_getMibIntVar(MIB_cellReselectHysteresis,&nvgen_int);
      printf("\n\t %s %d",csb->nv_command,nvgen_int);
      return;
    }
    SET_INT_MIB(MIB_cellReselectHysteresis,GETOBJ(int,1),7,
		"\nConfiguration cell reselect hysteresis failed");
    break;
  case BTS_CC_VALUE :
    if (csb->nvgen) {
      oam_getMibIntVar(MIB_bts_ncc,&nvgen_int);
      printf("\n\t bsic ncc %d",nvgen_int);
      oam_getMibIntVar(MIB_bts_cid,&nvgen_int);
      printf("\n\t bsic bcc %d",nvgen_int);
      return;
    }
    switch(GETOBJ(int,1)) {
    case BSIC_NCC :
      SET_INT_MIB(MIB_bts_ncc,GETOBJ(int,2),7,"\nConfiguration of network color code failed");
      break;
    case BSIC_BCC :
      SET_INT_MIB(MIB_bts_cid,GETOBJ(int,2),7,
		  "\nConfiguration of base station color code failed");
      break;
    }
    break;
  case BTS_GI_VALUE :
    if (csb->nvgen) {
      bts_gi_nvgen ();
      return;
    }
    SET_INT_MIB(bts_bsic_tags[GETOBJ(int,1)],GETOBJ(int,2),1,bsic_set_fail[GETOBJ(int,1)]);
    break;
  case BTS_ADMIN_STATE :
    if (csb->nvgen) {
      oam_getMibIntVar(MIB_bts_administrativeState,&nvgen_int);
      printf("\n\t administrative-state %s",bts_admin_state[nvgen_int]);
      return;
    }
    SET_INT_MIB(MIB_bts_administrativeState,GETOBJ(int,1),1,
		"\nConfiguration of bts administrative state failed");
    break;
  }
}
