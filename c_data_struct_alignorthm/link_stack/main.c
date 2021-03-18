#include "link_stack.h"

int main(void)
{
	int index, m, temp;

	linkStack  *pStack;
	
	LinkStack_init(&pStack);
	printf("input a parts:");
	scanf("%d", &m);

	for(index = 0; index < m; index++)
	{
		printf("input data %d:", index+1);
		scanf("%d", &temp);
		SeqStack_push(pStack, temp);
	}
	LinkStack_show(pStack);

	return 0;
