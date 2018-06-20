#include "parser_if.h"
 
#include "oam_api.h"
#include "mibtags.h"
#include "MnetModuleId.h"
/*
 * Parse chains for system information configuration commands
 */
#define ALTERNATE       NONE
#include "../include/cfg_sys_info.h"
LINK_POINT(cfg_sys_info, ALTERNATE);
#undef  ALTERNATE

/*
 * Parse chains for voip configuration commands
 */
#define ALTERNATE       NONE
#include "../include/cfg_voip.h"
LINK_POINT(cfg_voip, ALTERNATE);
#undef  ALTERNATE


/*
 * Parse chains for bts option commands
 */
#define ALTERNATE       NONE
#include "../include/cfg_bts_options.h"
LINK_POINT(cfg_bts_option, ALTERNATE);
#undef  ALTERNATE

/*
 * Parse chains for bts option commands
 */
#define ALTERNATE       NONE
#include "../include/cfg_sys_feat.h"
LINK_POINT(cfg_sys_feat, ALTERNATE);
#undef  ALTERNATE


/*
 * Parse chains for bts option commands
 */
#define ALTERNATE       NONE
#include "../include/cfg_mspwr_ctrl.h"
LINK_POINT(cfg_mspwr_ctrl, ALTERNATE);
#undef  ALTERNATE

/*
 * Parse chains for the global subconfig mode
 */

#define ALTERNATE NONE
#include "cfg_cell_exit.h" 
LINK_TRANS (configure_gp10_cell_extend_here, ALTERNATE);
NO_OR_DEFAULT (gp10_cell_config_no, configure_gp10_cell_extend_here,
               PRIV_OPR | PRIV_NONVGEN);
HELP (gp10_cell_config, gp10_cell_config_no,
      "Configure gp10 cell configuration parameters:\n");
#undef ALTERNATE

#define ALTERNATE NONE
#include "cfg_adjcell_exit.h" 
LINK_TRANS (configure_gp10_adjcell_extend_here, ALTERNATE);
NO_OR_DEFAULT (gp10_adjcell_config_no, configure_gp10_adjcell_extend_here,
               PRIV_OPR | PRIV_NONVGEN);
HELP (gp10_adjcell_config, gp10_adjcell_config_no,
      "Configure gp10  adjacent cell configuration parameters:\n");
#undef ALTERNATE

/*
 * Parse chain registration array for Exec
 */
static parser_extension_request cfg_sysinfo_init_table[] = {
    { PARSE_ADD_CFG_TOP_CMD, &pname(cfg_sys_info) },
    { PARSE_ADD_CFG_TOP_CMD, &pname(cfg_voip) },
    { PARSE_ADD_CFG_GP10_CELL, &pname(cfg_bts_option) },
    { PARSE_ADD_CFG_TOP_CMD, &pname(cfg_sys_feat) },
    { PARSE_ADD_CFG_TOP_CMD, &pname(cfg_mspwr_ctrl) },
	{ PARSE_LIST_END, NULL }
};

static parser_mode *config_adjcell;
static parser_mode *config_cell;
extern parser_mode *config_proto;
extern parser_mode *ccch_config;
extern parser_mode *bts_basic;

/* parser init routine */

void gp10_subconfig_init (void)
{
    parser_add_link_point(PARSE_ADD_CFG_GP10_CELL, "config-cell",
                          &pname(configure_gp10_cell_extend_here));
    config_cell = parser_add_mode("config-cell", "config-cell",
		    "Configure Cell Configuration Parameters", TRUE, TRUE, "configure",
		    NULL, NULL, &pname(gp10_cell_config),NULL);
    parser_add_link_point(PARSE_ADD_CFG_GP10_ADJCELL, "config-adjcell",
			  &pname(configure_gp10_adjcell_extend_here));
    config_adjcell = parser_add_mode("config-adjcell", "config-adjcell",
		    "Configure Adjacent Cell Configuration Parameters", TRUE, TRUE, "configure",
		    NULL, NULL, &pname(gp10_adjcell_config),NULL);
}

