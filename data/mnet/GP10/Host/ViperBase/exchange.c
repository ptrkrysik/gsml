/* exchange.c
 * $Id: exchange.c,v 1.4 1999-07-30 10:38:38-07 olet Exp $
 * Non Standard Interface between GK and H.323 task
 */

#include "support.h"
#include "exchange.h"

/*
 *
 */
void jcNonStdRasMessageRRQPack( NonStdRasMessageRRQ_t* pMsgRRQ, UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsgRRQ->pszMobileID, ppszBuffer, pnSize );
   jcULongPack( pMsgRRQ->hMobileCell, ppszBuffer, pnSize );

   return;
}

/*
 *
 */
void jcNonStdRasMessageRRQUnpack( NonStdRasMessageRRQ_t* pMsgRRQ, UCHAR** ppszBuffer )
{
   pMsgRRQ->pszMobileID = jcStringUnpack( ppszBuffer );
   pMsgRRQ->hMobileCell = jcULongUnpack( ppszBuffer );

   return;
}

/*
 *
 */
void jcNonStdRasMessageRCFPack( NonStdRasMessageRCF_t* pMsgRCF, UCHAR** ppszBuffer, int* pnSize )
{
   jcULongPack( pMsgRCF->hMobileGK, ppszBuffer, pnSize );
   jcULongPack( pMsgRCF->hMobileCell, ppszBuffer, pnSize );
   jcStringPack( pMsgRCF->pszNumber, ppszBuffer, pnSize );

   return;
}

/*
 *
 */
void jcNonStdRasMessageRCFUnpack( NonStdRasMessageRCF_t* pMsgRCF, UCHAR** ppszBuffer )
{
   pMsgRCF->hMobileGK = jcULongUnpack( ppszBuffer );
   pMsgRCF->hMobileCell = jcULongUnpack( ppszBuffer );
   pMsgRCF->pszNumber = jcStringUnpack( ppszBuffer );

   return;
}

/*
 *
 */
void jcNonStdRasMessageRRJPack( NonStdRasMessageRRJ_t* pMsgRRJ, UCHAR** ppszBuffer, int* pnSize )
{
   jcULongPack( pMsgRRJ->hMobileCell, ppszBuffer, pnSize );

   return;
}

/*
 *
 */
void jcNonStdRasMessageRRJUnpack( NonStdRasMessageRRJ_t* pMsgRRJ, UCHAR** ppszBuffer )
{
   pMsgRRJ->hMobileCell = jcULongUnpack( ppszBuffer );

   return;
}

/*
 *
 */
void jcNonStdRasMessageURQPack( NonStdRasMessageURQ_t* pMsgURQ, UCHAR** ppszBuffer, int* pnSize )
{
   jcULongPack( pMsgURQ->hMobileGK, ppszBuffer, pnSize );
   jcULongPack( pMsgURQ->hMobileCell, ppszBuffer, pnSize );

   return;
}

/*
 *
 */
void jcNonStdRasMessageURQUnpack( NonStdRasMessageURQ_t* pMsgURQ, UCHAR** ppszBuffer )
{
   pMsgURQ->hMobileGK = jcULongUnpack( ppszBuffer );
   pMsgURQ->hMobileCell = jcULongUnpack( ppszBuffer );

   return;
}

/*
 *
 */
void jcNonStdRasMessageUCFPack( NonStdRasMessageUCF_t* pMsgUCF, UCHAR** ppszBuffer, int* pnSize )
{
   jcULongPack( pMsgUCF->hMobileGK, ppszBuffer, pnSize );
   jcULongPack( pMsgUCF->hMobileCell, ppszBuffer, pnSize );

   return;
}

/*
 *
 */
void jcNonStdRasMessageUCFUnpack( NonStdRasMessageUCF_t* pMsgUCF, UCHAR** ppszBuffer )
{
   pMsgUCF->hMobileGK = jcULongUnpack( ppszBuffer );
   pMsgUCF->hMobileCell = jcULongUnpack( ppszBuffer );

   return;
}

/*
 *
 */
