
#define CIPHERING_ENABLE 1

/*
 * enum for command identification
 */
enum {
  CELL_BARR_ENABLE,
  SMS_CB_ENABLE,
  AUTH_FREQ_VAL,
  AUTH_CIPH_ENABLE
};


/*
 * sys-feature cell-barring enable
 */

EOLS (cell_barr_eol,sysfeature_commands,CELL_BARR_ENABLE);

KEYWORD (cell_barr_en,cell_barr_eol,no_alt,"enable",
	 "Enable the cell barring",PRIV_CONF);

NVGENS (cell_barr_nv,cell_barr_en,sysfeature_commands,CELL_BARR_ENABLE);

KEYWORD (cell_barr,cell_barr_nv,no_alt,"cell-barring",
	 "Enable cell barring feature",PRIV_CONF);

/*
 * sys-feature sms-cb enable
 */

EOLS (sms_cb_eol,sysfeature_commands,SMS_CB_ENABLE);

KEYWORD (sms_cb_en,sms_cb_eol,no_alt,"enable",
	 "Enable the cell broadcast",PRIV_CONF);

NVGENS (sms_cb_nv,sms_cb_en,sysfeature_commands,SMS_CB_ENABLE);

KEYWORD (sms_cb,sms_cb_nv,cell_barr,"sms-cb",
	 "Enable SMS cell broadcast system on the cell",PRIV_CONF);

/*
 * sys-feature authentication-frequency <value>
 */

EOLS (auth_freq_eol,sysfeature_commands,AUTH_FREQ_VAL);

NUMBER (auth_freq_val,auth_freq_eol,no_alt,OBJ(int,1),0,5,
	"Enter the frequency");

NVGENS (auth_freq_val_nv,auth_freq_val,sysfeature_commands,AUTH_FREQ_VAL);

NOPREFIX (auth_freq_val_no,auth_freq_val_nv,auth_freq_eol);

KEYWORD (auth_freq,auth_freq_val_no,sms_cb,"authentication-frequency",
	 "Frequency of mobile authentication",PRIV_CONF);


/*
 * sys-feature authentication-feature [ciphering] enable
 */

EOLS (auci_enable_eol,sysfeature_commands,AUTH_CIPH_ENABLE);

KEYWORD (auci_enable,auci_enable_eol,no_alt,"enable",
	 "Enable the method",PRIV_CONF);

KEYWORD_ID(sys_cipher,auci_enable,auci_enable,OBJ(int,1),CIPHERING_ENABLE,
	 "ciphering","Enable ciphering",PRIV_CONF);

NVGENS  (sys_authen_nv,sys_cipher,sysfeature_commands,AUTH_CIPH_ENABLE);

KEYWORD(sys_authen,sys_authen_nv,auth_freq,"authentication-feature",
	 "Enable authentication ",PRIV_CONF);


/*
 * sys-feature
 */

KEYWORD (cfg_sys_feature,sys_authen,no_alt,"sys-feature",
	 "Configure system feature control parameters",PRIV_CONF);

#undef ALTERNATE
#define ALTERNATE cfg_sys_feature
