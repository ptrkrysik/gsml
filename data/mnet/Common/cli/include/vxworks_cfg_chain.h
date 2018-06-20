


enum{
	OTHER,
	STARTUP_SCRIPT,
	FLAGS,
	TARGET_NAME,
	FTP_PASSWD,
	USER_NAME,
	GATEWAY_IP,
	HOST_IP,
	BACK_IP,
	ETHER_IP,
	FILE_NAME,
	HOST_NAME,
	PROC_NUM,
	UNIT_NUM,
	BOOT_DEV,
	EXIT_BOOT_MODE
};

/***********Boot change**********/

EOLS (exec_bootparam_exit_eol,changeBootParams,EXIT_BOOT_MODE);
KEYWORD (exec_bootparam_exit,exec_bootparam_exit_eol,no_alt,
			"exit","Exit from boot param mode",PRIV_ROOT);

EOLS (exec_bootparam_other_eol,changeBootParams,OTHER);
STRING  (exec_bootparam_other_string,exec_bootparam_other_eol,no_alt,OBJ(string,1),"Other");
KEYWORD (exec_bootparam_other,exec_bootparam_other_string,exec_bootparam_exit,
			"other","Other string used by applications",PRIV_ROOT);

EOLS (exec_bootparam_script_name_eol,changeBootParams,STARTUP_SCRIPT);
STRING  (exec_bootparam_script_name,exec_bootparam_script_name_eol,no_alt,OBJ(string,1),"Script name");
KEYWORD (exec_bootparam_script,exec_bootparam_script_name,exec_bootparam_other,
			"startup_script","Start up script name with directory path",PRIV_ROOT);

EOLS (exec_bootparam_flags_eol,changeBootParams,FLAGS);
NUMBER  (exec_bootparam_flags_number,exec_bootparam_flags_eol,no_alt,OBJ(int,1),0,100,"Flags");
KEYWORD (exec_bootparam_flags,exec_bootparam_flags_number,exec_bootparam_script,
			"flags","Flags (f)",PRIV_ROOT);


EOLS (exec_bootparam_target_eol,changeBootParams,TARGET_NAME);
STRING  (exec_bootparam_target_name,exec_bootparam_target_eol,no_alt,OBJ(string,1),"Name");
KEYWORD (exec_bootparam_target,exec_bootparam_target_name,exec_bootparam_flags,
			"target_name","Target name (tn)",PRIV_ROOT);


EOLS (exec_bootparam_ftp_passwd_eol,changeBootParams,FTP_PASSWD);
STRING  (exec_bootparam_ftp_passwd,exec_bootparam_ftp_passwd_eol,no_alt,OBJ(string,1),"Password");
KEYWORD (exec_bootparam_ftp,exec_bootparam_ftp_passwd,exec_bootparam_target,
			"ftp_password","Ftp password (pw)",PRIV_ROOT);

EOLS (exec_bootparam_user_eol,changeBootParams,USER_NAME);
STRING  (exec_bootparam_user_name,exec_bootparam_user_eol,no_alt,OBJ(string,1),"Name");
KEYWORD (exec_bootparam_user,exec_bootparam_user_name,exec_bootparam_ftp,
			"user","User name",PRIV_ROOT);

EOLS (exec_bootparam_gateway_inet_eol,changeBootParams,GATEWAY_IP);
STRING  (exec_bootparam_gateway_inet_ip,exec_bootparam_gateway_inet_eol,no_alt,OBJ(string,1),"Gateway inet (g)");
KEYWORD (exec_bootparam_gateway_inet,exec_bootparam_gateway_inet_ip,exec_bootparam_user,
			"gateway_inet","Gateway inet (g)",PRIV_ROOT);

EOLS (exec_bootparam_host_inet_eol,changeBootParams,HOST_IP);
STRING  (exec_bootparam_host_inet_ip,exec_bootparam_host_inet_eol,no_alt,OBJ(string,1),"Host inet (h)");
KEYWORD (exec_bootparam_host_inet,exec_bootparam_host_inet_ip,exec_bootparam_gateway_inet,
			"host_inet","Host inet (h)",PRIV_ROOT);

EOLS (exec_bootparam_inet_backplane_eol,changeBootParams,BACK_IP);
STRING  (exec_bootparam_inet_backplane_ip,exec_bootparam_inet_backplane_eol,no_alt,OBJ(string,1),"Inet on backplane (b)");
KEYWORD (exec_bootparam_inet_backplane,exec_bootparam_inet_backplane_ip,exec_bootparam_host_inet,
			"inet_on_backplane","Inet on backplane (b)",PRIV_ROOT);

EOLS (exec_bootparam_inet_ethernet_eol,changeBootParams,ETHER_IP);
STRING  (exec_bootparam_inet_ethernet_ip,exec_bootparam_inet_ethernet_eol,no_alt,OBJ(string,1),"Inet on ethernet (e)");
KEYWORD (exec_bootparam_inet_ethernet,exec_bootparam_inet_ethernet_ip,exec_bootparam_inet_backplane,
			"inet_on_ethernet","Inet on ethernet (e)",PRIV_ROOT);

