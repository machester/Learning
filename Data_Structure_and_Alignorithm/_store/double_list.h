#ifndef __DOUBLE_LIST_H
#define __DOUBLE_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
/*--------------------------------------*/
#include <unistd.h>
#include "dos_color.h"

#define DEBUG
// goods information
typedef struct _goods
{

    int serial_number;			// 商品序列号
    char name[60];				// 商品名称
    double price;				// 商品价格
    int quantity;				// 商品数量
    int stock;					// 商品库存

}goodsTypedef;

// payments choose 
typedef struct _payments
{
    int cash;
    int alipay;
    int wechat;
    int pos;

}paymentsTypedef;

// define buy protocals 
typedef struct _protocals
{

    goodsTypedef 		*info;
    paymentsTypedef 	*pay;

}protocalsTypedef;

// define double cycle list
typedef struct _double_list{

    protocalsTypedef *proto;
    struct _double_list *next;
    struct _double_list *prev;
    int nodeIndex;

}double_list;


/*-----------------------------------------------------------------------*/



extern void doublelist_init(double_list **L);
static void doublelist_insert_head(double_list *s, double_list *new);
static void doublelist_insert_tail(double_list *s, double_list *new);
/*-----------------------------------------------------------------------*/
static void doublelist_detele_current(double_list *s);
static void doublelist_detele_next(double_list *s);
static void doublelist_detele_prev(double_list *s);
extern bool doublelist_detele(double_list *s, int ID_number);
/*-----------------------------------------------------------------------*/
static bool double_list_is_empty(double_list *head);
/*-----------------------------------------------------------------------*/
static int doublelist_search_by_ID(double_list *s, int ID_number);
static int doublelist_search_by_name(double_list *s, char *name);
extern int doublelist_search(double_list *s, int ID_number, char *name);
/*-----------------------------------------------------------------------*/
extern void doublelist_show(double_list *head);
extern void doublelist_show_specifNode(double_list *s, int nodeIndex);
/*-----------------------------------------------------------------------*/

extern void doublelist_create(double_list *s);
/*-----------------------------------------------------------------------*/
extern void doublelist_show_goods(double_list *s);
extern void doublelist_show_payments(double_list *s);
/*-----------------------------------------------------------------------*/
extern void doublelist_change_stock(double_list *s, int ID_number);         //改变库存数量
extern void doublelist_change_data(double_list *s, int ID_number);          //改变所有信息
/*-----------------------------------------------------------------------*/
extern void doublelist_sort_data(double_list *s, int sortType);

/*-----------------------------------------------------------------------*/
extern double doublelist_get_price(double_list *s, int ID_number);
extern int  doublelist_get_stock(double_list *s, int ID_number);
/*-----------------------------------------------------------------------*/




#endif

/*----------------------------- END LINE ----------------------------------*/