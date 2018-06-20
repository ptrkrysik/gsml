
/*
 * enum to identify handover command
 */

enum {
  HANDOVER_N5,
  HANDOVER_N6,
  HANDOVER_P5,
  HANDOVER_P6,
  UPLINK,
  DOWNLINK,
  HANDOVER_AVERAGE,
  HANDOVER_QUALITY,
  HANDOVER_STRENGTH,
  LOG_CONSOLE,
  LOG_HOST,
  LOG_FILE
};

static keyword_options link_options[] = {
  {"uplink","Uplink received power/quality level",UPLINK},
  {"downlink","Downlink received power/quality level",DOWNLINK},
  {NULL,NULL,0}
};

static keyword_options handover_averages[] = {
  {"N5","Total number of averages of signal quality before handover",HANDOVER_N5},
  {"N6","Total number of averages of signal strength before handover",HANDOVER_N6},
  {"P5","Total number of averages of signal quality lower than threshold",HANDOVER_P5},
  {"P6","Total number of averages of signal strength lower than threshold",HANDOVER_P6},
  {NULL,NULL,0}
};


/*
 * logging file
 */
EOLS (log_file_eol,logging_option,LOG_FILE);

KEYWORD (log_file,log_file_eol,no_alt,"file",
	 "Log to the file viperlog.dat",PRIV_CONF);

/*
 * logging <ip-address> [port-number]
 */

EOLS   (log_ip_eol,logging_option,LOG_HOST);

NUMBER (log_port,log_ip_eol,log_ip_eol,OBJ(int,1),1025,65535,
	"Enter the port number");

IPADDR (log_ip_addr,log_port,log_file,OBJ(paddr,1),
	"Ip address of the host to send the log messages");

/*
 * logging console
 */

EOLS (log_cons_eol,logging_option,LOG_CONSOLE);

KEYWORD (log_cons,log_cons_eol,log_ip_addr,"console",
	 "Log to the console",PRIV_CONF);

NVGENNS  (log_nv,log_cons,logging_option);

/*
 * logging
 */

KEYWORD (cfg_logging,log_nv,no_alt,"logging",
	 "Configure the logging option",PRIV_CONF);


/*
 * handover-control average {N5 | N6 | P5 | P6} <0-31>
 */

EOLS (hand_avg_eol,handover_control_commands,HANDOVER_AVERAGE);

NUMBER (hand_avg_val,hand_avg_eol,no_alt,OBJ(int,2),0,31,
	"Enter average value");

NOPREFIX (hand_avg_val_no,hand_avg_val,hand_avg_eol);

KEYWORD_OPTIONS(hand_avg_opt,hand_avg_val_no,no_alt,handover_averages,
		OBJ(int,1),PRIV_CONF,0);

NVGENS (hand_avg_val_nv,hand_avg_opt,handover_control_commands,HANDOVER_AVERAGE);

KEYWORD (hand_ctrl_avg,hand_avg_val_nv,no_alt,"average",
	 "Averages to make handover decision",PRIV_CONF);

/*
 * handover-control {uplink|downlink} quality <0-7>}
 */

EOLS (hand_qty_eol,handover_control_commands,HANDOVER_QUALITY);

NUMBER (hand_qty_val,hand_qty_eol,no_alt,OBJ(int,2),0,7,
	"Enter quality level");

NOPREFIX (hand_qty_val_no,hand_qty_val,hand_qty_eol);

KEYWORD (hand_qty,hand_qty_val_no,no_alt,"quality",
	 "Quality level",PRIV_CONF);

/*
 * handover-control {uplink|downlink} strength <0-63>
 */

EOLS (hand_strgth_eol,handover_control_commands,HANDOVER_STRENGTH);

NUMBER (hand_strgth_val,hand_strgth_eol,no_alt,OBJ(int,2),0,63,
	"Enter power level");

NOPREFIX (hand_strgth_val_no,hand_strgth_val,hand_strgth_eol);

KEYWORD (hand_ctrl_strgth,hand_strgth_val_no,hand_qty,"strength",
	 "Strenth of the power",PRIV_CONF);

