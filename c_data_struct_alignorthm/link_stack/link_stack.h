#ifndef __LINK_STACK_H
#define __LINK_STACK_H

typedef struct _linkStack
{
	int data;
	struct _linkStack *next;

}linkStack;

void LinkStack_init(linkStack **s);
bool is_stack_full(linkStack s);
bool is_stack_empty(linkStack s);
void LinkStack_push(linkStack **s, int data);
void LinkStack_show(linkStack **s);


#endif
