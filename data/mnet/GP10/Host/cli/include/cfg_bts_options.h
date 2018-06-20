/*
 * enums for command identification
 */

enum {
  IMSI_ATTACH,
  CELL_BARRED,
  DOWNLINK_DTX,
  IMSI_FOR_EMER,
  RESTRICTED_EMER_CALL
};

enum {
  UPDATE_INTERVAL,
  ACCESS_CLASS,
  DTX_MODE_OPTIONS,
  ENABLE_OPTIONS
};


enum {
  MAY_USE_DTX,
  SHALL_USE_DTX,
  SHALL_NOT_USE_DTX,
};


enum {
  CFG_CELL_CCCH,
  CFG_CELL_TRX,
  CFG_CELL_BTS
};
/*
 *bts enable options
 */

static keyword_options bts_enable_options[] = {
       { "imsi-attach", "Enable IMSI attach/detach procedure in the cell",IMSI_ATTACH},
       { "cell-barred", "Enable barring of MS camping on a cell",CELL_BARRED},
       {"downlink-dtx","Enable availability of downlink discontinuous transmission",DOWNLINK_DTX},
       {"imsi-for-emergency","Enable requirement of IMSI for MS to make emergency call",
	                                                                      IMSI_FOR_EMER},
       {"restricted-emergency-call","Enable emergency calls restricted to MS from IMSI"
	"class 11 to 15",RESTRICTED_EMER_CALL},
       {NULL,NULL,0}
};


/*
 * dtx mode options
 */

static keyword_options dtx_mode_options[] = {
  { "ms-shall-use-dtx","MS shall use dtx for the uplink",SHALL_USE_DTX},
  { "ms-may-use-dtx","MS may use dtx for the uplink",MAY_USE_DTX},
  { "ms-shall-not-use-dtx","MS shall not use dtx for the uplink",SHALL_NOT_USE_DTX},
  {NULL,NULL,0}
};



/*
 * ccch-config
 */

EOLS    (cfg_ccch_sub_eol,gp10_cell_subconfig_cmds,CFG_CELL_CCCH);

NVGENS  (cfg_ccch_sub_nv,cfg_ccch_sub_eol,gp10_cell_subconfig_cmds,CFG_CELL_CCCH);

KEYWORD (cfg_bts_ccch,cfg_ccch_sub_nv,no_alt,"ccch-config",
	 "Configure bts ccch parameters",PRIV_CONF);

/*
 * bts-basic
 */

EOLS    (cfg_bts_sub_eol,gp10_cell_subconfig_cmds,CFG_CELL_BTS);

NVGENS (cfg_bts_sub_nv,cfg_bts_sub_eol,gp10_cell_subconfig_cmds,CFG_CELL_BTS);

KEYWORD (cfg_bts_sub,cfg_bts_sub_nv,cfg_bts_ccch,"bts-basic",
	 "Configure bts basic parameters",PRIV_CONF);


/*
 * transceiver
 */

EOLS    (cfg_sub_trx_num_eol,gp10_cell_subconfig_cmds,CFG_CELL_TRX);

NUMBER  (cfg_sub_trx_num,cfg_sub_trx_num_eol,no_alt,OBJ(int,1),
	 0,1,"Enter transceiver number");

NVGENS  (cfg_sub_trx_nv,cfg_sub_trx_num,gp10_cell_subconfig_cmds,CFG_CELL_TRX);

KEYWORD (cfg_trx_sub,cfg_sub_trx_nv,cfg_bts_sub,"transceiver",
	 "Configure transceiver parameters",PRIV_CONF);


/*
 *bts-options location-update-interval <0-255>
 */

EOLS (upd_ivl_eol,bts_options,UPDATE_INTERVAL);

NUMBER (upd_ivl_val,upd_ivl_eol,no_alt,OBJ(int,1),0,255,
	"Enter the interval");

NVGENS (upd_ivl_val_nv,upd_ivl_val,bts_options,UPDATE_INTERVAL);

NOPREFIX (upd_ivl_val_no,upd_ivl_val_nv,upd_ivl_eol);

KEYWORD (loc_upd_ivl,upd_ivl_val_no,no_alt,"location-update-interval",
	 "Interval for periodic location updates from the MS in decihours",PRIV_CONF);



/*
 *bts-options ms-access-class-not-allowed <0-15>
 */

EOLS (access_class_eol,bts_options,ACCESS_CLASS);

NUMBER (access_class_val,access_class_eol,no_alt,OBJ(int,1),0,15,
	"Enter the access class not allowed");

KEYWORD (bts_access_class,access_class_val,loc_upd_ivl,"ms-access-class-not-allowed",
	 "MS access classes not allowed to connect to the cell",PRIV_CONF);


/*
 * bts-options dtx-mode {ms-shall-use-dtx |
 *  ms-may-use-dtx | ms-shall-not-use-dtx}
 */


EOLS (dtx_mode_eol,bts_options,DTX_MODE_OPTIONS);

KEYWORD_OPTIONS (dtx_mode_opt,dtx_mode_eol,no_alt,dtx_mode_options,OBJ(int,1),
		 PRIV_CONF,0);

NOPREFIX (dtx_mode_opt_no,dtx_mode_opt,dtx_mode_eol);

KEYWORD(dtx_mode,dtx_mode_opt_no,bts_access_class,"dtx-mode",
		"Downlink discontinuous transmission mode options",PRIV_CONF);

/*
 * bts-options {imsi-attach | cell-barred | downlink-dtx | imsi-for-emergency |
 * restricted-emergency-call} enable
 */


EOLS (bts_en_options_eol,bts_options,ENABLE_OPTIONS);

KEYWORD (bts_opt_enable,bts_en_options_eol,no_alt,"enable",
	 "Enable the option",PRIV_CONF);

KEYWORD_OPTIONS(bts_enable_options,bts_opt_enable,dtx_mode,bts_enable_options,OBJ(int,1),
		PRIV_CONF,0);

/*
 * bts-options
 */

KEYWORD (cfg_bts_opt,bts_enable_options,cfg_trx_sub,"bts-options",
	 "Configure bts options",PRIV_CONF);

#undef ALTERNATE
#define ALTERNATE cfg_bts_opt
