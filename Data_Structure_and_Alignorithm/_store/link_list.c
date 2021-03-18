
#include "link_list.h"

void list_init(list **ll)
{
    *ll = (list *)malloc(sizeof(list));
    if(NULL == *ll)
    {
        printf("\nlist init error.\n");
        exit(1);
    }
    else
    {
        (*ll) ->nextNode = NULL;
    }
}

bool list_creat(list *ll, int serial_number)
{
    list *new = (list *)malloc(sizeof(list));
    if(NULL == new)
    {
        printf("\ncreat malloc error.\n");
        return false;
    }
    else
    {
        new ->serial_number = serial_number;
        
        new ->nextNode      = ll ->nextNode;
        ll ->nextNode       = new;

        return true;
    }
}
bool list_insert_tail(list *ll, int serial_number)
{
    list *new = (list *)malloc(sizeof(list));
    if(NULL == new)
    {
        printf("\ncreat malloc error.\n");
        return false;
    }
    else
    {
        new ->serial_number = serial_number;
        //find tail
        while(NULL != ll->nextNode)
        {
            ll = ll->nextNode;
        }
        
        // move pointer index
        new ->nextNode = ll ->nextNode;
        ll ->nextNode  = new;

        return true;
    }
}
bool list_insert_head(list *ll, int serial_number)
{
    list *new = (list *)malloc(sizeof(list));
    if(NULL == new)
    {
        printf("\ncreat malloc error.\n");
        return false;
    }
    else
    {
        new ->serial_number = serial_number;
        // move pointer index
        new ->nextNode      = ll ->nextNode;
        ll ->nextNode       = new;

        return true;
    }
}
// double list offset position inset to list tail
bool list_insert(list *ll, int serial_number, int offsetPosition, int quantity)
{
    list *new = (list *)malloc(sizeof(list));
    if(NULL == new)
    {
        printf("\ncreat malloc error.\n");
        return false;
    }
    else
    {
        new ->serial_number     = serial_number;
        new ->offsetPosition    = offsetPosition;
        new ->quantity          = quantity;
        //find tail
        while(NULL != ll->nextNode)
        {
            ll = ll->nextNode;
        }

        // move pointer index
        new ->nextNode = ll ->nextNode;
        ll ->nextNode  = new;

        return true;
    }
}
bool list_is_empty(list *ll)
{
    if(NULL == ll ->nextNode)
        return true;
    else
        return false;
}
bool list_delete(list *ll, int nodeNumber)
{
    list *temp;
    int index = 0;

    if(list_is_empty(ll))
        printf("list is empty.\n");
    else
    {
        while((NULL != ll) && index < (nodeNumber - 1))
        {
            ll = ll ->nextNode;
            index++;
        }
        // index not finished, node is not in the list.
        if((NULL == ll) && index < (nodeNumber - 1))
        {
            printf("Node is not in the list.\n");
            return true;
        }
        else
        {   
            //store current node
            temp = ll->nextNode;
            //current point to temp next, delete temp
            ll ->nextNode = temp ->nextNode;
                
            free(temp);
            temp = NULL;
            
            return true;
        }
    }
}

bool list_getItem(list *ll, int nodeNumber, int *serial_number)
{
    int index = 0;
    list *sll = ll;

    if(list_is_empty(sll))
        printf("list is empty.\n");
    else
    {
        while((NULL != sll) && index < nodeNumber)
        {
            sll = sll ->nextNode;
            index++;
        }
        // index not finished, node is not in the list.
        if((NULL == sll) && (index < nodeNumber)) 
        {
            printf("Node is not in the list.\n");
            return false;
        }
        else
        {   
            //current node is s
            *serial_number = sll ->serial_number;
            return true;
        }
    }
}

void func_list_show(list *ll)
{
    list *t = ll;
    for(t = ll->nextNode; NULL != t; t= t->nextNode)
        printf("---> %d", t->serial_number);
    printf("\n");
}


void func_list_creat(list *ll)
{
    int nodeNumber;
    int temp;
    int serial_number;

    printf("input node need to creat: ");
    temp = scanf("%d", &nodeNumber);
    if(temp > 0)
    {
        for(; nodeNumber > 0; nodeNumber--)
        {
            printf("input node param: ");
            temp = scanf("%d", &serial_number);
            if(temp > 0)
            {
                list_insert_tail(ll, serial_number);
                func_list_show(ll);
            }
            else
                printf("input error.\n");

        }

    }
    else
        printf("input error.\n");
}

// delete node and return deleted value
int func_list_deleteNode(list *ll, int nodeNumber)
{
    list *t = ll;
    int serial_numberTemp;
    if(!(list_getItem(t, nodeNumber, &serial_numberTemp)))
    {
        printf("function error.\n");
        return -1;
    }
    t = ll;
    if(!(list_delete(t, nodeNumber)))
    {
        printf("function error.\n");
        return -1;
    }
    return serial_numberTemp;
}

