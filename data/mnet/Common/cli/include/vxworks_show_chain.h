

/*************** definitions ******************/
#define TASK_ALL 1
#define TASK_GEN 2
#define TASK_DETAIL 3
#define TASK_SUMMARY 4
#define NAME 5
#define ALL  6
#define SUMMARY 7
#define DETAIL  8



/******Shows the symbolic name of  machine **********/
EOLNS   (show_host_eol,ShowHost);
KEYWORD (show_host, show_host_eol, no_alt,"hostname", "Shows the symbolic name of  machine", PRIV_MIN);


/**************   Shows environment variable *****************/

EOLNS   (show_env_name_eol,ShowEnv);
STRING 	(show_env_name,show_env_name_eol,no_alt,OBJ(string,1),"Variable name");
KEYWORD (show_env, show_env_name, show_host,"env", "Shows environment variable ", PRIV_MIN | PRIV_HIDDEN);

/*****************look up a symbol by name******************/

EOLS     (show_symbol_name_eol,Show_Symbol,NAME);
STRING  (show_symbol_name,show_symbol_name_eol,no_alt,
                     OBJ(string,1),"Symbol name");
KEYWORD  (show_symbol, show_symbol_name,show_env, 
         "symbol", "Look up a symbol by name", PRIV_MIN | PRIV_HIDDEN);



/***************** Shows the current status for loaded modules ******************/
EOLS      (show_module_name_all,Show_Module,ALL);

EOLS     (show_module_name_eol,Show_Module,NAME);
STRING  (show_module_name,show_module_name_eol,show_module_name_all,
                     OBJ(string,1),"Module name with correct directory path");
KEYWORD  (show_module, show_module_name,show_symbol, 
         "module", "Shows the current status for loaded modules ", PRIV_MIN);


/************* Shows task information from Task control block  *********/
EOLS (show_task_all_eol,Show_Task,TASK_ALL);

EOLS(show_tasks_general,Show_Task,TASK_GEN);

EOLS    (show_tasks_detail_eol,Show_Task,TASK_DETAIL);
KEYWORD (show_tasks_detail,show_tasks_detail_eol,show_tasks_general,
                 "detail","Displays the task details",PRIV_MIN);

EOLS    (show_tasks_summary_eol, Show_Task,TASK_SUMMARY);
KEYWORD (show_tasks_option,show_tasks_summary_eol,show_tasks_detail,
                 "summary","Displays task summary",PRIV_ROOT);

HEXNUM  (show_tasks_tid,show_tasks_option,show_task_all_eol,OBJ(int,1),"Task ID");
KEYWORD (show_tasks, show_tasks_tid,show_module, 
         "task", "Shows task information from Task control block ", PRIV_MIN);


/*************  Shows a stack trace of a task  *********/
EOLNS   (show_stack_trace_eol, Show_Stack_Trace);
HEXNUM  (show_stack_trace_tid,show_stack_trace_eol,no_alt,OBJ(int,1),"Task ID");
KEYWORD (show_stack_trace, show_stack_trace_tid,show_tasks, 
         "stack", " Shows a stack trace of a task ", PRIV_MIN);


/*************  Shows list of system drivers *********/
EOLNS   (show_drivers_eol, Show_Drivers);
KEYWORD (show_drivers, show_drivers_eol,show_stack_trace, 
         "drivers", " Shows list of system drivers", PRIV_MIN);

/********Shows the list of devices in the system ****************/

EOLNS   (show_devices_eol, Show_Devices);
KEYWORD (show_devices, show_devices_eol,show_drivers, 
         "devices", "Displays devices", PRIV_MIN);

/************** Shows list of file descriptor names in the system**************/
EOLNS   (show_fd_eol, Show_Fd);
KEYWORD (show_fd, show_fd_eol,  show_devices, 
         "fd", "Shows list of file descriptor names in the system", PRIV_MIN);



/************** Show ip **************/
EOLNS    (show_ip_route_eol,Show_ip_Route);
KEYWORD (show_ip_route, show_ip_route_eol,no_alt, 
         "route", "Displays ip route", PRIV_MIN);

EOLNS    (show_ip_traffic_eol,Show_ip_Traffic);
KEYWORD (show_ip_traffic, show_ip_traffic_eol,  show_ip_route, 
         "traffic", "Displays ip traffic", PRIV_MIN);


KEYWORD (show_ip, show_ip_traffic,  show_fd, 
         "ip", "Shows ip route / traffic", PRIV_MIN);


/************** Shows the attached network interfaces **************/
EOLNS   (show_interfaces_eol, Show_Interfaces);
KEYWORD (show_interfaces, show_interfaces_eol,  show_ip, 
         "interfaces", "Shows list of gp10 interfaces", PRIV_MIN);


/************** Show users **************/
EOLNS   (show_users_eol, Show_Users);
KEYWORD (show_users, show_users_eol,  show_interfaces, 
         "users", "Shows list of system users", PRIV_MIN | PRIV_HIDDEN);


/************** Show arp **************/
EOLNS   (show_arp_eol, Show_Arp);

KEYWORD (show_arp, show_arp_eol,  show_users, 
         "arp", "Shows entries in the system ARP table", PRIV_MIN);


/******************* Show Clock ***************/
EOLNS   (show_clock_eol, Show_Clock);

KEYWORD (show_clock, show_clock_eol,  show_arp, 
         "clock", "Shows system  clock", PRIV_MIN);



/******************* Show memory ***************/
EOLS   (show_memory_detail_eols, Show_Memory,DETAIL);
EOLS   (show_memory_summary_eols,Show_Memory,SUMMARY);
KEYWORD (show_memory_detail ,show_memory_detail_eols,no_alt,
          "detail", "Shows system  memory in detail", PRIV_MIN);   
KEYWORD (show_memory_summary,show_memory_summary_eols,show_memory_detail,
         "summary", "Shows summary of system  memory", PRIV_MIN);       
KEYWORD (show_memory, show_memory_summary,  show_clock, 
         "memory", "Shows system memory partition blocks and statistics", PRIV_MIN);





#undef  ALTERNATE
#define ALTERNATE       show_memory
