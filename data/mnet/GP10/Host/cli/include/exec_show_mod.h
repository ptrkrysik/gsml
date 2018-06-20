
enum {
  VERSION_OPT,
  DEBUG_OPT,
  CDR_DATA,
  ALARM_STATISTICS,
  VOIP_CALL_STATS,
  VOIP_CALL_CNT,
  VOIP_CALL_SHOW,
  VOIP_ALL_CALL,
};

/*
 * show version
 */

EOLS  (show_ver_eol,show_commands,VERSION_OPT);

KEYWORD (show_ver_opt,show_ver_eol,ALTERNATE,"version",
	 "Show version of GP10 software",PRIV_CONF);

/*
 * show debugging
 */
EOLS (show_debug_opt_eol,show_commands,DEBUG_OPT);

KEYWORD (show_debug_opt,show_debug_opt_eol,show_ver_opt,"debugging",
	 "show debug options",PRIV_CONF | PRIV_HIDDEN);

/*
 * show client cdr data
 */

EOLS (cdr_data_eol,show_commands,CDR_DATA);


KEYWORD (cdr_data,cdr_data_eol,no_alt,"data",
	 "Show details of CDR data",PRIV_CONF);

KEYWORD (cdr_cli_data,cdr_data,no_alt,"client",
	 "Show cdr client details",PRIV_CONF);

KEYWORD (show_cdr,cdr_cli_data,show_debug_opt,"cdr",
	 "Show cdr details",PRIV_CONF | PRIV_HIDDEN);



/*
 * show alarm statistics 
 */

EOLS (alarm_stat_eol,show_commands,ALARM_STATISTICS);


KEYWORD (alarm_stat,alarm_stat_eol,no_alt,"statistics",
	 "Show the statistics of alarm module",PRIV_CONF);

/*
 * show alarm
 */

KEYWORD (show_alarm,alarm_stat,show_cdr,"alarm",
	 "Show alarm module statistics",PRIV_CONF);


/*
 * show voip call-stats
 */

EOLS (voip_call_stats_eol,show_commands,VOIP_CALL_STATS);

KEYWORD (voip_call_stats,voip_call_stats_eol,no_alt,"call-stats",
	 "Show the voip call statistics",PRIV_CONF);


/*
 * show voip call-counters
 */

EOLS (voip_call_cnt_eol,show_commands,VOIP_CALL_CNT);

KEYWORD (voip_call_cnt,voip_call_cnt_eol,voip_call_stats,"call-counters",
	 "Show the voip call counters",PRIV_CONF);

/*
 * show voip call <ccb-value>
 */

/*
 * show voip call <ccb-value>
 */

EOLS (voip_call_eol,show_commands,VOIP_CALL_SHOW);

HEXNUM (voip_call_ccb,voip_call_eol,no_alt,OBJ(int,1),
	"Enter the correct address of ccb");

KEYWORD (voip_call,voip_call_ccb,voip_call_cnt,"calls",
	 "Show call having the call control block",PRIV_CONF);

/*
 * show voip all-calls
 */

EOLS (voip_all_eol,show_commands,VOIP_ALL_CALL);

KEYWORD (voip_all,voip_all_eol,voip_call,"all-calls",
	 "Show all calls currently active",PRIV_CONF);


KEYWORD (exec_show_mod,voip_all,show_alarm,"voip",
	 "Show voice over ip information",PRIV_CONF | PRIV_HIDDEN);

#undef ALTERNATE
#define ALTERNATE exec_show_mod




