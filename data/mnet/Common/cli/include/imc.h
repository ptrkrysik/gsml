#ifndef _IMC_H_
#define _IMC_H_

/*
===============================================================================

        Modules have a request, indication, management queues that are anchored 
    in the IMC Queue Anchor.

        IMC Q anchor block
                ________
           |            |
           |            |------> MgmtQ
           |            |------> Expedited IndQ
           |            |------> Expedited ReqQ
           |            |------> ReqQ
           |            |------> IndQ
           |            |--|
           |________|  |
                                   |
                                  \|/ Next Module Anchor block
                                   
===============================================================================
*/

#define MAX_IMC_COUNT  32



struct imc_qanchor {                            /* IMC queue anchor     */

    NODE        imc_element;        /* List Element */
    tSEMAPHORE  semId;              /* Couting Semaphore task is waiting on */
    tUOCTET4    taskId;             /* Task requesting IMC Services */
    tOCTET4     workCount;          /* Number of elements in Queues */
        LIST        pimc_mgmt;                  /* Management request queue */
        LIST        pimc_exp_ind;               /* Expedited Indication queue */
        LIST        pimc_exp_req;               /* Expedited request queue */
        LIST        pimc_ind;                   /* Normal Indication queue */
        LIST        pimc_req;                   /* Normal request queue */
} ;

typedef struct imc_qanchor      tIMC_QANCHOR, *tpIMC_QANCHOR;

/*
 *              Global Count Macro
*/

#define INCREMENT    1
#define DECREMENT    2


typedef enum IMC_QUEUE_TYPE {
        MGMT_QUEUE = 0,
        NREQ_QUEUE,
        EREQ_QUEUE,
        NIND_QUEUE,
        EIND_QUEUE
} IMC_QUEUE_TYPE;


/* Convenience macro for mid queues */
/* Use with caution by checking for NULL */

#define mMID2Q(mid)             pimcglp[(mid)]  
#define mMID2MGMT(mid)  (&(mMID2Q((mid))->pimc_mgmt))
#define mMID2EREQ(mid)  (&(mMID2Q((mid))->pimc_exp_req))
#define mMID2NREQ(mid)  (&(mMID2Q((mid))->pimc_req)) 
#define mMID2EIND(mid)  (&(mMID2Q((mid))->pimc_exp_ind))
#define mMID2NIND(mid)  (&(mMID2Q((mid))->pimc_ind))
#define mMID2SEM(mid)   (mMID2Q((mid))->semId)

#ifdef USE_IMC_MACROS

#define mCHANGEGLOBALCOUNT(action)              \
        {\
                if ( action == INCREMENT ) \
                        ++imc_global_count; \
                else if ( action == DECREMENT ) {\
                        --imc_global_count; \
                if ( imc_global_count < 0 ) { \
                                PSpanic("mCHANGEGLOBALCOUNT: Negative Count\n"); \
                        }\
                }\
                else { \
                trace_Msg("\n%s: ", taskName( taskIdSelf() ));\
                        trace_Msg("mCHANGEGLOBALCOUNT:Invalid Action\n"); \
                } \
        }
        
#define mTAKEINDEXSEM(which, rc ) \
        {                                                             \
                if ( PSsemTake( freeIndexSemId,WAIT_FOREVER ) == ERROR) { \
                        PSpanic("Could not Take freeIndex Semaphore\n");      \
                        rc = RC_ERR;                                          \
                } else                                                    \
                        rc = RC_OK;                                           \
        }                                                            

#define mGIVEINDEXSEM( which ) \
        {                                                             \
                if ( PSsemGive( freeIndexSemId) == ERROR) {               \
                        PSpanic("Could not Give freeIndex Semaphore\n");      \
                }                                                         \
        }

#define mCHECKMODULEIMC( which, mid,rc)                           \
        {                                                             \
                if (pimcglp[(mid)] == NULL ) {                            \
           trace_Msg("NULL IMC Anchor for Destination %ld\n",      \
                                        mid );                                        \
                   rc = RC_ERR;                                           \
                } else                                                    \
                        rc = RC_OK;                                           \
        }                                                            



