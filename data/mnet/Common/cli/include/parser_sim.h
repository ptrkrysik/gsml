#ifndef __PARSER_SIM_H
#define __PARSER_SIM_H

typedef struct parser_sim {
  tCOMSIM     comsim;               /* Generic Interaction Module */
  tOCTET      command[PARSEBUF];  /* Command to be processed    */  
  tOCTET      lastchar;           /* For Help Processing        */
  void        *index;             /* For More Processing        */
  tOCTET4     maxCount;           /* For More Processing        */
  parseinfo   *csb;               /* CSB of I/O Task */
} tPARSERSIM, *tpPARSERSIM;

        
#define mFILL_PARSER_SIM( pParserSim, Csb, origin, dest, service, request, \
                          cmd, Lastchar, Index, MaxCount ) \
        { \
                tpCOMSIM pComSim = &(pParserSim->comsim); \
                strcpy( (pParserSim->command), cmd );  \
                pParserSim->csb      = Csb;      \
                pParserSim->lastchar = Lastchar; \
                pParserSim->index    = Index;    \
                pParserSim->maxCount = MaxCount; \
                mFILL_COM_SIM( pComSim, origin, dest, service, request, NULL, 0, 0); \
        }
        

#define PARSER_CMD_PROC_REQ       0x3100
#define PARSER_CMD_PROC_IND       0x3200

#define IO_WAIT_COMPLETE_IND      0xF2FE
#define IO_CMD_COMPLETE_IND       0xF2FF

#endif /* __PARSER_SIM_H */