/*
 * Register the parser chain
 */
void commands_init (void)
{
  gp10_subconfig_init();
  parser_add_command_list(cfg_sysinfo_init_table,"config");
  exec_diag_parser_init();
  cfg_bts_basic_parser_init();
  cfg_bts_ccch_parser_init();
  cfg_bts_adjcell_parser_init (); 
  cfg_trx_parser_init();
  cfg_proto_parser_init();
}


static void submode_nvgen (parseinfo *csb,parser_mode *mode)
{
  parseinfo *nvcsb;
  
  nvcsb = get_csb("NV parser CSB");
  if (csb == NULL) {
    printf(nomemory);
    return;
  }
  nvcsb->nvgen = TRUE;
  nvcsb->sense = TRUE;
  nvcsb->priv = PRIV_MAX;
  nvcsb->priv_set = 0xf;
  nvcsb->mode = mode;
  push_node(nvcsb, get_mode_nv(nvcsb));
  parse_token(nvcsb);
  free_csb(&nvcsb);
}

/*
 * sub config modes
 */


void gp10_subconfig_cmds (parseinfo *csb)
{

  switch (csb->which) {
  case SUB_CONFIG_PROTO :
    if (csb->nvgen) {
      printf("\n\nconfig-proto");
      submode_nvgen(csb,config_proto);
      return;
    }
    set_mode_byname(&csb->mode, "config-proto", MODE_VERBOSE);
    break;
  case SUB_CONFIG_ADJCELL :
    if (csb->nvgen) {
      adjcell_nvgen ();
      return;
    }
    SETOBJ(udb, current1) = (void *)GETOBJ(int,1);
    set_mode_byname(&csb->mode, "config-adjcell", MODE_VERBOSE);
    break;
  case SUB_CONFIG_CELL :
    if (csb->nvgen) {
      submode_nvgen(csb,config_cell);
      return;
    }
    set_mode_byname(&csb->mode, "config-cell", MODE_VERBOSE);
    break;
  }
}
 
/*
 * cell sub config modes
 */

void gp10_cell_subconfig_cmds (parseinfo *csb)
{
  switch (csb->which) {
  case CFG_CELL_CCCH :
    if (csb->nvgen) {
      printf("\n\n ccch-config");
      submode_nvgen(csb,ccch_config);
      return;
    }
    set_mode_byname(&csb->mode, "ccch-config", MODE_VERBOSE);
    break;
  case CFG_CELL_TRX :
    if (csb->nvgen) {
      trx_nvgen();
      return;
    }
    SETOBJ(udb, current1) = (void *)GETOBJ(int,1);
    set_mode_byname(&csb->mode, "config-trx", MODE_VERBOSE);
    break;
  case CFG_CELL_BTS :
    if (csb->nvgen) {
      printf("\n\n bts-basic");
      submode_nvgen(csb,bts_basic);
      return;
    }
    set_mode_byname(&csb->mode, "bts-basic", MODE_VERBOSE);
    break;
  } 
}


char *
address_hexstring (ulong address,char *buf)
{
  sprintf(buf, ":%02x%02x%02x%02x",
		     (address >> 24) & 0xff, (address >> 16) & 0xff,
		     (address >> 8) & 0xff, address & 0xff);

  return buf;
}

/*
 * System configuration commands
 */
