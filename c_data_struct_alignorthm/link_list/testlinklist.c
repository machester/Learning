#include "linklist.h"

int main(void)
{
	datatype data;
	int temp;
	int fun_code;
	list *L;
	int loop = 100;
	
	list_init(&L);

	
	func_list_creat(L);
	while(loop)
	{
		printf("input function code: ");
		temp = scanf("%d", &fun_code);

		while('\n' != getchar()); //keyboard buff clean
		
		if(temp)
		{
			data = func_list_deleteNode(L, fun_code);
			printf("deleted data is: %d\n", data);
			func_list_show(L);
		}
	}
	return 0;
}
