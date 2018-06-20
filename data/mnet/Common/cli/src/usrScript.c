
#include "vxworks.h"
#include "stdio.h"
#include "ioLib.h"

/******************************************************************************
*
* usrStartupScript - make shell read initial startup script file
*
* This routine reads a startup script before the VxWorks
* shell comes up interactively.
*
* NOMANUAL
*/
/*****************************/
extern int __floatsisf;
extern int __divsf3 ;
extern int __mulsf3 ;
extern int __addsf3 ;



/****************************/

extern int consoleFd;

int x;


void cli_usrStartupScript 
    (
    char *fileName
    )
    {
    int old;
    int newStdIn,newStdOut;
/*******************************/
x=__floatsisf;
x=  __divsf3 ;
x= __mulsf3 ;
x= __addsf3 ;   
/**************************/
    if (fileName == NULL)
        return;

    newStdIn = open (fileName, O_RDONLY, 0);
    if (newStdIn != ERROR)
    { 
        printf ("Executing startup script debug %s ...\n", fileName);
        taskDelay (sysClkRateGet () / 2);
	

         
       	CliIOInit(newStdIn,consoleFd,consoleFd,FALSE);

        do
            taskDelay (sysClkRateGet ());
        while (taskNameToId ("tCli") != ERROR);
        
       
        close (newStdIn);
       
        

        printf ("\nDone executing startup script debug %s\n", fileName);
        taskDelay (sysClkRateGet () / 2);
    }
    else
        printf ("Unable to open startup script %s\n", fileName);

}