/*
 *              POST service interaction macros
 *                      Interaction post Failed if rc != RC_OK  
 *                      pComSim->destid is module id of destination 
 */

#define mPOST_SIM_NREQ(pComSim, rc)                                                                             \
        {                                                                                                                                       \
                if ( PSsemTake( freeIndexSemId,WAIT_FOREVER ) == ERROR) {       \
                        PSpanic("mPOST_SIM_NREQ:Could not get semaphore\n");        \
                        rc = RC_ERR;                                                \
                }                                                               \
                else if (pimcglp[(pComSim)->destid] == NULL ) {                 \
           trace_Msg("NULL IMC Anchor for Destination %ld",             \
                                        (pComSim)->destid );                                \
                   rc = RC_ERR;                                                 \
                }                                                               \
                else if ( (rc=lstAdd((mMID2NREQ((pComSim)->destid)),          \
                                        &(pComSim)->simq)) == RC_OK ) {                                         \
                        ++((pimcglp[(pComSim)->destid])->workCount);                            \
                        mCHANGEGLOBALCOUNT( INCREMENT );                                                        \
                        if ( PSsemGive(mMID2SEM((pComSim)->destid)) == ERROR ) {        \
                                PSpanic("mPOST_SIM_NREQ:Could not give Task semaphore\n");   \
                                rc = RC_ERR;                                            \
                        }                                                           \
                }                                                                                                                               \
                if ( PSsemGive( freeIndexSemId ) == ERROR) {                                    \
                        PSpanic("mPOST_SIM_NREQ:Could not give Index semaphore\n"); \
                        rc = RC_ERR;                                                \
                 }                                                              \
        }

#define mPOST_SIM_MGMT(pComSim, rc)                                                                             \
        {                                                                                                                                       \
                if ( PSsemTake( freeIndexSemId,WAIT_FOREVER ) == ERROR) {       \
                        PSpanic("mPOST_SIM_MGMT:Could not get semaphore\n");        \
                        rc = RC_ERR;                                                \
                }                                                               \
                else if ( (pimcglp[(pComSim)->destid] == NULL ) ) {             \
                        trace_Msg("%s: ", taskName( taskIdSelf() ));\
            trace_Msg("NULL IMC Anchor for Destination %ld"              \
                    ,(pComSim)->destid );                               \
           rc = RC_ERR;                                                 \
        }                                                               \
                else if ( (rc=lstAdd((mMID2MGMT((pComSim)->destid)),            \
                                        &(pComSim)->simq)) == RC_OK ) {                                         \
                        mCHANGEGLOBALCOUNT( INCREMENT );                                                        \
                        ++((pimcglp[(pComSim)->destid])->workCount);                            \
                        if ( PSsemGive( mMID2SEM((pComSim)->destid))== ERROR ) {        \
                                PSpanic("mPOST_SIM_MGMT:Could not give Task semaphore\n");   \
                                rc = RC_ERR;                                            \
                        }                                                           \
                }                                                                                                                               \
                if ( PSsemGive( freeIndexSemId ) == ERROR) {                                    \
                        PSpanic("mPOST_SIM_MGMT:Could not give Index semaphore\n");       \
                        rc = RC_ERR;                                                \
                }                                                               \
        }

/* Post Normal Indication Failed if rc != RC_OK */
#define mPOST_SIM_NIND(pComSim, rc)                                                                             \
        {                                                                                                                                       \
                if (PSsemTake( freeIndexSemId,WAIT_FOREVER ) == ERROR) {        \
                        PSpanic("mPOST_SIM_NIND:Could not get semaphore\n");        \
                        rc = RC_ERR;                                                \
                }                                                               \
                else if (pimcglp[(pComSim)->destid] == NULL ) {                 \
                   trace_Msg("%s: ", taskName( taskIdSelf() ));\
           trace_Msg("NULL IMC Anchor for Destination %ld",              \
                    (pComSim)->destid );                               \
           rc = RC_ERR;                                                 \
        }                                                               \
                else if ( (rc=lstAdd((mMID2NIND((pComSim)->destid)),            \
                                        &((pComSim)->simq))) == RC_OK ) {                                       \
                        ++((pimcglp[(pComSim)->destid])->workCount);                            \
                        mCHANGEGLOBALCOUNT( INCREMENT );                                                        \
                        if ( PSsemGive( mMID2SEM((pComSim)->destid)) == ERROR ) {       \
                                PSpanic("mPOST_SIM_NIND:Could not give Task semaphore\n");   \
                                rc = RC_ERR;                                            \
                        }                                                           \
                }                                                                                                                               \
                if ( PSsemGive( freeIndexSemId ) == ERROR) {                                    \
                        PSpanic("mPOST_SIM_NIND:Could not give Index semaphore\n");       \
                        rc = RC_ERR;                                                \
                }                                                               \
        }