/**
 * @param
 * @return
 */
bool list_sort(list *ll)
{
    list *pCurrent, *pPreNode, *pn, *temp1, *temp2;// **addPre, **addCurrent;
    int i, valueTemp1, valueTemp2, type;
    int lengthTemp = 0;

    pn = pPreNode = pCurrent = ll ->nextNode;      //get first node

    // get list length
    while (NULL != pn)
    {
        pn = pn ->nextNode;
        lengthTemp++;
    }

    pn = pCurrent = pPreNode = ll ->nextNode;    // pointer return to list heard


    type = 0;
    if(0 == type)       //sort by ID
    {
        for(i = 0; i < lengthTemp - 1; i++)
        {
            valueTemp1 = pPreNode->serial_number;
            pCurrent = pPreNode->nextNode;
            valueTemp2 = pCurrent->serial_number;

            if (valueTemp1 < valueTemp2)
            {

                pPreNode = pCurrent;

                pCurrent = pCurrent->nextNode;

            }
            else        // before > after
            {
                temp1 = pPreNode;
                temp1 ->nextNode = pPreNode ->nextNode;
                temp2 = pCurrent;
                temp2 ->nextNode = pCurrent ->nextNode;

                // nextNode address change
                pCurrent = temp1;
                pCurrent ->nextNode = temp1 ->nextNode;

                pPreNode = temp2;
                pPreNode ->nextNode = temp2 ->nextNode;

            }

        }
        printf("list has been sorted.\n");
    }
}

int list_length(list *ll)
{
        int listLength = 0;
        list *sll = ll;

    if(list_is_empty(sll))
    {
        printf("list is empty.\n");
        return 0;
    }
    else
    {
        sll = sll->nextNode;
        while(NULL != sll)
        {
            sll = sll ->nextNode;
            listLength++;
        }
        return listLength;
    }
}


/*----------------------------- END LINE ----------------------------------*/



















#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    False = 0,
    OK = !False
}status;

struct student{
    int ID;
    char name[20];
    double score;

}students;

typedef struct NodeDef{
    struct student item;
    //int item;
    struct Node *next;
}Node;

Node *nodeList;

void CreatList(Node **nodeHeard, int length);
void ShowList(Node *pList);
status DeleteList(Node **pList, int position);
status InsertList(Node **pList, int position);
status SearchList(Node *pList, int ID, char *name);
status SortList(Node *pList, int type);

status SortList(Node *pList, int type)
{
    Node *pCurrent, *pPreNode, *pn, *temp1, *temp2;// **addPre, **addCurrent;
    int i, valueTemp1, valueTemp2;
    int lengthTemp = 0;

    pn = pPreNode = pCurrent = pList ->next;      //get first node

    // get list length
    while (NULL != pn)
    {
        pn = pn ->next;
        lengthTemp++;
    }

    pn = pCurrent = pPreNode = pList ->next;    // pointer return to list heard

    if(0 == type)       //sort by ID
    {
        for(i = 0; i < lengthTemp - 1; i++)
        {
            valueTemp1 = pPreNode->item.ID;
            pCurrent = pPreNode->next;
            valueTemp2 = pCurrent->item.ID;

            if (valueTemp1 < valueTemp2)
            {

                pPreNode = pCurrent;

                pCurrent = pCurrent->next;

            }
            else        // before > after
            {
                temp1 = pPreNode;
                temp1 ->next = pPreNode ->next;
                temp2 = pCurrent;
                temp2 ->next = pCurrent ->next;

                // next address change
                pCurrent = temp1;
                pCurrent ->next = temp1 ->next;

                pPreNode = temp2;
                pPreNode ->next = temp2 ->next;

            }


        }
        printf("list has been sorted.\n");
    }
}

