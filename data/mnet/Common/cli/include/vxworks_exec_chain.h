/************** Definitions ********************/
#include "../include/macros.h"


#define LongList 1
#define NormalList 2
extern BOOL vxsh_flag;

 /*************** Definitions *********/


keyword_options         cli_month_options[] = {
        {"january", "JANUARY", 1},
        {"february", "FEBRUARY", 2},
	{"march", "MARCH", 3},
	{"april", "APRIL", 4},
	{"may", "MAY", 5},
	{"june", "JUNE", 6},
	{"july", "JULY", 7},
	{"august", "AUGUST", 8},
	{"september", "SEPTEMBER", 9},
	{"october", "OCTOBER", 10},
	{"november", "NOVEMBER", 11},
	{"december", "DECEMBER", 12},
        { NULL, NULL, 0 }
};


keyword_options    vxshell_options[]={
        {"ldvxsh", "Load Vxworks shell",1 },
	{"unldvxsh","Load Vxworks shell",2},
        {"vxsh" ,"Invoke vxworks shell",3},
           {NULL,NULL,0}
};

/*********************************/

EOLNS(vxsh_command_string_eol,execVxshCommand);
TEXT(vxsh_command_string,vxsh_command_string_eol,no_alt,OBJ(string,1),"Enter vxworks   commands");
/******************************* vxsh ************/
EOLNS		    (exec_vxsh_eol, vxshCommand);
STRING	    (exec_vxsh_string, exec_vxsh_eol, no_alt,OBJ(string,1), "file/env variable name");
KEYWORD_ID      (exec_vxsh_env, exec_vxsh_string, exec_vxsh_eol, OBJ(int,2), 2, "env","Environment variable",PRIV_ROOT);
KEYWORD_ID      (exec_vxsh_file, exec_vxsh_string, exec_vxsh_env,OBJ(int,2), 1, "file","File with directory path ", PRIV_ROOT);

KEYWORD_OPTIONS (exec_vxsh, exec_vxsh_file,no_alt, vxshell_options,OBJ(int,1),PRIV_ROOT | PRIV_HIDDEN,0);

ASSERT  (exec_vxsh_option,exec_vxsh,no_alt,vxsh_flag);

/******************/

/************************ Load running configurations ****************************/
EOLNS	 (exec_load_running_config_eol,loadRunning);
KEYWORD  (exec_load_running_config,exec_load_running_config_eol,exec_vxsh_option,
				"load-runnning","Load running configurations", PRIV_ROOT);

/************************ Save running configurations ****************************/
EOLNS	 (exec_save_running_config_eol,saveRunning);
KEYWORD  (exec_save_running_config,exec_save_running_config_eol,exec_load_running_config,
				"save-runnning","Save running configurations", PRIV_ROOT);

/****************/
/*****************/
EOLNS	(exec_print_file_eol,printFile);
STRING  (exec_print_file_name,exec_print_file_eol,no_alt,
					OBJ(string,1),"Name of the file to be created");
KEYWORD (exec_print_file,exec_print_file_name,exec_vxsh_option,
					"print-file","Print the contents a file",PRIV_ROOT | PRIV_HIDDEN);

/*********************/
EOLNS	(exec_create_eol,createFile);
STRING  (exec_create_file_name,exec_create_eol,no_alt,
					OBJ(string,1),"Name of the file to be created");
KEYWORD (exec_create_file,exec_create_file_name,exec_print_file,
					"create-file","Create a file",PRIV_ROOT | PRIV_HIDDEN);

/*************Set system clock ***********/
EOLNS   (cfg_clock_hour_min_secs_day_month_year_eol, ClockSet);
NUMBER (cfg_clock_hour_min_secs_day_month_year,cfg_clock_hour_min_secs_day_month_year_eol,
		no_alt,OBJ(int,6),2000,2050,"Year");
KEYWORD_OPTIONS (cfg_clock_hour_min_secs_day_month, cfg_clock_hour_min_secs_day_month_year,
                no_alt, cli_month_options, OBJ(int, 5),
                PRIV_ROOT, 0);
NUMBER (cfg_clock_hour_min_secs_day,cfg_clock_hour_min_secs_day_month,
			cfg_clock_hour_min_secs_day_month_year_eol,OBJ(int,4),1,31,"Day of the  month");
