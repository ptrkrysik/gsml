/*********************************************************************/
/*                                                                   */
/* (c) Copyright Cisco 2001                                          */
/* All Rights Reserved                                               */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/* File        : LudbLinkedList.h                                    */
/* Author(s)   : Kevin S. Lim                                        */
/* Create Date : 03-25-01 copy from GsLinkedList.h                   */
/* Description : A generic linked list made for hash table entries   */
/*               by adding new items to the tail                     */
/*               It can have two kind of keys: IMSI or TMSI          */
/*               IMSI will have 15 bytes characters as digit values  */
/*               TMSI will have 32 bit values - unsigned long        */
/*                                                                   */
/*               Initially we will only use IMSI hash table.         */
/*                                                                   */
/*********************************************************************/

#ifndef H_LUDB_LINKED_LIST
#define H_LUDB_LINKED_LIST

#define LUDB_MAX_LIST_KEY_LEN			20
#define LUDB_MAX_IMSI_KEY_LEN			15
#define LUDB_MAX_TMSI_KEY_LEN			 4

class LudbLinkedListItem {
public:
	LudbLinkedListItem *prev;
	LudbLinkedListItem *next;
	int 			key_len;
	unsigned char	ckey[LUDB_MAX_LIST_KEY_LEN];
	unsigned long	lkey;
	int				value;	// value to store, usually other table index 

	LudbLinkedListItem();
	LudbLinkedListItem(const int len, void *key, const int value);
	~LudbLinkedListItem();
	virtual int same_keys(const void *key);
	virtual void print();
};

class LudbLinkedList
{
private:
	LudbLinkedListItem *head;
	LudbLinkedListItem *tail;
	int num_items;
	int key_len;

public:
	LudbLinkedList();
	LudbLinkedList(int len);
	~LudbLinkedList();

	void SetKeyLength(int len);
	int GetKeyLength() { return key_len; }
	int GetNumItems() { return num_items; }
	virtual LudbLinkedListItem *find(const void *key); 
	virtual LudbLinkedListItem *find(const int value); 
	virtual int add(void *key, const int value); 
	virtual int remove(LudbLinkedListItem *item);
	virtual int remove(void *key); 
	virtual int remove(int value);
	virtual void print();
};

#endif

