#include <vxworks.h>
#include <stdioLib.h>
#include <taskLib.h>
#include <lstLib.h>
#include <inetLib.h>    /* for INET_ADDR_LEN */
#include <stdlib.h>
#include <string.h>


#include <../include/pstypes.h>
#include <../include/psvxif.h>

#include <../include/imsg.h>


/*COMMENTS*********************************************************
 *
 *   Func:  imsg_getsim
 *
 *   Desc:  Allocate Memory and create a Semaphore for Ownership
 *
 *   Inputs: tUOCTET4   size    Size of Memory to Allocate
 *
 *   Outputs: tpCOMSIM  Pointer to the SIM or NULL if allocate failed
 *
 *
 *END*********************************************************************/

tpCOMSIM imsg_getsim (tOCTET4 size)
{
    tpCOMSIM pComSim = NULL;
    
    pComSim = (tpCOMSIM)IMCcalloc(1, size);

    if ( !pComSim ) 
    {
                PSprintf("imsg_getsim(): mGETSIM Failed!; size=%ld\n", size);
                return ( NULL );
    } 
    else {      
        pComSim->ownerSemId = PSsemMCreate( PS_DEFAULT_MUTEX_SEM_FLAG );
        if ( pComSim->ownerSemId == NULL ) {
            PSpanic ( "imsg_getsim : Could not create owner id semaphore\n");
            return ( NULL );
          }
   }

   return ( pComSim );

}/* end imsg_getsim */



/*COMMENTS*********************************************************
 *
 *   Func:  imsg_zerosim
 *
 *   Desc:  Zero-out SIM
 *
 *   Inputs: tpCOMSIM   pSim   SIM to zero-out
 *
 *   Outputs: NONE
 *
 *   Outputs: For debugging aid. getsim does memset
 *
 *END*********************************************************************/

void imsg_zerosim( tpCOMSIM pSim )
{                                                       
        if ( !pSim ) { 
                printf("%s(mZEROSIM):", taskName( taskIdSelf() ));  
                printf("NULL SIM passed\n"); 
                return;
        }
        pSim->simq.next    = NULL; 
        pSim->simq.previous    = NULL; 
        pSim->ownership     = 0;
        pSim->origid        = 0;
        pSim->destid        = 0;
        pSim->requestid     = 0;
        pSim->serviceid     = 0;
        pSim->status        = 0;
        pSim->ownerSemId    = NULL;
        pSim->pLayerSim     = NULL;
                
}

#ifndef USE_IMC_MACROS

/*COMMENTS*********************************************************
 *
 *   Func:  imsg_freesim
 *
 *   Desc:  Free SIM
 *
 *   Inputs: tpCOMSIM   pSim   SIM to Free
 *
 *   Outputs: NONE
 *
 *   Outputs: 
 *
 *END*********************************************************************/

void imsg_freesim( tpCOMSIM pSim ) 
{

        /* Make sure no one does it at the same time */
        if ( PSsemTake(pSim->ownerSemId, WAIT_FOREVER ) == ERROR ) { 
                PSpanic("Could not Take Ownership Semaphore ");      
        } 
        else if (pSim->ownership == 0) { 

                /* We are the sole owner... */
                if(pSim->pLayerSim ) {
                        /* We cannot assume that this has been created using
             * IMC type of memory. This should be done using 
             * malloc/calloc ? 
             */
                        PSfree(pSim->pLayerSim); 
                        pSim->pLayerSim = NULL;
                }

                PSsemDelete(pSim->ownerSemId); 
                IMCfree(pSim);

        } 
        else {
                
                /* Someone else also wants it */
                --(pSim->ownership);

                if (PSsemGive(pSim->ownerSemId) == ERROR ) {
                        PSpanic("mFREESIM:Could not Give Ownership Semaphore\n");
                }
        }
}

#endif
