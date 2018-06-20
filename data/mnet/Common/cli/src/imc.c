#include <vxworks.h>
#include <stdioLib.h>
#include <taskLib.h>
#include <lstLib.h>
#include <inetLib.h>    /* for INET_ADDR_LEN */
#include <stdlib.h>
#include <string.h>


#include <../include/pstypes.h>
#include <../include/psvxif.h>
#include <../include/platdef.h>
#include <../include/imsg.h>
#include <../include/imc.h>

/*
 ** EXTERNAL VARIABLES 
 */

/*
 ** GLOBAL VARIABLES
 */
LIST            *pimcListHead = NULL;         /* Pointer to IMC ListHead */
tUOCTET4        freeIndex = LAST_MODULE + 1;  /* Free Index for dynamic IMC */
SEM_ID          freeIndexSemId;               /* Protect freeIndex */
tpIMC_QANCHOR   pimcglp[MAX_IMC_COUNT];       /* Array for Indexing */
boolean         imcInited = FALSE;
tOCTET4         imc_global_count = 0;         /* keeps track of the TOTAL number
                                                 of entries in the imc */
/*
 ** STATIC VARIABLES
 */

/*
 **
 **     Prototypes
 **
 */
static tOCTET2  imc_cleanup( tOCTET4 );

#ifndef USE_IMC_MACROS
static LIST* imc_get_queue( tOCTET4 , IMC_QUEUE_TYPE );
static void imc_change_global_count( tOCTET2 );
tOCTET2 imc_post_sim( tpCOMSIM , IMC_QUEUE_TYPE );
void imc_get_sim( tpCOMSIM *, tUOCTET4 , IMC_QUEUE_TYPE );
#endif

/*COMMENTS*********************************************************
 *
 *   Func:  imc_init
 *
 *   Desc:  Initialize the IMC Anchor and global count Semaphore
 *
 *   Inputs: None
 *
 *   Outputs: RC_OK
 *
 *
 *END*********************************************************************/

tOCTET2 imc_init( void )
{
        if ( !imcInited ) { 
        tUOCTET2 index = 0;
                imcInited = TRUE;
        
         /* Create global Mutex Semaphores */
        
                imc_global_count = 0;
        
         freeIndexSemId = PSsemMCreate ( PS_DEFAULT_MUTEX_SEM_FLAG | 
                                                                         SEM_DELETE_SAFE);
        
         if ( freeIndexSemId == NULL ) {
             printf("imc_init:Could not create Free index semaphore\n");
                        return RC_ERR;
         }
        
         /* The head of the Anchor Block List */
         if ( ( pimcListHead = (LIST*)IMCcalloc(1,sizeof(LIST))) == NULL ) {
             printf("imc_init:Could not create global list\n");
                        PSsemDelete ( freeIndexSemId );
                        return RC_ERR;
         }
                 
        lstInit(pimcListHead);

        for ( index = 0; index < MAX_IMC_COUNT ; index++ )
                pimcglp[index] = NULL;
        }
        return ( RC_OK );
}


/*COMMENTS*********************************************************
 *
 *   Func:  imc_add
 *
 *   Desc:  Dynamically allocate IMC for ** SUBTASKS **
 *          Well known modules * MUST * announce their Module ID 
 *          as *pIndex whereas SUBTASKS send it as ASSIGN_MODULE_ID 
 *          (-1 )
 *   Inputs: tUOCTET
 *
 *   Outputs: RC_OK / RC_ERR
 *
 *
 *END*********************************************************************/

