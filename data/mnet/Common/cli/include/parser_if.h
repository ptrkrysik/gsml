#ifndef __PARSER_IF_H__
#define __PARSER_IF_H__


#define CLI_MODULE_ID 100
#define CLI_MAX_STRING_LEN 128

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

static long nvgen_int;
static char nvgen_string[CLI_MAX_STRING_LEN+1];

#define SET_INT_MIB(mibtag,obj,def,string) if (csb->sense) {\
                   if (oam_setMibIntVar(CLI_MODULE_ID,mibtag,obj) !=STATUS_OK)\
                    printf(string);\
              } else {\
                   if (oam_setMibIntVar(CLI_MODULE_ID,mibtag,def) !=STATUS_OK)\
                       printf(string);\
              }

#define NVGEN_INT_MIB(mibtag,string) if (csb->nvgen) {\
                   if (oam_getMibIntVar(mibtag,&nvgen_int) !=STATUS_OK) {\
                     printf(string);\
                   } else {\
                     printf("\n %s %d",csb->nv_command,nvgen_int);\
                   }\
                   return;\
               }

#define NVGEN_IP_MIB(mibtag,string) if (csb->nvgen) {\
                   if (oam_getMibIntVar(mibtag,&nvgen_int) !=STATUS_OK) {\
                     printf(string);\
                   } else {\
                      address_string(nvgen_int,nvgen_string);\
                      printf("\n %s %s",csb->nv_command,nvgen_string);\
                   }\
                   return;\
               }

#define NVADDGEN_INT_MIB(mibtag,string,first,last) if (csb->nvgen) {\
                   if (oam_getMibIntVar(mibtag,&nvgen_int) !=STATUS_OK) {\
                     printf(string);\
                   } else {\
                      if (first)\
                        printf("\n %s %d",csb->nv_command,nvgen_int);\
                      else\
                        printf(" %d",nvgen_int);\
                   }\
                   if (last)\
                        return;\
               }

#define NVGEN_STRING_MIB(mibtag,string) if (csb->nvgen) {\
                   if (oam_getMibByteAry(mibtag,nvgen_string,CLI_MAX_STRING_LEN) !=STATUS_OK) {\
                     printf(string);\
                   } else {\
                      printf("\n %s %s",csb->nv_command,nvgen_string);\
                   }\
                   return;\
               }

#define NVADDGEN_STRING_MIB(mibtag,string,first,last) if (csb->nvgen) {\
                   if (oam_getMibByteAry(mibtag,nvgen_string,CLI_MAX_STRING_LEN) !=STATUS_OK) {\
                     printf(string);\
                   } else {\
                      if (first)\
                        printf("\n %s %s",csb->nv_command,nvgen_string);\
                      else\
                        printf(" %s",nvgen_string);\
                   }\
                   if (last)\
			return;\
               }


#define NVGEN_BOOL_MIB(mibtag,string) if (csb->nvgen) {\
                   if (oam_getMibIntVar(mibtag,&nvgen_int) !=STATUS_OK) {\
                     printf(string);\
                   } else {\
                     if (nvgen_int)\
                      printf("\n %s enable",csb->nv_command);\
		     else\
	              printf("\n no %s enable",csb->nv_command);\
                   }\
                   return;\
               }

#define NVGEN_TIMER_MIB(mibtag,timer) if (csb->nvgen) {\
                   oam_getMibIntVar(mibtag,&nvgen_int);\
	           printf("\n\t %s %s %d",csb->nv_command,timer,nvgen_int);\
                   return;\
               }

#define NVADDGEN_TIMER_MIB(mibtag,timer) if (csb->nvgen) {\
                   oam_getMibIntVar(mibtag,&nvgen_int);\
	           printf("\n\t %s %s %d",csb->nv_command,timer,nvgen_int);\
               }

#endif
