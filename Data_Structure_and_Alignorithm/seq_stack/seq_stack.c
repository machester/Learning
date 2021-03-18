#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_SIZE  30

typedef struct _seqStack
{
	int stack[MAX_SIZE];
	int top;

}seqstack;

void SeqStack_init(seqstack **s);
//bool is_stack_full(seqstack s);
//bool is_stack_empty(seqstack s);
void SeqStack_push(seqstack *s, int data);
void SeqStack_show(seqstack *s);


void SeqStack_init(seqstack **s)
{
	*s = (seqstack *)malloc(sizeof(seqstack));
	if(NULL == (*s))
	{
		printf("malloc error\n");
		exit(1);
	}
	(*s) ->top = -1;
}

bool is_stack_full(seqstack *s)
{
	if((MAX_SIZE -1) == s->top)
		return true;
	else
		return false;
}
bool is_stack_empty(seqstack *s)
{
	if(-1 == s->top)
		return true;
	else
		return false;
}
void SeqStack_push(seqstack *s, int data)
{
	if(is_stack_full(s))
	{
		printf("stack full!\n");
		exit(1);
	}
	else
	{	
		s ->top +=1;
		s ->stack[s ->top] = data;
	}
}

void SeqStack_show(seqstack *s)
{
	int i;
	printf("stack pull: ");
	for(i = s->top; i>=0; i--)
		printf("--->%d ", s->stack[i]);
	printf("\n");
}

int main(void)
{
	int index, m, temp;

	seqstack  *pStack;
	
	SeqStack_init(&pStack);
	printf("input a parts:");
	scanf("%d", &m);

	for(index = 0; index < m; index++)
	{
		printf("input data %d:", index+1);
		scanf("%d", &temp);
		SeqStack_push(pStack, temp);
	}
	SeqStack_show(pStack);

	return 0;
}
