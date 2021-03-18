#ifndef __DOUBLE_LIST_H
#define __DOUBLE_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef int datatype;

typedef struct _list{

	datatype data;
	struct _list *next;
    struct _list *pre;

}list;

extern void double_list_init(list **s);
static bool double_list_creat(list *s, datatype data);
static bool double_list_insert_tail(list *s, datatype data);
static bool double_list_insert_head(list *s, datatype data);
static bool double_list_is_empty(list *s);
static bool double_list_delete(list *s, datatype nodeNumber);
static bool double_list_getItem(list *s, datatype nodeNumber, datatype *data);

extern void func_double_list_show(list *s);
extern void func_double_list_creat(list *s);
// delete node and return deleted value
extern int func_double_list_deleteNode(list *s, datatype nodeNumber);





#endif
