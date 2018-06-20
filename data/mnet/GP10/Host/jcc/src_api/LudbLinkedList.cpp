/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2001                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* File        : LudbLinkedList.cpp                                  */
/* Author(s)   : Kevin S. Lim                                        */
/* Create Date : 03-25-01 copied from GsLinkedList.cpp               */
/* Description : A generic linked list made for hash table entries   */
/*               by adding new items to the tail                     */
/*               It can have two kind of keys: IMSI or TMSI          */
/*               IMSI will have 15 bytes characters as digit values  */
/*               TMSI will have 32 bit values - unsigned long        */
/*                                                                   */
/*********************************************************************/

#include "string.h"
#include "stdio.h"

#include "LudbLinkedList.h"

int ludb_linked_list_debug = 0;

/*********************************************************************/
LudbLinkedListItem::LudbLinkedListItem()
{
	prev = NULL;
	next = NULL;
	key_len = LUDB_MAX_IMSI_KEY_LEN;
}

LudbLinkedListItem::LudbLinkedListItem(const int len, void *key, const int value)
{
	prev = NULL;
	next = NULL;

	if(len == LUDB_MAX_IMSI_KEY_LEN){
		key_len = len;
		for(int i=0; i<LUDB_MAX_IMSI_KEY_LEN; i++){
			ckey[i] = ((unsigned char *)key)[i];
		}
	}
	else if(len == LUDB_MAX_TMSI_KEY_LEN){
		key_len = LUDB_MAX_TMSI_KEY_LEN;
		lkey = *(unsigned long *)key;
	}
	else{
		printf("LudbLinkedListItem::LudbLinkedListItem() unsupported hash key length encountered %d\n", len);
	}
	this->value = value;
}

LudbLinkedListItem::~LudbLinkedListItem()
{

}

int LudbLinkedListItem::same_keys(const void *key)
{
	if(key_len == LUDB_MAX_TMSI_KEY_LEN){
		return(lkey == *(unsigned long *)key);
	}
	else{
		for(int i=0; i<key_len; i++){
			if(ckey[i] != ((unsigned char *)key)[i])
				return 0;
		}
		return 1;
	}
}

void LudbLinkedListItem::print()
{
	if(key_len == LUDB_MAX_TMSI_KEY_LEN){
		printf("key: 0x%08x value: %03d\n", lkey, value);
	}
	else{
		printf("key: ");
		for(int i=0; i<key_len; i++){
			printf("%d", ckey[i]);
		}
		printf(" value: %03d\n", value);
	}
}

/*********************************************************************/
LudbLinkedList::LudbLinkedList()
{
	head = NULL;
	tail = NULL;
	num_items = 0;
	key_len = LUDB_MAX_LIST_KEY_LEN;

	if(ludb_linked_list_debug)
		printf("Hash list created\n");
}

LudbLinkedList::LudbLinkedList(int len)
{
	head = NULL;
	tail = NULL;
	num_items = 0;
	key_len = len;

	if(ludb_linked_list_debug)
		printf("Hash list created\n");
}

LudbLinkedList::~LudbLinkedList()
{

}

void LudbLinkedList::SetKeyLength(int len)
{
   key_len = len;
}

LudbLinkedListItem *LudbLinkedList::find(const void *key)
{
	LudbLinkedListItem *item;

	item = head;
	while(item != NULL){
		if(item->same_keys(key)){
			return item;
		}
		else{
			item = item->next;
		}
	}
	return NULL;
}

LudbLinkedListItem *LudbLinkedList::find(const int value)
{
	LudbLinkedListItem *item;

	item = head;
	while(item != NULL){
		if(item->value == value){
			return item;
		}
		else{
			item = item->next;
		}
	}
	return NULL;
}

int LudbLinkedList::add(void *key, const int value)
{
	LudbLinkedListItem *temp = tail;

	if(head == NULL){		// first one
		head = tail	= new LudbLinkedListItem(key_len, key, value);
	}
	else{
		temp->next = new LudbLinkedListItem(key_len, key, value);
		tail = tail->next;
		tail->prev = temp;
	}
	num_items++;
	return 0;
}

int LudbLinkedList::remove(LudbLinkedListItem *item)
{	
	if(item == tail){
		if(item == head){			// tail and head
			head = NULL;
			tail = NULL;
		}
		else{						// tail, not head
			tail = tail->prev;
			tail->next = NULL;
		}
	}			
	else{
		if(item == head){			// not tail, but head 
			head = head->next;
			head->prev = NULL;
		}
		else{						// not tail, not head
			item->prev->next = item->next;
			item->next->prev = item->prev;
		}
	}	
		
	delete item;
	num_items--;
	return 0;
}

int LudbLinkedList::remove(void *key)
{
	LudbLinkedListItem *item = find(key);
	if(item == NULL) return -1;
	else remove(item);
	return 0;
}

int LudbLinkedList::remove(int value)
{
	LudbLinkedListItem *item = find(value);
	if(item == NULL) return -1;
	else remove(item);
	return 0;
}

void LudbLinkedList::print()
{
	if(head == NULL){
		printf("EMPTY\n");
		return;
	}

	LudbLinkedListItem *item = head;
	printf("--------------------------------------------------------------\n");
	while(item != NULL){
		printf(" ");
		item->print();		
		item = item->next;
	}
	printf("--------------------------------------------------------------\n");
}
