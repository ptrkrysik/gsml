/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
**
** Description:   
**   Contains functions that perform TCH bad frame detection.
**
*****************************************************************************/
#include "gsmdata.h" 
#include "bbdata.h" 
#include "bbproto.h"  
#include "codec.h"
#include "diagdata.h"

/*****************************************************************************
** Routine Name: DetectBFI
**
** Description: 
**   Performs subjective encoding per GSM 06.10 table A2.1a for Full-rate
**   speech.   
**
** Method:
**   Any frame with a bad CRC check is marked as bad.  However, in an
**   "open-air" envionment, there is a 1-in-8 chance (there are 3 parity
**   bits) that the CRC will fail to detect errors.  Also CRC only checks
**   classIa bits.  Therefore, the Viterbi path metric of the convolutional
**   decoder is also examined.  If the metric is below a pre-determined
**   threshold, the frame is marked as bad.
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O:
**   perceptualMask (T/F) indicates whether to mask the voice frame 
**
** Returns:
**   Boolean indicating BFI status: TRUE-bad frame, FALSE-good frame.
**
** Data Store I/O: 
**
*****************************************************************************/

Bool detectBFI(t_chanSel chanSel, Bool parityError, UInt decoderMetric,
               UInt totalErrors, Bool *perceptualMask)
{
   Bool badFrame;

   /*
   * Initialize bad frame and perceptual masking to false. Set TRUE later
   * if any appropriate condition exists.
   */
   badFrame = FALSE;
   *perceptualMask = FALSE;
      
   /*
   * Each channel can have a different BFI algorithm
   */
   switch (chanSel)
   {
   case RACH:      /* RACH */
      /*
      * If CRC error, set BFI
      */
      if (parityError)
      {
         badFrame = TRUE;
      }
      /*
      * If viterbi metric is too low, set BFI
      */
      if (decoderMetric < g_RachMetricThresh)
      {
         badFrame = TRUE;
      }
      break;

   case TCHF:      /* Fullrate speech traffic */
      /*
      * If CRC error, set BFI
      */
      if (parityError)
      {
         badFrame = TRUE;
      }
      /*
      * If viterbi metric is too low or BER is too high, set 
      * perceptual masking indicator. Do not count toward BFI.
      */
      if ( decoderMetric < g_TchMetricThresh ||
           totalErrors   > g_TchBerThresh )
      {
         *perceptualMask = TRUE;
      }
      break;

   default:      /* Other channel types not supported */
      break;

   } /*end switch */

   return(badFrame);
}
