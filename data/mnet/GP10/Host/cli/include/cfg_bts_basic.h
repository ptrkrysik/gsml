
#define BSIC_NCC 0
#define BSIC_BCC 1

/*
 * enums for command identification
 */

enum {
  BTS_RECV_LEVEL,
  BTS_LINK_TIMEOUT,
  BTS_PER_NCC_ALL,
  BTS_PER_NCC,
  BTS_PHY_VALUE,
  BTS_RES_VALUE,
  BTS_GI_VALUE,
  BTS_CC_VALUE,
  BTS_ADMIN_STATE
};


/*
 * enums for options
 */

enum {
  STATE_LOCKED,
  STATE_UNLOCKED,
  STATE_SHUTDOWN
};

enum {
  GI_MCC,
  GI_MNC,
  GI_LAC,
  GI_CI
};

/*
 * administrative state options
 */

static keyword_options admin_state_options[] = {
       { "locked", "Cannot be accessed by mobile stations", STATE_LOCKED},
       { "unlocked", "Can be accessed by mobile stations",STATE_UNLOCKED},
       { "shutting-down","GP10 is in the process of becoming locked",STATE_SHUTDOWN},
       {NULL,NULL,0}
};

/*
 * bsic options
 */
static keyword_options bsic_options[] = {
       { "ncc", "Network color code for PLMN",BSIC_NCC},
       { "bcc", "Base station color code",BSIC_BCC},
       {NULL,NULL,0}
};

/*
 * options for global identity
 */

static keyword_options gi_options[] = {
    /*   { "mcc", "Mobile country code",GI_MCC},
       { "mnc", "Mobile network code",GI_MNC},*/
       { "lac", "Location area code", GI_LAC},
       { "ci", "Cell identifier", GI_CI},
       {NULL,NULL,0}
};


/*
 *receive-level-access <0-63>
 */

EOLS (recv_level_eol,bts_basic_parameters,BTS_RECV_LEVEL);

NUMBER (recv_level,recv_level_eol,no_alt,OBJ(int,1),0,63,
	"Enter access level value");

NVGENS (recv_level_nv,recv_level,bts_basic_parameters,BTS_RECV_LEVEL);

NOPREFIX (recv_level_no,recv_level_nv,recv_level_eol);

KEYWORD (cfg_recv_level_access,recv_level_no,no_alt,"receive-level-access",
	 "Min receive level at which a MS can access the system",PRIV_CONF);

/*
 * radio-link-timeout <0-15>
 */

EOLS (timeout_value_eol,bts_basic_parameters,BTS_LINK_TIMEOUT);

NUMBER (timeout_value,timeout_value_eol,no_alt,OBJ(int,1),0,15,
	"Enter timeout value");

NVGENS (timeout_value_nv,timeout_value,bts_basic_parameters,BTS_LINK_TIMEOUT);

NOPREFIX (timeout_value_no,timeout_value_nv,timeout_value_eol);

KEYWORD (cfg_link_timeout,timeout_value_no,cfg_recv_level_access,"radio-link-timeout",
	 "Max value of radio link counter need to detect radio link failure",PRIV_CONF);


/*
 * plmn-permitted-ncc {all | 0-7}
 */ 

EOLS (ncc_all_eol,bts_basic_parameters,BTS_PER_NCC_ALL);

KEYWORD (ncc_all,ncc_all_eol,no_alt,"all",
	 "Select all the ncc to use",PRIV_CONF);


EOLS (ncc_eol,bts_basic_parameters,BTS_PER_NCC);

NUMBER (per_ncc_value,ncc_eol,ncc_all,OBJ(int,1),0,7,
	"Enter the ncc MS will use");

NVGENS (per_ncc_value_nv,per_ncc_value,bts_basic_parameters,BTS_PER_NCC);

KEYWORD (cfg_per_ncc,per_ncc_value_nv,cfg_link_timeout,"plmn-permitted-ncc",
	 "Network color codes used by the network",PRIV_CONF);

/*
 * physical-repetition <0-7>
 */

EOLS (phy_eol,bts_basic_parameters,BTS_PHY_VALUE);

NUMBER (phy_value,phy_eol,no_alt,OBJ(int,1),0,7,
	"Number of repetitions");
