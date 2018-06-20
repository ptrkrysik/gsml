/*
 * enum for command identification
 */

enum {
  CCM_TIMER,
  RMM_TIMER,
  FACCH_TIMER,
  SACCH_TIMER ,
  SDCCH_TIMER 
};

/*
 * enum for various timers
 */
enum {
  SDCCH_SAPI0_TIMER,
  SDCCH_SAPI3_TIMER,
  SACCH_SDCCH_TIMER,
  TCH_SAPI0_TIMER,
  SDCCH_SDCCH_TIMER,
  RMM_IMM_ASSIGN,
  RMM_HANDOVER,
  RMM_PHY_INFO,
  RMM_ASSIGN,
  RMM_LOW_LAYER,
  RMM_CH_ACTIVATION,
  RMM_PAGING_MSG,
  CCM_ALERT,
  CCM_SETUP_SENT,
  CCM_CALL_CONFIRM,
  CCM_CONNECT_SENT,
  CCM_DISCONNECT_SENT,
  CCM_RELEASE_SENT
};

/*
 * layer2 sdcch timers
 */

static keyword_options sdcch_timers [] = {
       { "sapi-0-timer", "Sdcch service access point indicator 0 layer 2 timer",SDCCH_SAPI0_TIMER},
       { "sapi-3-timer","Sdcch service access point indicator 3 layer 2 timer",SDCCH_SAPI3_TIMER},
       { "sdcch-tch-timer","Stand-alone dedicated control channel timer",SDCCH_SDCCH_TIMER},
       {NULL,NULL,0}
};

/*
 * layer 2 sacch timers
 */

static keyword_options sacch_timers [] = {
       { "tch-0-timer", "Traffic channel service access point indicator 0 timer",TCH_SAPI0_TIMER},
       { "sdcch-timer","Stand-alone dedicated control channel timer",SACCH_SDCCH_TIMER},
       {NULL,NULL,0}
};



/*
 * layer 3 rmm timers
 */

static keyword_options layer3_rmm_timers [] = {
  {"immediate-assignment","Timer for immediate assignment commands",RMM_IMM_ASSIGN},
  { "handover-command","Timer for handover commands",RMM_HANDOVER},
  { "physical-information","Timer for physical information commands",RMM_PHY_INFO},
  {"assignment-command","Timer for the assignment commands",RMM_ASSIGN},
  {"lower-layer-failure","Timer for lower layer failures",RMM_LOW_LAYER},
  {"channel-activation","Timer for channel activation delays",RMM_CH_ACTIVATION},
  {"paging-messages","Timer for paging messages",RMM_PAGING_MSG},
  {NULL,NULL,0}
};

/*
 * layer 3 ccm timers
 */
static keyword_options layer3_ccm_timers []= {
  {"alerting-received","Alerting received timer",CCM_ALERT},
  {"setup-sent","Setup sent timer",CCM_SETUP_SENT},
  {"call-confirmed","Call confirmed received timer",CCM_CALL_CONFIRM},
  {"connect-sent","Connect sent timer",CCM_CONNECT_SENT},
  {"disconnect-sent","Disconnect sent timer",CCM_DISCONNECT_SENT},
  {"release-sent","Release sent timer",CCM_RELEASE_SENT},
  {NULL,NULL,0}
};

/*
 *laye3-ccm {alerting-received|  setup-sent | call-confirmed |connect-sent |
 *disconnect-sent |release-sent} <timer-value>
 */

EOLS (ccm_timer_eol,timer_configuration,CCM_TIMER);

NUMBER (ccm_timer_val,ccm_timer_eol,no_alt,OBJ(int,2),0,65535,"Enter timer value in 10ms units");

NOPREFIX (ccm_timer_val_no,ccm_timer_val,ccm_timer_eol);

KEYWORD_OPTIONS(ccm_timers,ccm_timer_val_no,no_alt,layer3_ccm_timers,OBJ(int,1),PRIV_CONF,0);

NVGENS (ccm_timers_nv,ccm_timers,timer_configuration,CCM_TIMER);