/* Post Expedited Request Failed if rc != RC_OK */
#define mPOST_SIM_EREQ(pComSim, rc)                                                                             \
        {                                                                                                                                       \
                if ( PSsemTake( freeIndexSemId,WAIT_FOREVER ) == ERROR) {       \
                        PSpanic("mPOST_SIM_EREQ:Could not get semaphore\n");        \
                        rc = RC_ERR;                                                \
                }                                                               \
                else if ( (pimcglp[(pComSim)->destid] == NULL ) ) {             \
                   trace_Msg("%s: ", taskName( taskIdSelf() ));\
           trace_Msg("NULL IMC Anchor for Destination %ld",              \
                    (pComSim)->destid );                                \
           rc = RC_ERR;                                                 \
        }                                                               \
                else if ( (rc=lstAdd(mMID2EREQ((pComSim)->destid),                      \
                                        &(pComSim)->simq)) == RC_OK ) {                                         \
                        mCHANGEGLOBALCOUNT( INCREMENT );                                                        \
                        ++((pimcglp[(pComSim)->destid])->workCount);                            \
                        if ( PSsemGive( mMID2SEM((pComSim)->destid)) == ERROR ) {       \
                                PSpanic("mPOST_SIM_EREQ:Could not give Task semaphore\n");   \
                                rc = RC_ERR;                                            \
                        }                                                           \
                }                                                                                                                               \
                if ( PSsemGive( freeIndexSemId ) == ERROR) {                                    \
                        PSpanic("mPOST_SIM_EREQ:Could not give semaphore\n");       \
                        rc = RC_ERR;                                                \
                }                                                               \
        }

/* Post Expedited Indication Failed if rc != RC_OK */
#define mPOST_SIM_EIND(pComSim, rc)                                                                             \
        {                                                                                                                                       \
                if ( PSsemTake( freeIndexSemId,WAIT_FOREVER ) == ERROR) {       \
                        PSpanic("mPOST_SIM_EIND:Could not get semaphore\n");        \
                        rc = RC_ERR;                                                \
                }                                                               \
                else if (pimcglp[(pComSim)->destid] == NULL ) {                 \
                   trace_Msg("%s: ", taskName( taskIdSelf() ));\
           trace_Msg("NULL IMC Anchor for Destination %ld",              \
                     (pComSim)->destid );                               \
           rc = RC_ERR;                                                 \
        }                                                               \
                if ( (rc=lstAdd(mMID2EIND((pComSim)->destid),                               \
                                        &(pComSim)->simq)) == RC_OK ) {                                         \
                        mCHANGEGLOBALCOUNT( INCREMENT );                                                        \
                        ++((pimcglp[(pComSim)->destid])->workCount);                            \
                        if ( PSsemGive( mMID2SEM((pComSim)->destid)) == ERROR ) {       \
                                PSpanic("mPOST_SIM_EIND:Could not give semaphore\n");   \
                                rc = RC_ERR;                                            \
                        }                                                           \
                }                                                                                                                               \
                if ( PSsemGive( freeIndexSemId ) == ERROR) {                                    \
                        PSpanic("mPOST_SIM_EIND:Could not give semaphore\n");       \
                        rc = RC_ERR;                                                \
                }                                                               \
        }

/*
 *              GET Service interactions for a module, Indications first
 */


/* Call with pComSim == NULLP , mid == module id  */
/* get Management requests, expedited indications and requests first, 
 * then the normal indications and requests */

