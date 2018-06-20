/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
**
** Filename: fileio.c
**
** Description:   
**   This file contains misc test functions in support of the test environemt. 
**   This file nor the routines in this file should be used in the actual
**   deliverable code.
** 
*****************************************************************************/
#include <stdio.h>
#undef CHAN_CODEC_DEF
#include "gsmdata.h"                
#include "bbdata.h" 
#include "bbproto.h"  

/*****************************************************************************
** Routine Name: unpackFromFile
**
** Description: 
**   Reads packed data from the specified file into an unpacked array.
**   numWords 16 bits words are read from the input file and stored in
**   the input array unpacked.   
**
** Method:
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**    f - specifies the input file to read
**    input - the unpacked array to store the unpacked file data
**    numWords - number of 16 bit words to read from the file
**
** Returns: 
**    Int res
**
** Data Store I/O: 
**
*****************************************************************************/
Int unpackFromFile(FILE *f,Char *input, Char numWords)                                                         
{                                                                           
   Int byteRead, res, numWordsProc;
   UInt i;    
   UInt j;
                                         
   numWordsProc=0;

   res=1;

   while ( (numWordsProc <numWords) & (res != EOF) & (res != 0) )  
   { 
      res=fscanf(f,"%x", &byteRead);
      
      if ((res ==0) | (res==EOF)) 
         return(res);  
        
      for (i=0,j=0x8000; i < 16 ; i++)
      {                   
         if (byteRead & j) 
            *input++=1;
         else 
            *input++=0; 

         j = (j/2);
      }
      
      numWordsProc++;
   }
   return(numWordsProc);
}

/*****************************************************************************
** Routine Name: packToFile
**
** Description: 
**   Writes unpacked data from the specified array into a packed file.
**   numBits bits are read from the input array and stored in the packed file.
**   The leftover bits in the last word if there are any are coded as "0".
**
** Method:
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**    f - specifies the output file to write
**    output - the unpacked array to write out
**    numBits - number of bits to write to the file
**
** Returns: 
**    None
**
** Data Store I/O: 
**
*****************************************************************************/

void packToFile(FILE *f, UChar *output, Int numBits)
{
   Int j, i, word, numFullWords, numRemainBits, numWordsProc;
   UInt k;
         
   i=0;     
   numWordsProc=0;
   numFullWords = (numBits/16);                 
   numRemainBits = (numBits - (numFullWords*16));  

   while (i < numBits)                  
   {       
      word = 0x0000;
      if (numWordsProc < numFullWords)     
      {
         for (j=0,k=0x8000; j <16; j++) 
         {   
            if (output[i] ==1) word |= k;
               k = (k / 2);  
            i++;
         }  
         fprintf(f,"%04x\n", word); 
         numWordsProc++;              
      }
      else                
      {
         for (j=0,k=0x8000; j < numRemainBits ; j++)
         { 
            if (output[i] ==1) word |= k;
               k = (k / 2);
            i++;
         }
          
         fprintf(f,"%04x\n", word); 
         numWordsProc++;
      }

   }
 
}                  