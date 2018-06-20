#include "parser_if.h"

#include "oam_api.h"
#include "mibtags.h"
#include "MnetModuleId.h"

/*
 * Parse chains for system information configuration commands
 */
#define ALTERNATE       NONE
#include "../include/cfg_timers.h"
LINK_POINT(cfg_timer, ALTERNATE);
#undef  ALTERNATE

#define ALTERNATE NONE
#include "cfg_proto_exit.h" 
LINK_TRANS (configure_gp10_proto_extend_here, ALTERNATE);
NO_OR_DEFAULT (gp10_proto_config_no, configure_gp10_proto_extend_here,
               PRIV_OPR | PRIV_NONVGEN);
HELP (gp10_proto_config, gp10_proto_config_no,
      "Configure gp10 protocol configuration parameters:\n");
#undef ALTERNATE

static parser_extension_request cfg_proto_init_table[] = {
  { PARSE_ADD_CFG_GP10_PROTO, &pname(cfg_timer) },
  { PARSE_LIST_END, NULL }
};

parser_mode *config_proto;
/*
 * Register the parser chain
 */
void cfg_proto_parser_init (void)
{
  parser_add_link_point(PARSE_ADD_CFG_GP10_PROTO, "config-proto",
                          &pname(configure_gp10_proto_extend_here));
  config_proto = parser_add_mode("config-proto", "config-proto",
		  "Configure Protocol Configuration Parameters", TRUE, TRUE, "configure",
		  NULL, NULL, &pname(gp10_proto_config),NULL);
  parser_add_command_list(cfg_proto_init_table,"config");
}
 


/* 
 * Mib tags of the timer
 */

static MibTag timer_mib_tags[] = {MIB_sdcchSAPI0,MIB_sdcchSAPI3,MIB_sacchSDCCH,
                                  MIB_sacchTCHSAPI0,MIB_sdcchTCHSAPI3,
				  MIB_t3101,MIB_t3103,MIB_t3105,MIB_t3107,MIB_t3109,
				  MIB_t3111,MIB_t3113,MIB_cc_alerting_T301,MIB_cc_setup_T303,
				  MIB_cc_call_confirmed_T310,MIB_cc_connect_T313,
				  MIB_cc_disconnect_T305,MIB_cc_release_T308
};

/*
 * String for printing SNMP failiure to set the timer
 */

static char *time_set_fail_str[] = {
  "\nConfiguration of Stand-alone dedicated control channel sapi0 timer failed",
  "\nConfiguration of Stand-alone dedicated control channel sapi3 timer failed",
  "\nConfiguration of Stand-alone dedicated control channel sdcch timer failed",  
  "\nConfiguration of sacch tracffic channel sapi0 timer failed",
  "\nConfiguration of Stand-alone dedicated control channel tch sapi3 timer failed",
  "\nConfiguration of immediate assignment timer failed",
  "\nConfiguration of handover command timer failed",
  "\nConfiguration of physical information timer failed",
  "\nConfiguration of assignment command timer failed",
  "\nConfiguration of lower layer failiure timer failed",
  "\nConfiguration of channel activation delay timer failed",
  "\nConfiguration of paging message timer failed",
  "\nConfiguration of alerting received timer failed",
  "\nConfiguration of setup sent timer failed",
  "\nConfiguration of call confirmation received timer failed",
  "\nConfiguration of connect sent timer failed",
  "\nConfiguration of disconnect sent timer failed",
  "\nConfiguration of release sent timer failed"
};


void rmm_timer_nvgen (parseinfo *csb)
{
  NVADDGEN_TIMER_MIB(MIB_t3101,"immediate-assignment");
  NVADDGEN_TIMER_MIB(MIB_t3103,"handover-command");
  NVADDGEN_TIMER_MIB(MIB_t3105,"physical-information");
  NVADDGEN_TIMER_MIB(MIB_t3107,"assignment-command");
  NVADDGEN_TIMER_MIB(MIB_t3109,"lower-layer-failure");
  NVADDGEN_TIMER_MIB(MIB_t3111,"channel-activation");
  NVADDGEN_TIMER_MIB(MIB_t3113,"paging-messages");
}

void ccm_timer_nvgen (parseinfo *csb)
{
  NVADDGEN_TIMER_MIB(MIB_cc_alerting_T301,"alerting-received");
  NVADDGEN_TIMER_MIB(MIB_cc_setup_T303,"setup-sent");
  NVADDGEN_TIMER_MIB(MIB_cc_call_confirmed_T310,"call-confirmed");
  NVADDGEN_TIMER_MIB(MIB_cc_connect_T313,"connect-sent");
  NVADDGEN_TIMER_MIB(MIB_cc_disconnect_T305,"disconnect-sent");
  NVADDGEN_TIMER_MIB(MIB_cc_release_T308,"release-sent");
}


/*
 * Timer confiuration routine
 */

void timer_configuration (parseinfo *csb)
{
  long nvgen_int;

  switch (csb->which) {
  case CCM_TIMER :
    if (csb->nvgen) {
      ccm_timer_nvgen(csb);
      return;
    }
    SET_INT_MIB(timer_mib_tags[GETOBJ(int,1)],GETOBJ(int,2),0,time_set_fail_str[GETOBJ(int,1)]);
    break;
  case RMM_TIMER :
    if (csb->nvgen) {
      rmm_timer_nvgen(csb);
      return;
    }
    SET_INT_MIB(timer_mib_tags[GETOBJ(int,1)],GETOBJ(int,2),0,time_set_fail_str[GETOBJ(int,1)]);
    break;
  case SACCH_TIMER :
    NVADDGEN_TIMER_MIB(MIB_sacchTCHSAPI0,"tch-0-timer");
    NVGEN_TIMER_MIB(MIB_sacchSDCCH,"sdcch-timer");
    SET_INT_MIB(timer_mib_tags[GETOBJ(int,1)],GETOBJ(int,2),0,time_set_fail_str[GETOBJ(int,1)]);
    break;
  case SDCCH_TIMER :
    NVADDGEN_TIMER_MIB(MIB_sdcchSAPI0,"sapi-0-timer");
    NVADDGEN_TIMER_MIB(MIB_sdcchSAPI3,"sapi-3-timer");
    NVGEN_TIMER_MIB(MIB_sdcchTCHSAPI3,"sdcch-tch-timer");
    SET_INT_MIB(timer_mib_tags[GETOBJ(int,1)],GETOBJ(int,2),0,time_set_fail_str[GETOBJ(int,1)]);
    break;
  case FACCH_TIMER :
    if (csb->nvgen) {
      oam_getMibIntVar(MIB_facchTCHF,&nvgen_int);
      printf("\n\t %s %d",csb->nv_command,nvgen_int);
      return;
    }
    
    SET_INT_MIB(MIB_facchTCHF,GETOBJ(int,1),0,"\nConfiguration of TCH full rate timer failed");
    break;
  }
}

