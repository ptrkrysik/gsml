

#include <vxworks.h>
#include <stdioLib.h>
#include <stdio.h>
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
#include <rebootLib.h>
#include <pingLib.h>
#include <symLib.h>
#include <dirent.h>
#include <stat.h>

 
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

#define maxMessages 100
BOOL ServerBreak;
BOOL  vxsh_flag=FALSE;
BOOL load_running_config=FALSE;
int backInFd;
extern boolean doneAuth;

/* Global Variables */

static int num_of_args; /* To keep track of number of arguments entered by the user */


#define  ALTERNATE NONE
#include "../include/vxworks_exec_chain.h"
LINK_POINT(basic_exec_command, ALTERNATE);
#undef  ALTERNATE




static parser_extension_request exec_chain_init_table[] = {
    {PARSE_ADD_EXEC_CMD, &pname(basic_exec_command)},
	{PARSE_ADD_VXSH_CHAIN, &pname(vxsh_command_string)},
    {PARSE_LIST_END, NULL}
};

void init_vxsh_submode (void)
{ 
  parser_add_mode("vxsh", "vxsh",
		    "VXworks shell", TRUE, TRUE, "configure",
		    NULL, NULL, &pname(vxsh_command_string),NULL);
}

void ExecParserInit(void)
{
         parser_add_command_list(exec_chain_init_table, "basic exec commands");
         init_vxsh_submode();

}


/****************************  Call back functions ***********************/

/*** Test that a remote host is reachable ***/
void Ping(parseinfo *csb){
        ping(GETOBJ(string,1),1,0);
}

/****Print the current default directory  ***/
void Pwd(parseinfo *csb){
        char buffer[512];
         pwd();

}
/**** List the contents of a directory   ***/
void Dir(parseinfo *csb){
    
        switch(csb->which){
                case LongList:
                     if(strlen(GETOBJ(string,1)))
                        ls(GETOBJ(string,1),1);
                     else
                        ls(0,1);
                     break;
                case NormalList:
                     if(strlen(GETOBJ(string,1)))
                        ls(GETOBJ(string,1),0);
                     else
                        ls(0,0);
                     break;
        }
     
}

/***********   Change directory     ********/
void ChangeDir(parseinfo *csb){ 
    struct stat fileStat;

    stat (GETOBJ(string,1), &fileStat);
    if (!S_ISDIR (fileStat.st_mode)){
     printf ("%s No such directory exists\n", GETOBJ(string,1));
     return;
    }else
        cd(GETOBJ(string,1));

}

/****     Make a directory   **************/
void Mkdir(parseinfo *csb){
        mkdir(GETOBJ(string,1));
}

/**Reset network devices and transfer control to boot ROMs **/
void Reload(parseinfo *csb){
		char c;
		printf("Do you want to reboot the system : [yes/no]  ");
		read(STD_IN,&c,1);
		write(STD_OUT,"\n",1);
		if(c!='y')
			return;
		else
			write(STD_OUT,"System is rebooting\n",strlen("\nSystem is rebooting\n"));
		taskDelay (sysClkRateGet ()*2);
        reboot(0x04);
}


/**** Delete a directory      ****/
void RmDir(parseinfo *csb){
    struct stat fileStat;

    stat (GETOBJ(string,1), &fileStat);
    if (!S_ISDIR (fileStat.st_mode)){
     printf ("%s No such directory exists\n", GETOBJ(string,1));
     return;
    }
   if(rmdir(GETOBJ(string,1)))
      printf("\nDirectory is not empty .Could not be deleted\n");

}

/******* Delete a file  ******/
void DeleteFile(parseinfo *csb){
 struct stat fileStat;

   stat (GETOBJ(string,1), &fileStat);
   if (S_ISDIR (fileStat.st_mode)){
      printf ("%s is not a file\n", GETOBJ(string,1));
      return;
   }
   if(rm(GETOBJ(string,1)))
      printf("\nFile could not be deleted\n");

}