void system_configuration (parseinfo *csb)
{
  long nvgen_int;
  char ip_string[16];

  switch (csb->which) {
  case CONF_CUSTADDR :
      NVGEN_STRING_MIB(MIB_viperCellCustomerAddress,
		       "\n Show configuration of vipercell customer address failed");

    if (csb->sense) {
      if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellCustomerAddress,
	    (unsigned char *)GETOBJ(string,1),strlen(GETOBJ(string,1))) != STATUS_OK) {
	printf("\n Configuration of vipercell customer address failed");
      }
    } else {
      if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellCustomerAddress,
			    (unsigned char *)" ",1) != STATUS_OK) {
	printf("\n Configuration of vipercell customer address failed");
      }
    }
    break;
    
    
  case CONF_CUSTINFO :
      NVGEN_STRING_MIB(MIB_viperCellCustomerInfo,
		       "\n Show Configuration of vipercell customer information failed");

    if (csb->sense) {
      if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellCustomerInfo,(unsigned char *)GETOBJ(string,1),
			    strlen(GETOBJ(string,1))) != STATUS_OK) {
	printf("\n Configuration of vipercell customer information failed");
      }
    } else {
      if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellCustomerInfo,(unsigned char *)" ",
			    1) != STATUS_OK) {
	printf("\n Configuration of vipercell customer information failed");
      }
    }
    break;
    
    
  case CONF_CUSTEMAIL :

      NVGEN_STRING_MIB(MIB_viperCellCustomerEmail,
		       "\n Show  Configuration of vipercell customer email failed");

    if (csb->sense) {
      if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellCustomerEmail,
	    (unsigned char *)GETOBJ(string,1),strlen(GETOBJ(string,1))) != STATUS_OK) {
	printf("\n Configuration of vipercell customer email failed");
      }
    } else {
      if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellCustomerEmail,(unsigned char *)" ",
			    1) != STATUS_OK) {
	printf("\n Configuration of vipercell customer email failed");
      }
    }
    break;

    
  case CONF_CUSTPHONE :

      NVGEN_STRING_MIB(MIB_viperCellCustomerPhone,
		       "\n Show Configuration of vipercell customer phone failed");
      if (csb->sense) {
	if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellCustomerPhone,
			      (unsigned char *)GETOBJ(string,1),strlen(GETOBJ(string,1))) != STATUS_OK) {
	  printf("\n Configuration of vipercell customer phone failed");
	}
      } else {
	if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellCustomerPhone,
			      (unsigned char *)" ",1) != STATUS_OK) {
	  printf("\n Configuration of vipercell customer phone failed");
	}
      }
      break;

    
  case CONF_CUSTNAME :
      NVGEN_STRING_MIB(MIB_viperCellCustomerName,
		       "\n Show Configuration of vipercell customer name failed");
    if (csb->sense) {
      if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellCustomerName,(unsigned char *)GETOBJ(string,1),
			      strlen(GETOBJ(string,1))) != STATUS_OK) {
	printf("\n Configuration of vipercell customer name failed");
      }
    } else {
      if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellCustomerName,(unsigned char *)" ",
			    1) != STATUS_OK) {
	printf("\n Configuration of vipercell customer name failed");
      }
    }
    break;
    
    
  case CONF_ASSET :
      NVGEN_STRING_MIB(MIB_viperCellAssetNumber,
		       "\n Show Configuration of vipercell asset number failed");

    if (csb->sense) {
      if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellAssetNumber,(unsigned char *)GETOBJ(string,1),
			      strlen(GETOBJ(string,1))) != STATUS_OK) {
	printf("\n Configuration of vipercell asset number failed");
      }
    } else {
      if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellAssetNumber,(unsigned char *)" ",
			    1) != STATUS_OK) {
	printf("\n Configuration of vipercell asset number failed");
      }
    }
    break;
          oam_getMibIntVar(MIB_viperCellDefGateway,&nvgen_int);
      address_string(nvgen_int,ip_string);
      printf("\n ip default-gateway %s",ip_string);;

    
  case CONF_LOCATION :

      NVGEN_STRING_MIB(MIB_viperCellLocation,
		       "\n Show  Configuration of vipercell location failed");

    if (csb->sense) {
      if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellLocation,(unsigned char *)GETOBJ(string,1),
			  strlen(GETOBJ(string,1))) != STATUS_OK) {
	printf("\n Configuration of vipercell location failed");
      }
    } else {
      if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellLocation,(unsigned char *)" ",
			  1) != STATUS_OK) {
	printf("\n Configuration of vipercell location failed");
      }
    }
    break;
    
    
  case CONF_GW_IPADDR :
     NVGEN_IP_MIB(MIB_viperCellDefGateway,
		     "\n Show  Configuration of vipercell gateway ip address failed");
     if (csb->sense) {
       if ((oam_setMibIntVar(CLI_MODULE_ID,MIB_viperCellDefGateway,
			     GETOBJ(paddr,1)->ip_addr))!= STATUS_OK) {
	 printf("\n Configuration of vipercell gateway IP address failed");
       }
     } else {
       if (oam_setMibIntVar(CLI_MODULE_ID,MIB_viperCellDefGateway,0)!= STATUS_OK) {
	 printf("\n Configuration of vipercell gateway IP address failed");
       }
     }
     break;
     

  case CONF_SYS_IPADDR :
    if (csb->nvgen) {
      sysIpAddrGet(nvgen_string);
      printf("\n %s %s",csb->nv_command,nvgen_string);
      return;
    }
    SET_INT_MIB(MIB_viperCellIPAddress,GETOBJ(paddr,1)->ip_addr,0,
		"\n Configuration of vipercell IP address failed");
    address_string(GETOBJ(paddr,1)->ip_addr,nvgen_string);
    address_hexstring(GETOBJ(paddr,2)->ip_addr,ip_string);
    strcat(nvgen_string,ip_string);
    sysIpAddrSet(nvgen_string);
    break;

  case CONF_SYS_NAME :
     NVGEN_STRING_MIB(MIB_viperCellName,
		       "\n Show  Configuration of vipercell name failed");
     if (csb->sense) {
       if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellName,(unsigned char *)GETOBJ(string,1),
			     strlen(GETOBJ(string,1))) != STATUS_OK) {
	 printf("\n Configuration of vipercell name failed");
       }
     } else {
       if (oam_setMibByteAry(CLI_MODULE_ID,MIB_viperCellName,(unsigned char *)" ",
			     1) != STATUS_OK) {
	 printf("\n Configuration of vipercell name failed");
       }
     }
     break;
  case CONF_SNMP_COMM :
    NVADDGEN_STRING_MIB(MIB_readCommunity,
	     "\n Show  Configuration of vipercell snmp read community failed",TRUE,FALSE);
    NVADDGEN_STRING_MIB(MIB_writeCommunity,
	"\n Show  Configuration of vipercell snmp write community failed",FALSE,TRUE);
    if (csb->sense) {
      oam_setMibByteAry(CLI_MODULE_ID,MIB_readCommunity,(unsigned char *) GETOBJ(string,1),
			strlen(GETOBJ(string,1)));
      oam_setMibByteAry(CLI_MODULE_ID,MIB_writeCommunity,(unsigned char *) GETOBJ(string,2),
			strlen(GETOBJ(string,2)));
    } else {
      oam_setMibByteAry(CLI_MODULE_ID,MIB_readCommunity,(unsigned char *)" ",1);
      oam_setMibByteAry(CLI_MODULE_ID,MIB_writeCommunity,(unsigned char *)" ",1);
    }
  }
}


  
/*
 * voice over ip parameter configuration
 */ 