tOCTET2 imc_add( tpOCTET4 pIndex, tSEMAPHORE semId, tTASKID taskId )
{
    
    
    if ( PSsemTake ( freeIndexSemId, WAIT_FOREVER ) == ERROR ) {
                panic("imc_add:Could not take free index semaphore\n");
                return ( RC_ERR );
    }
    
    {
        tUOCTET4 origIndex = 0;
        tpIMC_QANCHOR pimcElement = NULL;
    
        if ( pIndex == NULL ) {
            printf("imc_add:NULL Index passed to imc_add\n");
            PSsemGive( freeIndexSemId );
            return (RC_ERR);
        }

    
        if ( *pIndex == ASSIGN_MODULE_ID ) {
        
            origIndex = freeIndex;
            freeIndex %= MAX_IMC_COUNT;
            freeIndex =  ( freeIndex == 0 ) 
                ? (LAST_MODULE + 1) 
                    : freeIndex;

            while ( pimcglp[freeIndex] != NULL )  {
                freeIndex++;
                freeIndex %= MAX_IMC_COUNT;

                /* If wrap-around, do not assign any "Well-known" 
                 * Module Ids
                 */
                freeIndex =  ( freeIndex == 0 ) 
                    ? (LAST_MODULE + 1) 
                        : freeIndex;

                /* Make sure that we have space to add ! */
                if ( origIndex == freeIndex ) {
                    printf("imc_add:Could not add new IMC\n");
                    PSsemGive ( freeIndexSemId );
                    return (RC_ERR);
                }
                
            }
            *pIndex = freeIndex;
        }

        if ( pimcglp[*pIndex] != NULL ) { 
                printf("IMC for module %ld exists!\n", *pIndex );
            return (RC_ERR);
        }
    
        pimcElement = (tpIMC_QANCHOR) IMCmalloc( sizeof( tIMC_QANCHOR ) );
        memset( (void *)pimcElement, 0, sizeof(tIMC_QANCHOR));
        if ( pimcElement == NULL ) {
            printf("imc_add:Calloc failed from IMC Element\n");
            PSsemGive ( freeIndexSemId );
            return (RC_ERR);
        }
    
        pimcElement->taskId = taskId;
        pimcElement->semId = semId;
        pimcElement->workCount = 0;
        lstInit(&(pimcElement->pimc_req ));
        lstInit(&(pimcElement->pimc_ind ));
        lstInit(&(pimcElement->pimc_exp_req ));
        lstInit(&(pimcElement->pimc_exp_ind ));

        pimcglp[*pIndex]  = pimcElement;

        lstAdd( pimcListHead, &(pimcElement->imc_element) ); 
    
        if ( PSsemGive ( freeIndexSemId ) == ERROR ) {
            panic ("imc_add:Could not give free index semaphore\n");
            return ( RC_ERR );
        }
    
        return ( RC_OK );
    }    
} /* end imc_init */



/*COMMENTS*********************************************************
 *
 *   Func:  imc_remove
 *
 *   Desc:  Dynamically de-allocate IMC for ** SUBTASKS **
 *
 *   Inputs: tUOCTET4
 *
 *   Outputs: RC_OK / RC_ERR
 *
 *END*********************************************************************/

tOCTET2 imc_remove( tUOCTET4 index )
{ 

    if  ( PSsemTake ( freeIndexSemId, WAIT_FOREVER ) == ERROR ) {
                panic("imc_remove:Could not take free index semaphore\n");
                return ( RC_ERR );
    }
 
    if ( pimcglp[index] == NULL ) {
                printf("Cannot find IMC Entry\n");
                PSsemGive ( freeIndexSemId );
                return (RC_ERR);
    }
 
    imc_cleanup( index );

    if ( index > LAST_MODULE )
                freeIndex = index;

    pimcglp[index] = NULL;

    if ( PSsemGive ( freeIndexSemId ) == ERROR ) {
                panic ( "imc_remove:Could not give free index semaphore\n");
                return ( RC_ERR );
    }

    return ( RC_OK );
        
} /* imc_remove */


/*COMMENTS*********************************************************
 *
 *   Func:  imc_term
 *
 *   Desc:  Release imc queues 
 *
 *   Inputs: None.
 *
 *   Outputs: RC_OK
 *
 *
 *END*********************************************************************/

void imc_term()
{
    /* We assume that each layer has cleaned up their
       interactions without generating any more */
    int index = 0;

    /* Deque & Cleanup elements from the lists */
    /* in the anchor block */
    for ( index = 0; index < MAX_IMC_COUNT ; index++ ){
        if ( pimcglp[index] != NULL ) {
            imc_cleanup( index );
        }
    }

    lstFree( pimcListHead );

    PSsemDelete( freeIndexSemId );

}/*end imc_term */

