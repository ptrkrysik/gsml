
#define ONE_RETR_RACH   1
#define TWO_RETR_RACH   2
#define FOUR_RETR_RACH  4
#define SEVEN_RETR_RACH 7

/*
 * enum to identify ccch commands
 */
enum {
  CCCH_CELL_BAR,
  CCCH_OFFSET_PENALTY,
  CCCH_OFFSET_TEMP,
  CCCH_CELL_RESELECT,
  CCCH_SPREAD_SLOT,
  CCCH_MULT_FRAME,
  CCCH_ACCESS_GRANT,
  CCCH_MAX_TX_POWER,
  CCCH_MAX_RETR
};

/*
 * retransmission options
 */
static keyword_options ccch_ret_options[] = {
  {"1","Retransmit one time on the random access channel",ONE_RETR_RACH},
  {"2","Retransmit two time on the random access channel",TWO_RETR_RACH},
  {"4","Retransmit four time on the random access channel",FOUR_RETR_RACH},
  {"7","Retransmit seven time on the random access channel",SEVEN_RETR_RACH},
  {NULL,NULL,0}
};

/*
 * cell-bar-qualify enable
 */

EOLS (bar_qfy_eol,ccch_configuration,CCCH_CELL_BAR);

KEYWORD (bar_qfy_enable,bar_qfy_eol,no_alt,"enable",
	 "Enable the option",PRIV_CONF);

NVGENS (bar_qfy_nv,bar_qfy_enable,ccch_configuration,CCCH_CELL_BAR);

KEYWORD (cell_bar_qfy,bar_qfy_nv,no_alt,"cell-bar-qualify",
	 "Enable to consider cell low priority by MS",PRIV_CONF);

/*
 * offset penalty <0-31>
 */

EOLS (ccch_penalty_eol,ccch_configuration,CCCH_OFFSET_PENALTY);

NUMBER (ccch_penalty_val,ccch_penalty_eol,no_alt,OBJ(int,1),0,31,
	"Enter the penalty time");

NVGENS (ccch_penalty_val_nv,ccch_penalty_val,ccch_configuration,CCCH_OFFSET_PENALTY);

NOPREFIX (ccch_penalty_val_no,ccch_penalty_val_nv,ccch_penalty_eol);

KEYWORD (ccch_penalty,ccch_penalty_val_no,no_alt,"penalty",
	 "Penalty time during which the temporary offset is subracted from"
	 "reselection criterion",PRIV_CONF);

/*
 * offset temporary <0-7>
 */

EOLS (ccch_temp_eol,ccch_configuration,CCCH_OFFSET_TEMP);

NUMBER (ccch_temp_val,ccch_temp_eol,no_alt,OBJ(int,1),0,7,
	"Enter the reselect offset value");
NVGENS (ccch_temp_val_nv,ccch_temp_val,ccch_configuration,CCCH_OFFSET_TEMP);

NOPREFIX (ccch_temp_val_no,ccch_temp_val_nv,ccch_temp_eol);

KEYWORD (ccch_temp,ccch_temp_val_no,ccch_penalty,"temporary",
	 "Offset subtracted from C2 reselection criterion",PRIV_CONF);


/*
 * offset cell-reselect <0-63>
 */

EOLS (cell_reselect_eol,ccch_configuration,CCCH_CELL_RESELECT);

NUMBER (cell_reselect_val,cell_reselect_eol,no_alt,OBJ(int,1),0,63,
	"Enter the reselect offset value");
NVGENS (cell_reselect_val_nv,cell_reselect_val,ccch_configuration,CCCH_CELL_RESELECT);

NOPREFIX (cell_reselect_val_no,cell_reselect_val_nv,cell_reselect_eol);

KEYWORD(ccch_cell_reselect,cell_reselect_val_no,ccch_temp,"cell-reselect",
	"Offset added to C2 reselection criterion",PRIV_CONF);