void voip_parameters (parseinfo *csb)
{
  switch (csb->which) {
  case COUNTRY_CODE :
    NVGEN_INT_MIB(MIB_h323_t35CountryCode,
		  "\n Show Configuration voice-over-ip country code failed")
      SET_INT_MIB(MIB_h323_t35CountryCode,GETOBJ(int,1),181,
		  "\n Configuration voice-over-ip country code failed");
    break;
    
  case CALL_SIG_PORT_NUM :
    NVGEN_INT_MIB(MIB_h323_Q931CallSignalingPort,
		  "\n Show Configuration voice-over-ip signalling port failed");
    SET_INT_MIB(MIB_h323_Q931CallSignalingPort,GETOBJ(int,1),1720,
		"\n Configuration voice-over-ip signalling port failed");
    break;
    
  case GMC_IP_ADDR :
    NVGEN_IP_MIB(MIB_h323_GKIPAddress,
		  "\n Show Configuration gmc ip address failed");
    SET_INT_MIB(MIB_h323_GKIPAddress,GETOBJ(paddr,1)->ip_addr,0,
		"\n Configuration gmc ip address failed");
    break;
  case GMC_PORT_NUM :
    NVGEN_INT_MIB(MIB_h323_GKPort,
		  "\n Show Configuration gmc port failed");
    SET_INT_MIB(MIB_h323_GKPort,GETOBJ(int,1),1719,
		"\n Configuration gmc port failed");
    break;
  }
}

