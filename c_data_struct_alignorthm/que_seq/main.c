#include "seq_que.h"

int main(void)
{
    int number;
    int parts;
    int temp;
    int timmer = 0;

    seq_que *L;
    seq_que_init(&L);

    printf("Please input parts: ");

    scanf("%d", &parts);
    seq_que_creat(L, parts);

    while(1)
    {
        printf("input function data: \n");
        temp = scanf("%d", &number);
        while ('\n' != getchar());  //clear input buff

        if (temp > 0)
        {
            seq_que_push(L, number);
            printf("queue: ");
            seq_que_show(L);
            printf("\n");


        }
        else
        {
            seq_que_pull(L, &number);
            printf("pulled out data is : %d\n", number);
            printf("queue: ");
            seq_que_show(L);
            printf("\n");

        }
        timmer++;
    }
    return 0;
}

