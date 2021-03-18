#include "double_list.h"

// list init  doublelist_search_by_ID
void doublelist_init(double_list **L)
{
    *L = (double_list *)malloc(sizeof(double_list));
    if(NULL == *L)
    {
        perror("malloc failed");
        exit(1);
    }

    (*L)->next = *L;
    (*L)->prev = *L;
}

//inset node to list tail
void doublelist_insert_head(double_list *s, double_list *new)
{
    new->next     = s->next;
    s->next->prev = new;
    new->prev     = s;
    s->next       = new;
}

// insert node to list previous
void doublelist_insert_tail(double_list *s, double_list *new)
{
    new->prev     = s->prev;
    s->prev->next = new;
    s->prev       = new;
    new->next     = s;
}

// delete current node
void doublelist_detele_current(double_list *s)
{
    double_list *temp = s;
    temp ->prev->next = temp->next;
    temp->next->prev  = temp->prev;

    free(temp);
    temp = NULL;
}

//delete list next node
void doublelist_detele_next(double_list *s)
{
    doublelist_detele_current(s->next);
}

// delete tail
void doublelist_detele_prev(double_list *s)
{
    doublelist_detele_current(s->prev);
}
// delete node by ID
bool doublelist_detele(double_list *s, int ID_number)
{
    char temp;
    double_list *tt = s;

    doublelist_search_by_ID(s, ID_number);
    doublelist_show_specifNode(tt, tt->nodeIndex);

    printf("input y to confirm delete: ");
    scanf("%c", &temp);
    temp = getchar();
    if(('y' == temp) || ('Y' == temp))
    {
        do
        {
            tt = tt->next;
        }while(tt->nodeIndex <= 0);
        doublelist_detele_current(tt);

        s->nodeIndex = 0;
        return true;
    }
    else
        return false;
}

// s 返回查找到到元算 和序列头到偏移亮
int doublelist_search_by_ID(double_list *s, int ID_number)
{
    int nodeIndex = 0;
    double_list *tt = s;

    while(s != tt->next)
    {
        nodeIndex += 1;     //未找到累加
        tt = tt->next;
        if(ID_number == tt->proto->info->serial_number)
        {
            #ifdef DEBUG
                printf("\n \033[0;33m Serial_Number\t Name\t Price\t \
                        quantity\t Stock \033[m \n");
                printf("  %d\t\t %s\t %5.2lf\t\t %d\t %d\n", \
                        tt ->proto->info->serial_number,\
                        tt ->proto->info->name,\
                        tt ->proto->info->price,\
                        tt ->proto->info->quantity,\
                        tt ->proto->info->stock);
            #endif

            return (nodeIndex);
        }
    }
    nodeIndex = 0;
    printf("can not find data in data base.\n");
    return  -1;
}
// s 返回查找到到元算 和序列头到偏移亮
int doublelist_search_by_name(double_list *s, char *name)
{
    int nodeIndex = 0;
    char *nameTmep[60];

    double_list *tt = s;
    nodeIndex = 0;   //index init

    while(s != tt ->next)
    {
        tt = tt->next;
        nodeIndex += 1;     //未找到累加

        if(0 == strncmp(name, tt->proto->info->name, strlen(name)))
        {
            #ifdef DEBUG
            printf("\n \033[0;33m Serial_Number\t Name\t Price\t \
                    quantity\t Stock \033[m \n");
            printf("  %d\t\t %s\t %5.2lf\t\t %d\t %d\n", \
                    tt ->proto->info->serial_number,\
                    tt ->proto->info->name,\
                    tt ->proto->info->price,\
                    tt ->proto->info->quantity,\
                    tt ->proto->info->stock);

            #endif
            
            return (nodeIndex);

        }
    }
    nodeIndex = 0;
    printf("can not find data in data base.\n");
    return  -1;
}

