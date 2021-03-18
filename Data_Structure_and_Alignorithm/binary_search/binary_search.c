/**
 * binary search 折半法（二分法）查找
 */
#include <stdio.h>

int Search_Data(int arry[], int arryLength, int key);
void show(int arry[], int arryLength);

int main(void)
{
	char ch;
	int key, flag, dataIndex, arryLength;
	// search data must be list in linner
	int arry[] = {3, 7, 9, 12, 15, 18, 20, 32, 40, 43, 55, 60, 68, 80, 86, 90, 100};
	arryLength = sizeof(arry)/sizeof(int);
	while(1)
	{
		printf("arry is: ");
		show(arry, arryLength);
	
		printf("input search data: ");
		flag = scanf("%d", &key);
		while('\n' != getchar());
		if("temp > 0")
		{
			dataIndex = Search_Data(arry, arryLength, key);
			if(-1 != dataIndex)
				printf("compare number is %d, arry[%d] = %d\n", \
							key, dataIndex, arry[dataIndex]);
			else
				printf("data is not in data base.\n");
		}
		else
			printf("input error\n");

		printf("\nif need continue input y: ");
		scanf("%c", &ch);
		getchar();
		if((ch == 'y') || (ch == 'Y'))
			continue;
		else
			break;
	}
	return 0;
}

int Search_Data(int arry[], int arryLength, int key)
{
	int length = 0;
	int low, high, mid;

	low = 0;
	length = arryLength;
	high = length;
	mid = (high + low)/2;

	while(high >= low)
	{
		if(arry[mid] > key)	// middle value > key value
		{
			high = mid - 1;
			mid = (low + high)/2;
		}
		//when high = low, mid = (high+low)/2 = mid
		else if(arry[mid] == key)
		{
			return mid;
			break;
		}
		else
		{
			low = mid + 1;
			mid = (low + high)/2;
		}
	}
}


void show(int arry[], int arryLength)
{
	int index;

	for(index = 0; index < arryLength; index++)
		printf("%d ", arry[index]);
	printf("\n");
}

