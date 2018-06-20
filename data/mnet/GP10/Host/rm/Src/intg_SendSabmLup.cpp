/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __INTG_SENDSABMLUP_CPP__
#define __INTG_SENDSABMLUPTOLAPDM_CPP__

#include "lapdm\lapdm_l1intf.h"
void intg_SendSabmLupToLapdm(void);

void intg_SendSabmLupToLapdm(void)
{
     int           intg_SabmLupLen = 34;
     unsigned char intg_SabmLupData[34] =
     {
        0x1C, // length  = 28
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
        0x0B, //1-8. T1 and T3 hi part
        0x14, //1-9. T3 low par and T2
//        0x15, //2-10. length of L3 information
        0x03, //2-11. command
	0x3f, //2-12. poll bit 1
        0x49, //2-13. length:=18, See 04.06/3.6
        0x05, //3-14. SI=0, Protocol Descriminator=5
        0x08, //3-15. SD=0, Message Type=8
        0x70, //3-16. No CKSN, normal LUP
        0x00, //3-17. LAI: MCC1,MCC2
        0xF1, //3-18. LAI: MCC3
        0x10, //3-19. LAI: NNC1, MNC2
        0x00, //3-20. LAI: LAI 1
        0x01, //3-21. LAI: LAI 2
        0x2C, //3-22. Classmark 1: Rev=01(Ph2), ES=0(No ES IND), A5/1=0, RF Pow=011 (Class3)
        0x08, //3-23. Mobile Identity IE length
        0x09, //3-24. Digit 1=1. Odd, IMSI
        0x10, //3-25. digit 2, digit 3
        0x10, //3-26. digit 4, digit 5
        0x00, //3-27. digit 6, digit 7
        0x00, //3-28. digit 8, digit 9
        0x00, //3-29. digit 10, digit 11
        0x00, //3-30. digit 12, digit 13
        0x10  //3-31. digit 14, digit 15
     };

     CNI_LAPDM_Ph_Callback(
		     intg_SabmLupLen,
		     intg_SabmLupData
			);
}

#endif /* #define __INTG_SENDSABMLUPTOLAPDM_CPP__ */
