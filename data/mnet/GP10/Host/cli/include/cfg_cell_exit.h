EOLS    (gp10_cell_end_eol,exit_config_command,CMD_END);

KEYWORD (gp10_cell_end,gp10_cell_end_eol,no_alt,"end",
	 "Exit gp10 config-cell mode to exec mode",PRIV_CONF);

EOLNS   (gp10_cell_exit_eol,gp10_sub_config_exit);
KEYWORD (gp10_cell_exit, gp10_cell_exit_eol,gp10_cell_end, "exit",
         "Exit gp10 config-cell  mode", PRIV_CONF);

#undef	ALTERNATE
#define	ALTERNATE gp10_cell_exit