/*COMMENTS*********************************************************
 *
 *   Func:  imc_cleanup
 *
 *   Desc:  Dequeue elements, if any,  from IMC lists.
 *
 *   Inputs: index to the global array.
 *
 *   Outputs: RC_OK
 *
 *
 *END*********************************************************************/
tOCTET2 imc_cleanup ( tOCTET4 index )
{
    /* This is already protected by freeIndexSemId */
    /* Hence no need to Mutex this section */

    NODE* element = (NODE *)NULLP;
 
        /* Should Possibly be freeing the elements from non-empty queues*/

    while ( ( element = lstGet( &(pimcglp[index]->pimc_req) ) ) != NULLP )
        ;                               /* Do Nothing */

    while ( ( element = lstGet( &(pimcglp[index]->pimc_ind) ) ) != NULLP )
        ;                               /* Do Nothing */


    while ( ( element = lstGet(&(pimcglp[index]->pimc_mgmt) ) ) != NULLP )
        ;                               /* Do Nothing */

    while ( ( element = lstGet(&(pimcglp[index]->pimc_exp_req) ) ) != NULLP )
        ;                               /* Do Nothing */

    while ( ( element = lstGet(&(pimcglp[index]->pimc_exp_ind) ) ) != NULLP )
        ;                               /* Do Nothing */
 
    lstFree(&pimcglp[index]->pimc_req);
    lstFree(&pimcglp[index]->pimc_ind);
    lstFree(&pimcglp[index]->pimc_mgmt);
    lstFree(&pimcglp[index]->pimc_exp_req);
    lstFree(&pimcglp[index]->pimc_exp_ind);

        lstDelete( pimcListHead, &((pimcglp[index])->imc_element) );
 
    IMCfree ( pimcglp[index] );
 
        return ( RC_OK );
}


#ifndef USE_IMC_MACROS

/*COMMENTS*********************************************************
 *
 *   Func:  imc_post_sim
 *
 *   Desc:  Post a Sim to the Given Queue of the Module
 *
 *   Inputs: Post Type   enum POST_QUEUE
 *           SIM         tpCOMSIM pComSim
 *
 *   Outputs: RC_OK ( or )
 *            RC_ERR 
 *
 *
 *END*********************************************************************/

tOCTET2 imc_post_sim( tpCOMSIM pComSim, IMC_QUEUE_TYPE queue )
{                                                                   
        tOCTET2 rc = RC_OK;
        LIST *pListHead = NULL;

        if ( PSsemTake( freeIndexSemId,WAIT_FOREVER ) == ERROR) { 
                panic("mPOST_SIM:Could not get semaphore\n");        
                rc = RC_ERR;                                                
        }                                                               
        else if ( (pimcglp[pComSim->destid] == NULL ) ) {             
                printf("%s: ", taskName( taskIdSelf() ));
                printf("NULL IMC Anchor for Destination %ld",(pComSim)->destid ); 
                rc = RC_ERR;                                                 
        }                                                               
        else {

                pListHead = imc_get_queue( pComSim->destid, queue );
                if ( pListHead == NULL ) {
                        panic ("mPOST_SIM: Error occurred getting Queue to POST\n");
                        rc = RC_ERR;
                }else  { 
                        lstAdd(pListHead, &(pComSim->simq));
                        imc_change_global_count( INCREMENT );                            
                        ++((pimcglp[pComSim->destid])->workCount);                

                        if ( PSsemGive( mMID2SEM(pComSim->destid))== ERROR ) {    
                                panic("mPOST_SIM:Could not give Task semaphore\n");   
                                rc = RC_ERR;                                            
                        }                                                           
                }                                                               
        }

        if ( PSsemGive( freeIndexSemId ) == ERROR) {                    
                panic("mPOST_SIM_NREQ:Could not give Index semaphore\n"); 
                rc = RC_ERR;                                 
        }                                                               

        return ( rc );
}