NUMBER (cfg_clock_hour_min_secs,cfg_clock_hour_min_secs_day,no_alt,OBJ(int,3),0,59,"Seconds");

NUMBER  (cfg_clock_hour_min,cfg_clock_hour_min_secs,no_alt,OBJ(int,2),0,59,"Minutes");
NUMBER  (cfg_clock_hour,cfg_clock_hour_min,no_alt,OBJ(int,1),0,23,"Hour");
KEYWORD (cfg_clock, cfg_clock_hour,  exec_create_file, 
         "set-clock", "Configure clock", PRIV_ROOT);


/*************Log out of shell ************/
EOLNS   (exec_logout_eol, Logout);
KEYWORD 	(exec_logout, exec_logout_eol,  cfg_clock, 
        	 "logout", "Log out of shell ", PRIV_ROOT | PRIV_HIDDEN);


/************** Set an environment variable ***************/
EOLNS   (exec_setenv_eol, SetEnv);
pdecl(exec_setenv_args_var_id);

FUNC(exec_setenv_AcceptArg,exec_setenv_args_var_id, AcceptArg);

STRING  (exec_setenv_args_string,exec_setenv_AcceptArg,no_alt,OBJ(string,2),"String parameter(may be quoted)");
KEYWORD_ID (exec_setenv_args_string_id, exec_setenv_args_string, exec_setenv_eol , OBJ(int,2), 3,
		"string", "String parameter", PRIV_ROOT);

STRING  (exec_setenv_args_var,exec_setenv_AcceptArg,no_alt,OBJ(string,2),"variable name");
KEYWORD_ID (exec_setenv_args_var_id, exec_setenv_args_var, exec_setenv_args_string_id,  OBJ(int, 2),2,
		"var", "Var parameter", PRIV_ROOT);

STRING  	(exec_setenv_varname,exec_setenv_args_var_id,no_alt,OBJ(string,1),"Variable name");
KEYWORD 	(exec_setenv, exec_setenv_varname,  exec_logout, 
        	 "setenv", "Set an environment variable", PRIV_ROOT | PRIV_HIDDEN);

/***********Invoke  a function***********/

EOLNS   (exec_invoke_eol, Invoke);
pdecl(exec_invoke_args_number_id);


FUNC(exec_invoke_AcceptArg,exec_invoke_args_number_id, AcceptArg);


STRING  (exec_invoke_args_string,exec_invoke_AcceptArg,no_alt,OBJ(string,2),"String parameter(may be quoted)");
KEYWORD_ID (exec_invoke_args_string_id, exec_invoke_args_string, exec_invoke_eol , OBJ(int,2), 3,
		"string", "String parameter", PRIV_ROOT);

STRING  (exec_invoke_args_var,exec_invoke_AcceptArg,no_alt,OBJ(string,2),"Variable name");
KEYWORD_ID (exec_invoke_args_var_id, exec_invoke_args_var, exec_invoke_args_string_id,  OBJ(int, 2),2,
		"var", "Variable parameter", PRIV_ROOT);

NUMBER  (exec_invoke_args_number,exec_invoke_AcceptArg,no_alt,OBJ(int,1),0,65256,"Int param value");
KEYWORD_ID (exec_invoke_args_number_id, exec_invoke_args_number, exec_invoke_args_var_id,  OBJ(int, 2),1,
		"int", "Int parameter", PRIV_ROOT);

STRING  	(exec_invoke_name,exec_invoke_args_number_id,no_alt,OBJ(string,1),"Function name");
KEYWORD 	(exec_invoke, exec_invoke_name,  exec_setenv, 
        	 "invoke", "Invoke a function ", PRIV_ROOT | PRIV_HIDDEN);

/************** load an object module into memory  ********************/
EOLNS   (exec_loadmodule_name_eol, LoadModule);
STRING  (exec_loadmodule_name,exec_loadmodule_name_eol,no_alt,OBJ(string,1),"Module name with correct directory path");
KEYWORD (exec_loadmodule, exec_loadmodule_name,  exec_invoke, 
         "load", "Load an object module into memory", PRIV_ROOT | PRIV_HIDDEN);


