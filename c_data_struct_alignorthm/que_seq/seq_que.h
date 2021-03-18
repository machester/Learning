#ifndef __SEQ_QUE_H
#define __SEQ_QUE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


#define MAX_SIZE 20

typedef struct _seq_que{

	int data[MAX_SIZE];
	int tail;
	int front;

}seq_que;


extern void seq_que_init(seq_que **s);
extern void seq_que_creat(seq_que *s, int n);
extern void seq_que_push(seq_que *s, int data);
extern void seq_que_pull(seq_que *s, int *tempData);
extern void seq_que_show(seq_que *s);

bool que_is_full(seq_que *s);
bool que_is_empty(seq_que *s);

//static int seq_que_pushIndex(seq_que s);
//static int seq_que_pullIndex(seq_que s);


#endif