/*COMMENTS*********************************************************
 *
 *   Func:  imc_get_queue
 *
 *   Desc:  Get the Listhead of the Queue from a Module
 *
 *   Inputs: Module Id   tOCTET4 moduleId
 *           Queue Type  enum IMC_QUEUE_TYPE
 *
 *   Outputs: LIST*
 *            NULL on Error 
 *
 *
 *
 *END*********************************************************************/

static LIST* imc_get_queue( tOCTET4 moduleId, IMC_QUEUE_TYPE queue )
{
        LIST* pListHead = NULL;

        switch ( queue ) {

        case MGMT_QUEUE:
                pListHead = ( mMID2MGMT(moduleId) );
                break;

        case NIND_QUEUE:
                pListHead = ( mMID2NIND(moduleId) );
                break;

        case EIND_QUEUE:
                pListHead = ( mMID2EIND(moduleId) );
                break;

        case NREQ_QUEUE:
                pListHead =  ( mMID2NREQ(moduleId) );
                break;

        case EREQ_QUEUE:
                pListHead =  ( mMID2EREQ(moduleId) );
                break;
        
        default:
                pListHead = ( NULL );

        }

        return ( pListHead );
}


/*COMMENTS*********************************************************
 *
 *   Func:  imc_change_global_count
 *
 *   Desc: Change the global work count
 *
 *   Inputs:  INCREMENT / DECREMENT
 *
 *   Outputs: None
 *
 *END*********************************************************************/
void imc_change_global_count( tOCTET2 action )
{
        if ( action == INCREMENT ) 
                ++imc_global_count; 
        else if ( action == DECREMENT ) {
                --imc_global_count; 
                if ( imc_global_count < 0 ) { 
                        panic("mCHANGEGLOBALCOUNT: Negative Count\n"); \
                }
        } else { 
                printf("\n%s: ", taskName( taskIdSelf() ));\
                printf("mCHANGEGLOBALCOUNT:Invalid Action\n"); \
        } 
}


/*COMMENTS*********************************************************
 *
 *   Func:  imc_get_sim
 *
 *   Desc:  Get a Sim to the Module
 *
 *   Inputs: Post Type   enum POST_QUEUE
 *           Module Id   tOCTET2 moduleID   
 *           SIM         tpCOMSIM pComSim
 *
 *   Outputs: None
 *
 *
 *END*********************************************************************/

void imc_get_sim( tpCOMSIM *ppComSim, tUOCTET4 moduleId, IMC_QUEUE_TYPE queue )
{                                                                   
        tOCTET2 rc = RC_OK;
        LIST* pListHead = NULL;
        tOCTET2 workCount = 0;

        if ( PSsemTake( freeIndexSemId,WAIT_FOREVER ) == ERROR) {       
                panic("mGET_SIM:Could not get semaphore\n");        
                rc = RC_ERR;                                                
        }                                                               
        else if ( (pimcglp[moduleId] == NULL ) ) {             
                printf("%s: ", taskName( taskIdSelf() ));
                printf("NULL IMC Anchor for Destination %ld",moduleId ); 
                rc = RC_ERR;                                                 
        }                                                               
        else {
                pListHead = imc_get_queue( moduleId, queue );
                if ( pListHead == NULL ) {
                        panic ("mGET_SIM: Error occurred getting Queue to GET\n");
                        rc = RC_ERR;
                }else  {
                        
                        workCount = lstCount( pListHead );
                        if ( workCount < 0 ) {
                                panic("mGET_SIM:Negative Work count obtained\n");
                        } else if ( workCount > 0 ) {
                                *ppComSim = ( tpCOMSIM ) lstGet( pListHead );   
                                imc_change_global_count( DECREMENT );                            
                                --((pimcglp[moduleId])->workCount);                
                        } else 
                                *ppComSim = NULL;

                        if ( PSsemGive( freeIndexSemId ) == ERROR ) {    
                                panic("mGET_SIM:Could not give Free Index semaphore\n");   
                                rc = RC_ERR;                                            
                        }                                                           
                }                                                               
        }
}

#endif
