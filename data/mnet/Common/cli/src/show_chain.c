
#include <vxworks.h>
#include <stdioLib.h>
#include <tyLib.h>
#include <taskLib.h>
#include <errnoLib.h>
#include <lstLib.h>
#include <inetLib.h>    /* for INET_ADDR_LEN */
#include <timers.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <msgQLib.h>
#include <sigLib.h>
#include <usrLib.h>
#include <time.h>
#include <symLib.h>


#include <../include/cli_parser_private.h>
#include <../include/pstypes.h>
#include <../include/psvxif.h>
#include <../include/imsg.h>
#include <../include/imc.h>
#include <../include/address.h>
#include <../include/parsertypes.h>
#include <../include/config.h>
#include <../include/parser_sim.h>
#include <../include/parser.h>
#include <../include/parser_defs_exec.h>
#include <../include/parser_input.h>
#include <../include/parser_sim.h>
#include <../include/parser_util.h>
#include <../include/parser_commands.h>
#include <../include/nv.h>
#include <../include/parser_errno.h>
#include <../include/parser_ModNum.h>
#include <../include/defs.h>
#include <../include/parser_init.h>
#include <../include/macros.h>
#include <../include/parser_actions.h>


#define ALTERNATE NONE
#include "../include/vxworks_show_chain.h"
LINK_POINT( basic_show_commands, ALTERNATE );
#undef ALTERNATE




static parser_extension_request  show_chain_init_table[] = 
{
  { PARSE_ADD_SHOW_CMD, &pname( basic_show_commands)}, 
  { PARSE_LIST_END, NULL } 
};


void ShowParserInit(void)
{
          parser_add_command_list(show_chain_init_table, "show_commands");
        
}



/********   Shows version of gp10 software ****************

void Show_Version(parseinfo *csb){
	static char *date = __DATE__ ;
        static char *time = __TIME__ ;

        version();
	printf(" dated %s :  %s\n", date, time );
}
*/
/**Shows system memory partition blocks and statistics*/
void Show_Memory(parseinfo *csb){
       switch(csb->which){
	     case SUMMARY: 
                  memShow(0);
		  break;
             case DETAIL:
                  memShow(1);
                  break;
        }
}

/**** Shows system  clock **/
void Show_Clock(parseinfo *csb){
     struct timespec tp;
     clock_gettime(CLOCK_REALTIME,&tp);
     printf("\n%s\n",asctime(gmtime( &(tp.tv_sec) ) ));
    
}

/******* display entries in the system ARP table ********/
void Show_Arp(parseinfo *csb){
     arpShow();
}

/******** "Shows list of gp10 users  *********/
void Show_Users(parseinfo *csb){
     sysUserShow();
}

/******** Shows the attached network interfaces *****/
void Show_Interfaces(parseinfo *csb){
     ifShow(0);
}

/******* Shows ip traffic ***/
void Show_ip_Traffic(parseinfo *csb){
     icmpstatShow();
     tcpstatShow();
     udpstatShow(); 
}

/********* Shows ip route *****/
void Show_ip_Route(parseinfo *csb){
     routeShow();
}

/******* Shows list of system drivers  ****/
void Show_Drivers(parseinfo *csb){
     iosDrvShow();
}

/******* Shows the list of devices in the system ***/
void Show_Devices(parseinfo *csb){
     iosDevShow();
}

/******** Shows list of file descriptor names in the system *******/
void Show_Fd(parseinfo *csb){
     iosFdShow();
}

/********* Shows  stack trace of a task ****/
void Show_Stack_Trace(parseinfo *csb){
        
        tt(GETOBJ(int,1));
}

/**********Shows task information from Task control block ****/
void Show_Task(parseinfo *csb){

        switch(csb->which){
                case TASK_ALL:
                        taskShow(0,2);
                        break;
                case TASK_GEN:
            taskShow(GETOBJ(int,1),1);
                        break;
                case TASK_DETAIL:
                        taskShow(GETOBJ(int,1),1);
                        break;
                case TASK_SUMMARY:
                        taskShow(GETOBJ(int,1),0);
                        break;
        }
}
/****** Shows the current status for loaded modules **/
void Show_Module(parseinfo *csb){

        switch(csb->which){
                case ALL:
                        moduleShow(NULL,0);
                        break;
                case NAME:
                        moduleShow(GETOBJ(string,1),0);
                        break;
        }
}

extern void *sysSymTbl;


/**** look up a symbol by name  ***/

void Show_Symbol(parseinfo *csb){
    SYM_TYPE ptr;
    char *bufp;	
    
  /*  symFindByName(sysSymTbl,GETOBJ(string,1),&bufp,&ptr); 
    printf("\n%s\n",bufp);*/
    switch(csb->which){
          case ALL:
               lkup(NULL);
          case NAME:
	       if(GETOBJ(string,1)==NULL)break;
               	  lkup(GETOBJ(string,1));
               break;
               
               
        }

   
}	

/******* Shows environment variable *******/
void ShowEnv(parseinfo *csb){
  
   if((getenv(GETOBJ(string,1)))==NULL){
        printf("\nvariable not found\n");
        return;
   }else
        printf("\n Variable is = %s\n",getenv(GETOBJ(string,1)));
    
  
}


/******* Shows the symbolic name of  machine ***/
void ShowHost(parseinfo *csb){
     char hostname[128];
     gethostname(hostname,127);
     printf("\n%s\n",hostname);
}