/*
 * handover-control {uplink|downlink}
 */

KEYWORD_OPTIONS (hand_ctrl_choice,hand_ctrl_strgth,no_alt,link_options,OBJ(int,1),
		 PRIV_CONF,0);

NVGENS (hand_ctrl_choice_nv,hand_ctrl_choice,handover_control_commands,HANDOVER_STRENGTH);

KEYWORD (hand_ctrl_thr,hand_ctrl_choice_nv,hand_ctrl_avg,"threshold",
	 "Threshold to trigger the GP10 to handover MS",PRIV_CONF);


/*
 * handover-control
 */

KEYWORD (cfg_hand_ctrl,hand_ctrl_thr,cfg_logging,"handover-control",
	 "Configure handover control parameters",PRIV_CONF);


/*
 * enum for power control command identification
 */

enum {
  MAX_POWER_LEVEL,
  UPLINK_TARGET,
  SIGNAL_QUALITY,
  SIGNAL_STRENGTH
};

/*
 * ms-power-control max-tx-power <power-level>
 */


EOLS (max_tr_pwr_eol,mspower_commands,MAX_POWER_LEVEL);

NUMBER (max_tr_pwr_val,max_tr_pwr_eol,no_alt,OBJ(int,1),0,33,
	"Power level");
NVGENS (max_tr_pwr_val_nv,max_tr_pwr_val,mspower_commands,MAX_POWER_LEVEL);

NOPREFIX (max_tr_pwr_val_no,max_tr_pwr_val_nv,max_tr_pwr_eol);

KEYWORD (max_tr_pwr,max_tr_pwr_val_no,no_alt,"max-tx-power",
	 "Maximum transmit power level allowed for each MS",PRIV_CONF);


/*
 * ms-power-control uplink-target <power-level>
 */


EOLS (uplink_tgt_eol,mspower_commands,UPLINK_TARGET);

NUMBER (uplink_tgt_val,uplink_tgt_eol,no_alt,OBJ(int,1),0,63,
	"Power level");

NVGENS (uplink_tgt_val_nv,uplink_tgt_val,mspower_commands,UPLINK_TARGET);

NOPREFIX (uplink_tgt_val_no,uplink_tgt_val_nv,uplink_tgt_eol);

KEYWORD (uplink_tgt,uplink_tgt_val_no,max_tr_pwr,"uplink-target",
	 "Received power level GP10 attempts to maintain from each MS",PRIV_CONF);

/*
 * ms-power-control signal-quality-avg-window <value>
 */


EOLS (sig_qty_eol,mspower_commands,SIGNAL_QUALITY);

NUMBER (sig_qty_val,sig_qty_eol,no_alt,OBJ(int,1),0,31,
	"Period of computation");
NVGENS (sig_qty_val_nv,sig_qty_val,mspower_commands,SIGNAL_QUALITY);

NOPREFIX (sig_qty_val_no,sig_qty_val_nv,sig_qty_eol);

KEYWORD (sig_qty,sig_qty_val_no,uplink_tgt,"signal-quality-avg-window",
	 "Computation of average of uplink received quality",PRIV_CONF);

/*
 * ms-power-control  signal-strength-avg-window <value>
 */


EOLS (sig_strgth_eol,mspower_commands,SIGNAL_STRENGTH);

NUMBER (sig_strgth_val,sig_strgth_eol,no_alt,OBJ(int,1),0,31,
	"Period of computation");
NVGENS (sig_strgth_val_nv,sig_strgth_val,mspower_commands,SIGNAL_STRENGTH);

NOPREFIX (sig_strgth_val_no,sig_strgth_val_nv,sig_strgth_eol);

KEYWORD (sig_strgth,sig_strgth_val_no,sig_qty,"signal-strength-avg-window",
	 "Computation of average of uplink received levels",PRIV_CONF);


/*
 * ms-power-control 
 */

KEYWORD (cfg_pwr_ctrl,sig_strgth,cfg_hand_ctrl,"ms-power-control",
	 "Configure MS power control parameters",PRIV_CONF);

#undef ALTERNATE
#define ALTERNATE cfg_pwr_ctrl