// 查找不记位置
int doublelist_search(double_list *s, int ID_number, char *name)
{
    double_list *t = s;
    if(double_list_is_empty(t))
    {
        printf("Database is empty.\n");
        return -1;
    }
    else if (0 != ID_number)
        return (doublelist_search_by_ID(t, ID_number));

    else if ('\0' != name)
        return (doublelist_search_by_name(t, name));
    else
        return  -1;
}


bool double_list_is_empty(double_list *head)
{
    if(head->next == head)
        return true;
    else
        return false;
}


// current list node show
void doublelist_show_specifNode(double_list *s, int nodeIndex)
{
    double_list *tt = s;
    for(; nodeIndex > 0; nodeIndex--)
        tt = tt->next;

    printf("\n \033[0;33m 商品编号\t  商品名称\t  价格\t  库存数 \033[m \n");
    printf("  %d\t\t %s\t %5.2lf\t\t %d\n", \
                tt ->proto->info->serial_number,\
                tt ->proto->info->name,\
                tt ->proto->info->price,\
                tt ->proto->info->stock);
    printf("\n");
}
// full list node show
void doublelist_show_goods(double_list *head)
{
    double_list *t = head;

    //system("clear");
    //sleep(3);
    printf("\n \033[0;32;31m 库存信息 \033[m \n");
    printf("\n \033[0;33m 商品编号\t  商品名称\t  价格\t  库存数 \033[m \n");

    if(true != double_list_is_empty(t))
    {
        for(t = head ->next; t !=  head; t = t ->next)
            printf("  %d\t\t  %s\t\t %5.2lf\t\t %d\n", \
                    t ->proto->info->serial_number,\
                    t ->proto->info->name,\
                    t ->proto->info->price,\
                    t ->proto->info->stock);
        printf("\n");
    }
    else
    {
        printf("库存已空\n");
    }
}

void doublelist_show_payments(double_list *head)
{
    double_list *t;

    //system("clear");
    printf("\n \033[0;32;31m PAYMENT INFORMATION \033[m \n");
    printf("\n \033[0;33m Serial_Number   Name   Price   \
        quantity    Stock    payment \033[m \n");
    for(t=head->next;t!=head;t=t->next)
        printf("%d\n", t ->proto->pay->cash);
    printf("\n");
}
//creat double list
void doublelist_create(double_list *s)
{
    int couter, number, index;
    double_list         *new;

    //system("clear");
    printf("\n \033[0;32;31m GOODS INFORMATION \033[m \n");
    printf("First time init protocal running.\n");
    printf("请输入录入货物总数: ");
    scanf("%d", &number);

    //system("clear");
    printf("货物总数 %d 件，请录入。\n", number);
    printf("请录入货物信息，格式参照如下：\n");
    printf("\n \033[0;33m 商品编号   商品名称   价格   数量 \033[m \n");
    //getchar();

    for(couter = 0;couter < number; couter++)
    {
        new = (double_list *)malloc(sizeof(double_list));
        if(new == NULL)
        {
            perror("malloc failed");
            exit(1);
        }

        new ->proto = (protocalsTypedef *)malloc(sizeof(protocalsTypedef));
        if(NULL == new ->proto)
        {
            perror("malloc failed");
            exit(1);
        }

        new ->proto ->info = (goodsTypedef *)malloc(sizeof(goodsTypedef));
        if(NULL == new ->proto ->info)
        {
            perror("malloc failed");
            exit(1);
        }

        new ->proto ->pay = (paymentsTypedef *)malloc(sizeof(paymentsTypedef));
        if(NULL == new ->proto ->pay)
        {
            perror("malloc failed");
            exit(1);
        }

        scanf("%d", &(new->proto->info->serial_number));
        scanf("%s", new->proto->info->name);
        scanf("%lf",&(new->proto->info->price));
        scanf("%d", &(new->proto->info->quantity));

        new->proto->info->stock += new->proto->info->quantity;

        doublelist_insert_tail(s,new);
    }

    doublelist_show_goods(s);
}
/**
 * @param
 * @param
 */
