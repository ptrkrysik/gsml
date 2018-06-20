/* 
 * enum for command identification
 */

enum {
  MIB_FILE,
  CDR_SWITCH,
  CDR_STATUS,
  CHECK_EEPROM,
  WRITE_EEPROM,
  READ_EEPROM,
  MIB_IMPORT,
  MIB_EXPORT,
  SNMP_SET_MIB,
  SNMP_GET_MIB,
  MIB_TYPE_INT,
  MIB_TYPE_STR,
  SNMP_MIB_LKUP,
};


static keyword_options mib_options[] = {
  {"import","Import a mib file.Enter the complete directory path",MIB_IMPORT},
  {"export","Export a mib file.Enter the complete directory path",MIB_EXPORT},
  {NULL,NULL,0}
};

static keyword_options mib_type[] = {
  {"int","Integer type mib variable",MIB_TYPE_INT},
  {"string","String type mib variable",MIB_TYPE_STR},
  {NULL,NULL,0}
};

/*
 * Clear voip call-stats 	 
 */
EOLNS   (exec_clear_eol,clearVoipCall);
KEYWORD (exec_clear_voip_call_stats,exec_clear_eol,no_alt,
			"call-stats","Clear voip call stats",PRIV_CONF);
KEYWORD (exec_clear_chain_voip,exec_clear_voip_call_stats,no_alt,
			"voip","Clear voip call stats",PRIV_CONF);

/********/


EOLS    (snmp_lkup_eol,exec_module,SNMP_MIB_LKUP);

STRING  (snmp_mib_lkup_str,snmp_lkup_eol,no_alt,OBJ(string,1),
	 "Enter the mib tag");

KEYWORD (snmp_mib_lkup,snmp_mib_lkup_str,no_alt,"lookup",
	 "Lookup a mib tag",PRIV_CONF);

/*
 *snmp mib set	<tag>   [<row>] { int | string }
 */
EOLS    (mib_set_eol,exec_module,SNMP_SET_MIB);

STRING  (set_str_mib,mib_set_eol,no_alt,OBJ(string,2),
	 "Enter the string value of mib");

NUMBER  (set_int_mib,mib_set_eol,no_alt,OBJ(int,2),0,65535,
	 "Enter the integer value of mib");

IFELSE  (test_mib_type,set_int_mib,set_str_mib,(GETOBJ(int,1) == MIB_TYPE_INT));

KEYWORD_OPTIONS (set_mib_type,test_mib_type,no_alt,mib_type,
		 OBJ(int,1),PRIV_CONF,0);

NUMBER (set_mib_row,set_mib_type,set_mib_type,OBJ(int,3),0,255,
	"Enter the index of the variable");

STRING  (set_mib_tag,set_mib_row,no_alt,OBJ(string,1),
	 "Enter the mib tag");

KEYWORD (snmp_mib_set,set_mib_tag,snmp_mib_lkup,"set",
	 "Set value of a mib tag",PRIV_CONF);

/*
 *snmp mib get	<tag>   [<row>]
 */
EOLS    (mib_get_eol,exec_module,SNMP_GET_MIB);

NUMBER (get_mib_row,mib_get_eol,mib_get_eol,OBJ(int,2),0,255,
	"Enter the index of the variable");

STRING  (get_mib_tag,get_mib_row,no_alt,OBJ(string,1),
	 "Enter the mib tag");

KEYWORD (snmp_mib_get,get_mib_tag,snmp_mib_set,"get",
	 "Get value of a mib tag",PRIV_CONF);

/*
 * snmp mib {import | export} <filename>
 */

EOLS (mib_eol,exec_module,MIB_FILE);

STRING (mib_file,mib_eol,no_alt,OBJ(string,1),
	"Enter the mib file name with directory path");

KEYWORD_OPTIONS (snmp_ex_or_im,mib_file,snmp_mib_get,mib_options,
		 OBJ(int,1),PRIV_CONF,0);

/*
 * snmp mib
 */

KEYWORD (snmp_mib,snmp_ex_or_im,no_alt,"mib",
	 "Mib Export/Import/Get/Set/Lookup operations",PRIV_CONF);

/*
 * snmp
 */

KEYWORD (snmp_mod,snmp_mib,no_alt,"snmp",
	 "Snmp module commands",PRIV_CONF);

/*
 *cdr switch-server <ip-address>
 */

EOLS (cdr_switch_eol,exec_module,CDR_SWITCH);

IPADDR( cdr_ser_ip,cdr_switch_eol,no_alt,OBJ(paddr,1),
	"Enter the ip address of cdr server");

KEYWORD (cdr_switch,cdr_ser_ip,no_alt,"switch-server",
	 "Switch the CDR server",PRIV_CONF);


/*
 *cdr get-status
 */

EOLS (cdr_status_eol,exec_module,CDR_STATUS);

KEYWORD (cdr_status,cdr_status_eol,cdr_switch,"get-status",
	 "Get status of cdr module",PRIV_CONF);



/*
 *cdr client
 */

KEYWORD (cdr_client_key,cdr_status,no_alt,"client",
	 "Cdr client exec commands",PRIV_CONF);

KEYWORD (cdr_mod,cdr_client_key,snmp_mod,"cdr",
	 "Cdr module exec commands",PRIV_CONF);


/*
 * check-eeprom
 */

EOLS (check_eeprom_eol,exec_module,CHECK_EEPROM);

KEYWORD (check_eeprom,check_eeprom_eol,snmp_mod,"check-eeprom",
	 "Read tx calibration tables from eeprom ",PRIV_CONF | PRIV_HIDDEN);


/*
 * write-eeprom
 */

EOLS (write_eeprom_eol,exec_module,WRITE_EEPROM);

KEYWORD (write_eeprom,write_eeprom_eol,check_eeprom,"write-eeprom",
	 "Write tx calibration tables to  eeprom on the RF board",PRIV_CONF | PRIV_HIDDEN);


         
/*
 * read-eeprom
 */


EOLS (read_eeprom,exec_module,READ_EEPROM);

KEYWORD (exec_module,read_eeprom,write_eeprom,"read-eeprom",
	 "Read tx calibration tables from eeprom on the RF board",PRIV_CONF | PRIV_HIDDEN);

#undef ALTERNATE
#define ALTERNATE exec_module