/**************** Rename a  file *************/
EOLNS   (exec_rename_command_eol, Rename);
STRING  (exec_rename_command_newfilename,exec_rename_command_eol,no_alt,OBJ(string,2),"New file name");
STRING  (exec_rename_command_filename,exec_rename_command_newfilename,no_alt,OBJ(string,1)," File name");
KEYWORD (exec_rename_command, exec_rename_command_filename,  exec_loadmodule, 
         "rename","Rename a file", PRIV_ROOT);

/**************** copy a  file *************/
EOLNS   (exec_copy_command_eol, Copy);
STRING  (exec_copy_command_destname,exec_copy_command_eol,no_alt,OBJ(string,2),"Destination file name");
STRING  (exec_copy_command_srcname,exec_copy_command_destname,no_alt,OBJ(string,1),"Source file name");
KEYWORD (exec_copy_command, exec_copy_command_srcname,  exec_rename_command, 
         "copy", "Copy a  file", PRIV_ROOT);

/**************** Delete a file *************/
EOLNS   (exec_delete_command_filename_eol, DeleteFile);
STRING  (exec_delete_command_filename,exec_delete_command_filename_eol,no_alt,OBJ(string,1),"Filename");
KEYWORD (exec_delete_command, exec_delete_command_filename,  exec_copy_command, 
         "delete", "Delete a file", PRIV_ROOT);

/**************** Delete a directory *************/
EOLNS   (exec_rmdir_command_eol, RmDir);
STRING  (exec_rmdir_command_dirname,exec_rmdir_command_eol,no_alt,OBJ(string,1),"Directory name");
KEYWORD (exec_rmdir_command, exec_rmdir_command_dirname,  exec_delete_command, 
         "rmdir", "Delete a directory", PRIV_ROOT);

/****************  Make a directory  *************/
EOLNS   (exec_mkdir_command_eol, Mkdir);
STRING  (exec_mkdir_command_dirname,exec_mkdir_command_eol,no_alt,OBJ(string,1),"Directory name");
KEYWORD (exec_mkdir_command, exec_mkdir_command_dirname,  exec_rmdir_command, 
         "mkdir", "Make a directory", PRIV_ROOT);

/**************** Reset network devices and transfer control to boot ROMs*************/
EOLNS   (exec_reload_command_eol, Reload);

KEYWORD (exec_reload_command, exec_reload_command_eol,  exec_mkdir_command, 
         "reload", "Reset network devices and transfer control to boot ROMs", PRIV_ROOT);


/**************** Change directory *************/
EOLNS   (exec_cd_command_eol, ChangeDir);
STRING  (exec_cd_command_dirname,exec_cd_command_eol,no_alt,OBJ(string,1),"Directory name");
KEYWORD (exec_cd_command, exec_cd_command_dirname,  exec_reload_command, 
         "cd", "Change directory", PRIV_ROOT);


/**************** List the contents of a directory ************/
EOLS   (exec_dir_command_eol, Dir,NormalList);

EOLS   (exec_dir_command_long_eol, Dir,LongList);

KEYWORD (exec_long_list,exec_dir_command_long_eol,exec_dir_command_eol,"detail","Detailed listing of directory contents",PRIV_ROOT);
STRING  (exec_dirname,exec_long_list,exec_dir_command_eol,OBJ(string,1),"Directory name");
KEYWORD (exec_dir_command, exec_dirname,  exec_cd_command, 
         "dir", "List the contents of a directory", PRIV_ROOT);




/*************Print the current default directory****************/
EOLNS   (exec_pwd_command_eol, Pwd);
KEYWORD (exec_pwd_command, exec_pwd_command_eol,  exec_dir_command, 
         "pwd", "Print the current working directory path", PRIV_ROOT);


/*********** Test that a remote host is reachable ***********/
EOLNS   (exec_ping_command_eol, Ping);
STRING(exec_ping_command_ipaddress,exec_ping_command_eol,no_alt,OBJ(string,1),"Ip address of host");
KEYWORD (exec_test_command, exec_ping_command_ipaddress,  exec_pwd_command, 
         "ping", "Test that a remote host is reachable", PRIV_ROOT);


#undef  ALTERNATE
#define ALTERNATE       exec_test_command






