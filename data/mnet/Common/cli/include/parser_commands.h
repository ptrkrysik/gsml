#ifndef __PARSER_COMMANDS_H__
#define __PARSER_COMMANDS_H__
 
extern boolean cfg_dirty( void );
extern void write_memory( parseinfo *);
extern void reload_command( parseinfo *);
extern void exit_config_command( parseinfo *);
extern void parser_help(parseinfo *);
 
extern void capsGetIPAddress( char * );
extern void capsGetIPGateway( char * );
extern void capsGetIPNetmask( char * );
extern void capsGetIPBroadcast( char * );


extern int isTelnetdEnabled( void );
extern int isHttpEnabled( void );
extern int isFtpdEnabled( void );
extern int isTftpdEnabled( void );
extern int isCronEnabled( void );
extern int isSyslogdEnabled( void );        
extern int isBootpdEnabled( void );
extern int isRcpdEnabled( void );
extern int getHttpPort( void );

#endif  /* __PARSER_COMMANDS_H_ */

