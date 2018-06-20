#include <stdlib.h>
#include <oam_trap.h>


ListNodePtr oam_newListNode(MibTag tag, MSG_Q_ID qid) 
{
	ListNodePtr newNode = (ListNodePtr) malloc (sizeof (ListNode));
	if (newNode != NULL)
	{
		newNode->tag = tag;
		newNode->qid = qid;
		newNode->next = NULL;
	}
	
	return newNode;
}


STATUS oam_insertNode(ListHeadPtr head, ListNodePtr aNode)
{
	if (head != NULL && aNode != NULL)
	{
		if (head->first == NULL)
		{
			head->first = head->last = aNode;
			head->nodeCount = 1;
		} else {
			
			aNode->next = head->first;
			head->first = aNode;
			++head->nodeCount;
		}
		return OK;
	}
	return STATUS_INVALID_PARAMS;
	
}

STATUS oam_insertNodeSortedByTag(ListHeadPtr head, ListNodePtr aNode)
{
	
	ListNodePtr prevNode, curNode;
	int tblId;

	if ( head != NULL && aNode != NULL)
	{

		prevNode = NULL;
		curNode =  head->first;
		tblId = MIB_TBL(aNode->tag);
		
		while( (curNode != NULL) && ((MIB_TBL(curNode->tag) < tblId ) 
			                   || ((MIB_TBL(curNode->tag) == tblId ) 
							      && (curNode->mid <= aNode->mid ))))

		{
			prevNode = curNode;
			curNode = curNode->next;
		}

		
		if (prevNode == NULL)
		{
			/* insert at the beginning */
			aNode->next = head->first;
			head->first = aNode;
		} else {
			
			aNode->next = curNode;
			prevNode->next = aNode;
		}
		
		if (curNode == NULL)
		{
			/* insert at the end */
			head->last = aNode;
		}
		
		++head->nodeCount;
		return OK;
	}
	return STATUS_INVALID_PARAMS;
	
}


STATUS oam_appendNode(ListHeadPtr head, ListNodePtr aNode)
{
	
	if (head != NULL && aNode != NULL)
	{
		aNode->next = NULL;
		if (head->last == NULL)
		{
			head->first = head->last = aNode;
			head->nodeCount = 1;
		} else {
			
			head->last->next = aNode;
			head->last = aNode;
			++head->nodeCount;
		}
		return OK;
	}
	return STATUS_INVALID_PARAMS;
}

ListNodePtr oam_getNodeByID(ListHeadPtr head, MSG_Q_ID id)
{
	
	ListNodePtr tempNode = NULL;
	
	if(head != NULL)
	{
		tempNode = head->first;
		
		while(tempNode != NULL)
		{
			if (tempNode->qid == id)
				break;
			tempNode = tempNode->next;
		}
	}
	return tempNode;
}

ListNodePtr oam_getFirstNodeByTag(ListHeadPtr head, INT_32_T tag)
{
	
	ListNodePtr tempNode = NULL;
	int tblId = MIB_TBL(tag);
	
	if(head != NULL)
	{
		tempNode = head->first;
		
		while(tempNode != NULL)
		{
			if (MIB_TBL(tempNode->tag) >= tblId)
				break;
			tempNode = tempNode->next;
		}
	}
	return tempNode;
}


STATUS oam_deleteNode(ListHeadPtr head, MibTag tag, OCTET_T mid)
{
	
	ListNodePtr prevNode, curNode;
	int tblId = MIB_TBL(tag);

	
	if ( head != NULL)
	{
		prevNode = NULL;
		curNode =  head->first;

		/* skip until table id is smaller than the current one */
		while(curNode != NULL && (MIB_TBL(curNode->tag) < tblId))
		{
			prevNode = curNode;
			curNode = curNode->next;

		}
		
		/* search for the module Id and tag */
		while(curNode != NULL && (MIB_TBL(curNode->tag) == tblId) && (curNode->mid != mid) &&  (curNode->tag != tag))
		{
		
			prevNode = curNode;
			curNode = curNode->next;

		}
		
		if (curNode != NULL  && (MIB_TBL(curNode->tag) == tblId))
		{

			/* Node found */
			
			if(curNode == head->first || curNode == head->last)
			{
				if ( curNode == head->first) 
				{
					head->first = head->first->next;
				}
				if (curNode == head->last)
				{
					head->last = prevNode;
					if(prevNode != NULL)
						prevNode->next = NULL;
				}
			} else {
				prevNode->next = curNode->next;
			}
			
			--head->nodeCount;
			free(curNode);
			return OK;
		}
		return STATUS_NOT_FOUND;
	}
	return STATUS_INVALID_PARAMS;
	
}

STATUS oam_deleteNodeByModule(ListHeadPtr head, OCTET_T mid)
{
	
	ListNodePtr prevNode, curNode, nextNode;
	
	if ( head != NULL)
	{
		prevNode = NULL;
		curNode =  head->first;

		while(curNode != NULL)
		{
			if (curNode->mid == mid)
			{
				nextNode = curNode->next;
				if(curNode == head->first || curNode == head->last)
				{
					if ( curNode == head->first) 
					{
						head->first = head->first->next;
					}
					if (curNode == head->last)
					{
						head->last = prevNode;
						if(prevNode != NULL)
							prevNode->next = NULL;
					}
				} else {
					prevNode->next = curNode->next;
				}
				
				--head->nodeCount;
				free(curNode);
				curNode = nextNode;
			} else {
				prevNode = curNode;
				curNode = curNode->next;
			}
		}
		
		return STATUS_OK;	
	}
	return STATUS_INVALID_PARAMS;
}

STATUS oam_deleteNodeByQ(ListHeadPtr head, MSG_Q_ID qid)
{
	
	ListNodePtr prevNode, curNode, nextNode;
	
	if ( head != NULL)
	{
		prevNode = NULL;
		curNode =  head->first;

		while(curNode != NULL)
		{
			if (curNode->qid == qid)
			{
				nextNode = curNode->next;
				if(curNode == head->first || curNode == head->last)
				{
					if ( curNode == head->first) 
					{
						head->first = head->first->next;
					}
					if (curNode == head->last)
					{
						head->last = prevNode;
						if(prevNode != NULL)
							prevNode->next = NULL;
					}
				} else {
					prevNode->next = curNode->next;
				}
				
				--head->nodeCount;
				free(curNode);
				curNode = nextNode;
			} else {
				prevNode = curNode;
				curNode = curNode->next;
			}
		}
		
		return STATUS_OK;	
	}
	return STATUS_INVALID_PARAMS;
}


void oam_printList(ListHeadPtr head)
{
	ListNodePtr aNode;
	if (head != NULL)
	{
		printf("%10s%15s%13s\n", "Table ID",  "Module ID", "Tag");
		printf("%10s%15s%15s\n", "========",  "=========", "=========");

		aNode = head->first;
		while (aNode != NULL)
		{
			printf("%10d %13d      %s\n", MIB_TBL(aNode->tag), aNode->mid, snmp_tagName(aNode->tag));
			aNode = aNode->next;
		}
		printf ("Total items = %d\n", head->nodeCount);
	}
}

