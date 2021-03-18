#include "linklist.h"

void list_init(list **s)
{
	*s = (list *)malloc(sizeof(list));
	if(NULL == *s)
	{
		printf("\nlist init error.\n");
		exit(1);
	}
	else
	{
		(*s) ->nextNode = NULL;
	}
}

static bool list_creat(list *s, datatype data)
{
	list *new = (list *)malloc(sizeof(list));
	if(NULL == new)
	{
		printf("\ncreat malloc error.\n");
		return false;
	}
	else
	{
		new ->data = data;

		new ->nextNode = s ->nextNode;
		s ->nextNode = new;

		return true;
	}
}
static bool list_insert_tail(list *s, datatype data)
{
	list *new = (list *)malloc(sizeof(list));
	if(NULL == new)
	{
		printf("\ncreat malloc error.\n");
		return false;
	}
	else
	{
		new ->data = data;
		//find tail
		while(NULL != s->nextNode)
		{
			s = s->nextNode;
		}
		
		// move pointer index
		new ->nextNode = s ->nextNode;
		s ->nextNode = new;

		return true;
	}
}
static bool list_insert_head(list *s, datatype data)
{
	list *new = (list *)malloc(sizeof(list));
	if(NULL == new)
	{
		printf("\ncreat malloc error.\n");
		return false;
	}
	else
	{
		new ->data = data;
		// move pointer index
		new ->nextNode = s ->nextNode;
		s ->nextNode = new;

		return true;
	}
}
static bool list_is_empty(list *s)
{
	if(NULL == s ->nextNode)
		return true;
	else
		return false;
}
static bool list_delete(list *s, datatype nodeNumber)
{
	list *temp;
	int index = 0;

	if(list_is_empty(s))
		printf("list is empty.\n");
	else
	{
		while((NULL != s) && index < (nodeNumber - 1))
		{
			s = s ->nextNode;
			index++;
		}
		// index not finished, node is not in the list.
		if((NULL == s) && index < (nodeNumber - 1))
		{
			printf("Node is not in the list.\n");
			return true;
		}
		else
		{	
			//store current node
			temp = s->nextNode;
			//current point to temp next, delete temp
			s ->nextNode = temp ->nextNode;
				
			free(temp);
			temp = NULL;
			
			return true;
		}
	}
}

static bool list_getItem(list *s, datatype nodeNumber, datatype *data)
{
	int index = 0;

	if(list_is_empty(s))
		printf("list is empty.\n");
	else
	{
		while((NULL != s) && index < nodeNumber)
		{
			s = s ->nextNode;
			index++;
		}
		// index not finished, node is not in the list.
		if((NULL == s) && (index < nodeNumber)) 
		{
			printf("Node is not in the list.\n");
			return false;
		}
		else
		{	
			//current node is s
			*data = s ->data;
			return true;
		}
	}
}

void func_list_show(list *s)
{
	list *t = s;
	for(t = s->nextNode; NULL != t; t= t->nextNode)
		printf("---> %d", t->data);
	printf("\n");
}


void func_list_creat(list *s)
{
	int nodeNumber;
	int temp;
	int data;

	printf("input node need to creat: ");
	temp = scanf("%d", &nodeNumber);
	if(temp > 0)
	{
		for(; nodeNumber > 0; nodeNumber--)
		{
			printf("input node param: ");
			temp = scanf("%d", &data);
			if(temp > 0)
			{
				list_insert_tail(s, data);
				func_list_show(s);
			}
			else
				printf("input error.\n");

		}

	}
	else
		printf("input error.\n");
}

// delete node and return deleted value
int func_list_deleteNode(list *s, datatype nodeNumber)
{
	list *t = s;
	int dataTemp;
	if(!(list_getItem(t, nodeNumber, &dataTemp)))
	{
		printf("function error.\n");
		return -1;
	}
	t = s;
	if(!(list_delete(t, nodeNumber)))
	{
		printf("function error.\n");
		return -1;
	}
	return dataTemp;
}