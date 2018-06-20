/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#include "rm\debug.h"

void db_BDump(unsigned char type, unsigned char *pBuf, unsigned short len)
{
   unsigned short	i, j, row, last_row, wrongType;
   const char *rxTitle[] = { "<===L1:", "<===MD:", "<===RM:","<===MM:","<===CC"};
   const char *txTitle[] = { "L1===>:", "MD===>:", "RM===>:","MM===>:","CC===>"};

   /*
   ** Assume first byte of *pBuf is module ID which is used to access the two
   ** string arrays defined above.
   */
 
   wrongType = 0;
  
   switch(type)
   {
   case 
	MAREA__:
        printf("DUMP---\n");
	break;

   case RXMSG__:
	printf("%s\n", rxTitle[*pBuf]);
        break;

   case TXMSG__:
	printf("%s\n", txTitle[*pBuf]);
	break;
   default:
	printf(" WRONG TYPE fed into db_BDump\n" );
	wrongType = 1;
	break;

   } /* End of switch() */

   if ( !wrongType )
   {
	/* 
        ** Count how many rows to print and if there
        ** a non-full row 
        */
       
        row = len/ROW_LEN; last_row = len%ROW_LEN;

        /* Print all full rows */
	if ( row )
        { 
	     for ( i=0; i<row; i++ )
             {
                   for ( j=0;j<ROW_LEN;j++ )
			 printf("0x%02x ",*pBuf++);
		   printf("\n");
	     }
        }
        
 	/* Print last row if any */      
	if ( last_row )
	{            
	    for ( i=0; i<last_row; i++)
            	printf("0x%02x ", *pBuf++);

	    printf("\n");
        }
   }

} /* End of db_BDump() */