void mspower_commands (parseinfo *csb)
{
  switch(csb->which) {
  case MAX_POWER_LEVEL :
    NVGEN_INT_MIB(MIB_msTxPwrMaxCellDef,
		  "\n Show Configuration of ms-power-control max ms tx power failed");

    SET_INT_MIB(MIB_msTxPwrMaxCellDef,GETOBJ(int,1),30,
		"\n Configuration of ms-power-control max ms tx power failed");
    break;
  case  UPLINK_TARGET :
    NVGEN_INT_MIB(MIB_pcLowerThresholdLevParamRxLevelUL,
		"\n Show Configuration of ms-power-control uplink power control target failed");
    SET_INT_MIB(MIB_pcLowerThresholdLevParamRxLevelUL,GETOBJ(int,1),30,
		"\n Configuration of ms-power-control uplink power control target failed");
    break;

  case SIGNAL_QUALITY :
    NVGEN_INT_MIB(MIB_pcAveragingQualHreqave,
		"\n Show Configuration of ms-power-control signal-quality-avg-window failed");
    SET_INT_MIB(MIB_pcAveragingQualHreqave,GETOBJ(int,1),4,
		"\n Configuration of ms-power-control signal-quality-avg-window failed");
    break;
  case SIGNAL_STRENGTH :
    NVGEN_INT_MIB(MIB_pcAveragingLevHreqave,
		"\n Show Configuration of ms-power-control signal-strength-avg-window failed");
    SET_INT_MIB(MIB_pcAveragingLevHreqave,GETOBJ(int,1),4,
		"\n Configuration of ms-power-control signal-strength-avg-window failed");
    break;
  }
}


static MibTag bts_mib_tags[] = {MIB_allowIMSIAttachDetach,MIB_cellBarred,
				MIB_dtxDownlink,MIB_rm_nim_1_0,MIB_emergencyCallRestricted};


static char *bts_nv_str[] = {
  " imsi-attach ",
  " cell-barred ",
  " downlink-dtx ",
  " imsi-for-emergency ",
  " restricted-emergency-call "
};

static char *bts_set_fail[] = {
  "\n IMSI Attach enable/disable failed",
  "\n Cell barred enable/disable failed",
  "\n Downlink dex enable/disable failed",
  "\n Imsi for emergency required enable/disable failed",
  "\n Restricted emergency call enable/disable failed"
 
};

static char *bts_dtx_mode[] = {
  " ms-may-use-dtx ",
  " ms-shall-use-dtx ",
  " ms-shall-not-use-dtx "
};

static void bts_options_nvgen (void)
{
  long nvgen_int;
  int intbuf[2]= {0,0};
  int index;
  
  printf("\n\n config-cell");

  for (index = 0;index < 5;index++) {
    oam_getMibIntVar(bts_mib_tags[index],&nvgen_int);
    if (nvgen_int) {
      printf("\n\t bts-options %s enable",bts_nv_str[index]);
    } else {
      printf("\n\t no bts-options %s enable",bts_nv_str[index]);
    }
  }

  oam_getMibIntVar(MIB_timerPeriodicUpdateMS,&nvgen_int);
  printf("\n\t bts-options location-update-interval %d ",nvgen_int);
  
  for (index=0;index<16;index++) {
    oam_getMibTblEntry(MIBT_notAllowedAccessClassEntry,index,intbuf,sizeof(int)*2);
    if (intbuf[1]) {
      printf("\n\t bts-options ms-access-class-not-allowed %d",index);
    } else {
      printf("\n\t no bts-options ms-access-class-not-allowed %d",index);
    }
  }

  oam_getMibIntVar(MIB_dtxUplink,&nvgen_int);
  printf("\n\t bts-options dtx-mode %s",bts_dtx_mode[nvgen_int]);

}


