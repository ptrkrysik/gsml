#define DBG_LOG_CONTROL_TRACE_MASK	            0x0008
#define DBG_LOG_CALL_TRACE_MASK		            0x0010
#define DBG_LOG_INFORMATIONAL_MASK	            0x0020
#define DBG_LOG_WARNINGS_MASK	                    0x0040
#define DBG_LOG_ERRORS_MASK		            0x0080
#define DBG_LOG_TIMESTAMP_MASK				0x0100
#define DBG_LOG_ALL_TRACE_MASK                      0x00f8


/*
 * Inverse masks
 */
#define DBG_LOG_CONTROL_TRACE_INVERSE_MASK	    0xfff7
#define DBG_LOG_CALL_TRACE_INVERSE_MASK		    0xffef
#define DBG_LOG_INFORMATIONAL_INVERSE_MASK	    0xffdf
#define DBG_LOG_WARNINGS_INVERSE_MASK		    0xffbf
#define DBG_LOG_ERRORS_INVERSE_MASK		    0xff7f
#define DBG_LOG_ALL_TRACE_INVERSE_MASK              0xff07


/*
 * Enum for Module Identification. 
 * Imported from gp10/Host/Common_to_host/Logging/VcModules.h
 */

enum {

    DBG_CC_LAYER                = 0,     /* Value for DBG_CC_LAYER are from 0 to 24             */
    DBG_CC_SESSION_LAYER        = 1,    /* CC Session sub layer   */
    DBG_CC_CALLLEG_LAYER        = 2,    /* CC Call Leg sub layer  */
    DBG_CC_CISS_LAYER           = 3,    /* CC CISS sub layer      */
    DBG_CC_HALFCALL_LAYER       = 4,    /* CC Half Call sub layer */
    DBG_CC_HANDOVER_LAYER       = 5,    /* CC Handover sub layer  */
    DBG_CC_SMS_LAYER            = 6,    /* CM SMS sublayer        */


    DBG_CSU_LAYER               = 25,    /* Value for CSU_LAYER are from 25 to 49         */
    DBG_CSU_LAYER_SPEECH        = 26,
    DBG_CSU_LAYER_DEBUG         = 27,

    DBG_DSP_LAYER               = 50,    /* Value for DSP_LAYER are from 50 to 74         */

    DBG_H323_LAYER              = 75,    /* Value for H323_LAYER are from 75 to 99         */
    DBG_H323_PROTOCOL_LAYER     = 76,
    DBG_H323_RTP_LAYER          = 77,
    DBG_VBLINK_LAYER            = 80,
    
    DBG_LOGGING_LAYER           = 99,   /* Logging layer just needs one */

    DBG_JCC_LAYER               = 100,    /* Value for JCC_LAYER are from 100 to 124         */

    DBG_CDR_LAYER               = 110,    
    
    DBG_L1PROXY_LAYER           = 125,    /* Value for L1PROXY_LAYER are from 125 to 149     */
    
    DBG_LAPDM_LAYER             = 150,    /* Value for LAPDM_LAYER are from 150 to 174     */
    
    DBG_MM_LAYER                = 175,    /* Value for DBG_MM_LAYER are from 175 to 199         */
    DBG_MM_MSG_LAYER            = 176,
    DBG_MM_TABLE_LAYER          = 177,
    DBG_MM_GENERAL_LAYER        = 178,
    DBG_MM_DETAIL_LAYER         = 179,
    
    DBG_RIL3_LAYER              = 200,    /* Value for RIL3_LAYER are from 200 to 224     */
    
    DBG_RM_LAYER                = 225,    /* Value for RM_LAYER are from 225 to 249         */
    
    DBG_LUDB_LAYER              = 250,    /* Value for LUDB_LAYER are from 250 to 274     */
    DBG_LUDB_MSG_LAYER          = 251,
    DBG_LUDB_TABLE_LAYER        = 252,
    DBG_LUDB_GENERAL_LAYER      = 253,
    DBG_LUDB_DETAIL_LAYER       = 254,
    
