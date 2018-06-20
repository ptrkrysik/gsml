/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
**
** Description:   
**   Contains functions that perform SID code detection from a RTP frame of
**   264 bits
**
*****************************************************************************/
#include "gsmdata.h" 
#include "bbdata.h" 
#include "bbproto.h"  
#include "codec.h"

#define GSM_FR_SID_LEN  95

/*
*   GSM SID bit positions in 264-bits Traffic frame
*/     
Uint16 const gsmFrSidBits [GSM_FR_SID_LEN] = 
{ 
  58-4, 59-4, 61-4, 62-4, 64-4, 65-4, 67-4, 68-4, 70-4, 71-4, 73-4, 74-4, 76-4, 77-4, 79-4, 80-4, 82-4, 83-4, 85-4, 86-4, 88-4, 89-4, 91-4, 92-4, 94-4, 95-4, 114-4,
  115-4, 117-4, 118-4, 120-4, 121-4, 123-4, 124-4, 126-4, 127-4, 129-4, 130-4, 132-4, 133-4, 135-4, 136-4, 138-4, 139-4, 141-4, 142-4, 144-4, 145-4, 147-4,
  148-4, 150-4, 151-4, 170-4, 171-4, 173-4, 174-4, 176-4, 177-4, 179-4, 180-4, 182-4, 183-4, 185-4, 186-4, 188-4, 189-4, 191-4, 192-4, 194-4, 195-4, 197-4,
  198-4, 200-4, 201-4, 203-4, 204-4, 206-4, 207-4, 226-4, 227-4, 229-4, 230-4, 232-4, 233-4, 235-4, 236-4, 238-4, 241-4, 244-4, 247-4, 250-4, 253-4, 256-4, 259-4,
  262-4
};  

#define GSM_EFR_SID_LEN 95
static const Uint8 gsmEfrSidBits[GSM_EFR_SID_LEN] =
{
     45,  46,  48,  49,  50,  51,  52,  53,  54,  55,
     56,  57,  58,  59,  60,  61,  62,  63,  64,  65,
     66,  67,  68,  94,  95,  96,  98,  99, 100, 101,
    102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 148, 149, 150,
    151, 152, 153, 154, 155, 156, 157, 158, 159, 160,
    161, 162, 163, 164, 165, 166, 167, 168, 169, 170,
    171, 196, 197, 198, 199, 200, 201, 202, 203, 204,
    205, 206, 207, 208, 209, 212, 213, 214, 215, 216,
    217, 218, 219, 220, 221
};

Uint8 sidDetection(Uint8 *speechBits, t_vocAlgo vocoAlgo)
{

   Uint8  i;
  
   Uint8  sidCode = 0;
   Uint8  sidFlag;

   switch( vocoAlgo )
   {
      case VOC_GSMF:
         for(i=0; i<GSM_FR_SID_LEN; i++)
         { 
            sidCode += speechBits[gsmFrSidBits[i]];
         }

		 if(sidCode < 2)
		 {
			sidFlag = 2;
		 }
		 else if(sidCode < 16)
		 {
            sidFlag = 1;
		 }
		 else 
			sidFlag = 0;

		 break;
       
      case VOC_EFR:
         for(i=0; i<GSM_FR_SID_LEN; i++)
         { 
            sidCode += speechBits[gsmEfrSidBits[i]];
         }

		 if(sidCode > 93)
		 {
			sidFlag = 2;
		 }
		 else if(sidCode > 79)
		 {
            sidFlag = 1;
		 }
		 else 
			sidFlag = 0;

         break;

	  default:
         sidFlag = 0;
		 break;
   }    
   return(sidFlag);
}