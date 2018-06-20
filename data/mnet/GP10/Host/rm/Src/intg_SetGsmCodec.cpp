/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __INTG_SETGSMCODEC_CPP__
#define __INTG_SETGSMCODEC_CPP__

#include "rm\rm_head.h"

void intg_SetGsmCodec(u8 codec)
{
oam_gsmCodec = codec; //0x01-FR, 0x21-EFR, 0x41-superEFR
} /* intg_SetGsmCodec() */

void intg_TuneC1C2(u8 reselOffset, u8 tempOffset, u8 penaltyTime)
{
     printf("1st param--CellReselectOffset: 0-126 dB, 2dB steps; 0=0dB, 1=2dB...\n");
     printf("2nd param--TemporaryOffset:    0-60  dB,10dB steps; 0=0dB, 1=10dB,...,7=infinity\n");
     printf("3rd param--PenaltyTime:        0-30  20sec steps???, 31 meaning differently\n");
//     oam_CellReselectOffset=reselOffset;
//     oam_TemporaryOffset=tempOffset;
//     oam_PenaltyTime=penaltyTime;
     OAMrm_CELL_RESELECT_OFFSETa=reselOffset;
     OAMrm_TEMP_OFFESTa=tempOffset;
     OAMrm_PENALTY_TIMEa=penaltyTime;
     rm_SendSystemInfo3ToBcchTrx(0,3,OAMrm_RA_CELL_BARRED_STATE);
     rm_SendSystemInfo4ToBcchTrx(0,4,OAMrm_RA_CELL_BARRED_STATE);
}


#endif /*__INTG_SETGSMCODEC_CPP__*/
