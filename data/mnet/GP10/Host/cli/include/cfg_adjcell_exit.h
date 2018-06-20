
EOLS    (gp10_adjcell_end_eol,exit_config_command,CMD_END);

KEYWORD (gp10_adjcell_end,gp10_adjcell_end_eol,no_alt,"end",
	 "Exit gp10 config-adjcell mode to exec mode",PRIV_CONF);

EOLNS   (gp10_adjcell_exit_eol,gp10_sub_config_exit);
KEYWORD (gp10_adjcell_exit, gp10_adjcell_exit_eol,gp10_adjcell_end, "exit",
         "Exit gp10 config-adjcell  mode", PRIV_CONF);

#undef	ALTERNATE
#define	ALTERNATE gp10_adjcell_exit
