
/*
 * enum for command identification
 */

enum {
  COUNTRY_CODE,
  CALL_SIG_PORT_NUM,
  GMC_IP_ADDR,
  GMC_PORT_NUM
};


/*
 *voip t35-country-code <country-code>
 */

EOLS (country_code_eol,voip_parameters,COUNTRY_CODE);

NUMBER (country_code,country_code_eol,no_alt,OBJ(int,1),1,999,
	"Enter country code");

NVGENS (country_code_nv,country_code,voip_parameters,COUNTRY_CODE);

NOPREFIX (country_code_no,country_code_nv,country_code_eol);

KEYWORD (t35_country_code,country_code_no,no_alt,"t35-country-code",
	 "H.323 country code in decimal.Changes will be effective after the reboot of GP10",PRIV_CONF);


/*
 *voip call-signalling-port <port-num>
 */

EOLS (call_sig_port_eol,voip_parameters,CALL_SIG_PORT_NUM);

NUMBER (call_sig_port_num,call_sig_port_eol,no_alt,OBJ(int,1),1,65536,
	"Port number");

NVGENS (call_sig_port_num_nv,call_sig_port_num,voip_parameters,CALL_SIG_PORT_NUM);

NOPREFIX (call_sig_port_num_no,call_sig_port_num_nv,call_sig_port_eol);

KEYWORD (call_sig_port,call_sig_port_num_no,t35_country_code,"call-signalling-port",
	 "Configure port used for call setup and tear down.Changes will be effective after the reboot of GP10",PRIV_CONF);

/*
 * voip gmc-ip-address <ip-address>
 */

EOLS (gmc_ip_addr_eol,voip_parameters,GMC_IP_ADDR);

IPADDR (gmc_ip_addr,gmc_ip_addr_eol,no_alt,OBJ(paddr,1),"IP address of GMC");

NVGENS (gmc_ip_addr_nv,gmc_ip_addr,voip_parameters,GMC_IP_ADDR);

NOPREFIX (gmc_ip_addr_no,gmc_ip_addr_nv,gmc_ip_addr_eol);

KEYWORD (gmc_ip,gmc_ip_addr_no,call_sig_port,"gmc-ip-address",
	 "Configure IP address of GMC.Changes will be effective after the reboot of GP10",PRIV_CONF);


/*
 * voip gmc-port <port-num>
 */

EOLS (gmc_port_eol,voip_parameters,GMC_PORT_NUM);

NUMBER (gmc_port_num,gmc_port_eol,no_alt,OBJ(int,1),1,65536,
	"Port number");

NVGENS (gmc_port_num_nv,gmc_port_num,voip_parameters,GMC_PORT_NUM);

NOPREFIX (gmc_port_num_no,gmc_port_num_nv,gmc_port_eol);


KEYWORD (gmc_port,gmc_port_num_no,gmc_ip,"gmc-port",
	 "Port number on which the GMC is listening for GP10 traffic.Changes will be effective after the reboot of GP10",PRIV_CONF);

/*
 * voip
 */

KEYWORD (cfg_voip_parm,gmc_port,no_alt,"voip",
	 "Configure Voice Over IP Parameters",PRIV_CONF);

#undef ALTERNATE
#define ALTERNATE cfg_voip_parm
