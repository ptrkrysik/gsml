/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __INTG_SENDPHRAINDTOLAPDM_CPP__
#define __INTG_SENDPHRAINDTOLAPDM_CPP__

#include "lapdm\lapdm_l1intf.h"
void intg_SendPhRaIndToLapdm(void);

void intg_SendPhRaIndToLapdm(void)
{
     int           intg_PhRaIndLen = 19;
     unsigned char intg_PhRaIndData[19] =
     {
          0x10,
	  0x00,
	  0x00,
	  0x00,
	  0x02, //1. Protocol Descriminator
	  0x02, //2. Message Type MSB byte
	  0x00, //3. Message Type LSB byte
          0x00, //4. Trx
          0x88, //5. Channel Number MSB byte
          0x00, //6. Channel Number LSB byte
          0x20, //7. Link Identifier: N.A case
        //0x04, //8. CCCH block on which RA received
          0x0D, //9  Ref #: estCause=LUP, random no=13. NECI=0
          0x0B, //10. T1 and T3 hi part
          0x14, //11. T3 low par and T2
          0x00, //12. Access delay
          0x00, //13. SNIR byte 1 (MSB)
          0x00, //14. SNIR byte 2
          0x00, //15. SNIR byte 3
          0x00  //16. SNIR byte 4 (LSB)
     };

     CNI_LAPDM_Ph_Callback(
		     intg_PhRaIndLen,
		     intg_PhRaIndData
			);
}

#endif /* __INTG_SENDPHRAINDTOLAPDM_CPP__ */
