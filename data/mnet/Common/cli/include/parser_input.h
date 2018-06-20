#ifndef __PARSER_INPUT_H__
#define __PARSER_INPUT_H__


/* This should be >= MAX_TELNET_SESSIONS in telnetLib.c
 * so that all telnet sessions can monitor
 */
#define MAX_FDS 5

#define LINES          24
#define LENGTH_DEFAULT LINES

#define MAX_USERNAME_LEN  256
#define MAX_PASSWD_LEN    256
#define MAX_RETRIES       3
#define MAXHOSTNAMELEN    64


#define DEFAULT_EXEC_TIMEOUT ( 10 * 60 )

#define IO_SIGINT            0x01
#define IO_EXEC_TIMEOUT      0x02
#define IO_SIGFLAGS_DELETED  0x00001000
#define MAX_HISTORY    5

typedef struct ioInfo {
        parseinfo   *csb;
    tCSEMAPHORE ioSemId;
    tUOCTET4     ioModuleId;
    tUOCTET4     requestId;
    tUOCTET4     flags;
        tUOCTET4     sockFd;
        tpOCTET      execHistory[MAX_HISTORY];
        tpOCTET      configHistory[MAX_HISTORY];
} tIOINFO,*tpIOINFO;
 
typedef enum { 
        IO_RAW_MODE = 0,
        IO_LINE_MODE,
        IO_ECHO_MODE
} tTERMINALMODE;

/* Extern Declarations */
extern tCSEMAPHORE initSemId;
extern int consoleFd;
extern tINT fdArray[MAX_FDS];
extern boolean cgipipeDone;
extern tINT cgiStatus;
extern tINT cgiMode;
/*extern tUID cgiUid;*/
extern char usrPrompt[];
extern char passwdPrompt[];
extern char loginErrMsg[];

extern tOCTET2 registerFd( tINT );
extern tOCTET2 unregisterFd( tINT );
/*extern void io_work(tINT, tINT, boolean, tUID, SEM_ID, boolean );*/
extern void cgi_io_work(tINT, tINT );
extern tOCTET2 io_more_buffer( tINT, char *, time_t );
extern void moreFileOrBuffer( int , parseinfo *, int, boolean , time_t );

extern tINT ioSetTerminalMode( int, tTERMINALMODE ) ;
extern void   show_techsupport( parseinfo *) ;
extern void startIOTimer( parseinfo *csb );
extern void stopIOTimer( parseinfo *csb );
extern STATUS setExecTimeout(parseinfo *csb );


extern FUNCPTR cli_logoutf;
extern int cli_logVar;
extern int ShellTask;
extern int SSHTask;
extern int installedVxShell;
extern int TelnetTask;
extern int ConsoleTask;
extern boolean ExitFlag;
extern int infd;
extern int outfd;
extern BOOL defaultUserInUse;

#define PARSE_ADD_BOOT_PARAM 50
#define PARSE_ADD_VXSH_CHAIN 51

#endif  /* __PARSER_INPUT_H_ */
