
#define CELL_SELECTED 1
#define CELL_SYNCHRONISED 2

#define BSIC_NCC 0
#define BSIC_BCC 1


/*
 * enum to identify adjacent cell commands
 */

enum {
  ADJ_CELL_RES_BCCH,
  ADJ_CELL_RES,
  ADJ_CELL_CC_VALUE,
  ADJ_CELL_GI_VALUE,
  ADJ_CELL_BCCH,
  ADJ_CELL_SELECTION
};

/*
 * codes to identify a cell
 */
enum {
  GI_MCC,
  GI_MNC,
  GI_LAC,
  GI_CI
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
 * selection options
 */
static keyword_options selection_options[] = {
       { "selected", "Adjacent cell is  active", CELL_SELECTED},
       { "synchronised", "Origin cell is synchronised with the adjacent cell",CELL_SYNCHRONISED},
       {NULL,NULL,0}
};

/*
 * global identity options
 */
static keyword_options gi_options[] = {
    /*   { "mcc", "Mobile country code",GI_MCC},
       { "mnc", "Mobile network code",GI_MNC},*/
       { "lac", "Location area code", GI_LAC},
       { "ci", "Cell identifier", GI_CI},
       {NULL,NULL,0}
};

/*
 * reselection bcch-frequency <0-1023>
 */


EOLS (bcch_freq_val_eol,adj_cell_parameters,ADJ_CELL_RES_BCCH);

NUMBER (bcch_freq_value,bcch_freq_val_eol,no_alt,OBJ(int,1),0,1023,
	"Frequncy value");
NVGENS (bcch_freq_value_nv,bcch_freq_value,adj_cell_parameters,ADJ_CELL_RES_BCCH);

NOPREFIX (bcch_freq_value_no,bcch_freq_value_nv,bcch_freq_val_eol);

KEYWORD (res_bcch_freq,bcch_freq_value_no,no_alt,"bcch-frequency",
	 "The ARFCN of the BCCH for the adjacent cell",PRIV_CONF);


/*
 * reselection selected
 */

EOLS (res_index_eol,adj_cell_parameters,ADJ_CELL_RES);

KEYWORD (res_index_sel,res_index_eol,res_bcch_freq,"selected",
	 "Select the adjacent cell to identify an active adjacent cell",PRIV_CONF);

KEYWORD (cfg_reselection,res_index_sel,no_alt,"reselection",
	 "Parameters to identify the adjacent cells used for cell reselection",PRIV_CONF);

/*
 * handover bsic {NCC | BCC} <0-7>
 */

EOLS (cc_eol,adj_cell_parameters,ADJ_CELL_CC_VALUE);

NUMBER (cc_value,cc_eol,no_alt,OBJ(int,2),0,7,
	"Value of color code");

NOPREFIX (cc_value_no,cc_value,cc_eol);

KEYWORD_OPTIONS (cfg_cc,cc_value_no,no_alt,bsic_options,
	 OBJ(int,1),PRIV_CONF,0);

NVGENS (cc_value_nv,cfg_cc,adj_cell_parameters,ADJ_CELL_CC_VALUE);

KEYWORD (cell_bsic,cc_value_nv,no_alt,"bsic",
	 "Configure base station identity code elements",PRIV_CONF);


/*
 * handover cell-global-identity {mcc | mnc | lac | ci}<0-65535>
 */

EOLS (gi_cc_eol,adj_cell_parameters,ADJ_CELL_GI_VALUE);

NUMBER (gi_cc_value,gi_cc_eol,no_alt,OBJ(int,2),0,65535,
	"Value of  code");

NVGENS(gi_cc_value_nv,gi_cc_value,adj_cell_parameters,ADJ_CELL_GI_VALUE);

NOPREFIX(cfg_gi_cc_no,gi_cc_value_nv,gi_cc_eol);

		/****mnc*****/
NUMBER(cfg_mnc_value,gi_cc_eol,no_alt,OBJ(int,2),1,99,"Mobile network code");
NVGENS(cfg_mnc_value_nv,cfg_mnc_value,adj_cell_parameters,ADJ_CELL_GI_VALUE);
NOPREFIX(cfg_mnc_no,cfg_mnc_value_nv,gi_cc_eol);
KEYWORD(cell_gi_mnc,cfg_mnc_no,no_alt,
		"mnc","Mobile network code",PRIV_CONF);
	   /***mcc***/
NUMBER(cfg_mcc_value,gi_cc_eol,no_alt,OBJ(int,2),1,999,"Mobile country code");
NVGENS(cfg_mcc_value_nv,cfg_mcc_value,adj_cell_parameters,ADJ_CELL_GI_VALUE);
NOPREFIX(cfg_mcc_no,cfg_mcc_value_nv,gi_cc_eol);
KEYWORD(cell_gi_mcc,cfg_mcc_no,cell_gi_mnc,
		"mcc","Mobile country code",PRIV_CONF);
		/*************/

KEYWORD_OPTIONS (cfg_gi_cc,cfg_gi_cc_no,cell_gi_mcc,gi_options,
	 OBJ(int,1),PRIV_CONF,0);

KEYWORD (cell_gi,cfg_gi_cc,cell_bsic,"cell-global-identity",
	 "Configure parameters to identify GP10s adjacent cell",PRIV_CONF);

/*
 * handover  bcch-frequency  <0-1023> handover-margin <0-24>
 * max-tx-power <0-33> min-rx-level <0-63>
 */


EOLS (bcch_freq_eol,adj_cell_parameters,ADJ_CELL_BCCH);

NUMBER (rx_level_value,bcch_freq_eol,no_alt,OBJ(int,4),0,63,
	"Minimum signal level");

KEYWORD (rx_level,rx_level_value,no_alt,"min-rx-level",
	 "Min received signal level from adjacent cell to consider handover for MS",PRIV_CONF);


NUMBER (tx_power_value,rx_level,no_alt,OBJ(int,3),0,33,
	"For PCS 1800 radio max is 30 and for PCS 1900 radio max is 33");

KEYWORD (tx_power,tx_power_value,no_alt,"max-tx-power",
	 "Max transmit power a MS may use in adjacent cell",PRIV_CONF);

NUMBER (margin_value,tx_power,no_alt,OBJ(int,2),0,24,
	"Enter the handover margin value");

KEYWORD (margin,margin_value,no_alt,"handover-margin",
	 "Threshold value used to prevent repetitive handovers between adjacent cells",PRIV_CONF);


NUMBER (freq_value,margin,no_alt,OBJ(int,1),0,1023,
	"Enter the Absolute Radio Frequency Channel Number value");

NVGENS(freq_value_nv,freq_value,adj_cell_parameters,ADJ_CELL_BCCH);

KEYWORD (bcch_freq,freq_value_nv,cell_gi,"bcch-frequency",
	 "The ARFCN of the BCCH for the adjacent cell",PRIV_CONF);


/*
 * handover {selected | synchronised}
 */


EOLS (index_eol,adj_cell_parameters,ADJ_CELL_SELECTION);


KEYWORD_OPTIONS(index_sel,index_eol,bcch_freq,selection_options,OBJ(int,1),
		PRIV_CONF,0);
NVGENS (index_sel_nv,index_sel,adj_cell_parameters,ADJ_CELL_SELECTION);

KEYWORD(cfg_handover,index_sel_nv,cfg_reselection,"handover",
	"Configure parameters to identify cells around the cell created by GP10",PRIV_CONF);


#undef ALTERNATE
#define ALTERNATE cfg_handover
