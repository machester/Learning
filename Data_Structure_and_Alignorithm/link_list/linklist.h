#ifndef __LINKLIST_H
#define __LINKLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef int datatype;

typedef struct _list{

	datatype data;
	struct _list *nextNode;
}list;

extern void list_init(list **s);
static bool list_creat(list *s, datatype data);
static bool list_insert_tail(list *s, datatype data);
static bool list_insert_head(list *s, datatype data);
static bool list_is_empty(list *s);
static bool list_delete(list *s, datatype nodeNumber);
static bool list_getItem(list *s, datatype nodeNumber, datatype *data);

extern void func_list_show(list *s);
extern void func_list_creat(list *s);
// delete node and return deleted value
extern int func_list_deleteNode(list *s, datatype nodeNumber);

#endif
