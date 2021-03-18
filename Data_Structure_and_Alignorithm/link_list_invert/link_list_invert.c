#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

struct ListNode {
	struct ListNode * next;
	int val;
};
typedef struct ListNode listTypde;

listTypde *  list_invert_one(listTypde *list);
listTypde * list_invert_two(listTypde *list);
listTypde * list_invert_three(listTypde *list);

void show_list(listTypde *list)
{
	listTypde *tempList;

	printf("list: ");
	for (tempList = list ->next; NULL != tempList; tempList = tempList ->next)
	{
		printf("---> %d ", tempList ->val);
	}
	printf("\n");
}
void list_init(listTypde **list)
{
	static int counter = 0;

	if (counter == 0)
	{
		*list = (listTypde *)malloc(sizeof(listTypde));

		(*list)->next = NULL;

		for (int index = 0; index < 4; index++)
		{
			listTypde * newNode = (listTypde *)malloc(sizeof(listTypde));
			newNode ->val = index + 1;
			newNode ->next = (*list) ->next;
			(*list)->next = newNode;
		}
		show_list((*list));
	}
	else
	{
		*list = (listTypde *)malloc(sizeof(listTypde));

		(*list)->next = NULL;

		for (int index = 0; index < 4; index++)
		{
			listTypde * newNode = (listTypde *)malloc(sizeof(listTypde));
			newNode ->val = ((index + 1) * 2) + 1;
			newNode ->next = (*list) ->next;
			(*list)->next = newNode;
		}
		show_list((*list));
	}
	counter++;
}

/**
 * @description: one way to invert list by cut node and insert to a new list.
 * @param list : which list need to be inverted.
 * @return : return a new inverted list head.
 */
listTypde * list_invert_one(listTypde *list)
{
	listTypde * newList, *curNode, *nextNode;

	curNode = list->next;
	nextNode = list->next->next;	//nextNode = curNode ->next;

	newList = (listTypde *)malloc(sizeof(listTypde));
	newList ->next = NULL;

	while (NULL != curNode)
	{
		curNode ->next = newList ->next;
		newList ->next = curNode;

		curNode = nextNode;
		if(NULL != nextNode)	// last node
			nextNode = nextNode->next;
	}
	printf("---- invert finished -----\n");
	return newList;
}

/**
 * @description: one way to invert list by cut node and insert to a new list.
 * @param list : which list need to be inverted.
 * @return : return a new inverted list head.
 */
listTypde * list_invert_two(listTypde *list)
{
	listTypde * newList, *curNode, *preNode;

	curNode = list->next;

	newList = (listTypde *)malloc(sizeof(listTypde));
	newList ->next = NULL;

	while (NULL != curNode)
	{
		preNode = curNode;		//store current position and invert current node
		curNode ->next = newList ->next;
		newList ->next = curNode;

		curNode = preNode ->next;
	}
	printf("---- invert finished -----\n");
	return newList;
}

/**
 * @description: one way to invert list by cut node and insert to a new list.
 * @param list : which list need to be inverted.
 * @return : return a new inverted list head.
 */
listTypde * list_invert_three(listTypde *list)
{

}

int main() {

	listTypde * l1;
	listTypde * l2;


	list_init(&l1);
	list_init(&l2);
	listTypde * l3 = list_invert_one(l1);
	show_list(l3);
	listTypde * l4 = list_invert_two(l2);
	show_list(l4);

	printf("---------- FINISHED ------------\n");
	return 0;
}