void jcNonStdRasMessageCellLRQPack( NonStdRasMessageCellLRQ_t* pMsgCellLRQ,
                                    UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsgCellLRQ->pszCellID, ppszBuffer, pnSize );
   jcULongPack( pMsgCellLRQ->hCell, ppszBuffer, pnSize );

   return;
}

/*
 *
 */
void jcNonStdRasMessageCellLRQUnpack( NonStdRasMessageCellLRQ_t* pMsgCellLRQ, UCHAR** ppszBuffer )
{
   pMsgCellLRQ->pszCellID = jcStringUnpack( ppszBuffer );
   pMsgCellLRQ->hCell = jcULongUnpack( ppszBuffer );

   return;
}

/*
 *
 */
void jcNonStdRasMessageCellLCFPack( NonStdRasMessageCellLCF_t* pMsgCellLCF,
                                    UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsgCellLCF->pszCellID, ppszBuffer, pnSize );
   jcStringPack( pMsgCellLCF->pszIpAddress, ppszBuffer, pnSize );
   jcULongPack( pMsgCellLCF->hCell, ppszBuffer, pnSize );

   return;
}

/*
 *
 */
void jcNonStdRasMessageCellLCFUnpack( NonStdRasMessageCellLCF_t* pMsgCellLCF, UCHAR** ppszBuffer )
{
   pMsgCellLCF->pszCellID = jcStringUnpack( ppszBuffer );
   pMsgCellLCF->pszIpAddress = jcStringUnpack( ppszBuffer );
   pMsgCellLCF->hCell = jcULongUnpack( ppszBuffer );

   return;
}

/*
 *
 */
void jcNonStdRasMessageCellLRJPack( NonStdRasMessageCellLRJ_t* pMsgCellLRJ,
                                    UCHAR** ppszBuffer, int* pnSize )
{
   jcStringPack( pMsgCellLRJ->pszCellID, ppszBuffer, pnSize );
   jcULongPack( pMsgCellLRJ->hCell, ppszBuffer, pnSize );

   return;
}

/*
 *
 */
void jcNonStdRasMessageCellLRJUnpack( NonStdRasMessageCellLRJ_t* pMsgCellLRJ, UCHAR** ppszBuffer )
{
   pMsgCellLRJ->pszCellID = jcStringUnpack( ppszBuffer );
   pMsgCellLRJ->hCell = jcULongUnpack( ppszBuffer );

   return;
}

/*
 *
 */
void jcNonStdRasMessageMobileProfileRequestPack( NonStdRasMessageMobileProfileRequest_t* pMsg,
                                                 UCHAR** ppszBuffer, int* pnSize )
{
   jcULongPack( pMsg->hMobileGK, ppszBuffer, pnSize );
   jcULongPack( pMsg->hMobileCell, ppszBuffer, pnSize );

   return;
}

/*
 *
 */
void jcNonStdRasMessageMobileProfileRequestUnpack( NonStdRasMessageMobileProfileRequest_t* pMsg,
                                                   UCHAR** ppszBuffer )
{
   pMsg->hMobileGK = jcULongUnpack( ppszBuffer );
   pMsg->hMobileCell = jcULongUnpack( ppszBuffer );

   return;
}

/*
 *
 */
void jcNonStdRasMessageMobileProfilePack( NonStdRasMessageMobileProfile_t* pMsg,
                                          UCHAR** ppszBuffer, int* pnSize )
{
   jcULongPack( pMsg->hMobileGK, ppszBuffer, pnSize );
   jcULongPack( pMsg->hMobileCell, ppszBuffer, pnSize );
   jcUShortPack( pMsg->nProfileSize, ppszBuffer, pnSize );
   jcBufferPack( pMsg->pProfileData, pMsg->nProfileSize, ppszBuffer, pnSize );

   return;
}

/*
 *
 */
void jcNonStdRasMessageMobileProfileUnpack( NonStdRasMessageMobileProfile_t* pMsg,
                                            UCHAR** ppszBuffer )
{
   pMsg->hMobileGK = jcULongUnpack( ppszBuffer );
   pMsg->hMobileCell = jcULongUnpack( ppszBuffer );
   pMsg->nProfileSize = jcUShortUnpack( ppszBuffer );
   pMsg->pProfileData = jcBufferUnpack( pMsg->nProfileSize, ppszBuffer );

   return;
}
