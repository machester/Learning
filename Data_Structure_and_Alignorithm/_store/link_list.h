#ifndef __LINK_LIST_H
#define __LINK_LIST_H

#include "double_list.h"

typedef struct _list{

	int serial_number;                  // 商品序列号
    int offsetPosition;                 // 队doublelist而已偏移头节点位置
    int quantity;                       //数量
	struct _list *nextNode;
}list;

extern void list_init(list **ll);
extern bool list_creat(list *ll, int serial_number);
extern bool list_insert_tail(list *ll, int serial_number);
extern bool list_insert_head(list *ll, int serial_number);
static bool list_is_empty(list *ll);
extern bool list_delete(list *ll, int nodeNumber);
extern bool list_getItem(list *ll, int nodeNumber, int *serial_number);

extern void func_list_show(list *ll);
extern void func_list_creat(list *ll);
// delete node and return deleted value
extern int func_list_deleteNode(list *ll, int nodeNumber);

extern bool list_sort(list *ll);
extern int list_length(list *ll);

extern bool list_insert(list *ll, int serial_number, int offsetPosition, int quantity);



#endif


/*----------------------------- END LINE ----------------------------------*/