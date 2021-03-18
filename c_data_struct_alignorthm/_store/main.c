
#include "main.h"

int main(void)
{
    double_list *LL;
    int timmer;

    doublelist_init(&LL);
    doublelist_create(LL);
    while(timmer < 1000)
    {
        func_choose(LL);

        timmer++;
    }
    return 0;
}
void func_choose(double_list *st)
{
    list *ll;
    list_init(&ll);
    int serial_number;


    int func_select, flag, tempID, sortType, searchFlag, offset, quantity;
    double unit_price, price, totalPrice;
    char name[60] = {'\0'};

    printf("功能选择\n");
    printf("1. 库存管理\n");
    printf("2. 收银系统\n");
    printf("3. 退出系统\n");

    flag = scanf("%d", &func_select);
    while('\n' != getchar());
    if(1 == func_select)
    {
        func_select = 0; //clear

        //system("clear");
        printf("Stock Operate System.\n");
        printf("choose you operation:\n");
        printf("1.查找库存\t 2.插入数据\t 3.删除数据\n 4.更改数据\t5.数据排序\t 6.显示所有数据\n");
        printf("输入操作指令:\n");

        flag = scanf("%d", &func_select);
        while('\n' != getchar());
        switch(func_select)
        {
            case 1: // search
                func_select = 0; //clear
                printf("\n1.按商品序列号查找； 2.按商品名称查找 ：");
                flag = scanf("%d", &func_select);
                while('\n' != getchar());
                if(1 == func_select)
                {
                    memset(name, '\0', 60); //clear
                    printf("输入商品序列号: ");
                    flag = scanf("%d", &func_select);
                    printf("ss->name = %s\n", name);
                    doublelist_search(st, func_select, name);
                }
                else if(2 == func_select)
                {
                    func_select = 0; //clear
                    printf("输入商品名称: ");
                    flag = scanf("%s", name);
                    doublelist_search(st, func_select, name);
                }
                else
                    printf("search function select error.\n");
                break;

            case 2:     //insert
                func_select = 0; //clear
                doublelist_create(st);
                break;

            case 3:     //delete
                func_select = 0; //clear
                printf("\nInput the serial number which need to be delete: \n");
                scanf("%d", &tempID);
                doublelist_detele(st, tempID);
                break;
            case 4:     //change data
                func_select = 0; //clear
                printf("\n请选择 1.更改库存； 2.更改所有数据： \n");
                scanf("%d", &func_select);
                while('\n' != getchar());
                printf("请输入需要更改商品的序列号：");
                scanf("%d", &tempID);
                while('\n' != getchar());

                if(1 == func_select)
                    doublelist_change_stock(st, tempID);
                else if(2 == func_select)
                    doublelist_change_data(st, tempID);
                break;
            case 5:         //sort
                func_select = 0; //clear
                doublelist_sort_data(st, sortType);
                break;

            case 6:
                doublelist_show_goods(st);
                break;
            default:
                func_select = 0; //clear
                break;
        }

    }
    else if(2 == func_select)
    {
        func_select = 0; //clear

        //system("clear");
        printf("\t\t 买单系统\n");

        memset(name, '\0', sizeof(name)/ sizeof(char));
        func_select = 0; //clear

        printf("请输入商品编码，每个编码以回车键结束，输入任意字母结束录入\n");
        // insert serial_number stored to link_list ---->> ll
        do {
                printf("商品编号：");
                flag = scanf("%d", &serial_number);
                while('\n' != getchar());
                if(flag > 0)
                {
                    printf("出售数量：");
                    scanf("%d", &quantity);
                    while('\n' != getchar());
                    if(0 < flag)        //接收有效
                    {
                        searchFlag = doublelist_search(st, serial_number, name); // 搜索返回搜索结果相对链表头的位置
                        if(quantity < doublelist_get_stock(st, serial_number))      //判断库存
                        {
                            if (-1 != searchFlag)    //找到
                            {
                                unit_price = doublelist_get_price(st, serial_number);        //get price
                                list_insert(ll, serial_number, searchFlag, quantity);
                                price = (quantity * unit_price);
                                printf("\n数量: %d, 单价: %5.2lf, 单项总价 = %5.2lf\n", quantity, unit_price, price);
                            }
                            else
                                {
                                    printf("序列号 %d 没有商品记录\n", serial_number);
                                }
                        }
                        else
                        {
                            printf("编号：%d 商品库存不足！\n", serial_number);
                        }
                    }
                    totalPrice += price;
                }
        }while(flag > 0);   //接收到字符退出
        printf("\ntotal price = %5.2lf\n", totalPrice);
        buy_protocals(st, ll);
    }
    else if(3 == func_select)
    {
        exit(1);
    }
    else
    {
        printf("invalid input.\n");
    }


}