/******** copy a  file  *****/
void Copy(parseinfo *csb) {
        copy(GETOBJ(string,1),GETOBJ(string,2));
}

/****** Rename a file *******/
void Rename(parseinfo *csb){
  copy(GETOBJ(string,1),GETOBJ(string,2));
  rm(GETOBJ(string,1));
}

/*** load an object module into memory  ***/
void LoadModule(parseinfo *csb){
        ld(0,0,GETOBJ(string,1));
         
}



struct {
	int type;
	union {
		int i;
		char s[256];
	}value;
} argv[3];


/*********** Invoke  a function  ***********/
extern void *sysSymTbl;

void Invoke(parseinfo *csb){
       
	int (*f)();
	char command[256];
	SYM_TYPE  type;
	STATUS s;
	f=0;
	
#ifdef MNET_GP10
	strcpy(command,GETOBJ(string,1));

#else 
	strcpy(command,"_");
	strcat(command,GETOBJ(string,1));
#endif



	s= symFindByName(sysSymTbl, command, (char **) &f, &type);
	if((s == OK) && (f!=0) ){
	switch(num_of_args)
	{
		case 0: (*f)(); break;
		case 1:{
                     
                       if(argv[0].type==0)(*f)(argv[0].value.i);
                       else  (*f)(argv[0].value.s);
                       break;
		}
		case 2:{
                         switch(2*argv[0].type+argv[1].type){
                           case 0:(*f)(argv[0].value.i, argv[1].value.i); break;
                           case 1:(*f)(argv[0].value.i, argv[1].value.s); break;
			   case 2:(*f)(argv[0].value.s, argv[1].value.i); break;
                           case 3:(*f)(argv[0].value.s, argv[1].value.s); break;
			}
                        break;
 		}                
		case 3:{
		  switch((4*argv[0].type+2*argv[1].type+argv[2].type)){
                         case 0:(*f)(argv[0].value.i, argv[1].value.i, argv[2].value.i); break;
  		         case 1:(*f)(argv[0].value.i, argv[1].value.i, argv[2].value.s);  break;
			 case 2:(*f)(argv[0].value.i, argv[1].value.s, argv[2].value.i); break;
			 case 3:(*f)(argv[0].value.i, argv[1].value.s, argv[2].value.s); break;
			 case 4:(*f)(argv[0].value.s, argv[1].value.i, argv[2].value.i); break;
			 case 5:(*f)(argv[0].value.s, argv[1].value.i, argv[2].value.s); break;
			 case 6:(*f)(argv[0].value.s, argv[1].value.s, argv[2].value.i); break;
			 case 7:(*f)(argv[0].value.s, argv[1].value.s, argv[2].value.s); break;
                     }
                break;
                }
		default:printf("\nargc = %d\n",num_of_args); printf(" can not invoke functions with more parameters\n");
	}
	}else
          printf("\nsymbol could not be found\n");
	num_of_args=0;
	return;
}

/************* Function to accept parameters from command line *********/
void  AcceptArg(parseinfo *csb){


	if(num_of_args < 3) {
 	    switch(GETOBJ(int,2))
  	    {
		case 1: { 
		
		 	 argv[num_of_args].type=0;
		 	 argv[num_of_args].value.i= GETOBJ(int,1);
		 	 break;
		}
		case 2: { 
			
		 	argv[num_of_args].type=1;
		 	strcpy((char *)argv[num_of_args].value.s, getenv(GETOBJ(string,2)));
		 	break;
		}
		case 3: { 
		
		 	 argv[num_of_args].type=1;
			 strcpy((char *)argv[num_of_args].value.s,(char*)GETOBJ(string,2));
			 break;
		}
		default : 
  	  		printf(" unknown param\n");
   	   }
	   num_of_args++;
	 
	}
}



/******** Set an environment variable *****/
void SetEnv(parseinfo *csb){

  char variable[1024];
  int i;
 
  strcpy(variable,GETOBJ(string,1));
  strcat(variable,"=");
  for(i=0;i<num_of_args;i++)
     strcat(variable, argv[i].value.s);

  num_of_args=0;

  putenv(variable);
  
}




