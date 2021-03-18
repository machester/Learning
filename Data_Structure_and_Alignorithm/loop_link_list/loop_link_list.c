
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

<<<<<<< HEAD
//liner linked list
struct Node{
    students *NodeData;
    struct Node *nextNode;
}*NodeList;
=======
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
#if 0
                printf("Data->  ID: %d,     name: %s,       score: %5.2lf\n", \
                        pPreNode->item.ID, pPreNode->item.name, pPreNode->item.score);
                printf("Data->  ID: %d,     name: %s,       score: %5.2lf\n", \
                        pCurrent->item.ID, pCurrent->item.name, pCurrent->item.score);
#endif
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
#if 0

                printf("Data->  ID: %d,     name: %s,       score: %5.2lf\n",\
                        pPreNode->item.ID, pPreNode ->item.name, pPreNode ->item.score);
                printf("Data->  ID: %d,     name: %s,       score: %5.2lf\n",\
                        pCurrent->item.ID, pCurrent ->item.name, pCurrent ->item.score);
#endif
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
>>>>>>> 5328d63234b716b09f559f18679435c04e9ea5ca

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

<<<<<<< HEAD










/*******************************************
void SantexInput(students *pStdList)
{
    int i;

    printf("Input format: \nID\t name\t score\n");
    for(i = 0; i < parts; i++)
    {
        scanf("%d", &((pStdList + i)->ID));
        scanf("%s", &((pStdList + i)->name));
        scanf("%lf", &((pStdList + i)->score));
    }
    printf("\n---- Input Accepted. ----\n");
}
void DataCheck(students *pStdList)
{
    int i;

    if(0 == parts)
        printf("\nDatabase is empty.\n");
    else
    {
        printf("Data List:\n");
        for(i = 0; i < parts; i++)
        {
            printf("%d. ID: %d\t name:%s\t score: %4.2f\n", (i+1), \
		    (pStdList + i) ->ID, \
		    (pStdList + i) ->name, \
		    (pStdList + i) ->score);
        }
        printf("\n----- Data check completed. ----\n");
    }
}
status _F_Quit(void)
{
    return True;
}
**************************************************************/
=======
>>>>>>> 5328d63234b716b09f559f18679435c04e9ea5ca