/*
 * offset
 */

KEYWORD (ccch_offset,ccch_cell_reselect,cell_bar_qfy,"offset",
	 "Offset added to the C2 reselection criterion",PRIV_CONF);


/*
 * spread-slot-trans <0-15>
 */


EOLS (spread_slot_eol,ccch_configuration,CCCH_SPREAD_SLOT);

NUMBER (spread_slot_val,spread_slot_eol,no_alt,OBJ(int,1),0,15,
	"Enter the number of slots");

NVGENS (spread_slot_val_nv,spread_slot_val,ccch_configuration,CCCH_SPREAD_SLOT);

NOPREFIX (spread_slot_val_no,spread_slot_val_nv,spread_slot_eol);

KEYWORD (spread_slot,spread_slot_val_no,ccch_offset,"spread-slot-trans",
	 "Max number of random channel slots a MS must wait",PRIV_CONF);

/*
 * multi-frames <2-9>
 */


EOLS (mult_frame_eol,ccch_configuration,CCCH_MULT_FRAME);

NUMBER (mult_frame_val,mult_frame_eol,no_alt,OBJ(int,1),2,9,
	"Enter the number of frames");

NVGENS (mult_frame_val_nv,mult_frame_val,ccch_configuration,CCCH_MULT_FRAME);

NOPREFIX (mult_frame_val_no,mult_frame_val_nv,mult_frame_eol);

KEYWORD (mult_frame,mult_frame_val_no,spread_slot,"multi-frames",
	 "Number of multiframes between two transmissions",PRIV_CONF);

/*
 * access-grant-blocks <0-7>
 */


EOLS (access_grant_eol,ccch_configuration,CCCH_ACCESS_GRANT);

NUMBER (access_grant_val,access_grant_eol,no_alt,OBJ(int,1),0,7,
	"Enter the number of frames");

NVGENS (access_grant_val_nv,access_grant_val,ccch_configuration,CCCH_ACCESS_GRANT);

NOPREFIX (access_grant_val_no,access_grant_val_nv,access_grant_eol);

KEYWORD (ccch_access_grant,access_grant_val_no,mult_frame,"access-grant-blocks",
	 "Number of TDMA frames reserved for access grant channel",PRIV_CONF);

/*
 * ms-max-tx-power <0-33>
 */


EOLS (ccch_tx_pwr_eol,ccch_configuration,CCCH_MAX_TX_POWER);

NUMBER (ccch_tx_pwr_val,ccch_tx_pwr_eol,no_alt,OBJ(int,1),0,33,
	"For radiotype DCS1800 value is 0 to 30,for DCS1900 0 to 33");

NVGENS (ccch_tx_pwr_val_nv,ccch_tx_pwr_val,ccch_configuration,CCCH_MAX_TX_POWER);

NOPREFIX (ccch_tx_pwr_val_no,ccch_tx_pwr_val_nv,ccch_tx_pwr_eol);

KEYWORD (ccch_tx_pwr,ccch_tx_pwr_val_no,ccch_access_grant,"ms-max-tx-power",
	 "Max transmit power level a MS can use on the cell",PRIV_CONF);


/*
 * max-retransmission
 */

EOLS (max_retrx_eol,ccch_configuration,CCCH_MAX_RETR);

KEYWORD_OPTIONS(max_retrx_val,max_retrx_eol,no_alt,ccch_ret_options,OBJ(int,1),
		PRIV_CONF,0);

NVGENS (max_retrx_val_nv,max_retrx_val,ccch_configuration,CCCH_MAX_RETR);

NOPREFIX (max_retrx_val_no,max_retrx_val_nv,max_retrx_eol);

KEYWORD(cfg_ccch_options,max_retrx_val_no,ccch_tx_pwr,"max-retransmission",
	"Max number of retransmission MS can attempt",PRIV_CONF);

#undef ALTERNATE
#define ALTERNATE cfg_ccch_options