/***** Log out of shell  ****/
void Logout(parseinfo *csb){

	if(ShellTask==ConsoleTask){
		doneAuth = FALSE;
		return;
	}
	if (ShellTask==TelnetTask || ShellTask == SSHTask)
		ServerBreak=TRUE;
	if(cli_logoutf)	
		(*cli_logoutf)(cli_logVar);
	else
		printf("\nLogout routine could not be found\n");
}
void logoutTask(void){

	if(cli_logoutf)	
		(*cli_logoutf)(cli_logVar);
	
}
int shellTerminateCurrentSession(void){
	int retVal=1;

	if( ShellTask == ConsoleTask ){
		doneAuth = FALSE;
		return 0;
	}
	if ( ShellTask == TelnetTask || ShellTask == SSHTask){
			ServerBreak=TRUE;

			if(taskSpawn("tlogoutTask", 150, 0, 10000,(FUNCPTR)logoutTask,0,0,0,0,0,0,0,0,0,0)==ERROR)
				retVal=1;
			else
				retVal=0;

	}

    do
         taskDelay (sysClkRateGet ());
    while (taskNameToId ("tlogoutTask") != ERROR);
	

	return retVal;
}


#ifdef MNET_GMC
	extern void rtc_write( const struct tm *);
#endif
     
/********* Clock set *********/
void ClockSet(parseinfo *csb){
       
     struct timespec tp;
     char timeBuffer[128];	
    
     struct tm cli_gmtime;
     clock_gettime(CLOCK_REALTIME,&tp);

     if(gmtime_r( &(tp.tv_sec),&cli_gmtime )!=OK){
        printf("\nCould not get the time\n");
        return;
     }
     cli_gmtime.tm_hour=GETOBJ(int,1);
     cli_gmtime.tm_min=GETOBJ(int,2);
     cli_gmtime.tm_sec=GETOBJ(int,3);
     if(GETOBJ(int,4)!=0){
	cli_gmtime.tm_mday=GETOBJ(int,4);
	cli_gmtime.tm_mon=GETOBJ(int,5)-1;
	cli_gmtime.tm_year=GETOBJ(int,6)-1900;
	cli_gmtime.tm_isdst=0;
     }

#ifdef MNET_GMC
      rtc_write(&cli_gmtime); 
#endif

     tp.tv_sec=mktime(&cli_gmtime);
     tp.tv_nsec=0;
     clock_settime(CLOCK_REALTIME,&tp);	
	
   
}

extern FUNCPTR cli_logoutf;
extern int cli_logVar;



/*******************************/
int vxpipe;
int ptydev_m;
MSG_Q_ID vxshQueueID;
int workerTaskID;


int  readLine(int inFd,char* buffer){
	char c;
    int ret,i=0;
	while(1){
          ret = read(inFd, &c, 1);
		  if ( ret != 1 ) {buffer[i]='\0';return -1;}

	  if(c ==  '\n') { buffer[i]='\0'; return i; }
	  buffer[i++]=c;
    }
}
void workTask(void){
	char msg[PARSEBUF];
	int msgLen;
    STATUS (*myExecute)(char *);

	myExecute=(FUNCPTR)shellGetVxExecute();

	while(1){
		msgLen = msgQReceive(vxshQueueID,msg,PARSEBUF, WAIT_FOREVER );
		if(strlen(msg)!=0){
			if(strcmp(msg,"exit")==0){
				   msgQDelete (vxshQueueID);
				   return;
			}
			   msg[msgLen]=0;
			   (*myExecute)(msg);
		}
	}

}
void createWorkerTask(void){
	
	vxshQueueID = msgQCreate( maxMessages, PARSEBUF, MSG_Q_FIFO);
	if ( vxshQueueID == NULL ) {
                printf("Cannot create callback message queue for vxshell\n");
				return;
	}
    workerTaskID=taskSpawn("vxshWorkerTask",140, PARSER_TASK_OPTION, PARSER_TASK_STACK_SIZE,
                         (FUNCPTR)workTask, 0,0,0,0,0,0,0,0,0,0); 

}