/* Get Management work only */
#define mGET_MGMT_WORK_SIM(pComSim, mid)                                                                \
    {                                                                   \
                if (PSsemTake(freeIndexSemId, WAIT_FOREVER ) == ERROR) {        \
                        PSpanic("mGET_MGMT_WORK_SIM:Could not get semaphore\n");    \
                }                                                               \
        else if (pimcglp[(mid)] == NULL)  {                             \
           trace_Msg("mGET_MGMT_WORK_SIM:NULL IMC Anchor for Module %ld", \
                                         (tUOCTET4)mid);                                    \
           pComSim = NULL;                                              \
        }                                                               \
        else if (mMID2Q(mid)->workCount >  0 ) {                        \
                        tOCTET2 count = lstCount(mMID2MGMT(mid));                                       \
                        if (count < 0 ) {                                               \
                                PSpanic("mGET_MGMT_WORK_SIM:Could not get count\n");   \
                        } \
                        else if ( count > 0 ){ \
                                pComSim = (tpCOMSIM)lstGet(mMID2MGMT(mid));             \
                                if (pComSim == NULL) {                                          \
                                        PSpanic("mGET_MGMT_WORK_SIM:NULL Sim Dequeued\n");   \
                                } \
                                mCHANGEGLOBALCOUNT( DECREMENT );                                                \
                                --((pimcglp[mid])->workCount);                                                  \
                }                                                           \
        }                                                                                                                               \
                else {                                                                                                                  \
                        pComSim = NULL;                                                                                         \
        }                                                           \
                if ( PSsemGive( freeIndexSemId ) == ERROR) {                                    \
                        PSpanic("mGET_MGMT_WORK_SIM:Could not give semaphore\n");   \
                }                                                               \
        } 


/* Get Normal Indication work only */

#define mGET_NIND_WORK_SIM(pComSim, mid) \
        {                                                                   \
                if ( PSsemTake(freeIndexSemId,WAIT_FOREVER ) == ERROR) {        \
                        PSpanic("mGET_NIND_WORK_SIM:Could not get semaphore\n");    \
                }                                                               \
        else if ( mIS_THERE_WORK( (mid) ) ) {                           \
                        tOCTET2 count=lstCount(mMID2NIND((mid))); \
                        if (count < 0 ) {                                               \
                                PSpanic("mGET_NIND_WORK_SIM:Could not get count\n");   \
                        }\
                        else if ( count > 0 ){ \
                                pComSim = (tpCOMSIM)lstGet(mMID2NIND(mid));             \
                                if (pComSim == NULL) {                                          \
                                        PSpanic("mGET_NIND_WORK_SIM:NULL Sim Dequeued\n");   \
                                } \
                                --((pimcglp[mid])->workCount);                                                  \
                                mCHANGEGLOBALCOUNT( DECREMENT );                                            \
                }                                                                                                                       \
                } else {                                                                                                                \
                        pComSim = NULL;                                                                                         \
                }                                                               \
                if ( PSsemGive( freeIndexSemId ) == ERROR) {                                    \
                        PSpanic("mGET_NIND_WORK_SIM:Could not give semaphore\n");   \
                }                                                               \
        }

/* Get Expedited Indication work only */

#define mGET_EIND_WORK_SIM(pComSim, mid) \
        {                                                                   \
                if ( PSsemTake( freeIndexSemId,WAIT_FOREVER ) == ERROR) {       \
                        PSpanic("mGET_EIND_WORK_SIM:Could not get semaphore\n");    \
                }                                                               \
        else if ( mIS_THERE_WORK( (mid) ) ) {                           \
                        tOCTET2 count=lstCount(mMID2EIND((mid))); \
                        if (count < 0 ) {                                               \
                                PSpanic("mGET_EIND_WORK_SIM:Could not get count\n");   \
                        }\
                        else if ( count > 0 ){ \
                                pComSim = (tpCOMSIM)lstGet(mMID2EIND(mid));             \
                                if (pComSim == NULL) {                                          \
                                        trace_Msg("mGET_EIND_WORK_SIM:NULL Sim Dequeued\n");   \
                                } \
                                mCHANGEGLOBALCOUNT( DECREMENT );                                                \
                                --((pimcglp[mid])->workCount);                                                  \
                }                                                                                                                       \
                } else{                                                                                                                         \
                        pComSim = NULL;                                                                                         \
        }                                                                                                                       \
                if ( PSsemGive( freeIndexSemId ) == ERROR) {                                    \
                        PSpanic("mGET_EIND_WORK_SIM:Could not give semaphore\n");   \
                }                                                               \
        }       

