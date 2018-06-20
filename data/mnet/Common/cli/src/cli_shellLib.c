/**********  File contains  APIs used by telnet server and VxWorks to start or destroy the shell ***/

#include "vxWorks.h"                    /* always first */
#include <taskLib.h>
#include "stdio.h"
#include "ioLib.h"
#include  "symLib.h"
#include  "string.h"
#include  "loadLib.h"
#include  "usrLib.h"
#include   "sysLib.h"

#include <../include/psvxif.h>
/******* Global variables **********/

char    *shellTaskName="tShell";
char    *ios_shellTaskName="tShell";

int     shellTaskId;
int     ios_shellTaskId;

int 	myShellTaskId;
int 	myFd;
int 	outfd,errfd,infd;
BOOL ssh_enable_flag=FALSE;
BOOL telnet_enable_flag=TRUE;
BOOL ssh_present =FALSE;


int ConsoleTask=0;
int TelnetTask=1;
int SSHTask   =2;

int ShellTask=0;

FUNCPTR cli_logoutf=NULL;
int cli_logVar=0;

SEM_ID cli_semM;
BOOL isLoggedOut=FALSE;
extern	SYMTAB_ID	sysSymTbl;
extern BOOL ServerBreak;

int ParserCallBackTask[3];

/******* external variables **********/
extern int consoleFd;              /* fd of initial console device */
extern int CliIOInit (int,int,int,BOOL);
extern int  SysCommandCli(void) ;

void logmsg(char* string);
void logerr(char* string);
void PrintMsg(char* Msg);
int logfd;









enum {
	VX_SHELL,
	IOS_SHELL
};

static currentShell=IOS_SHELL;

BOOL    ios_shellLock(BOOL request);
STATUS  ios_execute (char *line);
STATUS  ios_shellInit (int stackSize, int arg);
void    ios_shell (BOOL interactive);
void    ios_shellHistory (int size);
void    ios_shellLoginInstall (FUNCPTR logRtn, int logVar);
void    ios_shellLogout (void);
void    ios_shellLogoutInstall (FUNCPTR logRtn, int logVar);
void    ios_shellOrigStdSet (int which, int fd);
void    ios_shellPromptSet (char *newPrompt);
void    ios_shellRestart (BOOL staySecure);
void    ios_shellRestart (BOOL staySecure);



BOOL    shellLock(BOOL request);
STATUS  execute (char *line);
STATUS  shellInit (int stackSize, int arg);
void    shell (BOOL interactive);
void    shellHistory (int size);
void    shellLoginInstall (FUNCPTR logRtn, int logVar);
void    shellLogout (void);
void    shellLogoutInstall (FUNCPTR logRtn, int logVar);
void    shellOrigStdSet (int which, int fd);
void    shellPromptSet (char *newPrompt);
void    shellRestart (BOOL staySecure);
void    shellRestart (BOOL staySecure);




enum {
	SHELLLOCK_ID,
	EXECUTE_ID,
	SHELLINIT_ID,
	SHELL_ID,
	SHELLHISTORY_ID,
	SHELLLOGININSTALL_ID,
	SHELLLOGOUT_ID,
	SHELLLOGOUTINSTALL_ID,
	SHELLORIGSTDSET_ID,
	SHELLPROMPTSET_ID,
	SHELLRESTART_ID,
	SHELLENDOFLIST_ID
};

typedef struct _stdShellVector {
	char  *shellTaskName;
	int	shellTaskId;
	/*
	void	*shellLock;
	void  *execute;
	void  *shellInit;
	void  *shell;
	void  *shellHistory;
	void  *shellLoginInstall;
	void  *shellLogout;
	void  *shellLogoutInstall;
 	void  *shellOrigStdSet;
	void  *shellPromptSet;
	void  *shellRestart;
	void  *shellendOfList;
	*/
	void  *symValue[12];

}stdShellVector;