status SearchList(Node *pList, int ID, char *name)
{
    int i;
    Node *pt;
    int temp1;
    char arry[20];

    pt = pList ->next;       //get first node

    if(0 != ID)
    {
        while (NULL != pt)
        {
            temp1 = pt->item.ID;
            if(ID == temp1)
            {
                printf("Data->  ID: %d,     name: %s,       score: %5.2lf\n",\
                        pt->item.ID, pt ->item.name, pt ->item.score);

                return OK;
            }
        }
        printf("ID not exist\n");

        return False;
    }
    else
    {
        while (NULL != pt)
        {
            for(i = 0; i < 20; i++)
            {
                arry[i] = pt ->item.name[i];
            }
            if(0 == strcmp(arry, name))
            {
                printf("Data->  ID: %d,     name: %s,       score: %5.2lf\n",\
                        pt->item.ID, pt ->item.name, pt ->item.score);

                return OK;
            }
        }
        printf("ID not exist\n");
        free(pt);
        pt = NULL;
        return False;
    }
}
status InsertList(Node **pList, int position)
{
    Node *s, *p;
    int i;

    p = *pList;

    if(0 == position)                  //default insert to list end
    {
        while (NULL != p->next)     //fined the last node
        {
            p = p ->next;
        }
        printf("Inpput format: ID\t name\t score\t\n");
        s = (Node *)malloc(sizeof(Node));

        scanf("%d", &(s ->item.ID));
        scanf("%s", &(s ->item.name));
        scanf("%lf", &(s ->item.score));
        s ->next = p ->next;
        p->next = s;

        return OK;
    }
    else
    {
        i = 1;
        while ((NULL != p) && (i < position))
        {
            p = p ->next;
            ++i;
        }
        if((NULL == p) || (i > position))
        {
            return False;
        }
        printf("Inpput format: ID\t name\t score\t\n");
        s = (Node *)malloc(sizeof(Node));

        scanf("%d", &(s ->item.ID));
        scanf("%s", &(s ->item.name));
        scanf("%lf", &(s ->item.score));
        s ->next = p ->next;
        p->next = s;

        return OK;
    }

}

void CreatList(Node **nodeHeard, int length)
{
    Node *p;
    Node *t;
    int i;

    *nodeHeard = (Node *)malloc(sizeof(Node));      //init list heard
    t = *nodeHeard;
    if(NULL == t)
        return;
    printf("Inpput format: ID\t name\t score\t\n");
    for(i = 0; i < length; i++)
    {
        p = (Node *)malloc(sizeof(Node));

        scanf("%d", &(p ->item.ID));
        scanf("%s", &(p ->item.name));
        scanf("%lf", &(p ->item.score));

        t ->next = p;
        t = p;
    }
    t ->next = NULL;            //list end
}
void ShowList(Node *pList)
{
    int i;
    Node *p;

    p = pList ->next;        //get heard address
    i = 1;
    if(NULL == p)
        printf("list is empty.\n");
    while (NULL != p)        // list not empty.
    {
        printf("%d:  ID: %d,     name: %s,       score: %5.2lf\n",\
       i, p->item.ID, p ->item.name, p ->item.score);

        p = p ->next;
        ++i;
    }
}
status DeleteList(Node **pList, int position)
{
    int i;
    Node *temp, *q, *p;
    temp = *pList;


    if(NULL == temp->next)              //heard is empty.
        printf("list has been clearned.");

    else if (0 == position)             //delete whole list
    {
        while (NULL != temp)            //heard nont empty
        {
            q = temp ->next;
            free(temp);
            temp = NULL;
            temp = q;
        }
        // set heard to empty
        temp = *pList;
        temp ->next = NULL;
    }
    else
    {
        i = 1;
        while ((NULL != p) && (i < position))
        {
            temp = temp ->next;
            ++i;
        }
        if((NULL == temp) || (i > position))
        {
            return False;
        }
        p = temp ->next;
        temp ->next = p->next;
        free(p);
        p = NULL;

        printf("node %d has been deleted.", position);
        return OK;
    }


}

#endif


#if 0
int main()
{

    int selection = 0;
    int parts = 0;
    int length, position;
    int temp;
    int tempID;
    char tempArry[20];

    status Flag = False;

    printf("Plese select:\n");
    while(1)
    {
        printf("\n1.Input:    2.Show List:   3.Search:        4.Insert\n");
        printf("5.Sort      6.Delete List  7.Quit\n");

        scanf("%d", &selection);
        switch (selection) {
            case 1:
                printf("input parts:");
                scanf("%d", &parts);
                CreatList(&nodeList, parts);
                break;
            case 2:
                ShowList(nodeList);
                break;
            case 3:
                temp = 0;
                tempID = 0;
                memset(tempArry, 0, 20);

                printf("Choose type: 1. search by ID.  2. search by name :");
                scanf("%d", &temp);
                if(1 == temp)
                {
                    printf("Input ID: ");
                    scanf("%d", &tempID);
                }
                else if(2 == temp)
                {
                    printf("Input name: ");
                    scanf("%s", &tempArry[0]);
                }
                else
                {
                    printf("Input error.\n");
                }
                printf("tempID =  %d,     tempArry = %s\n", tempID, tempArry);
                Flag = SearchList(nodeList, tempID, tempArry);
                break;
            case 4:
                printf("Insert Position, 0 for default : ");
                scanf("%d", &position);
                Flag = InsertList(&nodeList, position);
                break;
            case 5:
                Flag = SortList(nodeList, 0);
                break;
            case 6:
                printf("Insert Position, 0 for whole list : ");
                scanf("%d", &position);
                Flag = DeleteList(&nodeList ,position);
                break;
            case 7:
                exit(0);
                break;
            default:
                printf("\n/---- Select error. -----/\n");
                break;
        }
    }
}

#endif
