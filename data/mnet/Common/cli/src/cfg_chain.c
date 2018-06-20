
#include <vxworks.h>
#include <stdioLib.h>
#include <tyLib.h>
#include <taskLib.h>
#include <errnoLib.h>
#include <lstLib.h>
#include <inetLib.h>    /* for INET_ADDR_LEN  */
#include <timers.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <msgQLib.h>
#include <sigLib.h>
#include <usrLib.h>
#include <bootLib.h>

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


#ifdef MNET_GP10
#define BOOT_LINE_OFFSET 0x4200
#else
#define BOOT_LINE_OFFSET 0x1200
#endif

#define LOCAL_MEM_LOCAL_ADRS 0x00000000
#define BOOT_LINE_ADRS ((char *) (LOCAL_MEM_LOCAL_ADRS+BOOT_LINE_OFFSET))

static BOOT_PARAMS 	pBootParams;



extern void ShowParserInit(void);
extern void ExecParserInit(void);
int sysUserGetUserLevelByName  (char*);
extern char current_user[];


#define  ALTERNATE NONE
#include "../include/vxworks_cfg_chain.h"
LINK_POINT(basic_cfg_command, ALTERNATE);
#undef  ALTERNATE


static parser_mode *config_boot;
static parser_extension_request basic_chain_init_table[] = {
    {PARSE_ADD_CFG_TOP_CMD, &pname(basic_cfg_command)},
	{PARSE_ADD_BOOT_PARAM, &pname(exec_boot_change_boot_device)},
    {PARSE_LIST_END, NULL}
};

static void cli_submode_nvgen (parseinfo *csb,parser_mode *mode)
{
  parseinfo *nvcsb;
  
  nvcsb = get_csb("NV parser CSB");
  if (csb == NULL) {
    printf(nomemory);
    return;
  }
  nvcsb->nvgen = TRUE;
  nvcsb->sense = TRUE;
  nvcsb->priv = PRIV_MAX;
  nvcsb->priv_set = 0xf;
  nvcsb->mode = mode;
  push_node(nvcsb, get_mode_nv(nvcsb));
  parse_token(nvcsb);
  free_csb(&nvcsb);
}

void init_boot_submode (void)
{ 
   config_boot=parser_add_mode("boot-params", "boot-params",
		    "boot Parameters", TRUE, TRUE, "configure",
		    NULL, NULL, &pname(exec_boot_change_boot_device),NULL);
 
}


extern void init_boot_submode(void);
void BasicParserInit(void)
{
         parser_add_command_list(basic_chain_init_table, "basic commands");
         ShowParserInit();
         ExecParserInit();
		 init_boot_submode();

}

/********* Call back functions ****************/



/******** Set host name ***************/
void HostName(parseinfo *csb){
        sethostname(GETOBJ(string,1),strlen(GETOBJ(string,1)));
}


/**************** Add new system user *************/
void AddNewUser(parseinfo *csb){
    char password[128];
    char confirm_password[128];
    STATUS retstat=ERROR;

	if(csb->nvgen)
		return;

	if(sysUserGetUserLevelByName(current_user)!=0){
		printf("\nRoot privillege required for this operation\n");
		return;
	}

    if(csb->sense){
          printf("Enter the password with minimum 8 characters\n");
 	  if(getInput( STD_OUT,STD_IN,"password :", password, 127, TRUE )==FALSE){
  	     printf("\n Could not read password \n");
             return;
          }
  	  if(getInput( STD_OUT,STD_IN,"\nconfirm password :", confirm_password, 127, TRUE )==FALSE){
  	     printf("\nCould not read password \n");
             return;
	  }
      if(strcmp(password,confirm_password)!=0)
      {
         printf("\nPasswords do not match\n");
         return;
      }
        retstat=sysUserAdd(GETOBJ(string,1),(char*)password,GETOBJ(int,1));
          if(retstat==ERROR)
             printf("\nUser could not be added\n");
          else
             printf("\nAdded user successfully\n");
         
    }else{
         retstat=sysUserDelete(GETOBJ(string,1));
      if(retstat==ERROR)
           printf("\nUser %s could not be deleted \n", GETOBJ(string,1));
      else
           printf("\nDeleted user successfully\n");
      
    } 
    
}

/**********Enable SSH access *********/
extern BOOL ssh_enable_flag;
extern BOOL telnet_enable_flag;
extern BOOL ssh_present;

void EnableSsh(parseinfo *csb){
	if(csb->nvgen){
		if(ssh_enable_flag)
			printf("\nssh enable\n");
		return ;
	}
	if(!ssh_present){
		printf("\nSSH is not supported\n");
		return;
	}

	if(csb->sense){
  		ssh_enable_flag=TRUE;   
     }else{
        if(!telnet_enable_flag)
	   printf("\nSSH and Telnet can't be disabled at same time\n");
	else
	   ssh_enable_flag=FALSE;
     }
  
}
/**********Enable telnet access *********/