#ifdef MNET_GP10
char  *stdShellNames[] = {
	"shellLock",
	"execute",
	"shellInit",
	"shell",
	"shellHistory",
	"shellLoginInstall",
	"shellLogout",
	"shellLogoutInstall",
 	"shellOrigStdSet",
	"shellPromptSet",
	"shellRestart",
	0
};
#else
char  *stdShellNames[] = {
	"_shellLock",
	"_execute",
	"_shellInit",
	"_shell",
	"_shellHistory",
	"_shellLoginInstall",
	"_shellLogout",
	"_shellLogoutInstall",
 	"_shellOrigStdSet",
	"_shellPromptSet",
	"_shellRestart",
	0
};

#endif

static stdShellVector iosShellVector = 
{
	"tShell",
	0,
	{
		ios_shellLock,
		ios_execute,
		ios_shellInit,
		ios_shell,
		ios_shellHistory,
		ios_shellLoginInstall,
		ios_shellLogout,
		ios_shellLogoutInstall,
 		ios_shellOrigStdSet,
		ios_shellPromptSet,
		ios_shellRestart,
		0
	}
};

static stdShellVector defaultShellVector = 
{
	"tShell",
	0,
	{
		shellLock,
		execute,
		shellInit,
		shell,
		shellHistory,
		shellLoginInstall,
		shellLogout,
		shellLogoutInstall,
 		shellOrigStdSet,
		shellPromptSet,
		shellRestart,
		0
	}
};

static stdShellVector vxShellVector;
static stdShellVector  runShellVector=
{
	"tShell",
	0,
	{
		ios_shellLock,
		ios_execute,
		ios_shellInit,
		ios_shell,
		ios_shellHistory,
		ios_shellLoginInstall,
		ios_shellLogout,
		ios_shellLogoutInstall,
 		ios_shellOrigStdSet,
		ios_shellPromptSet,
		ios_shellRestart,
		0
	}
};




int installedVxShell=0;
static	void	*vxShellSymValue;

BOOL	stdFindVxShellSymbol( char *name, int val, SYM_TYPE type, int arg, UINT16 group)
{
	if ( strcmp(name, (char *) arg) == 0 )
	{
		/*printf("found symbol %s in group %d\n", (char *)arg, (int) group);*/
		if ( group != 0 )
		{
			vxShellSymValue=(void *)val;
			return FALSE;
		}
	}
	return TRUE;
}


int	 shellLoadVxShell(char *file)
{
	MODULE_ID ret;
	int next=0;


	ret=ld(0,0,file);
	if(ret==NULL){
       		return 0;
        }

	next=0;
	while(stdShellNames[next]!=0){
		vxShellSymValue=0;
		symEach(sysSymTbl, stdFindVxShellSymbol, (int) stdShellNames[next]);
		if(vxShellSymValue == 0)
			return 0;
		vxShellVector.symValue[next]=vxShellSymValue;
		next++;
	}
	installedVxShell=1;
      	return 1;
}

int isInstalledVxShell()
{
	return installedVxShell;
}

void	shellSetRunning(int shell)
{
	currentShell=shell;
	if(shell == VX_SHELL)
		runShellVector=vxShellVector;
	else
		runShellVector=iosShellVector;
}

int   shellGetRunning()
{
	return currentShell;
}

void    shellScriptAbort (void){
        logmsg("shellScriptAbort\n");;
}

BOOL shellLock(BOOL request){
	BOOL (*f)();
	
	f=runShellVector.symValue[SHELLLOCK_ID];
	return (*f)(request);
}

STATUS  execute (char *line){

	STATUS  (*f)();
	
	f=runShellVector.symValue[EXECUTE_ID];
	return (*f)(line);	

}

void *shellGetVxExecute()
{

	return vxShellVector.symValue[EXECUTE_ID];

}

STATUS  shellInit (int stackSize, int arg){

	STATUS  (*f)();
	
	f=runShellVector.symValue[SHELLINIT_ID];
	return (*f)(stackSize, arg);	

}
void    shell (BOOL interactive){
	
	void  (*f)();
	
	f=runShellVector.symValue[SHELL_ID];
	(*f)(interactive);	
}

void    shellHistory (int size){
	
	void  (*f)();
	
	f=runShellVector.symValue[SHELLHISTORY_ID];
	(*f)(size);

}

void    shellLoginInstall (FUNCPTR logRtn, int logVar){

	void  (*f)();

	f=runShellVector.symValue[SHELLLOGININSTALL_ID];
	(*f)(logRtn, logVar);

}

