#include "double_list.h"


void double_list_init(list **s)
{
    *s = (list *s)malloc(sizeof(list));
    if(NULL = *s)
    {
        perror("malloc error");
        exit(1);
    }
    else
    {
        (*s) ->next = NULL;
        (*s) ->pre = NULL;
    }
}
bool double_list_creat(list *s, datatype data)

bool double_list_insert_tail_link(list *s, list *new)
{
    while(NULL != s)
}
bool double_list_insert_head_link(list *s, list *new)
{

}
bool double_list_is_empty(list *s)
bool double_list_delete(list *s, datatype nodeNumber)
bool double_list_getItem(list *s, datatype nodeNumber, datatype *data)

void func_double_list_show(list *s)
void func_double_list_creat(list *s)
// delete node and return deleted value
int func_double_list_deleteNode(list *s, datatype nodeNumber)