void EnableTelnet(parseinfo *csb){
	if(csb->nvgen){
		if(!telnet_enable_flag)
			printf("\nno telnet enable\n");
		return ;
	}

     if(csb->sense){
  	telnet_enable_flag=TRUE;   
     }else{
        if(!ssh_enable_flag)
	   printf("\nTelnet and SSH can't be disabled at same time\n");
	else
	   telnet_enable_flag=FALSE;
     }
  
}
/*************** Change password for existing user ***************/
void ChangePasswd(parseinfo *csb){
    STATUS retstat=ERROR;
    char password[128];
    char new_password[128];
	
	 if(csb->nvgen)
		 return;
     
     if( (GETOBJ(string,1)==NULL) ){
          printf("\nPasswd could not be changed\n");
          return;
     }

	 if( (sysUserGetUserLevelByName(current_user)!=0) && strcmp(GETOBJ(string,1),current_user) !=  0 ){
	 	 printf("\nRoot privillege required for this operation\n");
		 return;
	 }
	 if(getInput( STD_OUT,STD_IN,"Old password :", password, 127, TRUE )==FALSE){
		 printf("\n Could not read password \n");
		 return;
	 }
     if(getInput( STD_OUT,STD_IN,"\nNew password :", new_password, 127, TRUE )==FALSE){
		    printf("\nCould not read password \n");
			return;
     }

     retstat=sysUserPasswdChange( GETOBJ(string,1),password,new_password );
     if(retstat==OK)
         printf("\n Passwd changed succesfully\n");
     else
        printf("\n Passwd could not be changed\n");

}         
extern BOOL vxsh_flag;
void EnableVxsh(parseinfo *csb){
	 
	if(csb->nvgen)
		return ;
	
  	if(csb->sense)
  		vxsh_flag = TRUE;
    else
		vxsh_flag = FALSE;

}



void readBootLine(void){
		char bootString[256];
		bootStringToStruct( (char *)BOOT_LINE_ADRS, &pBootParams);
		bootStructToString ( bootString, &pBootParams);
		strcpy((char *)BOOT_LINE_ADRS, bootString);
}
void setBootParamsMode(parseinfo *csb){

	if (csb->nvgen) {
			  printf("\nboot-params");
		      cli_submode_nvgen(csb,config_boot);
	}
	readBootLine();	
	set_mode_byname(&csb->mode,"boot-params",MODE_VERBOSE);
}

extern void sysNvRamSet(char *,int,int);

void changeBootParams(parseinfo *csb){
	char bootString[256];
	if(!GETOBJ(string,1) && !GETOBJ(int,1)){
		printf("\ncould not change the parameter\n");
	    return;
	}

	if (csb->nvgen) {
 	 		 bootStringToStruct( (char *)BOOT_LINE_ADRS, &pBootParams);

			 printf("\n\tboot_device  %s",pBootParams.bootDev);
			 printf("\n\tprocessor_number  %d",pBootParams.procNum);
			 printf("\n\tunit_number  %d",pBootParams.unitNum);
			 printf("\n\thost_name  %s",pBootParams.hostName);
			 printf("\n\tfile_name  %s",pBootParams.bootFile);
			 printf("\n\tinet_on_ethernet  %s",pBootParams.ead);
			 printf("\n\tinet_on_backplane  %s",pBootParams.bad);
			 printf("\n\tgateway_inet  %s",pBootParams.gad);
			 printf("\n\thost_inet  %s",pBootParams.had);
			 printf("\n\tuser  %s",pBootParams.usr);
			 printf("\n\tftp_password  %s",pBootParams.passwd);
			 printf("\n\ttarget_name  %s",pBootParams.targetName);
			 printf("\n\tflags  %s",pBootParams.flags);
			 printf("\n\tstartup_script  %s",pBootParams.startupScript);
			 printf("\n\tother  %s",pBootParams.other);
	}

	switch(csb->which){
		case EXIT_BOOT_MODE:
				bootStructToString ( bootString, &pBootParams);
		  		strcpy((char *)BOOT_LINE_ADRS, bootString);
				sysNvRamSet((char *)BOOT_LINE_ADRS, strlen (bootString) + 1, 0);
				set_mode_byname(&csb->mode,"configure",MODE_VERBOSE);
			    break;
		case BOOT_DEV:
			     strcpy(pBootParams.bootDev,GETOBJ(string,1));
				break;
		case UNIT_NUM:
				pBootParams.unitNum=GETOBJ(int,1);
				break;
		case PROC_NUM:
				pBootParams.procNum=GETOBJ(int,1);
				break;
		case HOST_NAME:
				strcpy(pBootParams.hostName,GETOBJ(string,1));
				break;
		case FILE_NAME:
				strcpy(pBootParams.bootFile,GETOBJ(string,1));
				break;
		case ETHER_IP:
				strcpy(pBootParams.ead,GETOBJ(string,1));
				break;
		case BACK_IP:
				strcpy(pBootParams.bad,GETOBJ(string,1));
				break;
		case HOST_IP:
				strcpy(pBootParams.had,GETOBJ(string,1));
				break;
		case GATEWAY_IP:
				strcpy(pBootParams.gad,GETOBJ(string,1));
				break;
		case USER_NAME:
				strcpy(pBootParams.usr,GETOBJ(string,1));
				break;
		case FTP_PASSWD:
				strcpy(pBootParams.passwd,GETOBJ(string,1));
				break;
		case TARGET_NAME:
				strcpy(pBootParams.targetName,GETOBJ(string,1));
				break;
		case FLAGS:
				pBootParams.flags=GETOBJ(int,1);
				break;
		case STARTUP_SCRIPT:
				strcpy(pBootParams.startupScript,GETOBJ(string,1));
				break;
        case OTHER:
				strcpy(pBootParams.other,GETOBJ(string,1));
			    break;
	}	
}