void bts_options (parseinfo *csb)
{
  int intbuf[2] = {0,1};
  int intnobuf[2] = {0,0};

  switch (csb->which) {
  case UPDATE_INTERVAL :
    if (csb->nvgen) {
      bts_options_nvgen ();
      return;
    }
    SET_INT_MIB(MIB_timerPeriodicUpdateMS,GETOBJ(int,1),30,
		"\n Configuration of update interval failed");
    break;

  case ACCESS_CLASS :
     if (csb->nvgen) {
      return;
    }
     if (csb->sense) {
       oam_setMibTblEntry(CLI_MODULE_ID,MIBT_notAllowedAccessClassEntry,GETOBJ(int,1),intbuf,
			 sizeof(int) *2);
     } else {
       oam_setMibTblEntry(CLI_MODULE_ID,MIBT_notAllowedAccessClassEntry,GETOBJ(int,1),intnobuf,sizeof(int) *2);
     }
     break;

  case DTX_MODE_OPTIONS :
    NVGEN_INT_MIB(MIB_dtxUplink,
		  "\n Show Configuration of downlink dtx failed");
    SET_INT_MIB(MIB_dtxUplink,GETOBJ(int,1),2,
		"\n Configuration of downlink dtx failed");
    break;
  case ENABLE_OPTIONS :
     SET_INT_MIB(bts_mib_tags[GETOBJ(int,1)],1,0,
		 bts_set_fail[GETOBJ(int,1)]);
     break;
  }
}


static void authentication_nvgen (parseinfo *csb)
{
  long nvgen_int1,nvgen_int2;
  oam_getMibIntVar(MIB_mm_authenticationRequired,&nvgen_int1);
  oam_getMibIntVar(MIB_mm_cipheringRequired,&nvgen_int2);

  if (nvgen_int1 && nvgen_int2){
    printf("\n %s ciphering enable",csb->nv_command);
    return;
  }
  
  if (nvgen_int1) {
    printf("\n %s enable",csb->nv_command);
    return;
  }
  
  if (!nvgen_int1) {
    printf("\n no %s ciphering enable",csb->nv_command);
  }
}


/*
 * system feature control parameters
 */ 

void sysfeature_commands (parseinfo *csb)
{
  switch (csb->which) {
  case CELL_BARR_ENABLE :
    NVGEN_BOOL_MIB(MIB_rm_nim_1_1,
		   "\n Show  Configuration of cell barring failed");
    SET_INT_MIB(MIB_rm_nim_1_1,csb->sense,0,
		"\n Configuration of cell barring failed");
    break;
    
  case SMS_CB_ENABLE :
    NVGEN_BOOL_MIB (MIB_rm_nim_0_1,
		    "\n Show Configuration of sms cb failed");
    SET_INT_MIB(MIB_rm_nim_0_1,csb->sense,0,
		"\n Configuration of sms cb failed");
    break;
    
  case AUTH_FREQ_VAL :
    NVGEN_INT_MIB(MIB_rm_nim_0_7,
		  "\n Show Configuration of authenticaion frequency failed");
    SET_INT_MIB(MIB_rm_nim_0_7,GETOBJ(int,1),1,
		"\n Configuration of authenticaion frequency failed");
    break;
  case AUTH_CIPH_ENABLE :
    if (csb->nvgen) {
      authentication_nvgen(csb);
      return;
    }
    SET_INT_MIB(MIB_mm_authenticationRequired,csb->sense,0,
		"\n Configuration of authenticaion  failed");
    SET_INT_MIB(MIB_mm_cipheringRequired,GETOBJ(int,1),0,
		"\n Configuration of ciphering  failed");
    
    break;
  }
} 

