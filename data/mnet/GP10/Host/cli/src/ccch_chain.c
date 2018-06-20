#include "parser_if.h"

#include "oam_api.h"
#include "mibtags.h"
#include "MnetModuleId.h"
 

/*
 * Parse chains for system information configuration commands
 */
#define ALTERNATE       NONE
#include "../include/cfg_ccch.h"
LINK_POINT(cfg_bts_ccch, ALTERNATE);
#undef  ALTERNATE

/*
 * Parse chains for the cell subconfig mode
 */

#define ALTERNATE NONE
#include "cfg_ccch_exit.h" 
LINK_TRANS (configure_config_cell_ccch_extend_here, ALTERNATE);
NO_OR_DEFAULT (config_cell_ccch_no, configure_config_cell_ccch_extend_here,
               PRIV_OPR | PRIV_NONVGEN);
HELP (config_cell_ccch, config_cell_ccch_no,
      "Configure  common control channel configuration parameters:\n");
#undef ALTERNATE


static parser_extension_request cfg_bts_ccch_init_table[] = {
  { PARSE_ADD_CFG_GP10_CELL_CCCH, &pname(cfg_bts_ccch) },
  { PARSE_LIST_END, NULL }
};

parser_mode *ccch_config;


/*
 * Register the parser chain
 */
void cfg_bts_ccch_parser_init (void)
{
  parser_add_link_point(PARSE_ADD_CFG_GP10_CELL_CCCH, "ccch-config",
			&pname(configure_config_cell_ccch_extend_here));
  
  ccch_config = parser_add_mode("ccch-config", "ccch-config",
		  "Configure Cell Ccch Configuration Parameters", TRUE, TRUE, "configure",NULL, NULL, &pname(config_cell_ccch),NULL);
  parser_add_command_list(cfg_bts_ccch_init_table,"config");
}



/*
 * CCCH Mib Tags
 */

/*
 * The first ited needs to be replaced. At this point of time engr doesn't know this
 */
static MibTag ccch_mib_tags[] = {MIB_rm_nim_0_6,
				 MIB_rm_nim_0_5,MIB_rm_nim_0_4,
				 MIB_rm_nim_0_3,
				 MIB_numberOfSlotsSpreadTrans,
				 MIB_noOfMultiframesBetweenPaging,
				 MIB_noOfBlocksForAccessGrant,MIB_mSTxPwrMaxCCH,
				 MIB_maxNumberRetransmissions
};


/*
 * Default values for mibs
 */
static int ccch_mib_def[] = {0,0,0,0,1,9,0,0,4};


/*
 * String for SNMP set failure information
 */

static char *ccch_set_fail[] = {
  "\n Selection of cell bar qualify failed",
  "\n Configuration of offset penaly failed",
  "\n Configuration of offset temporary failed",
  "\n Configuration of cell reselect failed",
  "\n Configuration of max number of RACH slots failed",
  "\n Configuration of number of multiframes failed",
  "\n Configuration of number of frames for access grant channel failed",
  "\n Configuration of max transmit power level failed",
  "\n Configuration of max number of retransmission failed",
};


/*
 * String for SNMP set failure information
 */

static char *ccch_show_fail[] = {
  "\n Show configuration of cell bar qualify failed",
  "\n Show Configuration of offset penaly failed",
  "\n Show Configuration of offset temporary failed",
  "\n Show Configuration of cell reselect failed",
  "\n Show Configuration of max number of RACH slots failed",
  "\n Show Configuration of number of multiframes failed",
  "\n Show Configuration of number of frames for access grant channel failed",
  "\n Show Configuration of max transmit power level failed",
  "\n Show Configuration of max number of retransmission failed",
};

/*
 * ccch command handler
 */

void ccch_configuration (parseinfo *csb)
{
  long nvgen_int;
  
  if (csb->which == CCCH_CELL_BAR) {
    if (csb->nvgen) {
      oam_getMibIntVar(ccch_mib_tags[csb->which],&nvgen_int);
      if (nvgen_int)
	printf("\n\t %s enable",csb->nv_command);
      else
	printf("\n\t no %s enable",csb->nv_command);
      return;
    }
    
    SET_INT_MIB(ccch_mib_tags[csb->which],csb->sense,0,ccch_set_fail[csb->which]);
  } else {
    if (csb->nvgen) {
      oam_getMibIntVar(ccch_mib_tags[csb->which],&nvgen_int);
      printf("\n\t %s %d",csb->nv_command,nvgen_int);
      return;
    }
    SET_INT_MIB(ccch_mib_tags[csb->which],GETOBJ(int,1),ccch_mib_def[csb->which],
		ccch_set_fail[csb->which]);
  }
}
