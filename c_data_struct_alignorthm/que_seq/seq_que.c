/**
 * define tail as insert port
 * define front as output part
 */
#include "seq_que.h"

#define DEBUG

void seq_que_init(seq_que **s)
{
    (*s) = (seq_que *)malloc(sizeof(seq_que));
    (*s) ->front = 0;
    (*s) ->tail = 0;
}

void seq_que_creat(seq_que *s, int n)
{
    int i;
    seq_que *t = s;
    for(i = 0; i < n; i++)
    {
        if(que_is_full(s))
            printf("queue is full...\n");
        else
        {
            printf("please input data %d:", i+1);
            scanf("%d", &s ->data[i]);
            (s->tail)++;
        }
    }
    #ifdef DEBUG
        seq_que_show(t);
    #endif
        printf("queue list has been created.\n");

}
bool que_is_empty(seq_que *s)
{
    //if((s->front == s->tail) && \
	//(MAX_SIZE == ((s->front))) && \
	//(MAX_SIZE == ((s->tail)+1)))
    if(s->front  == s->tail)
        return true;
    else
        return false;
}

bool que_is_full(seq_que *s)
{
    if(s->front == ((s->tail + 1) % MAX_SIZE))
        return true;
    else
        return false;
}

void seq_que_push(seq_que *s, int data)
{
    if(que_is_full(s))
        printf("seq queue is full.\n");
    else
    {
        s->data[s->tail] = data;
        //move index
        s->tail = (s->tail + 1) % MAX_SIZE;      //index++
    }
}

void seq_que_pull(seq_que *s, int *tempData)
{
    if(que_is_empty(s))
        printf("seq queue is empty.\n");
    else
    {
        *tempData = s->data[s->front];
        s->front = (s->front +1) % MAX_SIZE;
    }
}

void seq_que_show(seq_que *s)
{
    int front = s ->front;
    int tail = s->tail;
	if(!que_is_empty(s))
	{
    	printf("|queue heard|");

    	for(; front != tail; front++)
    	{
        	printf("--->%d ", s->data[front]);
    	}
    	printf("\n");
	}

}
#if 0
void seq_que_show(seq_que *s)
{
    int index = 0;
    index = s->front;
#ifdef DEBUG
    printf("pull->front = %d\n", index);
    printf("pull->tail = %d\n", )
#endif
    //    for(; true != que_is_empty(s); index = index + 1)
    for(;index != s->tail; index++)
    {
        printf("--->%d ", s->data[index]);
    }
    printf("\n");
}




//int seq_que_pushIndex(seq_que s)
//{
//    s.tail = (s.tail + 1) % MAX_SIZE;
//    return (s.tail);
//}

//int seq_que_pullIndex(seq_que s)
//{
//    s.front = (s.front + 1) % MAX_SIZE;
//    return (s.front);
//}
#endif
