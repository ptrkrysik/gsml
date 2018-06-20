#include "parser_if.h"

/* 
 * Include other api headers
 */ 
#include "Mch/MCHDefs.h"
#include "oam_api.h"
#include "mibtags.h"
#include "MnetModuleId.h"

/*
 * Parse chains for Exec diagnostic commands
 */
#define ALTERNATE       NONE
#include "../include/exec_diag_cmd.h"
LINK_POINT(exec_diag_commands, ALTERNATE);
#undef  ALTERNATE

/*
 * Parse chains for Exec show commands
 */
#define ALTERNATE       NONE
#include "../include/exec_show_mod.h"
LINK_POINT(exec_show_mod_command, ALTERNATE);
#undef  ALTERNATE

/*
 * Parse chains for module specific Exec  commands
 */
#define ALTERNATE       NONE
#include "../include/exec_module.h"
LINK_POINT(exec_mod_cmds, ALTERNATE);
#undef  ALTERNATE

/*
 * Parse chains for module specific Exec  commands
 */
#define ALTERNATE       NONE
#include "../include/exec_show_conf.h"
LINK_POINT(exec_show_conf, ALTERNATE);
#undef  ALTERNATE


/*
 * Parse chains for module specific Exec  commands
 */
#define ALTERNATE       NONE
#include "../include/exec_debug_gp10.h"
LINK_POINT(exec_dbg_gp10, ALTERNATE);
#undef  ALTERNATE

/*
 * Parse chain registration array for Exec
 */
static parser_extension_request exec_diag_chain_init_table[] = {
    { PARSE_ADD_EXEC_CMD, &pname(exec_diag_commands) },
    { PARSE_ADD_SHOW_CMD, &pname(exec_show_mod_command)},
    { PARSE_ADD_SHOW_CMD, &pname(exec_show_conf)},
    { PARSE_ADD_EXEC_CMD, &pname(exec_mod_cmds) },
    { PARSE_ADD_DEBUG_CMD,&pname(exec_dbg_gp10) },
	{ PARSE_ADD_CLEAR_CMD, &pname(exec_clear_chain_voip) },
    { PARSE_LIST_END, NULL }
};

void show_configuration (parseinfo *csb)
{
  parseinfo *nvcsb;
  
  nvcsb = get_csb("NV parser CSB");
  if (csb == NULL) {
    printf(nomemory);
    return;
  }
  printf("\nBuilding configuration...");
  nvcsb->nvgen = TRUE;
  nvcsb->sense = TRUE;
  
  nvcsb->priv = PRIV_MAX;
  nvcsb->priv_set = 0xf;
  nvcsb->mode = config_mode;
  push_node(nvcsb, get_mode_nv(nvcsb));
  parse_token(nvcsb);
  free_csb(&nvcsb);
  printf("\n");
}




void exec_diag_parser_init (void)
{
  parser_add_command_list(exec_diag_chain_init_table, "exec");
}


/* 
 * Global variables
 */
char cmd_line[MCH_MAX_COMMAND_LEN];

