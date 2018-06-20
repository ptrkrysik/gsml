EOLS    (gp10_proto_end_eol,exit_config_command,CMD_END);

KEYWORD (gp10_proto_end,gp10_proto_end_eol,no_alt,"end",
	 "Exit gp10 config-proto mode to exec mode",PRIV_CONF);

EOLNS   (gp10_proto_exit_eol,gp10_sub_config_exit);
KEYWORD (gp10_proto_exit, gp10_proto_exit_eol,gp10_proto_end, "exit",
         "Exit gp10 config-proto  mode", PRIV_CONF);

#undef	ALTERNATE
#define	ALTERNATE gp10_proto_exit