/* Get Normal Request work only */

#define mGET_NREQ_WORK_SIM(pComSim, mid) \
        {                                                                   \
                if ( PSsemTake( freeIndexSemId,WAIT_FOREVER ) == ERROR) {       \
                        PSpanic("mGET_NREQ_WORK_SIM:Could not get semaphore\n");    \
                }                                                               \
        else if ( mIS_THERE_WORK((mid))) {                                      \
                        tOCTET2 count=lstCount(mMID2NREQ((mid))); \
                        if (count < 0 ) {                                               \
                                PSpanic("mGET_NREQ_WORK_SIM:Could not get count\n");   \
                        } \
                        else if ( count > 0 ){ \
                                pComSim = (tpCOMSIM)lstCount(mMID2NREQ(mid));           \
                                if (pComSim == NULL) {                                          \
                                        PSpanic("mGET_NREQ_WORK_SIM:NULL Sim Dequeued\n");   \
                                } \
                                mCHANGEGLOBALCOUNT( DECREMENT );                                                \
                                --((pimcglp[mid])->workCount);                                                  \
                }                                                                                                                       \
                } else {                                                                                                                \
                        pComSim = NULL;                                                                                         \
        }                                                                                                                       \
                if ( PSsemGive( freeIndexSemId ) == ERROR) {                                    \
                        PSpanic("mGET_NREQ_WORK_SIM:Could not give semaphore\n");   \
                }                                                               \
        }

/* Get Expedited Request work only */

#define mGET_EREQ_WORK_SIM(pComSim, mid) \
        {                                                                   \
                if ( PSsemTake( freeIndexSemId,WAIT_FOREVER ) == ERROR) {       \
                        PSpanic("mGET_EREQ_WORK_SIM:Could not get semaphore\n");    \
                }                                                               \
        else if ( mIS_THERE_WORK((mid))) {                       \
                        tOCTET2 count=lstCount(mMID2EREQ((mid))); \
                        if (count < 0 ) {                                               \
                                PSpanic("mGET_EREQ_WORK_SIM:Could not get count\n");   \
                        }\
                        else if ( count > 0 ){ \
                                pComSim = (tpCOMSIM)lstGet(mMID2EREQ(mid));             \
                                if (pComSim == NULL) {                                          \
                                        PSpanic("mGET_EREQ_WORK_SIM:NULL Sim Dequeued\n");   \
                                } \
                                mCHANGEGLOBALCOUNT( DECREMENT );                                                \
                                --((pimcglp[mid])->workCount);                                                  \
                }                                                                                                                       \
                }                                                               \
                else {                                                                                                              \
                        pComSim = NULL;                                                                                         \
                }                                                               \
                if ( PSsemGive( freeIndexSemId ) == ERROR) {                                    \
                        PSpanic("mGET_EREQ_WORK_SIM:Could not give semaphore\n");   \
                }                                                               \
        }       


#else   /* USE_IMC_MACROS */

#define mCHANGEGLOBALCOUNT(action) \
        imc_change_global_count( (action)) 

#define mPOST_SIM_NREQ(pComSim, rc) \
                rc = imc_post_sim( (pComSim), NREQ_QUEUE)

#define mPOST_SIM_MGMT(pComSim, rc) \
                rc = imc_post_sim( (pComSim), MGMT_QUEUE)

/* Post Expedited Indication Failed if rc != RC_OK */
#define mPOST_SIM_EIND(pComSim, rc)     \
                rc = imc_post_sim( (pComSim), EIND_QUEUE)

/* Post Expedited Request Failed if rc != RC_OK */
#define mPOST_SIM_EREQ(pComSim, rc) \
                rc = imc_post_sim( (pComSim), EREQ_QUEUE)