void diagnostic_cmds (parseinfo *csb)
{

  switch (csb->which) {
  case DIAG_CFTEST_CMD :
    sprintf(cmd_line,"startdiag cftest %d",GETOBJ(int,1));
    break;
  case DIAG_STOPDIAG_CMD:
    sprintf(cmd_line," stopdiag %s",GETOBJ(string,1));
    break;
  case DIAG_HPIMEMACCESS_CMD:
    sprintf(cmd_line," startdiag hpimemoryaccess %d %d %d ",GETOBJ(int,1),GETOBJ(int,2),GETOBJ(int,3));
    break;
  case DIAG_HPIECHOTEST_CMD:
    sprintf(cmd_line," startdiag hpiechotest %d %d %d %d %d",
	    GETOBJ(int,1),GETOBJ(int,2),GETOBJ(int,3),GETOBJ(int,4),GETOBJ(int,5));
    break;
  case DIAG_HOREPORT_CMD:
    sprintf(cmd_line," startdiag horeport %x %x %d",GETOBJ(int,1),GETOBJ(int,2),GETOBJ(int,3));
    break;
  case DIAG_FPGATEST_CMD:
    sprintf(cmd_line," startdiag fpgatest %d",GETOBJ(int,1));
    break;
  case DIAG_DSPMEMTEST_CMD:
    sprintf(cmd_line," startdiag dspmemtest %d %d %d",GETOBJ(int,1),GETOBJ(int,2),GETOBJ(int,3));
    break;
  case DIAG_DSPLOOPBACK_CMD:
    sprintf(cmd_line," startdiag dsploopback %d %d %d",GETOBJ(int,1),GETOBJ(int,2),GETOBJ(int,3));
    break;
  case DIAG_DSPEXTMEMTEST_CMD:
    sprintf(cmd_line," startdiag dspextmemtest %d %d %d",
	    GETOBJ(int,1),GETOBJ(int,2),GETOBJ(int,3));
    break;
  case DIAG_DSPBBLBTEST_CMD:
    sprintf(cmd_line," startdiag dspbblbtest %d %d %d %d %d",
	  GETOBJ(int,1),GETOBJ(int,2),GETOBJ(int,3),GETOBJ(int,4),GETOBJ(int,5));
    break;
  case DIAG_DLREPORT_CMD:
    sprintf(cmd_line," startdiag dlreport %x %x %d",GETOBJ(int,1),GETOBJ(int,2),GETOBJ(int,3));
    break;
  case DIAG_IFREPORT_CMD:
    sprintf(cmd_line," startdiag ifreport %x %x %d",GETOBJ(int,1),GETOBJ(int,2),GETOBJ(int,3));
    break;
  case DIAG_MSREPORT_CMD:
    sprintf(cmd_line," startdiag msreport %x %x %d",GETOBJ(int,1),GETOBJ(int,2),GETOBJ(int,3));
    break;
  case DIAG_I2CTEST_CMD :
    sprintf(cmd_line," startdiag i2ctest");
    break;
  case DIAG_I2CLOOPBACK_CMD :
    sprintf(cmd_line,"startdiag i2cloopback numbytes %d duration %d %d %s",GETOBJ(int,1),
	    GETOBJ(int,2),GETOBJ(int,3),(GETOBJ(int,4)!=HALT_ON_ERROR)?"":"Haltonerror");
    break;
  }
  /*
   * call the MCH routine to handle the command
   */
  MaintCmd__FPc(cmd_line);
}

/* 
 * show command handler
 */

void show_commands (parseinfo *csb)
{
  switch (csb->which) {
  case VERSION_OPT :
    oam_getMibByteAry(MIB_viperCellSoftwareBuild,nvgen_string,CLI_MAX_STRING_LEN);
    printf("\n %s\n",nvgen_string);
    break;
  case DEBUG_OPT :
    ViperLogShowAllFilters();
    break;
  case CDR_DATA :
    CdrDataPrint();
    break;
  case ALARM_STATISTICS :
    AlarmPrintStatus();
    break;
  case VOIP_CALL_STATS :
    VoipShowCallStats__Fv();
    break;
  case VOIP_CALL_CNT :
    VoipShowCallCounters__Fv();
    break;
  case VOIP_CALL_SHOW :
		VoipShowCall__FP19_CALL_CONTROL_BLOCK(GETOBJ(int,1));
    break;
  case VOIP_ALL_CALL:
	  VoipShowAllCalls__Fv();
	  break;
  }
}

/*
 * module specific exec command handler
 */