void gp10_config_cdr_data(parseinfo *csb)
{
  
  switch (csb->which){

  case CONFIG_CDR_CLIENT_SSL_ENABLE:
    NVGEN_BOOL_MIB(MIB_cdrClientSSLEnable,
		   "\nShow configuration of cdr client ssl enable failed");
    SET_INT_MIB(MIB_cdrClientSSLEnable,csb->sense,0,
		"\nConfiguration of cdr client ssl failed");
    break;

  case CONFIG_CDR_PASSWORD:
    if (csb->nvgen) {
      return;
    }
      
    if(csb->sense)  {
      if(SavePwdCdr((unsigned char *)GETOBJ(string, 2)) == 0)  {
	printf("\nConfiguration of CDR Client password failed\n");
      }
    } else {
      if(SavePwdCdr("") == 0) {
	printf("\nConfiguration of CDR Client password failed\n");
      }
    }
    break;
  default:
    printf("\n Incorrect data");
    break;
  } /* END SWITCH */
} /* gp10_config_cdr_data() */

void handover_control_average_nv (parseinfo *csb)
{
  long nvgen_int;

  oam_getMibIntVar(MIB_hoThresholdLevParamNx,&nvgen_int);
  printf("\n %s N5 %d",csb->nv_command,nvgen_int);
  oam_getMibIntVar(MIB_hoThresholdQualParamNx,&nvgen_int);
  printf("\n %s N6 %d",csb->nv_command,nvgen_int);
  oam_getMibIntVar(MIB_hoThresholdLevParamPx,&nvgen_int);
  printf("\n %s P5 %d",csb->nv_command,nvgen_int);
  oam_getMibIntVar(MIB_hoThresholdQualParamPx,&nvgen_int);
  printf("\n %s P6 %d",csb->nv_command,nvgen_int);
}

static void handover_control_str_nv (parseinfo *csb)
{
  long nvgen_int;

  oam_getMibIntVar(MIB_hoThresholdQualParamRxQualUL,&nvgen_int);
  printf("\n %s uplink quality %d",csb->nv_command,nvgen_int);
  oam_getMibIntVar(MIB_hoThresholdQualParamRxQualDL,&nvgen_int);
  printf("\n %s downlink quality %d",csb->nv_command,nvgen_int);
  oam_getMibIntVar(MIB_hoThresholdLevParamRxLevelUL,&nvgen_int);
  printf("\n %s uplink strength %d",csb->nv_command,nvgen_int);
  oam_getMibIntVar(MIB_hoThresholdLevParamRxLevelDL,&nvgen_int);
  printf("\n %s downlink strength %d",csb->nv_command,nvgen_int);
}