EOLS (exec_boot_change_file_eol,changeBootParams,FILE_NAME);
STRING  (exec_bootparam_file_name,exec_boot_change_file_eol,no_alt,OBJ(string,1),"File name");
KEYWORD (exec_bootparam_file,exec_bootparam_file_name,exec_bootparam_inet_ethernet,
			"file_name","File name",PRIV_ROOT);

EOLS (exec_boot_change_host_eol,changeBootParams,HOST_NAME);
STRING  (exec_bootparam_host_name,exec_boot_change_host_eol,no_alt,OBJ(string,1),"Host name");
KEYWORD (exec_bootparam_host,exec_bootparam_host_name,exec_bootparam_file,
			"host_name","Host name",PRIV_ROOT);

EOLS (exec_boot_change_processor_eol,changeBootParams,PROC_NUM);
NUMBER  (exec_bootparam_processor_number,exec_boot_change_processor_eol,no_alt,OBJ(int,1),0,9,"Precessor number");
KEYWORD (exec_bootparam_processor,exec_bootparam_processor_number,exec_bootparam_host,
			"processor_number","Precessor number",PRIV_ROOT);

EOLS (exec_bootparam_unit_eol,changeBootParams,UNIT_NUM);
NUMBER  (exec_bootparam_unit_number,exec_bootparam_unit_eol,no_alt,OBJ(int,1),0,9,"Unit number");
KEYWORD (exec_bootparam_unit,exec_bootparam_unit_number,exec_bootparam_processor,
			"unit_number","Unit number",PRIV_ROOT);

EOLS (exec_bootparam_boot_device_eol,changeBootParams,BOOT_DEV);
STRING  (exec_bootparam_boot_device_name,exec_bootparam_boot_device_eol,no_alt,OBJ(string,1),"Device number");
KEYWORD (exec_boot_change_boot_device,exec_bootparam_boot_device_name,exec_bootparam_unit,
			"boot_device","Boot device",PRIV_ROOT);


/*************** Boot change *********/

/**************** Enable access to vxworks shell***************/
EOLNS	(cfg_vxsh_enable_eol,EnableVxsh);
NVGENNS	(cfg_vxsh_enable_nvgen,cfg_vxsh_enable_eol,EnableVxsh);
KEYWORD	(cfg_vxsh_enable,cfg_vxsh_enable_nvgen,no_alt,
		  "enable","Enable vxworks shell",PRIV_ROOT);	

KEYWORD (cfg_vxsh, cfg_vxsh_enable, no_alt, 
         "vxsh", "Enable vxworks shell", PRIV_ROOT | PRIV_HIDDEN);
/************ Change password for existing user **********/
EOLNS   (cfg_change_passwd_eol,ChangePasswd);
STRING  (cfg_change_passwd_uname,cfg_change_passwd_eol,no_alt,OBJ(string ,1),"User name");
NVGENNS	(cfg_change_passwd_nvgen,cfg_change_passwd_uname,ChangePasswd);
KEYWORD (cfg_change_passwd, cfg_change_passwd_nvgen, cfg_vxsh, 
         "change-passwd", "Change password for existing user", PRIV_ROOT);
/******** Change boot parameters*****************/

EOLNS	(cfg_boot_change_eol,setBootParamsMode);
NVGENNS	(cfg_boot_change_nvgen,cfg_boot_change_eol,setBootParamsMode);
KEYWORD	(cfg_boot_change,cfg_boot_change_nvgen,cfg_change_passwd,
		  "boot-params","Change the boot parameters ",PRIV_ROOT);
/******* Enable SSH access ***********/
EOLNS   (cfg_ssh_enable_eol, EnableSsh);
NVGENNS	(cfg_ssh_enable_nvgen,cfg_ssh_enable_eol,EnableSsh);
KEYWORD (cfg_ssh_enable,cfg_ssh_enable_nvgen,no_alt,
          "enable","Enable ssh",PRIV_ROOT);
KEYWORD (cfg_ssh, cfg_ssh_enable, cfg_boot_change, 
         "ssh", "Enable SSH access", PRIV_ROOT );

/******* Enable SSH access ***********/
EOLNS   (cfg_telnet_enable_eol, EnableTelnet);
NVGENNS	(cfg_telnet_enable_nvgen,cfg_telnet_enable_eol,EnableTelnet);
KEYWORD (cfg_telnet_enable,cfg_telnet_enable_nvgen,no_alt,
          "enable","Enable telnet",PRIV_ROOT);
KEYWORD (cfg_telnet, cfg_telnet_enable, cfg_ssh, 
         "telnet", "Enable telnet access", PRIV_ROOT );



/************* Add new system user ****************/

EOLNS   (cfg_user_name_eol, AddNewUser);
NUMBER  (cfg_user_name_level,cfg_user_name_eol,no_alt,OBJ(int,1),0,1,"User level(0 for super user and 1 for normal user,First user must be super user)");
STRING  (cfg_user_name_usrname,cfg_user_name_level,no_alt,OBJ(string,1),"User name");
NVGENNS	(cfg_user_name_usrname_nvgen,cfg_user_name_usrname,AddNewUser);
KEYWORD (cfg_user_name, cfg_user_name_usrname_nvgen,  cfg_telnet, 
         "username", "Add new system user", PRIV_ROOT);

#undef  ALTERNATE
#define ALTERNATE       cfg_user_name