NVGENS (phy_value_nv,phy_value,bts_basic_parameters,BTS_PHY_VALUE);

NOPREFIX (phy_value_no,phy_value_nv,phy_eol);

KEYWORD (cfg_phy_rep,phy_value_no,cfg_per_ncc,"physical-repetition",
	 "Max number of times GP10 should send the physical information to MSs",PRIV_CONF);

/*
 * cell-reselect-hysteresis <0-7>
 */

EOLS (res_eol,bts_basic_parameters,BTS_RES_VALUE);

NUMBER (res_value,res_eol,no_alt,OBJ(int,1),0,7,
	"Value of radio frequency power level");

NVGENS (res_value_nv,res_value,bts_basic_parameters,BTS_RES_VALUE);

NOPREFIX (res_value_no,res_value_nv,res_eol);

KEYWORD (cfg_cell_res,res_value_no,cfg_phy_rep,"cell-reselect-hysteresis",
	 "Power level hysteresis required for cell reselction",PRIV_CONF);



/*
 * cell-global-identity {mcc | mnc | lac | ci} <0-65535>
 */

EOLS (gi_cc_eol,bts_basic_parameters,BTS_GI_VALUE);

NUMBER (gi_cc_value,gi_cc_eol,no_alt,OBJ(int,2),0,65535,
	"Value of  code");

NOPREFIX (gi_cc_value_no,gi_cc_value,gi_cc_eol);

		/****mnc*****/
NUMBER(cfg_mnc_value,gi_cc_eol,no_alt,OBJ(int,2),1,99,"Mobile network code");
NOPREFIX(cfg_mnc_no,cfg_mnc_value,gi_cc_eol);
KEYWORD(cell_gi_mnc,cfg_mnc_no,no_alt,
		"mnc","Mobile network code",PRIV_CONF);
	   /***mcc***/
NUMBER(cfg_mcc_value,gi_cc_eol,no_alt,OBJ(int,2),1,999,"Mobile country code");
NOPREFIX(cfg_mcc_no,cfg_mcc_value,gi_cc_eol);
KEYWORD(cell_gi_mcc,cfg_mcc_no,cell_gi_mnc,
		"mcc","Mobile country code",PRIV_CONF);
	  /********/
KEYWORD_OPTIONS (cfg_gi_cc,gi_cc_value_no,cell_gi_mcc,gi_options,
	 OBJ(int,1),PRIV_CONF,0);

NVGENS (gi_cc_value_nv,cfg_gi_cc,bts_basic_parameters,BTS_GI_VALUE);

KEYWORD (cfg_gi,gi_cc_value_nv,cfg_cell_res,"cell-global-identity",
	 "Configure parameters to identify GP10s cell",PRIV_CONF);


/*
 * bsic {ncc | bcc} <value> 
 */

EOLS (cc_eol,bts_basic_parameters,BTS_CC_VALUE);

NUMBER (cc_value,cc_eol,no_alt,OBJ(int,2),0,7,
	"Value of color code");

NOPREFIX (cc_value_no,cc_value,cc_eol);

KEYWORD_OPTIONS (cfg_cc,cc_value_no,no_alt,bsic_options,
	 OBJ(int,1),PRIV_CONF,0);

NVGENS (cc_value_nv,cfg_cc,bts_basic_parameters,BTS_CC_VALUE);

KEYWORD (cfg_bsic,cc_value_nv,cfg_gi,"bsic",
	 "Configure base station identity code elements",PRIV_CONF);


/*
 * administrative-state [{locked | unlocked | shutdown}]
 */

EOLS (admin_state_eol,bts_basic_parameters,BTS_ADMIN_STATE);

KEYWORD_OPTIONS (admin_state,admin_state_eol,no_alt,admin_state_options,
		 OBJ(int,1),PRIV_CONF,0);
NVGENS (admin_state_nv,admin_state,bts_basic_parameters,BTS_ADMIN_STATE);

NOPREFIX (admin_state_no,admin_state_nv,admin_state_eol);

KEYWORD (cfg_bts_basic,admin_state_no,cfg_bsic,"administrative-state",
	 "State in which to place GP10",PRIV_CONF);

#undef ALTERNATE
#define ALTERNATE cfg_bts_basic