void    shellLogout (void){

	void  (*f)();

	f=runShellVector.symValue[SHELLLOGOUT_ID];
	(*f)();	
}

void    shellLogoutInstall (FUNCPTR logRtn, int logVar){

	void  (*f)();

	f=runShellVector.symValue[SHELLLOGOUTINSTALL_ID];
	(*f)(logRtn, logVar);

}

void    shellOrigStdSet (int which, int fd){

	void  (*f)();

	f=runShellVector.symValue[SHELLORIGSTDSET_ID];
	(*f)(which, fd);
	
}

void    shellPromptSet (char *newPrompt){

	void  (*f)();

	f=runShellVector.symValue[SHELLPROMPTSET_ID];
	(*f)(newPrompt);
}

void    shellRestart (BOOL staySecure){

	void  (*f)();

	f=runShellVector.symValue[SHELLRESTART_ID];
	(*f)(staySecure);
}



extern BOOL ExitFlag;

void callbackRestart(){

	   int callbackTaskID;
/*	   if(ServerBreak || ExitFlag) 
			return;*/
   	   if(ServerBreak)
			return;

	   
	   if((callbackTaskID=taskNameToId ("tCallback0") )!= ERROR){
			  /*printMsg("\nTask  0 deleted\n");*/
			  taskDelete(callbackTaskID);
		}
		if((callbackTaskID=taskNameToId ("tCallback1") )!= ERROR){
			 /* printMsg("\nTask  1 deleted\n");*/
			  taskDelete(callbackTaskID);
		}
		if((callbackTaskID=taskNameToId ("tCallback2") )!= ERROR){
			 /* printMsg("\nTask  2 deleted\n");*/
			  taskDelete(callbackTaskID);
		}
		ParserCallbackInit();
}
   

/*****************Get the status of shell *********************/

int shellGetCurrentStatus(char* status){
	int retVal=0;
	switch(ShellTask){
		case 0 :
			retVal=1;
			if(status)
				strcpy(status,"\nSystem is being accessed via Console connection\n");
			break;
		case 1 :
			retVal=2;
			if(status)
				strcpy(status,"\nSystem is being accessed via Telnet connection\n");
			break;
		case 2 :
			retVal=3;
			if(status)
				strcpy(status,"\nSystem is being accessed via SSH connection\n");
			break;
		default :
			retVal=0;
			if(status)
				strcpy(status,"\nThere is no active CLI user in the system\n");
			break;
	}
	return retVal;
}



/******** Locks the access to shell *********/
BOOL    ios_shellLock (BOOL request){

	
 
	if(request==FALSE) { 
		ShellTask=ConsoleTask;
		callbackRestart();
		if(ServerBreak){
			shellRestart(TRUE);
			ServerBreak=FALSE;
		}
		return TRUE;
	}else{
		if(!telnet_enable_flag)
			return FALSE;

		if(ShellTask==SSHTask){
			return FALSE;
		}else{ 
			callbackRestart();
			ShellTask=TelnetTask;
			return TRUE;
		}
	}
}

/**************/
STATUS  ios_execute (char *line){
         logmsg("shellexecute\n");
        return OK;
 }



extern void ParserCallbackRestart(int index);
int tShell()
{
   	char StatusOfTask[128];
	int index;
        for(;;){
	/*		for(index=0;index<3;index++){
	           if(taskStatusString(ParserCallBackTask[index],StatusOfTask)==ERROR)
					ParserCallbackRestart(index);
				   					
		
		}*/
                taskDelay (sysClkRateGet ()*10);
        }
        return OK;
}




/*****************Shellinit******************
Creates shell task for console
*********************************************/

STATUS  ios_shellInit (int stackSize, int arg){

        char taskName[100];

       logerr("\nshellinit");
     
	
       shellTaskId = taskSpawn("tShell", 200, 0, 10000,
                                 (FUNCPTR)tShell, 
                                  0,
                                  0,                      
                                  0,          
                                  0,  
                                  0,
                                  0,0,0,0,0);
        
        
        sprintf(taskName, "shellInit - %d\n", shellTaskId);
        logmsg(taskName);

        outfd=errfd=infd=consoleFd;
        ShellTask=ConsoleTask;

        myShellTaskId=CliIOInit(infd,outfd,errfd,TRUE);
        return  OK;
 }