void cleanupWorkerTask(void){
		if((workerTaskID=taskNameToId ("vxshWorkerTask") )!= ERROR){
  			  taskDelete(workerTaskID);
		}
		msgQDelete (vxshQueueID);
}

void execVxShellScript
    (
    char *fileName
    )
    {
    int old;
    int newStdIn;
    char command[PARSEBUF];
    BOOL ret=FALSE;
   
	newStdIn = open (fileName, O_RDONLY, 0);
	if(newStdIn <0){
		printf("\nCould not execute the script\n");
		return;
	}
	createWorkerTask();
   	command[0]=0;
	ret=readLine( newStdIn,command);
	while( ret != -1  && strcmp(command,"exit") ){
            if(strlen(command)!=0)
			 	if(msgQSend(vxshQueueID,command,strlen(command),WAIT_FOREVER, MSG_PRI_NORMAL)==ERROR){
					printf("\n Command could not be sent to the queue\n");
					cleanupWorkerTask();
					return;
				}
				command[0]=0;
 				ret=readLine( newStdIn,command);
	
	}
	msgQSend(vxshQueueID,"exit",strlen("exit"),WAIT_FOREVER, MSG_PRI_NORMAL);
	close(newStdIn);

 }



void vxshCommand(parseinfo *csb){
	int ret=-1;
	int oldTaskInFd,oldTaskOutFd,oldGlobalInFd,oldGlobalOutFd;
	switch(GETOBJ(int,1)){

		case 1 : /*load vxshell */
			if(!GETOBJ(int,2)){
					printf("\nCould not load vxworks shell\n");
					break;
			}
			if(GETOBJ(int,2)==1)
                     ret=shellLoadVxShell(GETOBJ(string,1));
            else if(GETOBJ(int,2)==2)
                     ret=shellLoadVxShell(getenv(GETOBJ(string,1)));

			if(!ret)
					printf("\nCould not load vxworks shell\n");
			else
				installedVxShell=1;
		
			break;	

		case 2 : /* unolad vxshell */
			installedVxShell=0;
			break;

		case 3 : /* vxshell */
                  
			if(!isInstalledVxShell()){
				printf("Vx Shell not installed\n");
				return;
			}
			if(!vxsh_flag){
				return;
			}
	
			if(GETOBJ(int,2)==NULL){
				createWorkerTask();
				set_mode_byname(&csb->mode,"vxsh",MODE_VERBOSE);
				break;
			}

			oldGlobalOutFd = ioGlobalStdGet(STD_OUT);
			oldGlobalInFd  = ioGlobalStdGet(STD_IN);
		    oldTaskOutFd   = ioTaskStdGet(0,STD_OUT);
			oldTaskInFd	   = ioTaskStdGet(0,STD_IN);

			if(GETOBJ(int,2)==1)
				execVxShellScript(GETOBJ(string,1));
			else
				execVxShellScript(getenv(GETOBJ(string,1)));
			
	        do
	            taskDelay (sysClkRateGet ());
		    while (taskNameToId ("vxshWorkerTask") != ERROR);
        
			ioGlobalStdSet(STD_OUT,oldGlobalOutFd);
			ioGlobalStdSet(STD_IN,oldGlobalInFd);
		    ioTaskStdSet(0,STD_OUT,oldTaskOutFd);
			ioTaskStdSet(0,STD_IN,oldTaskInFd);


   			break;
	}
}
void execVxshCommand(parseinfo *csb){
	int msgLen;
    BOOL ret=FALSE;
    STATUS (*myExecute)(char *);

    myExecute=(FUNCPTR)shellGetVxExecute();

	if(!GETOBJ(string,1)){
		printf("\nCommand input error\n");
		return;
	}
	if((msgLen=strlen(GETOBJ(string,1)))==0)
		return;

	if(strcmp(GETOBJ(string,1),"exit")==0){
			cleanupWorkerTask();
			csb->mode = exec_mode;
			return;
	}

	if(msgQSend(vxshQueueID,GETOBJ(string,1),msgLen,WAIT_FOREVER, MSG_PRI_NORMAL)==ERROR){
			printf("\n Command could not be sent to the queue\n");
			return;
	}

}

