#ifndef __PARSER_UTIL_H__
#define __PARSER_UTIL_H__

#define FUNCARGS0(argArray) 
#define FUNCARGS1(argsArray) argsArray[0] 
#define FUNCARGS2(argsArray) FUNCARGS1((argsArray)) ## , ## \
                                                                           argsArray[1]
#define FUNCARGS3(argsArray) FUNCARGS2((argsArray)) ## , ## argsArray[2] 
#define FUNCARGS4(argsArray) FUNCARGS3((argsArray)) ## , ## argsArray[3] 
#define FUNCARGS5(argsArray) FUNCARGS4((argsArray)) ## , ## argsArray[4] 

#define ARGS(argsArray,numArgs) FUNCARGS##numArgs(argsArray) 

extern boolean null (const char *s);
extern char *deblank( const char *);
extern boolean yes_or_no( const char *, boolean, boolean );
extern boolean confirm( char *);

extern void parser_return(parseinfo *, int errno_val);

#endif  /* __PARSER_UTIL_H_ */
