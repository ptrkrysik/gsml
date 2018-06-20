/*
 * enum for transceiver command identification
 */
enum {
  TRX_RADIO_FREQUENCY,
  TRX_POWER_REDUCTION,
  TRX_CHANNEL_TABLE,
  TRX_ADMIN_STATE,
  TRX_ARFCN_NUMBER
};

/*
 * enum for administrative state options
 */
enum {
  STATE_LOCKED,
  STATE_UNLOCKED,
  STATE_SHUTDOWN
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
 * radio-carrier radio-frequency <value>
 */

EOLS (rad_freq_eol,transceiver_parameters,TRX_RADIO_FREQUENCY);

NUMBER (rad_freq_val,rad_freq_eol,no_alt,OBJ(int,1),0,65535,
	"Enter the ARFCN number");

NVGENS (rad_freq_val_nv,rad_freq_val,transceiver_parameters,TRX_RADIO_FREQUENCY);

NOPREFIX (rad_freq_val_no,rad_freq_val_nv,rad_freq_eol);

KEYWORD (trx_rad_freq,rad_freq_val_no,no_alt,"radio-frequency",
	 "ARFCN assigned to the transceiver",PRIV_CONF);

/*
 * radio-carrier transmit-power-reduction <value>
 */

EOLS (pwr_red_eol,transceiver_parameters,TRX_POWER_REDUCTION);

NUMBER (pwr_red_val,pwr_red_eol,no_alt,OBJ(int,1),0,6,
	"Enter the power value");
NVGENS (pwr_red_val_nv,pwr_red_val,transceiver_parameters,TRX_POWER_REDUCTION);

NOPREFIX (pwr_red_val_no,pwr_red_val_nv,pwr_red_eol);

KEYWORD (trx_pwr_red,pwr_red_val_no,trx_rad_freq,"transmit-power-reduction",
	 "Max transmit power reduced from peak power",PRIV_CONF);

/*
 * radio-carrier
 */
KEYWORD (cfg_radio_carrier,trx_pwr_red,no_alt,"radio-carrier",
	 "Radio configuration parameters",PRIV_CONF);

/*
 * channel-table <time-slot> channel-combination TCH-FULL training-sequence USE-BCC 
 * administration-state {locked | unlocked | shutting-down}
 */

EOLS (trx_admin_state_eol,transceiver_parameters,TRX_CHANNEL_TABLE);

KEYWORD_OPTIONS (admin_state_opt,trx_admin_state_eol,no_alt,admin_state_options,
		 OBJ(int,2),PRIV_CONF,0);

KEYWORD (trx_admin_state,admin_state_opt,no_alt,"administrative-state",
	 "State in which to place channel",PRIV_CONF);

NOPREFIX (trx_admin_state_no,trx_admin_state,trx_admin_state_eol);

KEYWORD (seq_opt,trx_admin_state_no,no_alt,"USE-BCC",
	 "Use BTS color code",PRIV_CONF);

KEYWORD (training_seq,seq_opt,no_alt,"training-sequence",
	 "Training sequence code (TSC) for the channel",PRIV_CONF);

KEYWORD_ID(combn_opt_tch,training_seq,no_alt,OBJ(int,3),TCH_FULL,"TCH-FULL",
	   "Full rate traffic channel",PRIV_CONF);

KEYWORD_ID(combn_opt_bcch,training_seq,no_alt,OBJ(int,3),BCCH_COMBINED,
	   "BCCH-COMBINED", "Common control channel",PRIV_CONF);

IFELSE  (chan_combn_chk,combn_opt_bcch,combn_opt_tch, 
	 (((int) GETOBJ(udb,current1) == 0) && (GETOBJ(int,1) == 0)));

KEYWORD (chan_combn,chan_combn_chk,no_alt,"channel-combination",
	 "Channel combination",PRIV_CONF);

NUMBER (time_slot_trx,chan_combn,no_alt,OBJ(int,1),0,7,
	"Enter the time slot");

NVGENS (time_slot_trx_nv,time_slot_trx,transceiver_parameters,TRX_CHANNEL_TABLE);

KEYWORD (channel_table,time_slot_trx_nv,cfg_radio_carrier,"channel-table",
	 "Channel configurations",PRIV_CONF);

/*
 *administrative-state	{locked | unlocked | shutting-down}
 */

EOLS (admin_state_eol,transceiver_parameters,TRX_ADMIN_STATE);

KEYWORD_OPTIONS (admin_state,admin_state_eol,no_alt,admin_state_options,
		 OBJ(int,1),PRIV_CONF,0);
NVGENS (admin_state_nv,admin_state,transceiver_parameters,TRX_ADMIN_STATE);

NOPREFIX (admin_state_no,admin_state_nv,admin_state_eol);

KEYWORD (cfg_admin_state,admin_state_no,channel_table,"administrative-state",
	 "State in which to place GP10",PRIV_CONF);


/*
 * bts-cell-allocation <number>
 */

EOLS (arfcn_eol,transceiver_parameters,TRX_ARFCN_NUMBER);

NUMBER (arfcn_val,arfcn_eol,no_alt,OBJ(int,1),0,1023,
	"Enter the number");
NVGENS (arfcn_val_nv,arfcn_val,transceiver_parameters,TRX_ARFCN_NUMBER);

NOPREFIX (arfcn_val_no,arfcn_val_nv,arfcn_eol);

KEYWORD (cfg_transceiver,arfcn_val_no,cfg_admin_state,"bts-cell-allocation",
	 "ARFCN allocated and assigned to the cell",PRIV_CONF);



#undef ALTERNATE
#define ALTERNATE cfg_transceiver
