#ifndef _IMSG_H_
#define _IMSG_H_
/*
-------------------------------------------------------------------------------
        The common service interaction header
-------------------------------------------------------------------------------
*/

struct  com_sim {
        NODE        simq;               /* queue element */
        tUOCTET4    ownership;          /* if 0, it can be freed */
        tUOCTET4    origid;             /* originators module id  */
        tUOCTET4    destid;             /* destination module id  */
        tUOCTET4    requestid;  /* request id generated by orig.*/
        tUOCTET4    serviceid;  /* service interaction id */
        tUOCTET4    status;     /* carries return codes */
        SEM_ID      ownerSemId; /* Semaphore for ownership      */
        void       *pLayerSim;  /* pointer to module-specific sim       */
} ;

typedef struct com_sim  tCOMSIM, *tpCOMSIM;

struct idval { 
        tOCTET4     id;         /* Identifier */
        void*       value;      /* Value      */
};

typedef struct idval tIDVAL, *tpIDVAL;

#define MAX_CFG_ID_VAL 5

struct mgmt_sim { 
        tCOMSIM     comsim;     /* Common SIM */
        tUOCTET4    nid;
        tIDVAL      idvals[MAX_CFG_ID_VAL];
        boolean     sense;
        SEM_ID      configSemId;  
        int             inFd;
        int             outFd;
};

#define mGETID(pSim,index)  (pSim)->idvals[(index)-1].id
#define mGETVAL(pSim,index) (pSim)->idvals[(index)-1].value

#define mSETID( pSim,index,id )   mID((pSim),(index)) = (id)
#define mSETVAL( pSim,index,val ) mID((pSim),(index)) = (val)

typedef struct mgmt_sim tMGMTSIM, *tpMGMTSIM;

/*
------------------------------------------------------------------------------
                SIM manipulation macros
------------------------------------------------------------------------------
*/
#define IMCcalloc(nelem, elemSize)     PScalloc((nelem), (elemSize))
#define IMCmalloc(elemSize)            PSmalloc((elemSize))
#define IMCrealloc(elemPtr, elemSize)  PSrealloc((elemPtr), (elemSize))
#define IMCfree(elemPtr)               PSfree((elemPtr))



#define mGETSIM(size) imsg_getsim( (size) )

#define mFREEMGMTSIM(pSim)  \
        {   \
                mFREESIM( &(pSim->comsim) ); \
        }
#ifndef USE_IMC_MACROS
#define mFREESIM(pSim)  imsg_freesim( (pSim)) 
#else
#define mFREESIM(pSim) \
                          {     \
                                if ( PSsemTake(((tpCOMSIM)(pSim))->ownerSemId,\
                     WAIT_FOREVER ) == ERROR ) { \
                                        PSpanic("Could not Take Ownership Semaphore ");      \
                                }\
                                else if ( ((tpCOMSIM)(pSim))->ownership == 0) { \
                                        if( ((tpCOMSIM)(pSim))->pLayerSim )     \
                                                PSfree(((tpCOMSIM)(pSim))->pLayerSim); \
                                        PSsemDelete(((tpCOMSIM)(pSim))->ownerSemId); \
                                        PSfree((tpCOMSIM)pSim);\
                                } else {\
                                        --((tpCOMSIM)(pSim))->ownership;\
                                        if (PSsemGive(((tpCOMSIM)(pSim))->ownerSemId) == ERROR ) {\
                                                PSpanic("mFREESIM:Could not Give Ownership Semaphore\n");      \
                                        }\
                                }\
                        }
#endif /* USE_IMC_MACROS */

#define mSIMCOPY(pDestsim, pSrcSim) \
                        {                                                                                               \
                                PSsemTake(((tpCOMSIM)(pSrcSim))->ownerSemId, WAIT_FOREVER ); \
                                ((tpCOMSIM)(pSrcSim))->ownership++; \
                                pDestSim = pSrcSim;     \
                                PSsemGive(((tpCOMSIM)(pSrcSim))->ownerSemId); \
                        }

#define mZEROSIM(pSim)  imsg_zerosim( (pSim) )

#define mFILL_COM_SIM( pComSim, origin, dest, service, request, pLayer,owner, \
                       moreStatus )\
                { \
                        if ( !pComSim ) { \
                                PSpanic("NULL SIM in mFILL_COM_SIM\n"); \
                        } \
                        else  { \
                                pComSim->ownership = owner;   \
                                pComSim->origid = origin;     \
                                pComSim->destid = dest;       \
                                pComSim->requestid = request; \
                                pComSim->serviceid = service; \
                                pComSim->pLayerSim = pLayer;  \
                                pComSim->status =    moreStatus;  \
                        } \
                } 

#define mSIM_ORIG(pGsim)                        ((pGsim)->origid)
#define mSIM_DEST(pGsim)                        ((pGsim)->destid)
#define mSIM_REQ(pGsim)                         ((pGsim)->requestid)
#define mSIM_SERV(pGsim)                        ((pGsim)->serviceid)



/*
-------------------------------------------------------------------------------
                Service interaction codes for the various modules, bottom up
                Recomended convention for 4 byte service code:

                                                [0][1][2][3]

                        byte 0:         service provider id
                        byte 1:         direction 1: request, 2 indication
                        byte 2, 3       service class
                        
-------------------------------------------------------------------------------
*/

#define MODULE_MASK                     0xF000
#define DIRECTION_MASK          0x0200
#define SERVICE_CLASS_MASK      0x00FF
                                                                                                /* service to module            */

#define REQ_MASK                        0x0100                          /* Request Mask */
#define IND_MASK                        0x0200                          /* Indication Mask*/

#define IS_REQUEST(serviceid)    ((serviceid) & REQ_MASK )
#define IS_INDICATION(serviceid) ((serviceid) & IND_MASK )


/* Get moduleid from service id */
#define S2L(serviceid)          ( ((serviceid)&LAYER_MASK) >> 12 )

/* Service interaction codes */
#define TEST_MODULE     0x00        /* Test Module */
#define SMTPCL_MODULE   0x01            /* SMTP Client Module */
#define HTTP_MODULE         0x02                /* Http Module */
#define PARSER_MODULE   0x03            /* Parser Module */
#define TIMER_MODULE    0x04        /* timer service */
#define ACL_MODULE              0x05            /* ACL Module */
#define CAPS_MODULE     0x06            /* CAPS Module */

#define LAST_MODULE             CAPS_MODULE

#define ASSIGN_MODULE_ID -1

/*
**
** Intermodule service access points
**
*/
/* exported by imsg.c */
#ifndef USE_IMC_MACROS
extern void imsg_freesim( tpCOMSIM );
#endif
extern tpCOMSIM   imsg_getsim (tOCTET4 );
extern void       imsg_zerosim (tpCOMSIM );

#endif  /* _IMSG_H_ */