void handover_control_commands (parseinfo *csb)
{
  switch (csb->which) {
  case HANDOVER_AVERAGE :
    if (csb->nvgen) {
      handover_control_average_nv(csb);
      return;
    }
    switch (GETOBJ(int,1)) {
    case HANDOVER_N5 :
      NVGEN_INT_MIB(MIB_hoThresholdLevParamNx,
		    "\n Show Configuration of N5 failed");
      SET_INT_MIB(MIB_hoThresholdLevParamNx,GETOBJ(int,2),4,
		  "\n Configuration of N5 failed");
      break;
    case HANDOVER_N6 :
      NVGEN_INT_MIB(MIB_hoThresholdQualParamNx,
		  "\n Show Configuration of N6 failed");
      SET_INT_MIB(MIB_hoThresholdQualParamNx,GETOBJ(int,2),4,
		  "\n Configuration of N6 failed");
      break;
    case HANDOVER_P5 :
      NVGEN_INT_MIB(MIB_hoThresholdLevParamPx,
		  "\n Show Configuration of P5 failed");
      SET_INT_MIB(MIB_hoThresholdLevParamPx,GETOBJ(int,2),4,
		"\n Configuration of P5 failed");
      break;
    case HANDOVER_P6 :
      NVGEN_INT_MIB(MIB_hoThresholdQualParamPx,
		  "\n Show  Configuration of P6 failed");
      SET_INT_MIB(MIB_hoThresholdQualParamPx,GETOBJ(int,2),4,
		  "\n Configuration of P6 failed");
      break;
    }
    break;
    
  case HANDOVER_QUALITY :
    switch (GETOBJ(int,1)) {
    case UPLINK :
      SET_INT_MIB(MIB_hoThresholdQualParamRxQualUL,GETOBJ(int,2),4,
		  "\n Configuration of uplink quality failed");
      break;
    case DOWNLINK :
      SET_INT_MIB(MIB_hoThresholdQualParamRxQualDL,GETOBJ(int,2),4,
		"\n Configuration of downlink quality failed");
      break;
    }
    break;

  case HANDOVER_STRENGTH :
    if (csb->nvgen) {
      handover_control_str_nv(csb);
      return;
    }
    switch (GETOBJ(int,1)) {
    case UPLINK :
      SET_INT_MIB(MIB_hoThresholdLevParamRxLevelUL,GETOBJ(int,2),20,
		"\n Configuration of uplink strength failed");
      break;
    case DOWNLINK :
      SET_INT_MIB(MIB_hoThresholdLevParamRxLevelDL,GETOBJ(int,2),20,
		"\n Configuration of downlink strength failed");
      break;
    }
  }
}


#define LOG_DEST_CONSOLE 0x0001
#define LOG_DEST_NETWORK 0x0002
#define LOG_DEST_FILE    0x0004

int logging_dest=LOG_DEST_CONSOLE;
int log_port;
long log_host;


extern void ViperLogSetConsolePrintOptions__Fi(BOOL);
void logging_option (parseinfo *csb)
{
  char log_ipaddr[16];
  
  if (csb->nvgen) {

	  if(logging_dest==0){
		  printf("\nlogging not configured\n");
		  return;
	  }

	  if(logging_dest & LOG_DEST_CONSOLE){
			printf("\nlogging console");
	  }
	  if(logging_dest & LOG_DEST_NETWORK){
			  if (log_host != 0) {
				  if (log_port != 0) 
					printf("\nlogging %s %d",address_string(log_host,log_ipaddr),log_port);
				  else
					printf("\nlogging %s",address_string(log_host,log_ipaddr));
			  }
	  }
	  if(logging_dest & LOG_DEST_FILE){
			printf("\nlogging file");
	  }

	  return;
  }


  if (!csb->sense) {
	  switch(logging_dest){
		case LOG_DEST_CONSOLE:
			logging_dest &= ~LOG_DEST_CONSOLE;
			break;
		case LOG_DEST_FILE:
			logging_dest &= ~LOG_DEST_FILE;
			break;
		case LOG_DEST_NETWORK:
			logging_dest &= ~LOG_DEST_NETWORK;
			break;
	  }
	  return;
  }

  switch (csb->which) {
  case LOG_CONSOLE :
     logging_dest |= LOG_DEST_CONSOLE;
    break;
  case LOG_FILE  :
    logging_dest |= LOG_DEST_FILE;
    break;
  case LOG_HOST :
    logging_dest |= LOG_DEST_NETWORK;
    log_host = GETOBJ(paddr,1)->ip_addr;
    address_string(GETOBJ(paddr,1)->ip_addr,log_ipaddr);
    if (GETOBJ(int,1) != 0) {
      log_port = GETOBJ(int,1);
      ViperLogSetDestPort(GETOBJ(int,1));
    }
    ViperLogSetDestAddress(log_ipaddr);
    break;
  }
}



void gp10_sub_config_exit (parseinfo *csb)
{
  set_mode_byname(&csb->mode,"configure",MODE_VERBOSE);
}

void gp10_cell_sub_config_exit (parseinfo *csb)
{
  set_mode_byname(&csb->mode,"config-cell",MODE_VERBOSE);
}
