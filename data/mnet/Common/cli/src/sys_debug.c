#include <vxworks.h>
#include <stdioLib.h>
#include <lstLib.h>
#include <stdlib.h>
#include <inetLib.h>
#include <timers.h>
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
#include <../include/debug.h>
#include <../include/sys_debug.h>

#define MAX_HEADING_LEN 32

static SEM_ID debugSemId; 
static LIST debugShowFuncList;
static LIST debugAllFuncList;
#if 0
/*
 * RAMS : commented this for now
 */
extern void show_debug_http_all(void);
extern void show_debug_https_all(void);
extern void show_debug_icp_all (void);
extern void show_debug_log_all (void);
extern void show_debug_translog_all (void);
extern void show_debug_radius_all (void);
extern void show_debug_authentication_all(void);
extern void show_debug_tacacs_all (void);
extern void show_debug_snmp_all (void);
extern void show_debug_stat_all(void);
extern void show_debug_wccp_all(void);
extern void show_debug_wi_all(void);
extern void show_debug_url_all(void);
extern void show_debug_sreg_all(void);
extern void show_debug_rule_all();
#endif

static tDEBUG_FUNC_NODE* findDebugNode( LIST *, void *, const debug_item_type *);

STATUS add_debug_node( LIST *list, void *func, 
                                           const debug_item_type *debug_items, 
                                           const char* heading ) 
{ 
        STATUS status = OK;
        tDEBUG_FUNC_NODE *funcNode = NULL;

        if ( PSsemTake(debugSemId, WAIT_FOREVER) == ERROR ) { 
                return(ERROR);
        }

        if ( findDebugNode( list, func, debug_items ) == NULL ) { 
                funcNode = 
                        ( tDEBUG_FUNC_NODE *)PARSERcalloc(1, sizeof(tDEBUG_FUNC_NODE));
                if ( funcNode  ) {
                        funcNode->debug_items = (debug_item_type *)debug_items;
                        funcNode->debugFunction = func;
                        if ( heading &&
                                (funcNode->heading = 
                                 (char *)PARSERmalloc(sizeof(char) * (MAX_HEADING_LEN + 1)))
                                 != NULL){
                                strncpy(funcNode->heading, heading, MAX_HEADING_LEN - 1);
                                funcNode->heading[MAX_HEADING_LEN] = '\0';
                        }

                        lstAdd( list, (NODE*)funcNode);
                }
        } 

        if ( PSsemGive(debugSemId) == ERROR ) { 
                status = ERROR;
        } 
        return ( status );
}

STATUS add_debug_show(DEBUG_SHOW_FUNCTION_PTR func, 
                                          const debug_item_type *debug_items,
                                          const char *heading )
{
        return(add_debug_node(&debugShowFuncList, 
                                                 (void *)func, debug_items,
                                                  heading));
}


STATUS add_debug_all(DEBUG_ALL_FUNCTION_PTR func, 
                                     const debug_item_type *debug_items )
{
        if ((add_debug_node(&debugAllFuncList, 
                                                 (void *)func, debug_items,
                                                  NULL)) == OK ) { 
                if ( func == NULL ) 
                        generic_debug_all(debug_items, debug_all_p());
                else
                        func(debug_all_p());

                return( OK );
        } else
                return(ERROR);
}

void invoke_debug_all(boolean debug_all)
{
        tDEBUG_FUNC_NODE *function;
        

        if ( PSsemTake(debugSemId, WAIT_FOREVER) == ERROR ) { 
                return;
        }

        function = (tDEBUG_FUNC_NODE *)lstFirst(&debugAllFuncList);

        while ( function ) { 
                if ( function->debugFunction == NULL ) { 
                        if ( function->debug_items ) 
                                generic_debug_all( function->debug_items, debug_all);
                } else { 
                        ((DEBUG_ALL_FUNCTION_PTR)function->debugFunction)(debug_all);
                }
                function = (tDEBUG_FUNC_NODE *)lstNext((NODE*)function);
        }
                
        if ( PSsemGive(debugSemId) == ERROR ) { 
                return;
        }
}

void invoke_debug_show(void)
{
        tDEBUG_FUNC_NODE *function;

        if ( PSsemTake(debugSemId, WAIT_FOREVER) == ERROR ) { 
                return;
        }

        function = (tDEBUG_FUNC_NODE *)lstFirst(&debugShowFuncList);

        while ( function ) { 
                if ( function->debugFunction == NULL ) { 
                        if ( function->debug_items ) 
                                generic_debug_show( function->debug_items, function->heading);
                } else { 
                        ((DEBUG_SHOW_FUNCTION_PTR)function->debugFunction)();
                }
                function = (tDEBUG_FUNC_NODE *)lstNext((NODE*)function);
        }
#if 0
        /*
         * RAMS : commented this for now
         */
        show_debug_http_all();
        show_debug_https_all();
        show_debug_icp_all();
        show_debug_log_all();
        show_debug_translog_all();
        show_debug_radius_all();
        show_debug_authentication_all();
        show_debug_tacacs_all();
        show_debug_snmp_all();
        show_debug_stat_all();
        show_debug_wccp_all();
        show_debug_wi_all();
        show_debug_url_all();
        show_debug_sreg_all();
        show_debug_rule_all();
        
#endif  
        if ( PSsemGive(debugSemId) == ERROR ) { 
                return;
        }
}

void sys_debug_init(void)
{
        debugSemId   = PSsemMCreate( PS_DEFAULT_MUTEX_SEM_FLAG ) ;
        
        if ( debugSemId == NULL ) { 
                PSpanic("Cannot create Debug Semaphore\n");
        }
        lstInit(&debugShowFuncList);
        lstInit(&debugAllFuncList);
}

static tDEBUG_FUNC_NODE* findDebugNode( LIST *list, void *func,
                                                                                const debug_item_type *debug_items ) 
{
        tDEBUG_FUNC_NODE *pNode = NULL;
        boolean locateFunc = TRUE;

        if (list  == NULL ) 
                return(NULL);

        if ( func == NULL ) { 
                if ( debug_items == NULL ) 
                        return( NULL );

                locateFunc = FALSE;
        }

        pNode = (tDEBUG_FUNC_NODE *)lstFirst(list);
        
        while( pNode ) { 
                if ( locateFunc ) { 
                        if ( pNode == func )
                                return( pNode );
                } else if ( pNode->debug_items == debug_items ) { 
                        return ( pNode );
                }       
                pNode = (tDEBUG_FUNC_NODE *)lstNext((NODE*)pNode);
        }

        return(NULL);
}

STATUS remove_debug_node(LIST *list, void *func, 
                                                 const debug_item_type *debug_items )
{
        tDEBUG_FUNC_NODE *pNode = NULL;

        if ( PSsemTake(debugSemId, WAIT_FOREVER) == ERROR ) { 
                return(ERROR);
        }

        if ( (pNode = findDebugNode(list, func, debug_items)) == NULL )
                return(ERROR);

        lstDelete(list, (NODE *)pNode);
        PARSERfree(pNode);

        if ( PSsemGive(debugSemId) == ERROR ) { 
                return(ERROR);
        }
        return(OK);

}

STATUS remove_debug_all( DEBUG_ALL_FUNCTION_PTR func, 
                                             const debug_item_type *debug_items )
{
        return(remove_debug_node(&debugAllFuncList, (void*)func, debug_items));
}

STATUS remove_debug_show( DEBUG_SHOW_FUNCTION_PTR func, 
                                             const debug_item_type *debug_items )
{
        return(remove_debug_node(&debugShowFuncList, (void*)func, debug_items));
}


