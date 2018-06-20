/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __INTG_SENDDISCTOLAPDM_CPP__
#define __INTG_SENDDISCTOLAPDM_CPP__

#include "lapdm\lapdm_l1intf.h"

void intg_SendDiscToLapdm(void)
{
     int           intg_DiscLen = 14;
     unsigned char intg_DiscData[14] =
     {
        0x0A, // length  = 10
	0x00,
	0x00,
	0x00,
	0x02, //1-1. Protocol Descriminator
	0x00, //1-2. Message Type MSB byte
	0x02, //1-3. Message Type LSB byte
        0x00, //1-4. Trx
        0x20, //1-5. Channel Number MSB byte
        0x00, //1-6. Channel Number LSB byte
        0x00, //1-7. Link Identifier: main DCCH
//        0x0B, //1-8. T1 and T3 hi part
//        0x14, //1-9. T3 low par and T2
//        0x02, //2-10. length of L2 information
        0x03, //2-11. L2 frame Addr.
	0X43, //2-12. L2 frame DISC
	0x01  //2-13. L2 frame length
     };

     CNI_LAPDM_Ph_Callback(
		     intg_DiscLen,
		     intg_DiscData
			);
}

#endif /* #define __INTG_SENDDISCTOLAPDM_CPP__ */