KEYWORD (layer3_ccm_timer,ccm_timers_nv,no_alt,"layer3-ccm",
	 "Configure layer 3 ccm timers",PRIV_CONF);


/*
 * layer3-rrm { immediate-assignment | handover-command | physical-information |
 * assignment-command | lower-layer-failiure | channel-activation | paging-messages } 
 * <timer-value>
 */

EOLS (rmm_timer_eol,timer_configuration,RMM_TIMER);

NUMBER (rmm_timer_val,rmm_timer_eol,no_alt,OBJ(int,2),0,65535,"Enter timer value in 10ms units");

NOPREFIX (rmm_timer_val_no,rmm_timer_val,rmm_timer_eol);

KEYWORD_OPTIONS(rmm_timers,rmm_timer_val_no,no_alt,layer3_rmm_timers,OBJ(int,1),PRIV_CONF,0);

NVGENS (rmm_timers_nv,rmm_timers,timer_configuration,RMM_TIMER);

KEYWORD (layer3_rmm_timer,rmm_timers_nv,layer3_ccm_timer,"layer3-rrm",
	 "Configure layer 3 rrm timers",PRIV_CONF);


/*
 *layer2-lapdm facch <timer-value>
 */

EOLS (facch_timer_eol,timer_configuration,FACCH_TIMER);

NUMBER (facch_timer_val,facch_timer_eol,no_alt,OBJ(int,1),0,65535,"Enter timer value in 5ms units");

NOPREFIX (facch_timer_val_no,facch_timer_val,facch_timer_eol);

NVGENS (facch_timer_val_nv,facch_timer_val_no,timer_configuration,FACCH_TIMER);

KEYWORD (cfg_facch_timer,facch_timer_val_nv,no_alt,"facch",
	 "Configure full rate traffic channel timers",PRIV_CONF);



/*
 *layer2-lapdm sacch { tch-sapi-0-timer | tch-sapi-3-timer | sdcch-timer } <timer-value>
 */


EOLS (sacch_timer_eol,timer_configuration,SACCH_TIMER);

NUMBER (sacch_timer_val,sacch_timer_eol,no_alt,OBJ(int,2),0,65535,"Enter timer value in 5ms units");

NOPREFIX (sacch_timer_val_no,sacch_timer_val,sacch_timer_eol);

KEYWORD_OPTIONS(sacch_timer,sacch_timer_val_no,no_alt,sacch_timers,OBJ(int,1),PRIV_CONF,0);

NVGENS (sacch_timer_nv,sacch_timer,timer_configuration,SACCH_TIMER);

KEYWORD (cfg_sacch_timer,sacch_timer_nv,cfg_facch_timer,"sacch",
	 "Configure saach timers",PRIV_CONF);


/*
 *layer2-lapdm sdcch { sapi-0-timer | sapi-3-timer | sdcch-timer}
 *<timer-value>
 */

EOLS (sdcch_timer_eol,timer_configuration,SDCCH_TIMER);

NUMBER (sdcch_timer_val,sdcch_timer_eol,no_alt,OBJ(int,2),0,65535,"Enter timer value in 5ms units");

NOPREFIX (sdcch_timer_val_no,sdcch_timer_val,sdcch_timer_eol);

KEYWORD_OPTIONS(sdcch_timer,sdcch_timer_val_no,no_alt,sdcch_timers,OBJ(int,1),PRIV_CONF,0);

NVGENS (sdcch_timer_nv,sdcch_timer,timer_configuration,SDCCH_TIMER);

KEYWORD (cfg_sdcch_timer,sdcch_timer_nv,cfg_sacch_timer,"sdcch",
	 "Configure stand-alone dedicated control channel timer",PRIV_CONF);


/*
 *layer2-lapdm
 */

KEYWORD (cfg_proto_options,cfg_sdcch_timer,layer3_rmm_timer,"layer2-lapdm",
	 "Configure layer 2 LAPDm timers",PRIV_CONF);

#undef ALTERNATE
#define ALTERNATE cfg_proto_options