/*****************Shell******************/

void    ios_shell (BOOL interactive){
         logmsg("shelllogout\n");
 }



/*****************ShellHistory******************/
void    ios_shellHistory (int size){
        logmsg("shellHistory\n");;
}

static FUNCPTR loginf;




/*****************shellLoginInstall******************/
void    ios_shellLoginInstall (FUNCPTR logRtn, int logVar){
;        
}




/*****************shellLogout******************/
void    ios_shellLogout (void){
        myFd=consoleFd;
        logmsg("shellLogout\n");
        logerr("\nshellLogout\n");
       
}





/*****************shellLogoutInstall******************/
void    ios_shellLogoutInstall (FUNCPTR logRtn, int logVar){
                
        logmsg("shellLogoutInstall\n");
        cli_logoutf=logRtn;
        cli_logVar=logVar;
        logerr("\nshellLogoutInstall\n");
}

/*****************shellOrigStdSet******************/
void    ios_shellOrigStdSet (int which, int fd){

        logmsg("shellOrigStdSet\n");
	logerr("\nshellOrigStdSet\n");
        if(which == STD_OUT) outfd=fd;
        if(which == STD_IN) infd=fd;
        if(which == STD_ERR) errfd=fd;
}




/*****************shellPromptSet******************/

void    ios_shellPromptSet (char *newPrompt){
        logmsg("shellPromptSet\n");
       
}


extern void io_cleanup(   int TaskID);

/*****************shellRestart******************/
void    ios_shellRestart (BOOL staySecure){
        
	
		if((myShellTaskId=taskNameToId ("tCli") )!= ERROR){	
				taskDelete(myShellTaskId);
				io_cleanup(0);
		}
	    myShellTaskId=CliIOInit(infd,outfd,errfd,TRUE);
        switch(ShellTask){
		case 0 :
			printMsg("\nTask for console  created\n");
			break;
		case 1 :
			printMsg("\nTask for telnet session created\n");
			break;
		case 2 :
			printMsg("\nTask for SSH session created\n");
			break;	
	}
	
}

/*****************shellScriptAbort******************/
void    ios_shellScriptAbort (void){
        logmsg("shellScriptAbort\n");;
}

BOOL isSSHEnabled(void){
     return ssh_enable_flag;
}



BOOL SSHLock(void){
/*	  
      if(isLoggedOut==FALSE){
             isLoggedOut=TRUE;
             if(cli_logoutf){
				isLoggedOut=FALSE;
		   	    (*cli_logoutf)(cli_logVar);
			 }
             else{
                 isLoggedOut=FALSE;
             }
     }
   	 

      logerr("\nLockForSSH exit\n");*/
	
	if(cli_logoutf){
			  (*cli_logoutf)(cli_logVar);
	}
	

           
      return TRUE;
}

void LockForSSH(BOOL lock){
    callbackRestart();
	if(!lock){
		ShellTask=ConsoleTask;
		if(ServerBreak){
  			shellRestart(TRUE);
			ServerBreak=FALSE;
		}
	return;
    }
	
    if(ShellTask!=TelnetTask){	
		ShellTask=SSHTask;
		return;
    }

isLoggedOut=TRUE;
/******************* If and only if ssh is preempting the telnet ****/
    taskSpawn("tSSHLockTask", 150, 0, 10000,
                       (FUNCPTR)SSHLock, 
                        0,0,0,0,
                         0,
                       0,0,0,0,0);
    do
         taskDelay (sysClkRateGet ());
    while (taskNameToId ("tSSHLockTask") != ERROR);
	
    ShellTask=SSHTask;
}




void SshPresent(void){
        ssh_present=TRUE;
}
      
void printMsg(char* Msg){

write(consoleFd,Msg,strlen(Msg));

}

void logmsg(char* string){
     return;   
write(consoleFd,string,strlen(string));
}

void logerr(char* string){
return;
     write(consoleFd,string,strlen(string));
}



      