    DBG_MAINT_COMM_HDLR         = 275,    /* Value for MAINT_COMM_HDLR are from 275 to 299     */
    DBG_DIAG_TEST               = 276,
    DBG_DSP_DIAGS               = 277,

    DBG_PM_LAYER                = 299,  /* PM_LAYER needs only one module */
    DBG_ALARM_LAYER             = 300,    /* Alarm layer 300 to 310 */
    DBG_L3_MS_TRACE             = 325,

 
    DBG_RLC_MAC                 = 350,    /* RLC_MAC LAYER */
    DBG_RLC_MAC_MSG             = 351,
    DBG_RLC_MAC_MSG_DETAIL      = 352,
#if 0
    /*
     * The following were commented because
     * they have been added later into the development
     * and not present in GP1_5_13_BLD
     */
    DBG_RLC_MAC_INTF            = 353,
    DBG_RLC_MAC_RLC_LINK        = 354,

	DBG_GRR_LAYER				= 400,
	DBG_GRR_LAYER_INIT			= 401,
	DBG_GRR_LAYER_OAM			= 402,
	DBG_GRR_LAYER_DSP			= 403,
	DBG_GRR_LAYER_ALARM			= 404,
	DBG_GRR_LAYER_INTF			= 405,
	DBG_GRR_LAYER_PROC			= 406,
    

	DBG_BSSGP_LAYER				= 425,
	DBG_BSSGP_ENCODER_LAYER		= 426,
	DBG_BSSGP_DECODER_LAYER		= 427,
	DBG_BSSGP_NET_LAYER			= 428,
	DBG_BSSGP_UNITDATA_LAYER	= 429,
#endif
        DBG_SSH_MODULE                  = 460,

    MAX_VIPERCELL_MODULES   = 500,
	GP10_DEBUG_ALL			=1000


};

/*
 * gp10 debug options
 */

