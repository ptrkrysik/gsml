
/*
 ****************************************************************************************
 *																						*
 *			Copyright Cisco Systems, Inc 2000 All rights reserved						*
 *																						*
 *--------------------------------------------------------------------------------------*
 *																						*
 *	File				: vipermem.cpp													*
 *																						*
 *	Description			: Wrapper functions for memory allocation						*
 *																						*
 *	Author				: Dinesh Nambisan												*
 *																						*
 *----------------------- Revision history ---------------------------------------------*
 * Time  | Date    | Name   | Description 												*
 *--------------------------------------------------------------------------------------*
 * 00:00 |03/24/99 | DSN    | File created												*
 *		 |		   |		|															*
 *		 |		   |	    |															*
 ****************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vxworks.h>



void *ViperAlloc(int BlockSize, unsigned char InitValue)
{
	void *NewBlock=NULL;
	NewBlock = malloc(BlockSize);
	if (NewBlock == NULL)
		return NewBlock;

	memset(NewBlock,InitValue,BlockSize);

	return NewBlock;
}


void ViperFree(void *Block)
{
	free(Block);
}