extern void VoipClearCallStats__Fv(void);
void exec_module (parseinfo *csb)
{
  char cdr_ipaddr[16];
  int tag;

  switch (csb->which) {
  case MIB_FILE :
    switch (GETOBJ(int,1))
      {
      case MIB_IMPORT :
	snmp_loadMib(GETOBJ(string,1));
	break;
      case MIB_EXPORT :
	snmp_saveMib(GETOBJ(string,1));
	break;
      }
    break;

  case SNMP_SET_MIB :
    if (tag = snmp_nameToTag(GETOBJ(string,1))) {
      if (MTYPE_IS_TABLE(MIB_TYP(tag))) {
	if (GETOBJ(int,1) == MIB_TYPE_INT) {
	  if (SnmpMibSet(GETOBJ(string,1),GETOBJ(int,3),GETOBJ(int,2)) != STATUS_OK )
	    printf(" Setting of mib %s failed",GETOBJ(string,1));
	} else {
	  if (SnmpMibSet(GETOBJ(string,1),GETOBJ(int,3),GETOBJ(string,2)) != STATUS_OK)
	    printf(" Setting of mib %s failed",GETOBJ(string,1));
	}
      } else {
	if (GETOBJ(int,1) == MIB_TYPE_INT) {
	  if (SnmpMibSet(GETOBJ(string,1),GETOBJ(int,2)) != STATUS_OK )
	    printf(" Setting of mib %s failed",GETOBJ(string,1));
	} else {
	  if (SnmpMibSet(GETOBJ(string,1),GETOBJ(string,2)) != STATUS_OK)
	    printf(" Setting of mib %s failed",GETOBJ(string,1));
	}
      }
    } else {
      printf("Unable to locate MIB tag from supplied mib string '%s'\n", GETOBJ(string,1));
    }
    break;
    
  case SNMP_GET_MIB :
    SnmpMibGet(GETOBJ(string,1),GETOBJ(int,2));
    break;
  case SNMP_MIB_LKUP :
    SnmpMibLookup(GETOBJ(string,1));
    break;
  case CDR_SWITCH :
    address_string(GETOBJ(paddr,1)->ip_addr,cdr_ipaddr);
    _cdrSwitchServer(cdr_ipaddr);
    break;
  case CDR_STATUS :
    cdrGetStatus();
    break;
  case CHECK_EEPROM :
    CheckEeprom__Fv();
    break;
  case WRITE_EEPROM :
    WriteEeprom__Fv();
    break;
  case READ_EEPROM :
    ReadEeprom__Fv();
    break;
  }
}

/*
 * Mask for turning on debugs
 */
static int gp10_debug_mask[] = {DBG_LOG_CONTROL_TRACE_MASK ,
				DBG_LOG_CALL_TRACE_MASK ,
				DBG_LOG_INFORMATIONAL_MASK , 
				DBG_LOG_WARNINGS_MASK , 
				DBG_LOG_ERRORS_MASK,
				DBG_LOG_ALL_TRACE_MASK,
				DBG_LOG_TIMESTAMP_MASK
};

/*
 * Mask for turning off debugs
 */

static int gp10_debug_inversemask[] = {DBG_LOG_CONTROL_TRACE_INVERSE_MASK ,
				       DBG_LOG_CALL_TRACE_INVERSE_MASK ,
				       DBG_LOG_INFORMATIONAL_INVERSE_MASK , 
				       DBG_LOG_WARNINGS_INVERSE_MASK , 
				       DBG_LOG_ERRORS_INVERSE_MASK,
				       DBG_LOG_ALL_TRACE_INVERSE_MASK
};

static int debug_option_flag = 0;

void set_gp10_debug_flag (parseinfo *csb)
{
  if (csb->sense) {
    debug_option_flag |= gp10_debug_mask[GETOBJ(int,2)];
  } else {
    debug_option_flag &= gp10_debug_inversemask[GETOBJ(int,2)];
  }
}

/*
 * Handler routine for debug commands
 */			 
extern int logging_dest;
void gp10_debug_command( parseinfo *csb)
{
  char c;
  
  if(GETOBJ(int,1)==GP10_DEBUG_ALL && csb->sense){
  		printf("This may severely impact network performance: Continue: [yes/no] ");
		read(STD_IN,&c,1);
		write(STD_OUT,"\n",1);
		if(c!='y')
			return;
  }

  if (debug_option_flag == 0) {
    if (csb->sense) {
		  
	      debug_option_flag = gp10_debug_mask[DBG_ALL_LOG];
    } else {
      debug_option_flag = gp10_debug_inversemask[DBG_ALL_LOG];
    }
  }
  

  debug_option_flag |= logging_dest;
  ViperLogSetModuleFilters(GETOBJ(int,1),debug_option_flag);
  debug_option_flag = 0;
}


void  clearVoipCall(parseinfo *csb){
    VoipClearCallStats__Fv();
}
