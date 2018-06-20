
/*
 * enum for command identification
 */

enum {
  CONFIG_CDR_PASSWORD,
  CONFIG_CDR_CLIENT_SSL_ENABLE,
  CONF_SNMP_COMM,
  SUB_CONFIG_PROTO,
  SUB_CONFIG_ADJCELL,
  SUB_CONFIG_CELL,
  CONF_CUSTADDR,
  CONF_CUSTINFO,
  CONF_CUSTEMAIL,
  CONF_CUSTPHONE,
  CONF_CUSTNAME,
  CONF_ASSET,
  CONF_LOCATION,
  CONF_GW_IPADDR,
  CONF_SYS_IPADDR,
  CONF_SYS_NAME
};



/*
 *	cdr password <string>
 *  no cdr password
 */

EOLS(config_cdr_password_eols, gp10_config_cdr_data, CONFIG_CDR_PASSWORD);

STRING(config_cdr_password_val, config_cdr_password_eols, no_alt, 
	   OBJ(string, 2), "Password");

NVGENS (config_cdr_password_nv, config_cdr_password_val, gp10_config_cdr_data, CONFIG_CDR_PASSWORD);

NOPREFIX(config_cdr_password_no, config_cdr_password_nv, config_cdr_password_eols);

KEYWORD(config_cdr_password, config_cdr_password_no, no_alt, 
		"password", "Configure CDR Client password", PRIV_CONF);


/*
 *	cdr client ssl enable
 *  no cdr client ssl enable
 */

EOLS(config_cdr_client_ssl_enable_eols, gp10_config_cdr_data, CONFIG_CDR_CLIENT_SSL_ENABLE);

KEYWORD(config_cdr_client_ssl_enable, config_cdr_client_ssl_enable_eols, no_alt, 
	"enable", "Enable/Disable encryption between CDR Client & Server :NOTE: use no command to disable", PRIV_CONF);

NVGENS (config_cdr_client_ssl_nv, config_cdr_client_ssl_enable, gp10_config_cdr_data, CONFIG_CDR_CLIENT_SSL_ENABLE);

KEYWORD(config_cdr_client_ssl, config_cdr_client_ssl_nv, no_alt, 
		"ssl", "Configure CDR Client SSL data", PRIV_CONF);

KEYWORD(config_cdr_client, config_cdr_client_ssl, config_cdr_password, 
		"client", "Configure CDR Client data", PRIV_CONF);


/*
 * cdr
 */

KEYWORD(config_cdr_data, config_cdr_client, no_alt, 
		"cdr", "Commands related to CDR Client configuration", PRIV_CONF);

/**********************************************************/
/*			 CDR Command Line Interface					  */
/*			            END		           				  */
/**********************************************************/


/*
 *[no] snmp community	<read community> <write community>	
 */

EOLS    (cfg_snmp_comm_eol,system_configuration,CONF_SNMP_COMM);

STRING  (cfg_snmp_write,cfg_snmp_comm_eol,no_alt,OBJ(string,2),
	 "Enter the write community string");

STRING  (cfg_snmp_read,cfg_snmp_write,no_alt,OBJ(string,1),
	 "Enter the read community string");

NOPREFIX (cfg_snmp_comm_no,cfg_snmp_read,cfg_snmp_comm_eol);

NVGENS   (cfg_snmp_comm_nv,cfg_snmp_comm_no,system_configuration,CONF_SNMP_COMM);

KEYWORD (cfg_snmp_comm_key,cfg_snmp_comm_nv,no_alt,"community",
	 "Configure snmp community strings",PRIV_CONF);

KEYWORD (cfg_snmp_comm,cfg_snmp_comm_key,config_cdr_data,"snmp",
	 "Configure snmp mibs",PRIV_CONF);



/*
 *config-proto
 */

EOLS    (cfg_proto_param_eol,gp10_subconfig_cmds,SUB_CONFIG_PROTO);