void buy_protocals(double_list *ss, list *sll)
{
    char temp, goodsName[60];
    int qua, nodeOffsetTemp, paymt, nodeOffset, serial_number, flag, flag2, length, index;
    double totalPrice;
    nodeOffset = -1;
    totalPrice = 0;
    protocalsTypedef *protocals;

    double_list *tt = ss;
    list *ll = sll;

    printf("支持付款方式: 1.现金，2.支付宝, 3.微信， 4.POS机\n");
    printf("请输入付款方式： ");
    scanf("%d", &paymt);
    while('\n' != getchar());   //clear buff
    length = list_length(ll);

    switch(paymt)
    {
        case 1:
            //printf("销售产品：\n");
            //doublelist_show_specifNode(tt, 0);
            printf("支付方式：现金\n");
            break;
        case 2:
            //printf("销售产品：\n");
            //doublelist_show_specifNode(tt, 0);
            printf("支付方式：支付宝\n");
            break;
        case 3:
            //printf("销售产品：\n");
            //doublelist_show_specifNode(tt, 0);
            printf("支付方式：微信\n");
            break;
        case 4:
            //printf("销售产品：\n");
            //doublelist_show_specifNode(tt, 0);
            printf("支付方式：POS机\n");
            break;
        default:
            break;
    }
    printf("input y to confirm sale: ");
    scanf("%c", &temp);
    //temp = getchar();
    if(('y' == temp) || ('Y' == temp))
    {
        nodeOffset = 0;
        for(index = 0; index < length; index++)
        {
            /*------------------ 索引链表tt位置 ------------------*/
            double_list *templist = tt;
            ll = ll->nextNode;
            //由于ll索引地址偏移受上一次索引地址影响，所在本次索引地址需要减去上一次索引偏移量(nodeOffset)
            nodeOffset = ll->offsetPosition;
            do
            {
                templist = templist->next;
                nodeOffset--;
            }while(nodeOffset > 0);
            /*------------------------------------------------------*/
            //更改库存
            templist->proto->info->quantity  = templist->proto->info->stock - ll->quantity;
            templist->proto->info->stock     = templist->proto->info->stock - ll->quantity;
            doublelist_show_specifNode(templist, 0);//显示当前索引链表
        }
        printf("买单成功。\n");
    }
    else
    {
        printf("Operat not success.\n");
    }
}




#if 0
// ------------------------ 可执行程序备份
void buy_protocals(double_list *ss, int ID_number, char *goodsName)
{
    char temp, tempName[60];
    int qua, nodeTemp, paymt, nodeOffset;
    nodeOffset = -1;

    protocalsTypedef *protocals;

    double_list *tt = ss;

//    goodsTypedef *goods;
//    goods = (goodsTypedef *)malloc(sizeof(goodsTypedef));
//    temp = '\0';

    printf("---------------------------------\n");
    nodeOffset = doublelist_search(tt, ID_number, goodsName);
    printf("---------------------------------\n");
    printf("nodeOffset = %d\n", nodeOffset);
    if(-1 != nodeOffset)
    {
        doublelist_show_specifNode(tt, nodeOffset);
        nodeTemp = nodeOffset;
        printf("---------------------------------\n");
        printf("tt->nodeOffset = %d\n", nodeOffset);
        printf("nodeTemp = %d\n", nodeTemp);
        printf("输入出售数量: ");
        scanf("%d", &qua);
        getchar();   //clear buff

        do
        {
            tt = tt->next;
            nodeTemp--;
        }while(nodeTemp > 0);

        doublelist_show_specifNode(tt, 0);

        if(0 > (tt->proto->info->stock - qua))
            printf("\n库存不足!\n");
        else
        {
            // sleep(2);
            printf("---------------------------------\n");

            printf("输入方式选择: 1.现金，2.支付宝, 3.微信， 4.POS机\n");
            printf("请输入付款方式： ");
            scanf("%d", &paymt);
            while('\n' != getchar());   //clear buff
            printf("node offset = %d\n", nodeOffset);
            switch(paymt)
            {
                case 1:
                    printf("销售产品：\n");
                    doublelist_show_specifNode(tt, 0);
                    printf("支付方式：现金\n");
                    break;
                case 2:
                    printf("销售产品：\n");
                    doublelist_show_specifNode(tt, 0);
                    printf("支付方式：支付宝\n");
                    break;
                case 3:
                    printf("销售产品：\n");
                    doublelist_show_specifNode(tt, 0);
                    printf("支付方式：微信\n");
                    break;
                case 4:
                    printf("销售产品：\n");
                    doublelist_show_specifNode(tt, 0);
                    printf("支付方式：POS机\n");
                    break;
                default:
                    break;
            }
            printf("input y to confirm sale: ");
            scanf("%c", &temp);
            //temp = getchar();
            if(('y' == temp) || ('Y' == temp))
            {

                /*------- 索引链表tt位置已经记录，下面到代码不需要----------*/
//                nodeTemp = nodeOffset;
//                do
//                {
//                    //sleep(2);
//                    printf("---------------------------------\n");
//                    printf("nodeTemp = %d\n", nodeTemp);
//                    tt = tt->next;
//                    nodeTemp--;
//                }while(nodeTemp > 0);
                /*------------------------------------------------------*/
                tt->proto->info->quantity  = tt->proto->info->stock - qua;
                tt->proto->info->stock     = tt->proto->info->stock - qua;

                printf("买单成功\n");
                printf("商品剩余库存：\n");
                doublelist_show_specifNode(tt, 0);
            }
            else
            {
                printf("Operat not success.\n");
            }
        }
    }
    else
    {
        printf("serial number is not in the data base.\n");
    }
}
#endif



void serial_number_compare(double_list *ss, list *ll)
{

}
/*------------------ END LINE ----------------------*/


