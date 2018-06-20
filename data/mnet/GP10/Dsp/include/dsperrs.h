/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
                          
#undef GPGLOB                
#ifdef GPDEFINE
#define GPGLOB
#else
#define GPGLOB extern
#endif            
    
#define ERROR_EVENT_QUEUE_LENGTH 	16

typedef struct
{
   Uchar    timeslot;	/* timeslot reporting error 0-7 or 255 if not applicable */
   Uchar    channel;		/* channel reporting error.  Use an enum */
   Uchar    ULorDL;		/* event on uplink, downlink or NA */   
   Uint32   frameNumber;/* TDMA frame number of error */
   Uchar    eventCode;	/* error event code.  Use an enum */
} t_errorEventEntry;


typedef Uint32 T_bufferStatusEnum;  /* TBD */
typedef struct{
   t_errorEventEntry    queue[ERROR_EVENT_QUEUE_LENGTH];
   Uchar                giveIndex;
   Uchar                takeIndex;
   T_bufferStatusEnum   conditionFlag;
   Uint16   dlUnderflowCount[NUM_RFS_PER_DSP][NUM_TN_PER_RF];
   Uint16   dlOverflowCount[NUM_RFS_PER_DSP][NUM_TN_PER_RF];
   Uint16   dlOutOfSequenceCount[NUM_RFS_PER_DSP][NUM_TN_PER_RF];
}t_errorEvents;

GPGLOB t_errorEvents g_errorEvents;

/*-------------------------------------------------------------
 * Error enable for sending messages only once per SF 
 *------------------------------------------------------------*/ 
#pragma DATA_SECTION(g_errorEnable, ".extData"); 
GPGLOB Int16 g_errorEnable;


                     