/* Post Normal Indication Failed if rc != RC_OK */
#define mPOST_SIM_NIND(pComSim, rc)     \
                rc = imc_post_sim( (pComSim), NIND_QUEUE)

/* Get Management work only */
#define mGET_MGMT_WORK_SIM(pComSim, mid) \
                imc_get_sim( &(pComSim), (mid), MGMT_QUEUE );

/* Get Normal Indication work only */
#define mGET_NIND_WORK_SIM(pComSim, mid) \
                imc_get_sim( &(pComSim), (mid), NIND_QUEUE );

/* Get Expedited Indication work only */
#define mGET_EIND_WORK_SIM(pComSim, mid) \
                imc_get_sim( &(pComSim), (mid), EIND_QUEUE );

/* Get Normal Request work only */
#define mGET_NREQ_WORK_SIM(pComSim, mid) \
                imc_get_sim( &(pComSim), (mid), NREQ_QUEUE );

/* Get Expedited Request work only */
#define mGET_EREQ_WORK_SIM(pComSim, mid) \
                imc_get_sim( &(pComSim), (mid), EREQ_QUEUE );


#endif /* USE_IMC_MACROS */


/* Get Request work only */

#define mGET_REQ_WORK_SIM(pComSim, mid)         \
    {                                           \
                mGET_EREQ_WORK_SIM( pComSim, mid );             \
                if ( pComSim == NULL ) {                                \
                        mGET_NREQ_WORK_SIM( pComSim, mid );     \
        }                                                                               \
        }
                        
/* Get indication work only */

#define mGET_IND_WORK_SIM(pComSim, mid)         \
    {                                           \
                mGET_EIND_WORK_SIM( pComSim, mid );             \
                if ( pComSim == NULL ) {                                \
                        mGET_NIND_WORK_SIM( pComSim, mid );     \
        }                                                                               \
        }



/* Get Any work. Enforces a policy of servicing order */
/* Management, Expedited Indication, Expedited Requests */
/* Normal Indications, Normal Requests. The order given */
/* is also exhaustive and hence can starve other queues */

#define mGET_WORK_SIM(pComSim, mid)                             \
    {                                           \
                mGET_MGMT_WORK_SIM( pComSim, mid );             \
                if ( pComSim == NULL ) {                                \
                        mGET_EIND_WORK_SIM( pComSim, mid );     \
        }                                                                               \
                if ( pComSim == NULL ){                                 \
                        mGET_EREQ_WORK_SIM( pComSim, mid );     \
        }                                                                               \
                if ( pComSim == NULL ){                                 \
                        mGET_NREQ_WORK_SIM( pComSim, mid );     \
        }                                                                               \
                if ( pComSim == NULL ){                                 \
                        mGET_NIND_WORK_SIM( pComSim, mid );     \
        }                                                                               \
        }

/*      mIS_THERE_WORK(mid) returns TRUE or FALSE depending on whether there
        is work for this module or not */

#define mIS_THERE_WORK(mid) \
        ( ( \
                (mMID2Q((mid)) == NULL ) ? FALSE : \
                (mMID2Q((mid))->workCount > 0 ) ? \
                TRUE : FALSE ) )

/* checks if there is work in any of the imc queues */

#define mANY_WORK imc_global_count

/* exported by imc.c */
extern tOCTET4    imc_global_count;       /* keeps track of the TOTAL number
                                             of entries in the imc facility */
extern  LIST            *pimcListHead;   /* Pointer to IMC ListHead */
extern  tpIMC_QANCHOR   pimcglp[MAX_IMC_COUNT]; 
/* Global MUTEX Semaphores */
extern  tSEMAPHORE freeIndexSemId;       /* Protect freeIndex and hence IMC */
extern boolean imcInited;
extern void     imc_term(void);
extern tOCTET2 imc_init( void );
extern tOCTET2 imc_add( tpOCTET4 index, tSEMAPHORE semId, tTASKID taskId );
extern tOCTET2 imc_remove( tUOCTET4 index );
extern tOCTET2 imc_post_sim(tpCOMSIM , IMC_QUEUE_TYPE);
extern void imc_get_sim( tpCOMSIM *, tUOCTET4 , IMC_QUEUE_TYPE);

#endif  /* _IMC_H_ */
