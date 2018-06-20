/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2000                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* File        : LudbHashTbl.cpp                                     */
/* Author(s)   : Kevin S. Lim                                        */
/* Create Date : 03-25-01 copied from GsHashTbl.cpp                  */
/* Description : supports only IMSI(15) or TMSI(4) key               */
/*                                                                   */
/*********************************************************************/

#include "string.h"
#include "stdio.h"

#include "LudbHashTbl.h"

int ludb_hash_tbl_debug = 0;

LudbHashTbl::LudbHashTbl()
{
	list_len = LUDB_DEFAULT_HASH_TBL_LEN;
	table = new LudbLinkedList[list_len];
	key_len = LUDB_MAX_IMSI_KEY_LEN;
	for(int i=0; i<list_len; i++){
		table[i].SetKeyLength(key_len);
	}
	
	if(ludb_hash_tbl_debug)
		printf("Hash created with length %d\n", list_len);
}

LudbHashTbl::LudbHashTbl(int l_len, int k_len)
{
	list_len = l_len;
	table = new LudbLinkedList[list_len];
	if((k_len == LUDB_MAX_IMSI_KEY_LEN)||(k_len == LUDB_MAX_TMSI_KEY_LEN)){
		key_len = k_len;
		for(int i=0; i<list_len; i++){
			table[i].SetKeyLength(key_len);
		}
	}
	else{
		printf("LudbHashTbl::LudbHashTbl unsupported key len encountered %d\n",k_len);
	}

	if(ludb_hash_tbl_debug)
		printf("Hash created with length %d\n", list_len);
}

LudbHashTbl::~LudbHashTbl()
{

}

int LudbHashTbl::GetHashInd(const void *key)
{
	unsigned long temp = 0;
	int i;

	for(i=0; i<key_len; i++) temp += ((unsigned char *)key)[i];

	if(ludb_hash_tbl_debug)
		printf("LudbHashTbl::HashInd returns %d\n", temp % list_len);

	return (temp % list_len);
}

LudbLinkedListItem *LudbHashTbl::find(const void *key, int& hash_ind)
{
	hash_ind = GetHashInd(key);
	return (table[hash_ind].find(key));
}

LudbLinkedListItem *LudbHashTbl::find(const int value, int& hash_ind)
{
	LudbLinkedListItem *item;
	for(int i=0; i<list_len; i++){
		item = table[i].find(value); 
		if(item != NULL){
			hash_ind = i;
			return item;
		}
	}
	hash_ind = -1;
	return NULL;
}

int LudbHashTbl::add(void *key, const int value)
{
	int hash_ind = GetHashInd(key);
	if(table[hash_ind].find(value) != NULL){
		printf("LudbHashTbl::add() value %d already exists\n", value);
		return -1;
	}
	return (table[hash_ind].add(key, value));
}

int LudbHashTbl::remove(void *key)
{
	int hash_ind = GetHashInd(key);
	if(table[hash_ind].find(key) != NULL){
		return (table[hash_ind].remove(key));
	}
	else{
		printf("LudbHashTbl::remove() key 0x%08x doesn't exist\n", key);
		return -1;
	}
}

int LudbHashTbl::remove(int value)
{
	int hash_ind;
	LudbLinkedListItem *item = find(value, hash_ind);
	if(item != NULL)
		return (table[hash_ind].remove(value));
	else{
		printf("LudbHashTbl::remove() value %d doesn't exist\n", value);
		return -1;
	}
}

void LudbHashTbl::print()
{
	for(int i=0; i<list_len; i++){
		if(table[i].GetNumItems() > 0){
			printf("bucket [%03d] \n", i);
			table[i].print();
		}
	}
}