void doublelist_change_stock(double_list *s, int ID_number)
{
    char temp;
    double_list *tt = s;
    int nodeIndex;

    goodsTypedef *goods;
    goods = (goodsTypedef *)malloc(sizeof(goodsTypedef));

    nodeIndex = doublelist_search_by_ID(s, ID_number);
    if(-1 != nodeIndex)
    {
        printf("---------------------------------\n");
        doublelist_show_specifNode(tt, nodeIndex);

        printf("请输入更改后的库存数:\n");
        scanf("%d", &(goods->stock));
        getchar();

        printf("input y to confirm change: ");
        scanf("%c", &temp);
        
        if(('y' == temp) || ('Y' == temp))
        {
            do
            {
                tt = tt->next;
                nodeIndex--;
            }while(nodeIndex > 0);

            tt->proto->info->stock = tt->proto->info->quantity = goods->stock;

            printf("stock changed success.\n");
            printf("---------------------------------\n");
        }
    }
    else
    {
        printf("data is not in the data base.\n");
    }
}

void doublelist_change_data(double_list *s, int ID_number)
{
    char temp;
    double_list *tt = s;
    int nodeIndex;

    goodsTypedef *goods;
    goods = (goodsTypedef *)malloc(sizeof(goodsTypedef));

    nodeIndex = doublelist_search_by_ID(s, ID_number);
    if(-1 != nodeIndex)
    {
        printf("---------------------------------\n");
        doublelist_show_specifNode(tt, nodeIndex);

        printf("input changed meesage:\n");

        scanf("%d", &(goods->serial_number));
        scanf("%s", goods->name);
        scanf("%lf",&(goods->price));
        scanf("%d", &(goods->quantity));
        scanf("%d", &(goods->stock));

        printf("input y to confirm change: ");
        scanf("%c", &temp);
        temp = getchar();
        if(('y' == temp) || ('Y' == temp))
        {
            do
            {
                tt = tt->next;
                nodeIndex--;
            }while(nodeIndex > 0);

            strcpy(tt->proto->info->name, goods->name);

            tt->proto->info->serial_number  = goods->serial_number;
            tt->proto->info->price          = goods->price;
            tt->proto->info->quantity       = goods->quantity;
            tt->proto->info->stock          = goods->stock;

            tt->proto->info->quantity = tt->proto->info->stock;

            printf("stock changed success.\n");
            printf("---------------------------------\n");
        }
    }
    else
    {
        printf("data is not in the data base.\n");
    }
}
void doublelist_sort_data(double_list *s, int sortType)
{
    double_list *p;
    double_list *t;

    p = s->prev;

    while(p != s)
    {
        if((p->proto->info->serial_number) < (p->next->proto->info->serial_number))
            p = p->prev;
        else
        {
            t = p;
            p = p->prev;

            doublelist_insert_tail(s,t);
            doublelist_detele_current(t);
        }
    }
    printf("更改后库存详细信息：\n");
    doublelist_show_goods(s);
}


double doublelist_get_price(double_list *s, int ID_number)
{
    double_list *tt = s;
    int nodeIndex;

    nodeIndex = doublelist_search_by_ID(s, ID_number);
    if(-1 != nodeIndex)
    {
            do
            {
                tt = tt->next;
                nodeIndex--;
            }while(nodeIndex > 0);

            printf("price = %5.2lf\n", tt->proto->info->price);
            return (tt->proto->info->price);
    }
    else
    {
        printf("data is not in the data base.\n");
        return 0;
    }
}

int  doublelist_get_stock(double_list *s, int ID_number)
{
    double_list *tt = s;
    int nodeIndex;

    nodeIndex = doublelist_search_by_ID(s, ID_number);
    if(-1 != nodeIndex)
    {
        do
        {
            tt = tt->next;
            nodeIndex--;
        }while(nodeIndex > 0);
#ifdef DEBUG
        printf("stock = %d\n", tt->proto->info->stock);
#endif
        return (tt->proto->info->stock);
    }
    else
    {
        printf("data is not in the data base.\n");
        return 0;
    }
}



/*----------------------------- END LINE ----------------------------------*/