NVGENS (cfg_proto_param_nv,cfg_proto_param_eol,gp10_subconfig_cmds,SUB_CONFIG_PROTO);

KEYWORD (cfg_proto_param,cfg_proto_param_nv,cfg_snmp_comm,"config-proto",
	 "Configure protocol management parameters",PRIV_CONF);

/*
 * config-adjcell
 */

EOLS    (cfg_adjcell_param_eol,gp10_subconfig_cmds,SUB_CONFIG_ADJCELL);

NUMBER  (cfg_adjcell_num,cfg_adjcell_param_eol,no_alt,OBJ(int,1),1,16,
	 "Enter the adjacent cell number");

NVGENS  (cfg_adjcell_param_nv,cfg_adjcell_num,gp10_subconfig_cmds,SUB_CONFIG_ADJCELL);

KEYWORD (cfg_adjcell_param,cfg_adjcell_param_nv,cfg_proto_param,"config-adjcell",
	 "Configure adjacent cell parameters",PRIV_CONF);

/*
 * config-cell
 */

EOLS    (cfg_cell_param_eol,gp10_subconfig_cmds,SUB_CONFIG_CELL);

NVGENS ( cfg_cell_param_nv,cfg_cell_param_eol,gp10_subconfig_cmds,SUB_CONFIG_CELL);

KEYWORD (cfg_cell_param,cfg_cell_param_nv,cfg_adjcell_param,"config-cell",
	 "Configure cell parameters",PRIV_CONF);

/*
 * customer-address <address-string>
 */

EOLS  (custaddr_eol,system_configuration,CONF_CUSTADDR);

STRING (custaddr,custaddr_eol,no_alt,OBJ(string,1),
	"GP10 customer address");
NVGENS (custaddr_nv,custaddr,system_configuration,CONF_CUSTADDR);

NOPREFIX (custaddr_no,custaddr_nv,custaddr_eol);

KEYWORD (cfg_custaddr,custaddr_no,cfg_cell_param,"customer-address",
	 "Configure GP10 customer address.Changes will be effective only after GP10 reboot",PRIV_CONF);

/*
 * customer-info <info-string>
 */

EOLS  (custinfo_eol,system_configuration,CONF_CUSTINFO);

STRING (custinfo,custinfo_eol,no_alt,OBJ(string,1),
	"GP10 customer information");
NVGENS (custinfo_nv,custinfo,system_configuration,CONF_CUSTINFO);

NOPREFIX (custinfo_no,custinfo_nv,custinfo_eol);

KEYWORD (cfg_custinfo,custinfo_no,cfg_custaddr,"customer-info",
	 "Configure GP10 customer information.Changes will be effective only after GP10 reboot",PRIV_CONF);

/*
 * customer-email <email-id-string>
 */

EOLS  (custemail_eol,system_configuration,CONF_CUSTEMAIL);

STRING (custemail,custemail_eol,no_alt,OBJ(string,1),
	"GP10 customer email id");
NVGENS (custemail_nv,custemail,system_configuration,CONF_CUSTEMAIL);

NOPREFIX (custemail_no,custemail_nv,custemail_eol);

KEYWORD (cfg_custemail,custemail_no,cfg_custinfo,"customer-email",
	 "Configure GP10 customer email id.Changes will be effective only after GP10 reboot",PRIV_CONF);


/*
 * customer-phone <phone-number-string>
 */

EOLS  (custphone_eol,system_configuration,CONF_CUSTPHONE);

STRING (custphone,custphone_eol,no_alt,OBJ(string,1),
	"Phone number of the GP10 customer");

NVGENS (custphone_nv,custphone,system_configuration,CONF_CUSTPHONE);

NOPREFIX (custphone_no,custphone_nv,custphone_eol);

KEYWORD (cfg_custphone,custphone_no,cfg_custemail,"customer-phone",
	 "Configure GP10 customer phone number.Changes will be effective only after GP10 reboot",PRIV_CONF);


/*
 * customer-name <name-string>
 */

