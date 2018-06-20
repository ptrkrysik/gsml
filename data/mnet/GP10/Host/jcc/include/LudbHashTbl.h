/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* File        : LudbHashTbl.h                                       */
/* Author(s)   : Kevin S. Lim                                        */
/* Create Date : 03-25-01 copied from GsHashTbl.h                    */
/* Description : supports only IMSI(15) or TMSI(4) key               */
/*                                                                   */
/*********************************************************************/

#ifndef H_LUDB_HASH_TBL
#define H_LUDB_HASH_TBL

#include "LudbLinkedList.h"

#define LUDB_DEFAULT_HASH_TBL_LEN	97	

class LudbHashTbl
{
private:
	LudbLinkedList *table;
	int list_len;
	int key_len;

public:
	LudbHashTbl();
	LudbHashTbl(int l_len, int k_len);
	~LudbHashTbl();

	int GetListLen() { return list_len; }
	int GetKeyLen() { return key_len; }
	virtual int GetHashInd(const void *key);
	virtual LudbLinkedListItem *find(const void *key, int& hash_ind); 
	virtual LudbLinkedListItem *find(const int value, int& hash_ind); 
	virtual int add(void *key, const int value); 
	virtual int remove(void *key); 
	virtual int remove(int value);
	virtual void print();
};

#endif

