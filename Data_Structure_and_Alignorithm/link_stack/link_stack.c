#include "link_stack.h"

void LinkStack_init(linkStack **s)
{
	*s = (linkStack *)malloc(sizeof(linkStack));
	if(NULL == (*s))
	{
		printf("malloc error\n");
		exit(1);
	}
	(*s) ->next = NULL;
}

bool is_stack_empty(linkStack *s)
{
	if(NULL == s)
		return true;
	else
		return false;
}
void LinkStack_push(linkStack **s, int data)
{
	linkStack *new;
	new = (linkStack *)malloc(linkStack);
	if(NULL == new)
	{
		printf("malloc error.\n");
		exit(1);		
	}

	new ->data = data;

	new -> next = (*s) ->next;
	(*s) ->next = new;
}

void LinkStack_show(linkStack **s)
{
	int i;
	printf("stack pull: ");
	while(NULL == (*s))
	{
		printf("--->%d ", *s->data);
		(*s) ->next;
	}
	printf("\n");
}

