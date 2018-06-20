// *******************************************************************
//
// (c) Copyright CISCO Systems, Inc 2001
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : debug.cpp
// Author(s)   : Igal Gutkin
// Create Date : 8/10/01
// Description :  
//
// *******************************************************************

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#include "dbgfunc.h"


/* print debug messages directly to the stdio */
void dbg_print (char *format, ...) 
{
    va_list  marker;
    va_start(marker, format);   /* Initialize variable arguments. */
    vprintf (format, marker);
    fflush  (stdout);
    va_end  (marker);           /* Reset variable arguments. */
}


/*  Put the inbound data in a hex dump format into stdio
      Arguments:          
        pSrc     - input data     
        length      - specifies the number of bytes to dump
        pDest    - output buffer                         */
void HexDump (unsigned char *buffer, int length)
{
    FormatHexDump (buffer, length, NULL);
}


/*  Put the inbound data in a hex dump format into the buffer
      Arguments:          
        pSrc     - input data     
        length      - specifies the number of bytes to dump
        pDest    - output buffer, if NULL print to stdio */
void FormatHexDump (unsigned char *pSrc, int length, char *pDest)
{
  int size   ;
  int idx    ;
  int dwPos  = 0;
  int offset = 0;

    while (length > 0)
    {
        // Take min of 16 or length
        size = min (16, length);

        // Build text line
        if (pDest)
            offset += sprintf (pDest+offset, "  %04X ", dwPos);
        else
            printf  ("  %04X ", dwPos);

        for (idx = 0; idx < size; idx++)
        {
            if (pDest)
                offset += sprintf (pDest+offset, "%02X ", pSrc[idx]);
            else
                printf  ("%02X ", pSrc[idx]);
        }

        // Add spaces for short lines
        while (idx < 16)
        {
            if (pDest)
                offset += sprintf (pDest+offset, "   ");
            else
                printf  ("   ");
            idx++;
        }

        // Add ASCII chars
        for (idx = 0; idx < size; idx++)
        {
            if (pDest)
                offset += sprintf (pDest+offset, "%c", (isprint(pSrc[idx])) ? pSrc[idx] : '.');
            else
                printf  ("%c", (isprint(pSrc[idx])) ? pSrc[idx] : '.');
        }

        if (pDest)
            offset += sprintf (pDest+offset, "\n");
        else
            printf  ("\n");

        // Advance positions
        length -= size;
        dwPos  += size;
        pSrc += size;
    }
}


/*  Put the inbound data in a plain hex format into the buffer
      Arguments:          
        pSrc     - input data     
        length      - specifies the number of bytes to dump
        pDest    - output buffer                         */
void PlainHexDump (unsigned char *pSrc, int length, char *pDest)
{
  int  offset = 0;

    for (int count = 0; count < length; count++)
    {
        if (pDest)
            offset += sprintf (pDest+offset, "%02x ", *(pSrc+count));
        else
            printf ("%02x ", *(pSrc+count));
    }

    if (pDest)
        offset += sprintf (pDest+offset, "\n");
    else
        printf ("\n");
}


void DbgCheckPoint (char * pFile, int line)
{
  static unsigned count = 0;
    
  printf ("Checkpoint num. %u at %s, line %u\n", ++count, pFile, line);
}