void createFile(parseinfo *csb){
	
   struct stat fileStat;
   char c;
   BOOL create_flag=FALSE;

   if(stat (GETOBJ(string,1), &fileStat)==ERROR){
	   create_flag=TRUE;
   }else{
		if (S_ISDIR (fileStat.st_mode))
			create_flag=TRUE;
   }

   if(!create_flag){
		printf("This file already exists.Do you want to overwrite [yes/no]  ");
		read(STD_IN,&c,1);
		write(STD_OUT,"\n",1);
		if(c!='y')
			return;
   }

	if(GETOBJ(string,1)){
		if(creat(GETOBJ(string,1),O_RDWR) == ERROR)
			printf("\n File %s could not be created\n",GETOBJ(string,1));
		else
			printf("\n File %s created\n",GETOBJ(string,1));

	}

}

void printFile(parseinfo *csb){

	char c;
    int ret,i=0;
	int fileFd;

	if(GETOBJ(string,1) == NULL){
		printf("\n File could not be opend\n");
		return;
	}

    fileFd = open(GETOBJ(string,1), O_RDONLY,0);
	if(fileFd == -1){
		printf("\n File %s could not be opend\n",GETOBJ(string,1));
		return;
	}
		
	while(1){
          ret = read(fileFd, &c, 1);
	      if ( ret <= 0 ) return;
		  write(STD_OUT,&c,1);
	}



}

/******** Load and save configuration ************/

void saveRunning (parseinfo *csb)
{
  parseinfo *nvcsb;
  char filePath[128];
  int fileFd,backTaskFd,backGlobalFd;

#ifdef MNET_GP10
  strcpy(filePath,getenv("VIPERCALL_BASE"));
#else
  strcpy(filePath,getenv("MNET_BASE"));
#endif
	printf("\n %s\n",filePath);
  strcat(filePath,"/vx/configfile");
  printf("\n %s\n",filePath);

  fileFd = open (filePath, O_WRONLY | O_CREAT, 0644);
  if (fileFd == -1)
  {	
		printf("\n Could not open the file \n");
		return;
  }
  
  nvcsb = get_csb("NV parser CSB");
  if (csb == NULL) {
    printf(nomemory);
    return;
  }
  printf("\nBuilding configuration...\n");
  nvcsb->nvgen = TRUE;
  nvcsb->sense = TRUE;
  
 

  backTaskFd = ioTaskStdGet  (0,STD_OUT);
  backGlobalFd = ioGlobalStdGet(STD_OUT);

  ioGlobalStdSet(STD_OUT,fileFd);
  ioTaskStdSet  (0,STD_OUT,fileFd);

  nvcsb->priv = PRIV_MAX;
  nvcsb->priv_set = 0xf;
  nvcsb->mode = config_mode;
  push_node(nvcsb, get_mode_nv(nvcsb));
  parse_token(nvcsb);
  free_csb(&nvcsb);
  
  ioGlobalStdSet(STD_OUT,backGlobalFd);
  ioTaskStdSet  (0,STD_OUT,backTaskFd);
  close(fileFd);
 }
void loadRunning( parseinfo *csb ){
	int fileFd;
	char filePath[128];
  

#ifdef MNET_GP10
  strcpy(filePath,getenv("VIPERCALL_BASE"));
#else
  strcpy(filePath,getenv("MNET_BASE"));
#endif
	strcat(filePath,"/vx/configfile");
 
	if((fileFd = open (filePath, O_RDONLY,0644))==-1){
		printf("\nConfig file could not be opend\n");
		return;
	}
	backInFd= csb->inFd;
	csb->inFd=fileFd;
	csb->mode=config_mode;
	load_running_config=TRUE;

}