EOLS  (custname_eol,system_configuration,CONF_CUSTNAME);

STRING (custname,custname_eol,no_alt,OBJ(string,1),
	"Customer name of the GP10 host");
NVGENS (custname_nv,custname,system_configuration,CONF_CUSTNAME);

NOPREFIX (custname_no,custname_nv,custname_eol);

KEYWORD (cfg_custname,custname_no,cfg_custphone,"customer-name",
	 "Configure GP10 customer name.Changes will be effective only after GP10 reboot",PRIV_CONF);

/*
 * asset <asset-string>
 */

EOLS  (asset_eol,system_configuration,CONF_ASSET);

STRING (asset,asset_eol,no_alt,OBJ(string,1),
	"Asset of the GP10 host");
NVGENS (asset_nv,asset,system_configuration,CONF_ASSET);

NOPREFIX (asset_no,asset_nv,asset_eol);

KEYWORD (cfg_asset,asset_no,cfg_custname,"asset",
	 "Configure GP10 asset number.Changes will be effective only after GP10 reboot",PRIV_CONF);

/*
 *location <location-string>
 */


EOLS  (location_eol,system_configuration,CONF_LOCATION);

STRING (location,location_eol,no_alt,OBJ(string,1),
	"Location of the GP10 host");
NVGENS (location_nv,location,system_configuration,CONF_LOCATION);

NOPREFIX (location_no,location_nv,location_eol);

KEYWORD (cfg_location,location_no,cfg_asset,"location",
	 "Configure GP10 location.Changes will be effective only after GP10 reboot",PRIV_CONF);

/*
 * ip default-gateway <ip-address>
 */

EOLS (gw_ip_addr_eol,system_configuration,CONF_GW_IPADDR);

IPADDR (gw_ip_addr,gw_ip_addr_eol,no_alt,OBJ(paddr,1),
	"Ip address of the default gateway");

NVGENS (gw_ip_addr_nv,gw_ip_addr,system_configuration,CONF_GW_IPADDR);

NOPREFIX (gw_ip_addr_no,gw_ip_addr_nv,gw_ip_addr_eol);

KEYWORD (default_gw,gw_ip_addr_no,no_alt,"default-gateway",
	 "Configure default IP gateway.Changes will be effective only after GP10 reboot",PRIV_CONF);

/*
 *ip address <ip-address> <subnet-mask>
 */

EOLS  (ip_addr_eol,system_configuration,CONF_SYS_IPADDR);

IPADDR_MASK(host_ip_addr,ip_addr_eol,no_alt,OBJ(paddr,1),OBJ(paddr,2),
       "Enter IP Address and subnet mask of the GP10");
NVGENS (host_ip_addr_nv,host_ip_addr,system_configuration,CONF_SYS_IPADDR);

NOPREFIX (host_ip_addr_no,host_ip_addr_nv,ip_addr_eol);

KEYWORD (cfg_ip_addr_key,host_ip_addr_no,default_gw,"address",
	 "Configure IP Address.Changes will be effective only after GP10 reboot",PRIV_CONF);

KEYWORD (cfg_ip_addr,cfg_ip_addr_key,cfg_location,"ip",
	 "Configure IP Address.Changes will be effective only after GP10 reboot",PRIV_CONF);

/*
 * hostname <name>
 */


EOLS  (sys_name_eol,system_configuration,CONF_SYS_NAME);

STRING (sys_name,sys_name_eol,no_alt,OBJ(string,1),
	"Name of the GP10 host");
NVGENS (sys_name_nv,sys_name,system_configuration,CONF_SYS_NAME);

NOPREFIX (sys_name_no,sys_name_nv,sys_name_eol);

KEYWORD (cfg_sys_name,sys_name_no,cfg_ip_addr,"hostname",
	 "Configure GP10 host name.Changes will be effective only after GP10 reboot",PRIV_CONF);

#undef ALTERNATE
#define ALTERNATE cfg_sys_name