static keyword_options gp10_debug [] = {
  {"cc_session_layer","Debug cc sesseion sub layer",DBG_CC_SESSION_LAYER},
  {"cc_callleg_layer","Debug cc call leg sub layer",DBG_CC_CALLLEG_LAYER},
  {"cc_ciss_layer","Debug cc CISS sub layer",DBG_CC_CISS_LAYER},
  {"cc_halfcall_layer","Debug cc Half call sub layer",DBG_CC_HALFCALL_LAYER },
  {"cc_handover_layer","Debug cc handover sub layer",DBG_CC_HANDOVER_LAYER},
  {"cc_sms_layer","Debug cc SMS sub layer",DBG_CC_SMS_LAYER},
  {"csu_layer_speech","Debug CSU layer speech",DBG_CSU_LAYER_SPEECH},
  {"csu_layer_debug","Debug CSU layer",DBG_CSU_LAYER_DEBUG},
  {"h323_protocol_layer","Debug H323 protocol layer",DBG_H323_PROTOCOL_LAYER},
  {"h323_rtp_layer","Debug H323 rtp layer",DBG_H323_RTP_LAYER},
  {"vblink_layer","Debug vblink layer",DBG_VBLINK_LAYER},
  {"mm_msg_layer","Debug MM message layer",DBG_MM_MSG_LAYER},
  {"mm_table_layer","Debug MM table layer",DBG_MM_TABLE_LAYER},
  {"mm_general_layer","Debug MM general layer",DBG_MM_GENERAL_LAYER},
  {"mm_detail_layer","Debug MM detail layer",DBG_MM_DETAIL_LAYER},
  {"rlc_mac_msg","Debug RLC MAC messages",DBG_RLC_MAC_MSG},
  {"rlc_mac_msg_detail","Debug RLC MAC message detail",DBG_RLC_MAC_MSG_DETAIL},
#if 0
  {"rlc_mac_intf","Debug RLC MAC interface",DBG_RLC_MAC_INTF},
  {"rlc_mac_rlc_link","Debug RLC MAC link",DBG_RLC_MAC_RLC_LINK},
  {"grr_layer_init","Debug GRR initialisation layer",DBG_GRR_LAYER_INIT},
  {"grr_layer_oam","Debug GRR OAM layer",DBG_GRR_LAYER_OAM},
  {"grr_layer_dsp","Debug GRR DSP layer",DBG_GRR_LAYER_DSP},
  {"grr_layer_alarm","Debug GRR alarm layer",DBG_GRR_LAYER_ALARM},
  {"grr_layer_intf","Debug GRR interface layer",DBG_GRR_LAYER_INTF},
  {"grr_layer_proc","Debug GRR process",DBG_GRR_LAYER_PROC},
#endif
  {"ludb_msg_layer","Debug LUDB messages layer",DBG_LUDB_MSG_LAYER},
  {"ludb_table_layer","Debug LUDB table layer",DBG_LUDB_TABLE_LAYER  },
  {"ludb_general_layer","Debug LUDB general layer",DBG_LUDB_GENERAL_LAYER },
  {"ludb_detail_layer","Debug LUDB detail layer",DBG_LUDB_DETAIL_LAYER},
#if 0
  {"bssgp_encoder_layer","Debug BSSGP encoder layer",DBG_BSSGP_ENCODER_LAYER},
  {"bssgp_decoder_layer","Debug BSSGP decoder layer",DBG_BSSGP_DECODER_LAYER},
  {"bssgp_net_layer","Debug BSSGP network layer",DBG_BSSGP_NET_LAYER},
  {"bssgp_unitdata_layer","Debug BSSGP unitdata layer",DBG_BSSGP_UNITDATA_LAYER},
#endif
  {"logging_layer","Debug loggin layer",DBG_LOGGING_LAYER},
  {"jcc_layer","Debug jcc layer",DBG_JCC_LAYER },
  {"cdr_layer","Debug cdr layer",DBG_CDR_LAYER },
  {"l1proxy_layer","Debug L1 Proxy layer",DBG_L1PROXY_LAYER},
  {"lapdm_layer","Debug lapdm layer",DBG_LAPDM_LAYER},
  {"ril3_layer","Debug RIL3 layer",DBG_RIL3_LAYER},
  {"rm_layer","Debug RM Layer",DBG_RM_LAYER}, 
  {"maint_cmd_handler","Debug maintenance command handler",DBG_MAINT_COMM_HDLR}, 
  {"diagnostic_test","Debug diagnostic tests",DBG_DIAG_TEST}, 
  {"dsp_diags","Debug DSP diagnostics",DBG_DSP_DIAGS}, 
  {"pm_layer","Debug performance management (PM) layer",DBG_PM_LAYER}, 
  {"alarm_layer","Debug ALARM layer",DBG_ALARM_LAYER}, 
  {"l3_ms_trace","Debug layer3 messages",DBG_L3_MS_TRACE},
  {"all","Enable debugging for all",GP10_DEBUG_ALL},
  {NULL,NULL,0}
};

enum {
  DBG_FUNC_LOG,
  DBG_CALL_LOG,
  DBG_MSG_LOG,
  DBG_WARN_LOG,
  DBG_ERR_LOG,
  DBG_ALL_LOG,
  DBG_TIMESTAMP_LOG
};


static keyword_options gp10_debug_options [] = {
  {"function","Function tracing required",DBG_FUNC_LOG},
  {"call","Call tracing required",DBG_CALL_LOG},
  {"message","Message tracing required",DBG_MSG_LOG},
  {"warning","Log only warings",DBG_WARN_LOG},
  {"error","Log only errors",DBG_ERR_LOG},
  {"timestamp","Log with Timestamp",DBG_TIMESTAMP_LOG},
  {"all","Turn on all log messages",DBG_ALL_LOG},
  {NULL,NULL,0}
};

EOLNS (gp10_debug_eol,gp10_debug_command);
pdecl(gp10_debug_opt);

FUNC(gp10_debug_flag_func,gp10_debug_opt,set_gp10_debug_flag);

KEYWORD_OPTIONS(gp10_debug_opt,gp10_debug_flag_func,gp10_debug_eol,gp10_debug_options,OBJ(int,2),
		PRIV_CONF,0);

KEYWORD_OPTIONS(exec_debug_gp10,gp10_debug_opt,no_alt,gp10_debug,OBJ(int,1),
		PRIV_CONF,0);


#undef ALTERNATE
#define ALTERNATE exec_debug_gp10
