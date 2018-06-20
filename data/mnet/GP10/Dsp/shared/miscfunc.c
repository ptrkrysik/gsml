/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
**
** Filename: miscfunc.c
**
** Description:   
**   This file contains various generic/misc routines.
**
*****************************************************************************/
#include <stdio.h>
#undef CHAN_CODEC_DEF
#include "gsmdata.h" 
#include "bbdata.h" 
#include "bbproto.h"

/*****************************************************************************
** ROUTINE NAME: unpackFromByteBuffer
**
** Description:      
**
** Method:
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**    None
**
** Data Store  I/O: 
**
*****************************************************************************/

void unpackFromByteBuffer(UChar *input, UChar *output, Int numBits)
{
   UChar byteRead;
   UChar numBytesProc;
   UChar numFullBytes;
   UChar numRemainBits;
   UInt i;    
   UInt j;
                   
   numBytesProc  = 0;
   numFullBytes  = numBits/8;
   numRemainBits = (numBits - (numFullBytes*8));  

   /*
   * First process the full words in the buffer
   */
   while(numBytesProc < numFullBytes)  
   { 
      byteRead = *input++;
      j = 0x01;
      
      for(i=0; i<8; i++)
      {                   
         if(byteRead & j)
         {
            *output++ = 1;
         }
         else 
         {
            *output++ = 0; 
         }

         j <<= 1;
      }
      
      numBytesProc++;
   }

   /*
   * Now process the remaining bits in the last word
   */
   byteRead = *input;
   j = 0x01;
   
   for(i=0; i < numRemainBits ; i++)
   {                   
      if(byteRead & j)
      {
         *output++ = 1;
      }
      else 
      {
         *output++ = 0; 
      }

      j <<= 1;
   }

}

/*****************************************************************************
** Routine Name: packToByteBuffer
**
** Description: 
**
** Method:
**   
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**
** Data Store I/O: 
**
*****************************************************************************/
void packToByteBuffer(UChar *input, UChar *output, Int numBits)
{
   Int j, i, numFullBytes, numRemainBits, numBytesProc;
   UChar k;
   UChar byte;
         
   i=0;     
   numBytesProc=0;
   numFullBytes = (numBits/8);                 
   numRemainBits = (numBits - (numFullBytes*8));  

   while (i < numBits)                  
   {       
      byte = 0x00;
      if (numBytesProc < numFullBytes)     
      {
         for (j=0,k=0x01; j <8; j++) 
         {   
            if (input[i]==1) 
               byte |= k;
            k <<= 1;  
            i++;
         } 
         *output++ = byte; 
         numBytesProc++;              
      }
      else                
      {
         for (j=0,k=0x01; j < numRemainBits ; j++)
         { 
            if (input[i] ==1) 
               byte |= k;
            k <<= 1;
            i++;
         }
         *output++ = byte;
         numBytesProc++;
      }

   }
 
}   


/*****************************************************************************
** ROUTINE NAME: reverseBits
**
** Description:   
**
** Method:
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**    None
**
** Data Store  I/O: 
**
*****************************************************************************/

UShort reverseBits(UShort value)
{
   UInt i;
   UShort value2;
   Bool bit;

   value2 = 0x0000;

   for(i=0; i<15; i++)
   {
      bit = (value>>i) & 0x0001;
      value2 |= bit;
      value2 <<= 1;
   }

   bit = (value>>15) & 0x0001;
   value2 |= bit;

   return(value2);
}
                                  
                                  
/*****************************************************************************
** ROUTINE NAME: unpackByteMSB
**
** Description:      
**
** Method:
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**    None
**
** Data Store  I/O: 
**
*****************************************************************************/

void unpackByteMSB(UChar *input, UChar *output, Int numBits)
{
   UChar byteRead;
   UChar numBytesProc;
   UChar numFullBytes;
   UChar numRemainBits;
   UInt i;    
   UInt j;
                   
   numBytesProc  = 0;
   numFullBytes  = numBits/8;
   numRemainBits = (numBits - (numFullBytes*8));  

   /*
   * First process the full words in the buffer
   */
   while(numBytesProc < numFullBytes)  
   { 
      byteRead = *input++;

      j = 7;
        
      for(i=0; i<8; i++)
      {                   
         *output++ = (byteRead>>j) & 1;
         j -= 1;
      }
      numBytesProc++;
   }

   /*
   * Now process the remaining bits in the last word
   */
   j = 7;

   for(i=0; i < numRemainBits ; i++)
   {                   
      *output++ = (byteRead>>j) & 1;
      j -= 1;
   }

}              



/*****************************************************************************
** Routine Name: packToByteMSB
**
** Description: 
**
** Method:
**   
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**
** Data Store I/O: 
**
*****************************************************************************/
void packToByteMSB(UChar *input, UChar *output, Int numBits)
{
   Int j, i, numFullBytes, numRemainBits, numBytesProc;
   UChar k;
   UChar byte;
         
   i=0;     
   numBytesProc=0;
   numFullBytes = (numBits/8);                 
   numRemainBits = (numBits - (numFullBytes*8));  

   while (i < numBits)                  
   {       
      byte = 0x00;
      if (numBytesProc < numFullBytes)     
      {
         for (j=0,k=7; j <8; j++) 
         {   
            byte |= input[i]<<k;
            k--;  
            i++;
         } 
         *output++ = byte; 
         numBytesProc++;              
      }
      else                
      {
         for (j=0,k=7; j < numRemainBits ; j++)
         { 
            byte |= input[i]<<k;
            k--;  
            i++;        
         }
         *output++ = byte;
         numBytesProc++;
      }

   }
 
}   



                               